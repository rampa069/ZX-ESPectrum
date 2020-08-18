#include "Emulator/Keyboard/PS2Kbd.h"
#include "Emulator/Memory.h"
#include "Emulator/z80main.h"
#include "ZX-ESPectrum.h"
#include "def/ascii.h"
#include "def/files.h"
#include "def/msg.h"
#include "def/types.h"
#include "osd.h"
#include <FS.h>
#include <SPIFFS.h>
#include "Wiimote2Keys.h"

void errorHalt(String errormsg);
void IRAM_ATTR kb_interruptHandler(void);
void zx_reset();

// Globals
boolean cfg_demo_mode_on = false;
unsigned short cfg_demo_every;
String cfg_arch = "128K";
String cfg_ram_file = NO_RAM_FILE;
String cfg_rom_set = "SINCLAIR";
String cfg_sna_file_list;
boolean cfg_slog_on = true;
boolean cfg_wconn = false;
String cfg_wssid = "none";
String cfg_wpass = "none";

void IRAM_ATTR mount_spiffs() {
    if (!SPIFFS.begin())
        errorHalt(ERR_MOUNT_FAIL);

    vTaskDelay(2);
}

String getAllFilesFrom(const String path) {
    KB_INT_STOP;
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
    KB_INT_START;
    return listing;
}

void listAllFiles() {
    KB_INT_STOP;
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
    KB_INT_START;
}

File open_read_file(String filename) {
    File f;
    filename.replace("\n", " ");
    filename.trim();
    if (cfg_slog_on)
        Serial.printf("%s '%s'\n", MSG_LOADING, filename.c_str());
    if (!SPIFFS.exists(filename.c_str())) {
        KB_INT_START;
        errorHalt((String)ERR_READ_FILE + "\n" + filename);
    }
    f = SPIFFS.open(filename.c_str(), FILE_READ);
    vTaskDelay(2);

    return f;
}

void load_ram(String sna_file) {
    File lhandle;
    uint16_t size_read;
    byte sp_h, sp_l;
    uint16_t retaddr;
    int sna_size;
    zx_reset();

    Serial.printf("%s SNA: %ub\n", MSG_FREE_HEAP_BEFORE, ESP.getFreeHeap());

    KB_INT_STOP;

    loadKeytableForGame(sna_file.c_str());

    lhandle = open_read_file(sna_file);
    sna_size = lhandle.size();

    if (cfg_arch == "48K") {
        rom_latch = 0;
        rom_in_use = 0;
        bank_latch = 0;
        paging_lock = 1;
    }
    if (sna_size < 50000 && cfg_arch != "48K") {
        rom_in_use = 1;
        rom_latch = 1;
        paging_lock = 1;
        bank_latch = 0;
        video_latch = 0;
        paging_lock = 1;
        bank_latch = 0;
        video_latch = 0;
    }
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

    if (sna_size < 50000) {
        uint16_t thestack = _zxCpu.registers.word[Z80_SP];
        uint16_t buf_p = 0x4000;
        while (lhandle.available()) {
            writebyte(buf_p, lhandle.read());
            buf_p++;
        }

        // uint16_t offset = thestack - 0x4000;
        // retaddr = ram5[offset] + 0x100 * ram5[offset + 1];
        retaddr = readword(thestack);
        Serial.printf("%x\n", retaddr);
        _zxCpu.registers.word[Z80_SP]++;
        _zxCpu.registers.word[Z80_SP]++;
    } else {
        uint16_t buf_p;
        for (buf_p = 0x4000; buf_p < 0x8000; buf_p++) {
            writebyte(buf_p, lhandle.read());
        }
        for (buf_p = 0x8000; buf_p < 0xc000; buf_p++) {
            writebyte(buf_p, lhandle.read());
        }

        for (buf_p = 0xc000; buf_p < 0xffff; buf_p++) {
            writebyte(buf_p, lhandle.read());
        }

        byte machine_b = lhandle.read();
        Serial.printf("Machine: %x\n", machine_b);
        byte retaddr_l = lhandle.read();
        byte retaddr_h = lhandle.read();
        retaddr = retaddr_l + retaddr_h * 0x100;
        byte tmp_port = lhandle.read();

        byte tmp_byte;
        for (int a = 0xc000; a < 0xffff; a++) {
            bank_latch = 0;
            tmp_byte = readbyte(a);
            bank_latch = tmp_port & 0x07;
            writebyte(a, tmp_byte);
        }

        byte tr_dos = lhandle.read();
        byte tmp_latch = tmp_port & 0x7;
        for (int page = 0; page < 8; page++) {
            if (page != tmp_latch && page != 2 && page != 5) {
                bank_latch = page;
                Serial.printf("Page %d actual_latch: %d\n", page, bank_latch);
                for (buf_p = 0xc000; buf_p < 0xFFFF; buf_p++) {
                    writebyte(buf_p, lhandle.read());
                }
            }
        }

        video_latch = bitRead(tmp_port, 3);
        rom_latch = bitRead(tmp_port, 4);
        paging_lock = bitRead(tmp_port, 5);
        bank_latch = tmp_latch;
        rom_in_use = rom_latch;
    }
    lhandle.close();

    _zxCpu.pc = retaddr;
    Serial.printf("%s SNA: %u\n", MSG_FREE_HEAP_AFTER, ESP.getFreeHeap());
    Serial.printf("Ret address: %x Stack: %x AF: %x Border: %x sna_size: %d rom: %d bank: %x\n", retaddr,
                  _zxCpu.registers.word[Z80_SP], _zxCpu.registers.word[Z80_AF], borderTemp, sna_size, rom_in_use,
                  bank_latch);
    KB_INT_START;
}

void save_ram(String sna_file) {
    KB_INT_STOP;

    if (cfg_arch != "48K") {
        Serial.println("save_ram: only 48K supported at the moment");
        return;
    }

    // open file
    File f = SPIFFS.open(sna_file, FILE_WRITE);
    if (!f) {
        Serial.printf("save_ram: failed to open %s for writing\n", sna_file.c_str());
        return;
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
    byte bordercol = 0; // TBD!!!
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

    KB_INT_START;
}

String getFileEntriesFromDir(String path) {
    KB_INT_STOP;
    Serial.printf("Getting entries from: '%s'\n", path.c_str());
    String filelist;
    File root = SPIFFS.open(path.c_str());
    if (!root || !root.isDirectory()) {
        errorHalt((String)ERR_DIR_OPEN + "\n" + root);
    }
    File file = root.openNextFile();
    if (!file)
        Serial.println("No entries found!");
    while (file) {
        Serial.printf("Found %s: %s...%ub...", (file.isDirectory() ? "DIR" : "FILE"), file.name(), file.size());
        String filename = file.name();
        byte start = filename.indexOf("/", path.length()) + 1;
        byte end = filename.indexOf("/", start);
        filename = filename.substring(start, end);
        Serial.printf("%s...", filename.c_str());
        if (filename.startsWith(".")) {
            Serial.println("HIDDEN");
        } else if (filename.endsWith(".txt")) {
            Serial.println("IGNORING TXT");
        } else if (filename.endsWith("quick.sna")) {
            Serial.println("IGNORING QUICKSAVE");
        } else if (cfg_arch == "48K" & file.size() > SIZE48K) {
            Serial.println("128K SKIP");
        } else {
            if (filelist.indexOf(filename) < 0) {
                Serial.println("ADDING");
                filelist += filename + "\n";
            } else {
                Serial.println("EXISTS");
            }
        }
        file = root.openNextFile();
    }
    KB_INT_START;
    return filelist;
}

unsigned short countFileEntriesFromDir(String path) {
    String entries = getFileEntriesFromDir(path);
    unsigned short count = 0;
    for (unsigned short i = 0; i < entries.length(); i++) {
        if (entries.charAt(i) == ASCII_NL) {
            count++;
        }
    }
    return count;
}

void load_rom(String arch, String romset) {
    KB_INT_STOP;
    String path = "/rom/" + arch + "/" + romset;
    Serial.printf("Loading ROMSET '%s'\n", path.c_str());
    byte n_roms = countFileEntriesFromDir(path);
    if (n_roms < 1) {
        errorHalt("No ROMs found at " + path + "\nARCH: '" + arch + "' ROMSET: " + romset);
    }
    Serial.printf("Processing %u ROMs\n", n_roms);
    for (byte f = 0; f < n_roms; f++) {
        File rom_f = open_read_file(path + "/" + (String)f + ".rom");
        Serial.printf("Loading ROM '%s'\n", rom_f.name());
        for (int i = 0; i < rom_f.size(); i++) {
            switch (f) {
            case 0:
                rom0[i] = rom_f.read();
                break;
            case 1:
                rom1[i] = rom_f.read();
                break;

#ifdef BOARD_HAS_PSRAM

            case 2:
                rom2[i] = rom_f.read();
                break;
            case 3:
                rom3[i] = rom_f.read();
                break;
#endif
            }
        }
        rom_f.close();
    }

    KB_INT_START;
}

// Get all sna files
String getSnaFileList() { return getFileEntriesFromDir(DISK_SNA_DIR); }

// Read config from FS
void config_read() {
    KB_INT_STOP;
    String line;
    File cfg_f;

    // Boot config file
    Serial.printf("Loading config file '%s':\n", DISK_BOOT_FILENAME);
    cfg_f = open_read_file(DISK_BOOT_FILENAME);
    for (int i = 0; i < cfg_f.size(); i++) {
        char c = (char)cfg_f.read();
        if (c == '\n') {
            if (line.compareTo("slog:false") == 0) {
                cfg_slog_on = false;
                Serial.printf("  + slog:%s\n", (cfg_slog_on ? "true" : "false"));
                if (Serial)
                    Serial.end();
            } else if (line.startsWith("ram:")) {
                cfg_ram_file = line.substring(line.lastIndexOf(':') + 1);
                Serial.printf("  + ram: '%s'\n", cfg_ram_file.c_str());
            } else if (line.startsWith("arch:")) {
                cfg_arch = line.substring(line.lastIndexOf(':') + 1);
                Serial.printf("  + arch: '%s'\n", cfg_arch.c_str());
            } else if (line.startsWith("romset:")) {
                cfg_rom_set = line.substring(line.lastIndexOf(':') + 1);
                Serial.printf("  + romset: '%s'\n", cfg_rom_set.c_str());
            } else if (line.startsWith("demo_on:")) {
                cfg_demo_mode_on = (line.substring(line.lastIndexOf(':') + 1) == "true");
                Serial.printf("  + demo_on: '%s'\n", (cfg_demo_mode_on ? "true" : "false"));
            } else if (line.startsWith("demo_every:")) {
                cfg_demo_every = line.substring(line.lastIndexOf(':') + 1).toInt();
                Serial.printf("  + demo_every: '%u'\n", cfg_demo_every);
            } else if (line.startsWith("wconn:")) {
                cfg_wconn = (line.substring(line.lastIndexOf(':') + 1) == "true");
                Serial.printf("  + wconn: '%s'\n", (cfg_wconn ? "true" : "false"));
            } else if (line.startsWith("wssid:")) {
                cfg_wssid = line.substring(line.lastIndexOf(':') + 1);
                Serial.printf("  + wssid: '%s'\n", cfg_wssid.c_str());
            } else if (line.startsWith("wpass:")) {
                cfg_wpass = line.substring(line.lastIndexOf(':') + 1);
                Serial.printf("  + wpass: '%s'\n", cfg_wpass.c_str());
            } else if (line.startsWith("slog:")) {
                cfg_slog_on = (line.substring(line.lastIndexOf(':') + 1) == "true");
                Serial.printf("  + slog_on: '%s'\n", (cfg_slog_on ? "true" : "false"));
            }
            line = "";
        } else {
            line.concat(c);
        }
    }
    cfg_f.close();
    Serial.println("Config file loaded OK");
    cfg_sna_file_list = (String)MENU_SNA_TITLE + "\n" + getSnaFileList();
    KB_INT_START;
}

// Dump actual config to FS
void config_save() {
    KB_INT_STOP;
    Serial.printf("Saving config file '%s':\n", DISK_BOOT_FILENAME);
    File f = SPIFFS.open(DISK_BOOT_FILENAME, FILE_WRITE);
    // Architecture
    Serial.printf("  + arch:%s\n", cfg_arch.c_str());
    f.printf("arch:%s\n", cfg_arch.c_str());
    // ROM set
    Serial.printf("  + romset:%s\n", cfg_rom_set.c_str());
    f.printf("romset:%s\n", cfg_rom_set.c_str());
    // RAM SNA
    Serial.printf("  + ram:%s\n", cfg_ram_file.c_str());
    f.printf("ram:%s\n", cfg_ram_file.c_str());
    // Demo ON/OFF
    Serial.printf("  + demo_on:%s\n", (cfg_demo_mode_on ? "true" : "false"));
    f.printf("demo_on:%s\n", (cfg_demo_mode_on ? "true" : "false"));
    // Demo time
    Serial.printf("  + demo_every:%u\n", cfg_demo_every);
    f.printf("demo_every:%u\n", cfg_demo_every);
    // WiFi ON/OFF
    Serial.printf("  + wconn:%s\n", (cfg_wconn ? "true" : "false"));
    f.printf("wconn:%s\n", (cfg_wconn ? "true" : "false"));
    // WiFi SSID
    Serial.printf("  + wssid:%s\n", cfg_wssid.c_str());
    f.printf("wssid:%s\n", cfg_wssid.c_str());
    // WiFi Password
    Serial.printf("  + wpass:%s\n", cfg_wpass.c_str());
    f.printf("wpass:%s\n", cfg_wpass.c_str());
    // Serial logging
    Serial.printf("  + slog:%s\n", (cfg_slog_on ? "true" : "false"));
    f.printf("slog:%s\n", (cfg_slog_on ? "true" : "false"));
    f.close();
    vTaskDelay(5);
    Serial.println("Config saved OK");
    KB_INT_START;
    config_read();
}
