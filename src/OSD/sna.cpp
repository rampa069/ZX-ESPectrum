#include "Disk.h"
#include "Emulator/Keyboard/PS2Kbd.h"
#include "Emulator/z80main.h"
#include "ZX-ESPectrum.h"
#include "def/files.h"
#include "def/msg.h"
#include "osd.h"
#include <FS.h>
#include <SPIFFS.h>

// Change running snapshot
void changeSna(String sna_filename) {
    osdCenteredMsg((String)MSG_LOADING + ": " + sna_filename, LEVEL_INFO);
    zx_reset();
    Serial.printf("Loading sna: %s\n", sna_filename.c_str());
    load_ram((String)DISK_SNA_DIR + "/" + sna_filename);
    osdCenteredMsg(MSG_SAVE_CONFIG, LEVEL_WARN);
    cfg_ram_file = sna_filename;
    config_save();
}

// Demo mode on off
void setDemoMode(boolean on, unsigned short every) {
    cfg_demo_mode_on = on;
    cfg_demo_every = (every > 0 ? every : 60);
    if (on) {
        osdCenteredMsg(OSD_DEMO_MODE_ON, LEVEL_OK);
    } else {
        osdCenteredMsg(OSD_DEMO_MODE_OFF, LEVEL_WARN);
    }
    Serial.printf("DEMO MODE %s every %u seconds.", (cfg_demo_mode_on ? "ON" : "OFF"), cfg_demo_every);
    vTaskDelay(200);
    osdCenteredMsg(MSG_SAVE_CONFIG, LEVEL_WARN);
    config_save();
}

bool is_persist_sna_available()
{
    String filename = DISK_PSNA_FILE;
    return SPIFFS.exists(filename.c_str());
}

bool save_ram(String sna_file) {
    KB_INT_STOP;

    // only 48K snapshot supported at the moment
    if (cfg_arch != "48K") {
        Serial.println("save_ram: only 48K supported at the moment");
        KB_INT_START;
        return false;
    }

    // open file
    File f = SPIFFS.open(sna_file, FILE_WRITE);
    if (!f) {
        Serial.printf("save_ram: failed to open %s for writing\n", sna_file.c_str());
        KB_INT_START;
        return false;
    }

    // write registers: begin with I
    f.write(_zxCpu.i);

    // store registers
    unsigned short HL = _zxCpu.registers.word[Z80_HL];
    unsigned short DE = _zxCpu.registers.word[Z80_DE];
    unsigned short BC = _zxCpu.registers.word[Z80_BC];
    unsigned short AF = _zxCpu.registers.word[Z80_AF];

    // put alternates in registers
    _zxCpu.registers.word[Z80_HL] = _zxCpu.alternates[Z80_HL];
    _zxCpu.registers.word[Z80_DE] = _zxCpu.alternates[Z80_DE];
    _zxCpu.registers.word[Z80_BC] = _zxCpu.alternates[Z80_BC];
    _zxCpu.registers.word[Z80_AF] = _zxCpu.alternates[Z80_AF];

    // write alternates
    f.write(_zxCpu.registers.byte[Z80_L]);
    f.write(_zxCpu.registers.byte[Z80_H]);
    f.write(_zxCpu.registers.byte[Z80_E]);
    f.write(_zxCpu.registers.byte[Z80_D]);
    f.write(_zxCpu.registers.byte[Z80_C]);
    f.write(_zxCpu.registers.byte[Z80_B]);
    f.write(_zxCpu.registers.byte[Z80_F]);
    f.write(_zxCpu.registers.byte[Z80_A]);

    // restore registers
    _zxCpu.registers.word[Z80_HL] = HL;
    _zxCpu.registers.word[Z80_DE] = DE;
    _zxCpu.registers.word[Z80_BC] = BC;
    _zxCpu.registers.word[Z80_AF] = AF;

    // write registers
    f.write(_zxCpu.registers.byte[Z80_L]);
    f.write(_zxCpu.registers.byte[Z80_H]);
    f.write(_zxCpu.registers.byte[Z80_E]);
    f.write(_zxCpu.registers.byte[Z80_D]);
    f.write(_zxCpu.registers.byte[Z80_C]);
    f.write(_zxCpu.registers.byte[Z80_B]);
    f.write(_zxCpu.registers.byte[Z80_IYL]);
    f.write(_zxCpu.registers.byte[Z80_IYH]);
    f.write(_zxCpu.registers.byte[Z80_IXL]);
    f.write(_zxCpu.registers.byte[Z80_IXH]);

    byte inter = _zxCpu.iff2 ? 0x04 : 0;
    f.write(inter);
    f.write(_zxCpu.r);

    f.write(_zxCpu.registers.byte[Z80_F]);
    f.write(_zxCpu.registers.byte[Z80_A]);

    // read stack pointer and decrement it for pushing PC
    unsigned short SP = _zxCpu.registers.word[Z80_SP];
    SP -= 2;
    byte sp_l = SP & 0xFF;
    byte sp_h = SP >> 8;
    f.write(sp_l);
    f.write(sp_h);

    f.write(_zxCpu.im);
    byte bordercol = borderTemp;
    f.write(bordercol);

    // push PC to stack
    unsigned short PC = _zxCpu.pc;
    byte pc_l = PC & 0xFF;
    byte pc_h = PC >> 8;
    writebyte(SP+0, pc_l);
    writebyte(SP+1, pc_h);

    // dump memory to file
    for (int addr = 0x4000; addr <= 0xFFFF; addr++) {
        byte b = readbyte(addr);
        f.write(b);
    }

    f.close();
    return true;
    KB_INT_START;
}

static byte* snabuf = NULL;

bool is_quick_sna_available()
{
    return snabuf != NULL;
}

bool save_ram_quick()
{
    KB_INT_STOP;

    // only 48K snapshot supported at the moment
    if (cfg_arch != "48K") {
        Serial.println("save_ram_quick: only 48K supported at the moment");
        KB_INT_START;
        return false;
    }

    // allocate buffer it not done yet
    if (snabuf == NULL)
    {
#ifdef BOARD_HAS_PSRAM
        snabuf = (byte*)ps_malloc(49179);
#else
        snabuf = (byte*)malloc(49179);
#endif
        if (snabuf == NULL) {
            Serial.println("save_ram_quick: cannot allocate memory for snapshot buffer");
            KB_INT_START;
            return false;
        }
    }

    byte* snaptr = snabuf;

    // write registers: begin with I
    *snaptr++ = _zxCpu.i;

    // store registers
    unsigned short HL = _zxCpu.registers.word[Z80_HL];
    unsigned short DE = _zxCpu.registers.word[Z80_DE];
    unsigned short BC = _zxCpu.registers.word[Z80_BC];
    unsigned short AF = _zxCpu.registers.word[Z80_AF];

    // put alternates in registers
    _zxCpu.registers.word[Z80_HL] = _zxCpu.alternates[Z80_HL];
    _zxCpu.registers.word[Z80_DE] = _zxCpu.alternates[Z80_DE];
    _zxCpu.registers.word[Z80_BC] = _zxCpu.alternates[Z80_BC];
    _zxCpu.registers.word[Z80_AF] = _zxCpu.alternates[Z80_AF];

    // write alternates
    *snaptr++ = _zxCpu.registers.byte[Z80_L];
    *snaptr++ = _zxCpu.registers.byte[Z80_H];
    *snaptr++ = _zxCpu.registers.byte[Z80_E];
    *snaptr++ = _zxCpu.registers.byte[Z80_D];
    *snaptr++ = _zxCpu.registers.byte[Z80_C];
    *snaptr++ = _zxCpu.registers.byte[Z80_B];
    *snaptr++ = _zxCpu.registers.byte[Z80_F];
    *snaptr++ = _zxCpu.registers.byte[Z80_A];

    // restore registers
    _zxCpu.registers.word[Z80_HL] = HL;
    _zxCpu.registers.word[Z80_DE] = DE;
    _zxCpu.registers.word[Z80_BC] = BC;
    _zxCpu.registers.word[Z80_AF] = AF;

    // write registers
    *snaptr++ = _zxCpu.registers.byte[Z80_L];
    *snaptr++ = _zxCpu.registers.byte[Z80_H];
    *snaptr++ = _zxCpu.registers.byte[Z80_E];
    *snaptr++ = _zxCpu.registers.byte[Z80_D];
    *snaptr++ = _zxCpu.registers.byte[Z80_C];
    *snaptr++ = _zxCpu.registers.byte[Z80_B];
    *snaptr++ = _zxCpu.registers.byte[Z80_IYL];
    *snaptr++ = _zxCpu.registers.byte[Z80_IYH];
    *snaptr++ = _zxCpu.registers.byte[Z80_IXL];
    *snaptr++ = _zxCpu.registers.byte[Z80_IXH];

    byte inter = _zxCpu.iff2 ? 0x04 : 0;
    *snaptr++ = inter;
    *snaptr++ = _zxCpu.r;

    *snaptr++ = _zxCpu.registers.byte[Z80_F];
    *snaptr++ = _zxCpu.registers.byte[Z80_A];

    // read stack pointer and decrement it for pushing PC
    unsigned short SP = _zxCpu.registers.word[Z80_SP];
    SP -= 2;
    byte sp_l = SP & 0xFF;
    byte sp_h = SP >> 8;
    *snaptr++ = sp_l;
    *snaptr++ = sp_h;

    *snaptr++ = _zxCpu.im;
    byte bordercol = borderTemp;
    *snaptr++ = bordercol;

    // push PC to stack
    unsigned short PC = _zxCpu.pc;
    byte pc_l = PC & 0xFF;
    byte pc_h = PC >> 8;
    writebyte(SP+0, pc_l);
    writebyte(SP+1, pc_h);

    // dump memory to file
    for (int addr = 0x4000; addr <= 0xFFFF; addr++) {
        byte b = readbyte(addr);
        *snaptr++ = b;
    }

    KB_INT_START;
    return true;
}

bool load_ram_quick()
{
    // only 48K snapshot supported at the moment
    if (cfg_arch != "48K") {
        Serial.println("save_ram_quick: only 48K supported at the moment");
        KB_INT_START;
        return false;
    }

    if (NULL == snabuf) {
        // nothing to read
        Serial.println("save_ram_quick: nothing to load");
        KB_INT_START;
        return false;
    }

    byte* snaptr = snabuf;

    // Read in the registers
    _zxCpu.i = *snaptr++;
    _zxCpu.registers.byte[Z80_L] = *snaptr++;
    _zxCpu.registers.byte[Z80_H] = *snaptr++;
    _zxCpu.registers.byte[Z80_E] = *snaptr++;
    _zxCpu.registers.byte[Z80_D] = *snaptr++;
    _zxCpu.registers.byte[Z80_C] = *snaptr++;
    _zxCpu.registers.byte[Z80_B] = *snaptr++;
    _zxCpu.registers.byte[Z80_F] = *snaptr++;
    _zxCpu.registers.byte[Z80_A] = *snaptr++;

    _zxCpu.alternates[Z80_HL] = _zxCpu.registers.word[Z80_HL];
    _zxCpu.alternates[Z80_DE] = _zxCpu.registers.word[Z80_DE];
    _zxCpu.alternates[Z80_BC] = _zxCpu.registers.word[Z80_BC];
    _zxCpu.alternates[Z80_AF] = _zxCpu.registers.word[Z80_AF];

    _zxCpu.registers.byte[Z80_L] = *snaptr++;
    _zxCpu.registers.byte[Z80_H] = *snaptr++;
    _zxCpu.registers.byte[Z80_E] = *snaptr++;
    _zxCpu.registers.byte[Z80_D] = *snaptr++;
    _zxCpu.registers.byte[Z80_C] = *snaptr++;
    _zxCpu.registers.byte[Z80_B] = *snaptr++;
    _zxCpu.registers.byte[Z80_IYL] = *snaptr++;
    _zxCpu.registers.byte[Z80_IYH] = *snaptr++;
    _zxCpu.registers.byte[Z80_IXL] = *snaptr++;
    _zxCpu.registers.byte[Z80_IXH] = *snaptr++;

    byte inter = *snaptr++;
    _zxCpu.iff2 = (inter & 0x04) ? 1 : 0;
    _zxCpu.r = *snaptr++;

    _zxCpu.registers.byte[Z80_F] = *snaptr++;
    _zxCpu.registers.byte[Z80_A] = *snaptr++;

    byte sp_l = *snaptr++;
    byte sp_h = *snaptr++;
    _zxCpu.registers.word[Z80_SP] = sp_l + sp_h * 0x100;

    _zxCpu.im = *snaptr++;
    byte bordercol = *snaptr++;

    borderTemp = bordercol;

    _zxCpu.iff1 = _zxCpu.iff2;

    uint16_t thestack = _zxCpu.registers.word[Z80_SP];
    for (int addr = 0x4000; addr <= 0xFFFF; addr++) {
        writebyte(addr, *snaptr++);
    }

    uint16_t retaddr = readword(thestack);
    Serial.printf("%x\n", retaddr);
    _zxCpu.registers.word[Z80_SP]++;
    _zxCpu.registers.word[Z80_SP]++;

    _zxCpu.pc = retaddr;

    KB_INT_START;
    return true;
}