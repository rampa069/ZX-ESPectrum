#include "Arduino.h"
#include "FS.h"
#include "PS2Kbd.h"
#include "SPIFFS.h"
#include "Z80.h"

extern byte *bank0;
extern int start_im1_irq;
extern byte borderTemp;
extern void Z80_SetRegs(Z80_Regs *Regs);
extern String cfg_ram_file;
extern boolean cfg_slog_on;

typedef int32_t dword;
typedef signed char offset;

void load_speccy() {
    File lhandle;
    uint16_t size_read;
    Z80_Regs i;

    if (cfg_slog_on)
        Serial.printf("Free Heap before SNA: %d\n", system_get_free_heap_size());

    if (!SPIFFS.begin()) {
        if (cfg_slog_on)
            Serial.println("Internal memory Mount Failed");
        return;
    }

    // open a file for input
    lhandle = SPIFFS.open(cfg_ram_file, FILE_READ);

    size_read = 0;
    if (lhandle != NULL) {
        Serial.println("Loading:");
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

        Serial.printf("AF : %x\n", i.AF);
        Serial.printf("inter: %x\n", inter);
        i.SP.D = i.SP.B.l + i.SP.B.h * 0x100;
        Serial.printf("SP address: %x\n", i.SP.D);
        Serial.printf("R : %x\n", i.R);
        Serial.printf("IM : %x\n", i.IM);
        Serial.printf("Border : %x\n", bordercol);
        borderTemp = bordercol;

        i.IFF1 = i.IFF2;

        uint16_t thestack = i.SP.D;
        uint16_t buf_p = 0;
        while (lhandle.available()) {
            bank0[buf_p] = lhandle.read();
            buf_p++;
        }
        lhandle.close();
        if (cfg_slog_on) {
            Serial.printf("noof bytes: %x\n", buf_p);
            Serial.printf("STACK:\n");
        }
        if (cfg_slog_on) {
            for (int yy = 0; yy < 16; yy++)
                Serial.printf("%x -> %x\n", thestack + yy, bank0[thestack - 0x4000 + yy]);
        }

        uint16_t offset = thestack - 0x4000;
        uint16_t retaddr = bank0[offset] + 0x100 * bank0[offset + 1];
        if (cfg_slog_on)
            Serial.printf("SP before: %x\n", i.SP.D);
        i.SP.D++;
        i.SP.D++;
        if (cfg_slog_on)
            Serial.printf("SP after: %x\n", i.SP.D);

        i.PC.D = retaddr;
        // i.PC.D = 0x8400;
        start_im1_irq = i.IM;
        if (cfg_slog_on)
            Serial.printf("ret address: %x\n", retaddr);

        Z80_SetRegs(&i);
        if (cfg_slog_on)
            Serial.printf("Free Heap after SNA: %d\n", system_get_free_heap_size());
    } else {
        if (cfg_slog_on)
            Serial.println("Couldn't Open SNA file ");
        return;
    }
}
