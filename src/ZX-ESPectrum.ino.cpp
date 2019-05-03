# 1 "/var/folders/gv/jmp9y6cj1qv6tg2tl0t1b6j80000gn/T/tmpTxn6p2"
#include <Arduino.h>
# 1 "/Users/queru/Documents/Desarrollo/spectrum/ZX-ESPectrum/src/ZX-ESPectrum.ino"
# 9 "/Users/queru/Documents/Desarrollo/spectrum/ZX-ESPectrum/src/ZX-ESPectrum.ino"
#include "Emulator/Keyboard/PS2Kbd.h"
#include "Emulator/msg.h"
#include "Emulator/z80emu/z80emu.h"
#include "Emulator/z80user.h"


#include "machinedefs.h"
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>
#include <esp_bt.h>
#include <esp_task_wdt.h>





extern boolean writeScreen;
extern boolean cfg_mode_sna;
extern boolean cfg_slog_on;
extern boolean cfg_debug_on;
extern String cfg_ram_file;
extern String cfg_rom_file;
extern CONTEXT _zxContext;
extern Z80_STATE _zxCpu;
extern int _total;
extern int _next_total;

void load_rom(String);
void load_ram(String);
volatile byte keymap[256];
volatile byte oldKeymap[256];



void zx_setup(void);
void zx_loop();
void zx_reset();
void setup_cpuspeed();
void config_read();
void do_OSD();
void errorHalt(String);
void mount_spiffs();


volatile byte *bank0;
volatile byte z80ports_in[128];
byte borderTemp = 7;
byte soundTemp = 0;
unsigned int flashing = 0;
byte lastAudio = 0;
boolean xULAStop = false;
boolean xULAStopped = false;
boolean writeScreen = false;
byte tick;



#ifdef COLOUR_8
VGA3Bit vga;
#endif

#ifdef COLOUR_16
VGA14Bit vga;
#endif
void setup();
void videoTask(void *parameter);
byte calcY(byte offset);
byte calcX(byte offset);
unsigned int zxcolor(int c, int bright);
void do_keyboard();
void loop();
#line 74 "/Users/queru/Documents/Desarrollo/spectrum/ZX-ESPectrum/src/ZX-ESPectrum.ino"
void setup() {

    esp_bt_controller_deinit();
    esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);


    mount_spiffs();
    config_read();

    if (cfg_slog_on) {
        Serial.println(MSG_CHIP_SETUP);
        Serial.println(MSG_VGA_INIT);
    }

#ifdef COLOUR_8
    vga.init(vga.MODE360x200, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
#endif

#ifdef COLOUR_16
    vga.init(vga.MODE360x200, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
#endif

    Serial.printf("VGA RGB: %x\n", vga.RGBA(0xff, 0xff, 0xff, 0xff));

    pinMode(SOUND_PIN, OUTPUT);
    digitalWrite(SOUND_PIN, LOW);

    kb_begin();



    bank0 = (byte *)malloc(49152);
    if (bank0 == NULL)
        errorHalt((String)ERR_BANK_FAIL + "0");

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    Serial.printf("%s bank %u: %ub\n", MSG_FREE_HEAP_AFTER, 0, system_get_free_heap_size());
#pragma GCC diagnostic warning "-Wall"

    setup_cpuspeed();


    if (cfg_slog_on)
        Serial.println(MSG_Z80_RESET);
    zx_setup();


    for (int t = 0; t < 32; t++) {
        z80ports_in[t] = 0x1f;
    }

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    Serial.printf("%s %u\n", MSG_EXEC_ON_CORE, xPortGetCoreID());
    Serial.printf("%s Z80 RESET: %ub\n", MSG_FREE_HEAP_AFTER, system_get_free_heap_size());
#pragma GCC diagnostic warning "-Wall"

    xTaskCreatePinnedToCore(videoTask,
                            "videoTask",
                            1024,
                            NULL,
                            20,
                            NULL,
                            0);

    load_rom(cfg_rom_file);
    if (cfg_mode_sna)
        load_ram(cfg_ram_file);
}



void videoTask(void *parameter) {
    unsigned int ff, i, byte_offset;
    unsigned char color_attrib, pixel_map, flash, bright;
    unsigned int zx_vidcalc, calc_y;
    unsigned int old_border;
    unsigned int ts1, ts2;
    word zx_fore_color, zx_back_color, tmp_color;

    while (1) {
        while (xULAStop) {
            xULAStopped = true;
            delay(5);
        }
        xULAStopped = false;

        ts1 = millis();

        if (flashing++ > 32)
            flashing = 0;

        for (unsigned int vga_lin = 0; vga_lin < 200; vga_lin++) {
            tick = 0;
            if (vga_lin < 4 || vga_lin > 194) {
                for (int bor = 32; bor < 328; bor++)
                    vga.dotFast(bor, vga_lin, zxcolor(borderTemp, 0));
            } else {
                for (int bor = 32; bor < 52; bor++) {
                    vga.dotFast(bor, vga_lin, zxcolor(borderTemp, 0));
                    vga.dotFast(bor + 276, vga_lin, zxcolor(borderTemp, 0));
                }

                for (ff = 0; ff < 32; ff++)
                {

                    byte_offset = (vga_lin - 3) * 32 + ff;

                    color_attrib = bank0[0x1800 + (calcY(byte_offset) / 8) * 32 + ff];
                    pixel_map = bank0[byte_offset];
                    calc_y = calcY(byte_offset);

                    for (i = 0; i < 8; i++)
                    {

                        zx_vidcalc = ff * 8 + i;
                        byte bitpos = (0x80 >> i);
                        bright = (color_attrib & 0B01000000) >> 6;
                        flash = (color_attrib & 0B10000000) >> 7;
                        zx_fore_color = zxcolor((color_attrib & 0B00000111), bright);
                        zx_back_color = zxcolor(((color_attrib & 0B00111000) >> 3), bright);
                        if (flash && (flashing > 16)) {
                            tmp_color = zx_fore_color;
                            zx_fore_color = zx_back_color;
                            zx_back_color = tmp_color;
                        }

                        writeScreen = true;
                        if ((pixel_map & bitpos) != 0)
                            vga.dotFast(zx_vidcalc + 52, calc_y + 3, zx_fore_color);
                        else
                            vga.dotFast(zx_vidcalc + 52, calc_y + 3, zx_back_color);
                        writeScreen = false;
                    }
                }
            }
        }
        tick = 1;
        ts2 = millis();
    }
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
    if (ts2 - ts1 < 20)
        delay(20 - (ts2 - ts1));

}




byte calcY(byte offset) {
    return ((offset >> 11) << 6)
           + ((offset % 2048) >> 8)
           + ((((offset % 2048) >> 5) - ((offset % 2048) >> 8 << 3)) << 3);
}


byte calcX(byte offset) { return (offset % 32) << 3; }

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
        vga_color |= 0xCE7;
#endif

    return vga_color;
}


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


    z80ports_in[0x1f] = 0;
    bitWrite(z80ports_in[0x1f], 0, !keymap[0x74]);
    bitWrite(z80ports_in[0x1f], 1, !keymap[0x6b]);
    bitWrite(z80ports_in[0x1f], 2, !keymap[0x72]);
    bitWrite(z80ports_in[0x1f], 3, !keymap[0x75]);
    bitWrite(z80ports_in[0x1f], 4, !keymap[0x73]);
# 348 "/Users/queru/Documents/Desarrollo/spectrum/ZX-ESPectrum/src/ZX-ESPectrum.ino"
}





void loop() {
    while (1) {
        do_keyboard();
        do_OSD();

        zx_loop();
        TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
        TIMERG0.wdt_feed = 1;
        TIMERG0.wdt_wprotect = 0;
        vTaskDelay(1);
    }
}
# 1 "/Users/queru/Documents/Desarrollo/spectrum/ZX-ESPectrum/src/osd.ino"
#include "OSD/osd.h"




void do_OSD() {
    static byte last_sna_row = 0;
    if (checkAndCleanKey(KEY_F12)) {

        last_sna_row++;
        if (last_sna_row > menuRowCount(cfg_sna_file_list) - 1) {
            last_sna_row = 1;
        }

        changeSna(menuGetRow(cfg_sna_file_list, last_sna_row));
    } else if (checkAndCleanKey(KEY_F1)) {

        stopULA();
        switch (do_Menu(MENU_MAIN)) {
        case 1:

            break;
        case 2: {

            unsigned short snanum = do_Menu(cfg_sna_file_list);
            if (snanum > 0) {
                changeSna(menuGetRow(cfg_sna_file_list, snanum));
            }
            break;
        }
        case 3:

            switch (do_Menu(MENU_RESET)) {
            case 1:

                zx_reset();
                if (cfg_mode_sna)
                    load_ram(cfg_ram_file);
                break;
            case 2:

                zx_reset();
                cfg_mode_sna = false;
                cfg_ram_file = 'none';
                config_save();
                break;
            }
        case 4:

            drawOSD();
            osdAt(2, 0);
            vga.setTextColor(zxcolor(7, 0), zxcolor(1, 0));
            vga.print(OSD_HELP);
            while (!checkAndCleanKey(KEY_F1) && !checkAndCleanKey(KEY_ESC) && !checkAndCleanKey(KEY_ENTER))
                vTaskDelay(5);
        }


        startULA();
    }
}