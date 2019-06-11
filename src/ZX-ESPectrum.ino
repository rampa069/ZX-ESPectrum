// -------------------------------------------------------------------
//  ESPectrum Emulator
//  Ramón Martínez & Jorge fuertes 2019
//  Though from ESP32 Spectrum Emulator (KOGEL esp32) Pete Todd 2017
//  You are not allowed to distribute this software commercially
//  lease, notify me, if you make any changes to this file
// -------------------------------------------------------------------

#include "Emulator/Keyboard/PS2Kbd.h"
#include "Emulator/Memory.h"
#include "Emulator/clock.h"
#include "Emulator/z80emu/z80emu.h"
#include "Emulator/z80main.h"
#include "Emulator/z80user.h"
#include <Arduino.h>

#include "MartianVGA.h"

#include "def/Font.h"
#include "def/files.h"
#include "def/hardware.h"
#include "def/msg.h"
#include "net.h"

#include "driver/timer.h"
#include "soc/timer_group_struct.h"
#include <esp_bt.h>

// EXTERN VARS

extern boolean cfg_slog_on;
extern String cfg_ram_file;
extern String cfg_rom_set;
extern String cfg_arch;
extern CONTEXT _zxContext;
extern Z80_STATE _zxCpu;
extern int _total;
extern int _next_total;

void load_rom(String, String);
void load_ram(String);

volatile byte keymap[256];
volatile byte oldKeymap[256];

// EXTERN METHODS

void setup_cpuspeed();
void config_read();
void do_OSD();
void errorHalt(String);
void mount_spiffs();

// GLOBALS
volatile byte z80ports_in[128];
volatile byte borderTemp = 7;
volatile byte flashing = 0;
volatile boolean xULAStop = false;
volatile boolean xULAStopped = false;
volatile byte tick;
const int SAMPLING_RATE = 44100;
const int BUFFER_SIZE = 2000;

int halfsec, sp_int_ctr, evenframe, updateframe;

QueueHandle_t vidQueue;
TaskHandle_t videoTaskHandle;
volatile bool videoTaskIsRunning = false;
uint16_t *param;

// SETUP *************************************
#ifdef COLOUR_8
VGA3Bit vga;
#else
VGA6Bit vga;
#endif


const PinConfig PINCONFIG


void setup() {
    // Turn off peripherals to gain memory (?do they release properly)
    esp_bt_controller_deinit();
    esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

    Serial.begin(115200);
    if (cfg_slog_on) {
        Serial.println(MSG_VGA_INIT);
    }

    Serial.printf("HEAP BEGIN %d\n", ESP.getFreeHeap());

    mount_spiffs();
    config_read();
    // wifiConn();
    Serial.printf("HEAP AFTER WIFI %d\n", ESP.getFreeHeap());

#ifdef BOARD_HAS_PSRAM
    rom0 = (byte *)ps_malloc(16384);
    rom1 = (byte *)ps_malloc(16384);
    rom2 = (byte *)ps_malloc(16384);
    rom3 = (byte *)ps_malloc(16384);

    ram0 = (byte *)ps_malloc(16384);
    ram1 = (byte *)ps_malloc(16384);
    ram2 = (byte *)ps_malloc(16384);
    ram3 = (byte *)ps_malloc(16384);
    ram4 = (byte *)ps_malloc(16384);
    ram5 = (byte *)malloc(16384);
    ram6 = (byte *)ps_malloc(16384);
    ram7 = (byte *)malloc(16384);
#else
    rom0 = (byte *)malloc(16384);
    rom1 = (byte *)malloc(16384);
    //rom2 = (byte *)malloc(16384);

    ram0 = (byte *)malloc(16384);
    ram1 = (byte *)malloc(16384);
    ram2 = (byte *)malloc(16384);
    ram3 = (byte *)malloc(16384);
    ram4 = (byte *)malloc(16384);
    ram5 = (byte *)malloc(16384);
    ram6 = (byte *)malloc(16384);
    ram7 = (byte *)malloc(16384);

    #endif



vga.init(vga.MODE360x200, pinConfig);

    Serial.printf("HEAP after vga  %d \n", ESP.getFreeHeap());

    vga.clear(0);

    pinMode(SPEAKER_PIN, OUTPUT);
    pinMode(EAR_PIN, INPUT);
    pinMode(MIC_PIN, OUTPUT);
    digitalWrite(SPEAKER_PIN, LOW);
    digitalWrite(MIC_PIN, LOW);

    kb_begin();

    Serial.printf("%s bank %u: %ub\n", MSG_FREE_HEAP_AFTER, 0, ESP.getFreeHeap());
    Serial.printf("CALC TSTATES/PERIOD %u\n", CalcTStates());

    // START Z80
    Serial.println(MSG_Z80_RESET);
    zx_setup();

    // make sure keyboard ports are FF
    for (int t = 0; t < 32; t++) {
        z80ports_in[t] = 0x1f;
    }

    Serial.printf("%s %u\n", MSG_EXEC_ON_CORE, xPortGetCoreID());
    Serial.printf("%s Z80 RESET: %ub\n", MSG_FREE_HEAP_AFTER, ESP.getFreeHeap());

    vidQueue = xQueueCreate(1, sizeof(uint16_t *));
    xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, ( 2 | portPRIVILEGE_BIT ), &videoTaskHandle, 0);
    load_rom(cfg_arch, cfg_rom_set);
    if ((String)cfg_ram_file != (String)NO_RAM_FILE) {
        load_ram("/sna/" + cfg_ram_file);
    }

    Serial.println("End of setup");
}

// VIDEO core 0 *************************************

void videoTask(void *unused) {
    unsigned int ff, i, byte_offset;
    unsigned char color_attrib, pixel_map, flash, bright;
    unsigned int zx_vidcalc, calc_y;

    word zx_fore_color, zx_back_color, tmp_color;
    // byte active_latch;

    videoTaskIsRunning = true;
    uint16_t *param;

    while (1) {
        xQueuePeek(vidQueue, &param, portMAX_DELAY);
        if ((int)param == 1)
            break;



        for (unsigned int vga_lin = 0; vga_lin < 200; vga_lin++) {
            // tick = 0;
            if (vga_lin < 3 || vga_lin > 194) {

                for (int bor = 32; bor < 328; bor++)
                    vga.dotFast(bor, vga_lin, zxcolor(borderTemp, 0));
            } else {

                for (int bor = 32; bor < 52; bor++) {
                    vga.dotFast(bor, vga_lin, zxcolor(borderTemp, 0));
                    vga.dotFast(bor + 276, vga_lin, zxcolor(borderTemp, 0));
                }

                for (ff = 0; ff < 32; ff++) // foreach byte in line
                {

                    byte_offset = (vga_lin - 3) * 32 + ff;
                    calc_y = calcY(byte_offset);
                    if (!video_latch)
                    {
                       color_attrib = readbyte(0x5800 + (calc_y / 8) * 32 + ff); // get 1 of 768 attrib values
                       pixel_map = readbyte(byte_offset + 0x4000);
                     } else
                      {
                        color_attrib = ram7[0x1800 + (calc_y / 8) * 32 + ff]; // get 1 of 768 attrib values
                        pixel_map = ram7[byte_offset];
                      }

                    for (i = 0; i < 8; i++) // foreach pixel within a byte
                    {

                        zx_vidcalc = ff * 8 + i;
                        byte bitpos = (0x80 >> i);
                        bright = (color_attrib & 0B01000000) >> 6;
                        flash = (color_attrib & 0B10000000) >> 7;
                        zx_fore_color = zxcolor((color_attrib & 0B00000111), bright);
                        zx_back_color = zxcolor(((color_attrib & 0B00111000) >> 3), bright);

                        if (flash && flashing)
                            swap_flash(&zx_fore_color, &zx_back_color);

                        if ((pixel_map & bitpos) != 0)
                            vga.dotFast(zx_vidcalc + 52, calc_y + 3, zx_fore_color);

                        else
                            vga.dotFast(zx_vidcalc + 52, calc_y + 3, zx_back_color);
                    }
                }
            }
        }

        xQueueReceive(vidQueue, &param, portMAX_DELAY);
        videoTaskIsRunning = false;
    }
    videoTaskIsRunning = false;
    vTaskDelete(NULL);

    while (1) {
    }
}

void swap_flash(word *a, word *b) {
    word temp = *a;
    *a = *b;
    *b = temp;
}

// SPECTRUM SCREEN DISPLAY
//
/* Calculate Y coordinate (0-192) from Spectrum screen memory location */
int calcY(int offset) {
    return ((offset >> 11) << 6)                                            // sector start
           + ((offset % 2048) >> 8)                                         // pixel rows
           + ((((offset % 2048) >> 5) - ((offset % 2048) >> 8 << 3)) << 3); // character rows
}

/* Calculate X coordinate (0-255) from Spectrum screen memory location */
int calcX(int offset) { return (offset % 32) << 3; }

unsigned int zxcolor(int c, int bright) {
    word vga_color;

    switch (c) {

    case 0:
        vga_color = BLACK;
        break;
    case 1:
        vga_color = BLUE;
        break;
    case 2:
        vga_color = RED;
        break;
    case 3:
        vga_color = MAGENTA;
        break;
    case 4:
        vga_color = GREEN;
        break;
    case 5:
        vga_color = CYAN;
        break;
    case 6:
        vga_color = YELLOW;
        break;
    case 7:
        vga_color = WHITE;
        break;
    }

#ifdef COLOUR_16
    if (bright && c != 0)
        vga_color |= 0b00010101;
#endif

    return vga_color;
}

/* Load zx keyboard lines from PS/2 */
void do_keyboard() {
    byte kempston = 0;

    bitWrite(z80ports_in[0], 0, keymap[0x12]);
    bitWrite(z80ports_in[0], 1, keymap[0x1a]);
    bitWrite(z80ports_in[0], 2, keymap[0x22]);
    bitWrite(z80ports_in[0], 3, keymap[0x21]);
    bitWrite(z80ports_in[0], 4, keymap[0x2a]);

    bitWrite(z80ports_in[1], 0, keymap[0x1c]);
    bitWrite(z80ports_in[1], 1, keymap[0x1b]);
    bitWrite(z80ports_in[1], 2, keymap[0x23]);
    bitWrite(z80ports_in[1], 3, keymap[0x2b]);
    bitWrite(z80ports_in[1], 4, keymap[0x34]);

    bitWrite(z80ports_in[2], 0, keymap[0x15]);
    bitWrite(z80ports_in[2], 1, keymap[0x1d]);
    bitWrite(z80ports_in[2], 2, keymap[0x24]);
    bitWrite(z80ports_in[2], 3, keymap[0x2d]);
    bitWrite(z80ports_in[2], 4, keymap[0x2c]);

    bitWrite(z80ports_in[3], 0, keymap[0x16]);
    bitWrite(z80ports_in[3], 1, keymap[0x1e]);
    bitWrite(z80ports_in[3], 2, keymap[0x26]);
    bitWrite(z80ports_in[3], 3, keymap[0x25]);
    bitWrite(z80ports_in[3], 4, keymap[0x2e]);

    bitWrite(z80ports_in[4], 0, keymap[0x45]);
    bitWrite(z80ports_in[4], 1, keymap[0x46]);
    bitWrite(z80ports_in[4], 2, keymap[0x3e]);
    bitWrite(z80ports_in[4], 3, keymap[0x3d]);
    bitWrite(z80ports_in[4], 4, keymap[0x36]);

    bitWrite(z80ports_in[5], 0, keymap[0x4d]);
    bitWrite(z80ports_in[5], 1, keymap[0x44]);
    bitWrite(z80ports_in[5], 2, keymap[0x43]);
    bitWrite(z80ports_in[5], 3, keymap[0x3c]);
    bitWrite(z80ports_in[5], 4, keymap[0x35]);

    bitWrite(z80ports_in[6], 0, keymap[0x5a]);
    bitWrite(z80ports_in[6], 1, keymap[0x4b]);
    bitWrite(z80ports_in[6], 2, keymap[0x42]);
    bitWrite(z80ports_in[6], 3, keymap[0x3b]);
    bitWrite(z80ports_in[6], 4, keymap[0x33]);

    bitWrite(z80ports_in[7], 0, keymap[0x29]);
    bitWrite(z80ports_in[7], 1, keymap[0x14]);
    bitWrite(z80ports_in[7], 2, keymap[0x3a]);
    bitWrite(z80ports_in[7], 3, keymap[0x31]);
    bitWrite(z80ports_in[7], 4, keymap[0x32]);

    // Kempston joystick
    z80ports_in[0x1f] = 0;
    bitWrite(z80ports_in[0x1f], 0, !keymap[KEY_CURSOR_RIGHT]);
    bitWrite(z80ports_in[0x1f], 1, !keymap[KEY_CURSOR_LEFT]);
    bitWrite(z80ports_in[0x1f], 2, !keymap[KEY_CURSOR_DOWN]);
    bitWrite(z80ports_in[0x1f], 3, !keymap[KEY_CURSOR_UP]);
    bitWrite(z80ports_in[0x1f], 4, !keymap[KEY_ALT_GR]);
}

/* +-------------+
   | LOOP core 1 |
   +-------------+
 */
void loop() {
    // static byte last_ts = 0;
    unsigned long ts1, ts2;

    if (halfsec) {
        flashing = ~flashing;
    }
    sp_int_ctr++;
    halfsec = !(sp_int_ctr % 25);

    do_keyboard();
    do_OSD();

    // ts1 = millis();
    zx_loop();
    // ts2 = millis();

    xQueueSend(vidQueue, &param, portMAX_DELAY);

    while (videoTaskIsRunning) {
    }

    /*
    if ((ts2 - ts1) != last_ts) {
        Serial.printf("PC:  %d time: %d\n", _zxCpu.pc, ts2 - ts1);
        last_ts = ts2 - ts1;
    }*/

    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
    vTaskDelay(0); // important to avoid task watchdog timeouts - change this to slow down emu
}
