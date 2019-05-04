#include "Emulator/Disk.h"
#include "Emulator/Memory.h"
#include "Emulator/z80main.h"

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

File IRAM_ATTR open_read_file(String filename) {
    mount_spiffs();
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

void IRAM_ATTR load_ram(String sna_file) {
    File lhandle;
    uint16_t size_read;
    byte sp_h, sp_l;
    // force 48K mode
    zx_reset();
    rom_latch = 1;
    paging_lock = 1;
    bank_latch = 0;
    video_latch = 0;

#pragma GCC diagnostic ignored "-Wall"
    Serial.printf("%s SNA: %ub\n", MSG_FREE_HEAP_BEFORE, ESP.getFreeHeap());
#pragma GCC diagnostic warning "-Wall"

    KB_INT_STOP;
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
    uint16_t buf_p = 0x4000;
    while (lhandle.available()) {
        writebyte(buf_p, lhandle.read());
        buf_p++;
    }
    lhandle.close();

    uint16_t offset = thestack - 0x4000;
    uint16_t retaddr = ram5[offset] + 0x100 * ram5[offset + 1];

    _zxCpu.registers.word[Z80_SP]++;
    _zxCpu.registers.word[Z80_SP]++;

    _zxCpu.pc = retaddr;

#pragma GCC diagnostic ignored "-Wall"
    if (cfg_slog_on) {
        Serial.printf("%s SNA: %u\n", MSG_FREE_HEAP_AFTER, ESP.getFreeHeap());
        Serial.printf("Ret address: %x Stack: %x AF: %x Border: %x\n", retaddr, _zxCpu.registers.word[Z80_SP],
                      _zxCpu.registers.word[Z80_AF], borderTemp);
    }
#pragma GCC diagnostic warning "-Wall"
    KB_INT_START;
}

void IRAM_ATTR load_ram_128(String sna_file) {
    File lhandle;
    uint16_t size_read;
    byte sp_h, sp_l;
    zx_reset();
#pragma GCC diagnostic ignored "-Wall"
    Serial.printf("%s SNA: %ub\n", MSG_FREE_HEAP_BEFORE, ESP.getFreeHeap());
#pragma GCC diagnostic warning "-Wall"

    KB_INT_STOP;
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

    uint16_t buf_p;
    for (buf_p = 0x4000; buf_p < 0x8000; buf_p++) {
        writebyte(buf_p, lhandle.read());
    }
    for (buf_p = 0x8000; buf_p < 0xc000; buf_p++) {
        writebyte(buf_p, lhandle.read());
    }
    // bank_latch=6;
    for (buf_p = 0xc000; buf_p < 0xffff; buf_p++) {
        writebyte(buf_p, lhandle.read());
    }

    byte machine = lhandle.read();
    byte retaddr_l = lhandle.read();
    byte retaddr_h = lhandle.read();
    word retaddr = retaddr_l + retaddr_h * 0x100;
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

    lhandle.close();

    video_latch = bitRead(tmp_port, 3);
    rom_latch = bitRead(tmp_port, 4);
    paging_lock = bitRead(tmp_port, 5);
    bank_latch = tmp_latch;

    // sleep(5);
    _zxCpu.pc = retaddr;

#pragma GCC diagnostic ignored "-Wall"
    if (cfg_slog_on) {
        Serial.printf("%s SNA: %u\n", MSG_FREE_HEAP_AFTER, ESP.getFreeHeap());
        Serial.printf(
            "Ret address: %x Stack: %x AF: %x Border: %x bank_latch: %x rom_latch %x video_latch: %x last_port: %x\n",
            retaddr, _zxCpu.registers.word[Z80_SP], _zxCpu.registers.word[Z80_AF], borderTemp, bank_latch, rom_latch,
            video_latch, tmp_port);
    }
#pragma GCC diagnostic warning "-Wall"
    KB_INT_START;
}

String getFileEntriesFromDir(String path) {
    KB_INT_STOP;
    Serial.printf("Getting entries from: %s\n", path.c_str());
    String filelist;
    File root = SPIFFS.open(path.c_str());
    if (!root || !root.isDirectory()) {
        errorHalt((String)ERR_DIR_OPEN + "\n" + root);
    }
    File file = root.openNextFile();
    while (file) {
        Serial.printf("Found %s: %s...", (file.isDirectory() ? "DIR" : "FILE"), file.name());
        String filename = file.name();
        byte start = filename.indexOf("/", path.length()) + 1;
        byte end = filename.indexOf("/", start);
        filename = filename.substring(start, end);
        Serial.printf("%s...", filename.c_str());
        if (filename.startsWith(".")) {
            Serial.println("HIDDEN");
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
    byte n_roms = countFileEntriesFromDir(path);
    for (byte f = 0; f < n_roms; f++) {
        File rom_f = open_read_file(path + "/" + (String)f + ".rom");
        for (int i = 0; i < rom_f.size(); i++) {
            switch (f) {
            case 0:
                rom0[i] = (byte)rom_f.read();
                break;
            case 1:
                rom1[i] = (byte)rom_f.read();
                break;
            }
        }
        rom_f.close();
        vTaskDelay(2);
    }
    KB_INT_START;
}

// Dump actual config to FS
void IRAM_ATTR config_save() {
    KB_INT_STOP;
    Serial.printf("Saving config file '%s'...", DISK_BOOT_FILENAME);
    File f = SPIFFS.open(DISK_BOOT_FILENAME, FILE_WRITE);
    f.printf("arch:%u\n", cfg_arch);
    f.printf("romset:%s\n", cfg_rom_set.c_str());
    f.printf("ram:%s\n", cfg_ram_file.c_str());
    f.printf("slog:%s\n", (cfg_slog_on ? "true" : "false"));
    f.close();
    vTaskDelay(5);
    Serial.println("OK");
    KB_INT_START;
}

// Get all sna files
String getSnaFileList() { return getFileEntriesFromDir(DISK_SNA_DIR); }

// Read config from FS
void config_read() {
    String line;
    File cfg_f;

    // if (cfg_slog_on)
    //    Serial.begin(115200);
    while (!Serial)
        delay(5);

    // Boot config file
    KB_INT_STOP;
    cfg_f = open_read_file(DISK_BOOT_FILENAME);
    for (int i = 0; i < cfg_f.size(); i++) {
        char c = (char)cfg_f.read();
        if (c == '\n') {
            Serial.println("CFG LINE " + line);
            if (line.compareTo("slog:false") == 0) {
                cfg_slog_on = false;
                if (Serial)
                    Serial.end();
            } else if (line.startsWith("ram:")) {
                cfg_ram_file = line.substring(line.lastIndexOf(':') + 1);
            } else if (line.startsWith("arch:")) {
                cfg_arch = line.substring(line.lastIndexOf(':') + 1).toInt();
            } else if (line.startsWith("romset:")) {
                cfg_rom_set = line.substring(line.lastIndexOf(':') + 1);
            }
            line = "";
        } else {
            line.concat(c);
        }
    }
    cfg_f.close();
    cfg_sna_file_list = (String)MENU_SNA_TITLE + "\n" + getSnaFileList();
    KB_INT_START;
}
