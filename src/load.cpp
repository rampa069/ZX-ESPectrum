#pragma GCC diagnostic ignored "-Wall"
#include "Z80.h"
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#pragma GCC diagnostic warning "-Wall"
#include "PS2Kbd.h"
#include "msg.h"

extern byte *bank0;
extern int start_im1_irq;
extern byte borderTemp;
extern void Z80_SetRegs(Z80_Regs *Regs);
extern boolean cfg_slog_on;

byte specrom[16384];

typedef int32_t dword;
typedef signed char offset;

void mount_spiffs() {
    while (!SPIFFS.begin()) {
        if (cfg_slog_on)
            Serial.println(MSG_MOUNT_FAIL);
        delay(500);
    }
}

File open_read_file(String filename) {
    File f;
    mount_spiffs();
    if (cfg_slog_on)
        Serial.println(MSG_LOADING + filename);
    f = SPIFFS.open(filename, FILE_READ);
    while (!f) {
        if (cfg_slog_on)
            Serial.println(MSG_READ_FILE_FAIL + filename);
        sleep(10);
        f = SPIFFS.open(filename, FILE_READ);
    }
    return f;
}

String getDirAsMenu(String title, String path) {
    String menu = title + "\n";
    return menu;
}

void load_ram(String sna_file) {
    File lhandle;
    Z80_Regs i;

    Serial.println(MSG_FREE_HEAP_BEFORE + "SNA: " + (String)system_get_free_heap_size());

    lhandle = open_read_file(sna_file);
    // Read in the registers
    i.I = lhandle.read();
    i.HL2.B.l = lhandle.read();
    i.HL2.B.h = lhandle.read();
    i.DE2.B.l = lhandle.read();
    i.DE2.B.h = lhandle.read();
    i.BC2.B.l = lhandle.read();
    i.BC2.B.h = lhandle.read();
    i.AF2.B.l = lhandle.read();
    i.AF2.B.h = lhandle.read();
    i.HL.B.l = lhandle.read();
    i.HL.B.h = lhandle.read();
    i.DE.B.l = lhandle.read();
    i.DE.B.h = lhandle.read();
    i.BC.B.l = lhandle.read();
    i.BC.B.h = lhandle.read();
    i.IY.B.l = lhandle.read();
    i.IY.B.h = lhandle.read();
    i.IX.B.l = lhandle.read();
    i.IX.B.h = lhandle.read();

    byte inter = lhandle.read();
    i.IFF2 = (inter & 0x04) ? 1 : 0;

    i.R = lhandle.read();
    i.AF.B.h = lhandle.read();
    i.AF.B.l = lhandle.read();
    i.SP.B.l = lhandle.read();
    i.SP.B.h = lhandle.read();
    i.IM = lhandle.read();
    byte bordercol = lhandle.read();

    i.SP.D = i.SP.B.l + i.SP.B.h * 0x100;

    borderTemp = bordercol;

    i.IFF1 = i.IFF2;

    uint16_t thestack = i.SP.D;
    uint16_t buf_p = 0;
    while (lhandle.available()) {
        bank0[buf_p] = lhandle.read();
        buf_p++;
    }
    lhandle.close();

    uint16_t offset = thestack - 0x4000;
    uint16_t retaddr = bank0[offset] + 0x100 * bank0[offset + 1];
    i.SP.D++;
    i.SP.D++;

    i.PC.D = retaddr;
    start_im1_irq = i.IM;

    Z80_SetRegs(&i);
    Serial.println(MSG_FREE_HEAP_AFTER + "SNA: " + (String)system_get_free_heap_size());
}

void load_rom(String rom_file) {
    File rom_f = open_read_file(rom_file);
    for (int i = 0; i < rom_f.size(); i++) {
        specrom[i] = (byte)rom_f.read();
    }
}
