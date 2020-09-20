#include "Wiimote2Keys.h"

#include "Emulator/Keyboard/PS2Kbd.h"
#include <Arduino.h>
#include <string.h>

#include <SPIFFS.h>

#include "ESP32Wiimote/ESP32Wiimote.h"
static ESP32Wiimote wiimote;

extern byte z80ports_wiin[128];

static bool logWiimoteEvents = false;

// definition for spectrum keys
// high nibble is port index, low nibble is affected bit

#define ZX_KEY_SHIFT 0x00
#define ZX_KEY_Z     0x01
#define ZX_KEY_X     0x02
#define ZX_KEY_C     0x03
#define ZX_KEY_V     0x04

#define ZX_KEY_A     0x10
#define ZX_KEY_S     0x11
#define ZX_KEY_D     0x12
#define ZX_KEY_F     0x13
#define ZX_KEY_G     0x14

#define ZX_KEY_Q     0x20
#define ZX_KEY_W     0x21
#define ZX_KEY_E     0x22
#define ZX_KEY_R     0x23
#define ZX_KEY_T     0x24

#define ZX_KEY_1     0x30
#define ZX_KEY_2     0x31
#define ZX_KEY_3     0x32
#define ZX_KEY_4     0x33
#define ZX_KEY_5     0x34

#define ZX_KEY_0     0x40
#define ZX_KEY_9     0x41
#define ZX_KEY_8     0x42
#define ZX_KEY_7     0x43
#define ZX_KEY_6     0x44

#define ZX_KEY_P     0x50
#define ZX_KEY_O     0x51
#define ZX_KEY_I     0x52
#define ZX_KEY_U     0x53
#define ZX_KEY_Y     0x54

#define ZX_KEY_ENTER 0x60
#define ZX_KEY_L     0x61
#define ZX_KEY_K     0x62
#define ZX_KEY_J     0x63
#define ZX_KEY_H     0x64

#define ZX_KEY_SPACE 0x70
#define ZX_KEY_SYMBS 0x71
#define ZX_KEY_M     0x72
#define ZX_KEY_N     0x73
#define ZX_KEY_B     0x74

#define ZX_KEY_NOKEY 0xFF

typedef void (*W2KPROC)(uint16_t);

W2KPROC w2kproc = NULL;

static void w2kproc_osd(uint16_t button);
static void w2kproc_spectrum(uint16_t button);

void initWiimote2Keys()
{
    wiimote.init();

    w2kproc = &w2kproc_spectrum;
    //w2kproc = &w2kproc_osd;
}

static uint8_t keytable[] = {
    ZX_KEY_NOKEY,   // 0001 (2)
    ZX_KEY_NOKEY,   // 0002 (1)
    ZX_KEY_NOKEY,   // 0004 TRIG
    ZX_KEY_NOKEY,   // 0008 (A)
    ZX_KEY_NOKEY,   // 0010 (-)
    ZX_KEY_NOKEY,   // 0020
    ZX_KEY_NOKEY,   // 0040
    ZX_KEY_NOKEY,   // 0080 HOME
    ZX_KEY_NOKEY,   // 0100 LEFT
    ZX_KEY_NOKEY,   // 0200 RIGHT
    ZX_KEY_NOKEY,   // 0400 DOWN
    ZX_KEY_NOKEY,   // 0800 UP
    ZX_KEY_NOKEY,   // 1000 (+)
    ZX_KEY_NOKEY,   // 2000 
    ZX_KEY_NOKEY,   // 4000 
    ZX_KEY_NOKEY,   // 8000 
};

static uint8_t keyForChar(uint8_t ch)
{
    uint8_t res = ZX_KEY_NOKEY;
    switch(ch) {
        case 'h': res = ZX_KEY_SHIFT; break;
        case 'Z': res = ZX_KEY_Z    ; break;
        case 'X': res = ZX_KEY_X    ; break;
        case 'C': res = ZX_KEY_C    ; break;
        case 'V': res = ZX_KEY_V    ; break;

        case 'A': res = ZX_KEY_A    ; break;
        case 'S': res = ZX_KEY_S    ; break;
        case 'D': res = ZX_KEY_D    ; break;
        case 'F': res = ZX_KEY_F    ; break;
        case 'G': res = ZX_KEY_G    ; break;

        case 'Q': res = ZX_KEY_Q    ; break;
        case 'W': res = ZX_KEY_W    ; break;
        case 'E': res = ZX_KEY_E    ; break;
        case 'R': res = ZX_KEY_R    ; break;
        case 'T': res = ZX_KEY_T    ; break;

        case '1': res = ZX_KEY_1    ; break;
        case '2': res = ZX_KEY_2    ; break;
        case '3': res = ZX_KEY_3    ; break;
        case '4': res = ZX_KEY_4    ; break;
        case '5': res = ZX_KEY_5    ; break;

        case '0': res = ZX_KEY_0    ; break;
        case '9': res = ZX_KEY_9    ; break;
        case '8': res = ZX_KEY_8    ; break;
        case '7': res = ZX_KEY_7    ; break;
        case '6': res = ZX_KEY_6    ; break;

        case 'P': res = ZX_KEY_P    ; break;
        case 'O': res = ZX_KEY_O    ; break;
        case 'I': res = ZX_KEY_I    ; break;
        case 'U': res = ZX_KEY_U    ; break;
        case 'Y': res = ZX_KEY_Y    ; break;

        case 'e': res = ZX_KEY_ENTER; break;
        case 'L': res = ZX_KEY_L    ; break;
        case 'K': res = ZX_KEY_K    ; break;
        case 'J': res = ZX_KEY_J    ; break;
        case 'H': res = ZX_KEY_H    ; break;

        case 's': res = ZX_KEY_SPACE; break;
        case 'y': res = ZX_KEY_SYMBS; break;
        case 'M': res = ZX_KEY_M    ; break;
        case 'N': res = ZX_KEY_N    ; break;
        case 'B': res = ZX_KEY_B    ; break;
    }
    return res;
}

void loadKeytableForGame(const char* sna_fn)
{
    if (0 == sna_fn || 0 == *sna_fn) return;
    Serial.printf("Wiimote2Keys: loading table for game %s\n", sna_fn);

    uint fnlen = strlen(sna_fn);
    if (fnlen > 63) {
        Serial.println("ERROR: filename too long");
        return;
    }

    char txt_fn[64];
    strcpy(txt_fn, sna_fn);
    txt_fn[fnlen-3] = 't';
    txt_fn[fnlen-2] = 'x';
    txt_fn[fnlen-1] = 't';

    Serial.printf("Opening %s...\n", txt_fn);
    File f = SPIFFS.open(txt_fn, FILE_READ);
    vTaskDelay(2);

    if (NULL == f) {
        Serial.println("ERROR: cannot open file");
    }

    for (uint8_t i = 0; i < 16; i++) {
        int ch = f.read();
        uint8_t key = keyForChar(ch);
        Serial.printf("%u -> %c [%02X]\n", i, ch, key);
        keytable[i] = key;
    }

    f.close();
}

void updateWiimote2Keys()
{
    // no kempston
    z80ports_wiin[0x1f] = 0;

    wiimote.task();
    if (wiimote.available() > 0) {
        uint16_t button = wiimote.getButtonState();

        if (logWiimoteEvents) Serial.printf("Wiimote button mask: %04X\n", button);

        w2kproc_spectrum(button);
    
        if (button & 0x0080)
            emulateKeyChange(KEY_F1, 1);    }
}

void updateWiimote2KeysOSD()
{
    vTaskDelay(1);
    wiimote.task();
    if (wiimote.available() > 0) {
        uint16_t button = wiimote.getButtonState();

        if (logWiimoteEvents) Serial.printf("Wiimote button mask: %04X\n", button);

        w2kproc_osd(button);
    }
}

void w2kproc_osd(uint16_t button)
{
    uint8_t padl = button & 0x0100 ? 1 : 0;
    uint8_t padr = button & 0x0200 ? 1 : 0;
    uint8_t padb = button & 0x0400 ? 1 : 0;
    uint8_t padt = button & 0x0800 ? 1 : 0;
    uint8_t btwo = button & 0x0001 ? 1 : 0;
    uint8_t bone = button & 0x0002 ? 1 : 0;
    uint8_t trig = button & 0x0004 ? 1 : 0;
    uint8_t baaa = button & 0x0008 ? 1 : 0;
    uint8_t bmin = button & 0x0010 ? 1 : 0;
    uint8_t bhom = button & 0x0080 ? 1 : 0;
    uint8_t bplu = button & 0x1000 ? 1 : 0;

    if (bhom) emulateKeyChange(KEY_F1, 1);
    if (padr) emulateKeyChange(KEY_CURSOR_UP, 1);
    if (padl) emulateKeyChange(KEY_CURSOR_DOWN, 1);
    if (padt) emulateKeyChange(KEY_CURSOR_UP, 1);
    if (padb) emulateKeyChange(KEY_CURSOR_DOWN, 1);
    if (baaa) emulateKeyChange(KEY_ENTER, 1);
    if (bone) emulateKeyChange(KEY_ENTER, 1);
    if (btwo) emulateKeyChange(KEY_ENTER, 1);
}

// wiimote
void w2kproc_spectrum(uint16_t button)
{
    uint16_t mask = 0x0001;
    for (uint8_t i = 0; i < 16; i++)
    {
        uint8_t keycode = keytable[i];
        if (keycode != ZX_KEY_NOKEY)
        {
            uint8_t portidx = keycode >> 4;
            uint8_t bitidx  = keycode & 0x0F;

            uint8_t bitvalue = button & mask ? 0 : 1;
            bitWrite(z80ports_wiin[portidx], bitidx, bitvalue);
        }
        mask <<= 1;
    }
}

