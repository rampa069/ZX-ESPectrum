#include "Emulator/Keyboard/PS2Kbd.h"
#include "Emulator/msg.h"
#include "Emulator/z80emu/z80emu.h"
#include "Emulator/machines.h"

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

extern byte *bank0;
extern byte borderTemp;
extern Z80_STATE _zxCpu;
void errorHalt(String errormsg);
extern boolean cfg_slog_on;


boolean cfg_mode_sna = false;
boolean cfg_debug_on = false;
boolean cfg_slog_on = true;
String cfg_ram_file = "noram";
String cfg_rom_file = "norom";
String cfg_rom_set = "noromset";
byte cfg_machine_type = MACHINE_ZX48;
String cfg_rom_file_list;
String cfg_sna_file_list;

const String boot_filename = "/boot.cfg";

void zx_reset();

byte specrom[16384];

typedef int32_t dword;
typedef signed char offset;

void mount_spiffs() {
    if (!SPIFFS.begin())
        errorHalt(ERR_MOUNT_FAIL);

    vTaskDelay(2);

}

String getAllFilesFrom(const String path) {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    String listing;

    while (file) {
        file = root.openNextFile();
        String filename = file.name();
        if (filename.startsWith(path) && !filename.startsWith(path + "/.")) {
            listing.concat(filename.substring(path.length() + 1));
            listing.concat("\n");
        }
    }
    vTaskDelay(2);
    return listing;
}

void listAllFiles() {
    File root = SPIFFS.open("/");
    Serial.println("fs opened");
    File file = root.openNextFile();
    Serial.println("fs openednextfile");

    while (file) {
        Serial.print("FILE: ");
        Serial.println(file.name());
        file = root.openNextFile();
    }
    vTaskDelay(2);

}

File open_read_file(String filename) {
    File f;
    filename.replace("\n", " ");
    filename.trim();
    if (cfg_slog_on)
        Serial.println(MSG_LOADING + filename);
    if (!SPIFFS.exists(filename.c_str())) {
        errorHalt(ERR_READ_FILE + "\n" + filename);
    }
    f = SPIFFS.open(filename.c_str(), FILE_READ);
    vTaskDelay(2);

    return f;
}

void load_ram(String sna_file) {
    File lhandle;
    uint16_t size_read;
    byte sp_h, sp_l;
    zx_reset();
#pragma GCC diagnostic ignored "-Wall"
    if (cfg_slog_on)
        Serial.println(MSG_FREE_HEAP_BEFORE + "SNA: " + (String)system_get_free_heap_size());
#pragma GCC diagnostic warning "-Wall"

    lhandle = open_read_file(sna_file);
    vTaskDelay(10);
    size_read = 0;
    // Read in the registers
    _zxCpu.i = lhandle.read();
    _zxCpu.registers.byte[Z80_L] = lhandle.read();
    _zxCpu.registers.byte[Z80_H] = lhandle.read();
    _zxCpu.registers.byte[Z80_E] = lhandle.read();
    _zxCpu.registers.byte[Z80_D] = lhandle.read();
    _zxCpu.registers.byte[Z80_C] = lhandle.read();
    _zxCpu.registers.byte[Z80_B] = lhandle.read();
    _zxCpu.registers.byte[Z80_F] = lhandle.read();
    _zxCpu.registers.byte[Z80_A] = lhandle.read();

    _zxCpu.alternates[Z80_HL] = _zxCpu.registers.word[Z80_HL];
    _zxCpu.alternates[Z80_DE] = _zxCpu.registers.word[Z80_DE];
    _zxCpu.alternates[Z80_BC] = _zxCpu.registers.word[Z80_BC];
    _zxCpu.alternates[Z80_AF] = _zxCpu.registers.word[Z80_AF];

    _zxCpu.registers.byte[Z80_L] = lhandle.read();
    _zxCpu.registers.byte[Z80_H] = lhandle.read();
    _zxCpu.registers.byte[Z80_E] = lhandle.read();
    _zxCpu.registers.byte[Z80_D] = lhandle.read();
    _zxCpu.registers.byte[Z80_C] = lhandle.read();
    _zxCpu.registers.byte[Z80_B] = lhandle.read();
    _zxCpu.registers.byte[Z80_IYL] = lhandle.read();
    _zxCpu.registers.byte[Z80_IYH] = lhandle.read();
    _zxCpu.registers.byte[Z80_IXL] = lhandle.read();
    _zxCpu.registers.byte[Z80_IXH] = lhandle.read();

    byte inter = lhandle.read();
    _zxCpu.iff2 = (inter & 0x04) ? 1 : 0;
    _zxCpu.r = lhandle.read();

    _zxCpu.registers.byte[Z80_F] = lhandle.read();
    _zxCpu.registers.byte[Z80_A] = lhandle.read();

    sp_l = lhandle.read();
    sp_h = lhandle.read();
    _zxCpu.registers.word[Z80_SP] = sp_l + sp_h * 0x100;

    _zxCpu.im = lhandle.read();
    byte bordercol = lhandle.read();

    borderTemp = bordercol;

    _zxCpu.iff1 = _zxCpu.iff2;

    uint16_t thestack = _zxCpu.registers.word[Z80_SP];
    uint16_t buf_p = 0;
    while (lhandle.available()) {
        bank0[buf_p] = lhandle.read();
        buf_p++;
    }
    lhandle.close();

    uint16_t offset = thestack - 0x4000;
    uint16_t retaddr = bank0[offset] + 0x100 * bank0[offset + 1];

    _zxCpu.registers.word[Z80_SP]++;
    _zxCpu.registers.word[Z80_SP]++;

    _zxCpu.pc = retaddr;
    //_zxCpu.pc=0x8400;

#pragma GCC diagnostic ignored "-Wall"
    if (cfg_slog_on) {
        Serial.println(MSG_FREE_HEAP_AFTER + "SNA: " + (String)system_get_free_heap_size());
        Serial.printf("Ret address: %x Stack: %x AF: %x Border: %x\n", retaddr, _zxCpu.registers.word[Z80_SP],
                      _zxCpu.registers.word[Z80_AF], borderTemp);
    }
#pragma GCC diagnostic warning "-Wall"
}

void load_rom(String rom_file) {
    File rom_f = open_read_file(rom_file);
    for (int i = 0; i < rom_f.size(); i++) {
        specrom[i] = (byte)rom_f.read();
    }
    rom_f.close();
    vTaskDelay(2);

}

// Dump actual config to FS
void IRAM_ATTR config_save() {
    Serial.printf("Saving config file.....\n" );
    File f = SPIFFS.open("/boot.cfg", FILE_WRITE);
    f.printf("machine:%u\n", cfg_machine_type);
    f.printf("romset:%s\n", cfg_rom_set.c_str());
    f.print("mode:");
    if (cfg_mode_sna) {
        f.print("sna\n");
    } else {
        f.print("basic\n");
    }
    f.print("ram:");
    String ram_file = cfg_ram_file;
    if (cfg_ram_file.lastIndexOf("/") >= 0) {
        ram_file = cfg_ram_file.substring((cfg_ram_file.lastIndexOf("/") + 1));
    }
    f.println(ram_file.c_str());
    if (cfg_debug_on) {
        f.print("debug:true\n");
    } else {
        f.print("debug:false\n");
    }
    if (cfg_slog_on) {
        f.print("slog:true\n");
    } else {
        f.print("slog:false\n");
    }
    f.close();
    vTaskDelay(5);
}

// Read config from FS
void config_read() {
    String line;
    File cfg_f;

    if (cfg_slog_on)
        Serial.begin(115200);
    while (!Serial)
        delay(5);

    // Boot config file
    cfg_f = open_read_file(boot_filename);
    for (int i = 0; i < cfg_f.size(); i++) {
        char c = (char)cfg_f.read();
        if (c == '\n') {
            Serial.println("CFG LINE " + line);
            if (line.compareTo("debug:true") == 0) {
                cfg_debug_on = true;
            } else if (line.compareTo("slog:false") == 0) {
                cfg_slog_on = false;
                if (Serial)
                    Serial.end();
            } else if (line.compareTo("mode:sna") == 0) {
                cfg_mode_sna = true;
            } else if (line.startsWith("rom:")) {
                cfg_rom_file = "/rom/" + line.substring(line.lastIndexOf(':') + 1);
            } else if (line.startsWith("ram:")) {
                cfg_ram_file = "/sna/" + line.substring(line.lastIndexOf(':') + 1);
            } else if (line.startsWith("machine:")) {
                cfg_machine_type = line.substring(line.lastIndexOf(':') + 1).toInt();
            } else if (line.startsWith("romset:")) {
                cfg_rom_set = line.substring(line.lastIndexOf(':') + 1);
            }
            line = "";
        } else {
            line.concat(c);
        }
    }
    cfg_f.close();

    // ROM file selection
    cfg_rom_file = "/rom/";
    switch (cfg_machine_type) {
    case MACHINE_ZX48:
        cfg_rom_file += "48K/";
        break;
    case MACHINE_ZX128:
        cfg_rom_file += "128K/";
        break;
    case MACHINE_PLUS2A:
        cfg_rom_file += "PLUS2A/";
        break;
    case MACHINE_PLUS3:
        cfg_rom_file += "PLUS3/";
        break;
    case MACHINE_PLUS3E:
        cfg_rom_file += "PLUS3E/";
        break;
    }
    cfg_rom_file += cfg_rom_set;
    cfg_rom_file += "/0.rom";

    // Rom file list;
    cfg_rom_file_list = getAllFilesFrom("/rom");
    cfg_sna_file_list = "Select snapshot to run\n" + getAllFilesFrom("/sna");
}
