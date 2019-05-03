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
    zx_reset();
    rom_latch=1;
    paging_lock=1;
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
        writebyte(buf_p,lhandle.read());
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


    uint16_t buf_p = 0x4000;
    while (lhandle.available() && buf_p < 0x8000) {
        writebyte(buf_p,lhandle.read());
        buf_p++;
    }
    buf_p = 0x8000;
    while (lhandle.available() && buf_p < 0xc000) {
        writebyte(buf_p,lhandle.read());
        buf_p++;
    }
    buf_p = 0xc000;
    while (lhandle.available() && buf_p < 0xFFFF) {
        writebyte(buf_p,lhandle.read());
        buf_p++;
    }

    byte retaddr_l = lhandle.read();
    byte retaddr_h = lhandle.read();

    byte tmp_port=lhandle.read();


    byte tr_dos=lhandle.read();
    byte tmp_latch=tmp_port&0x7;
    for (int page = 0;page < 8;page++)
    {
      if (page !=tmp_latch)
      {
        bank_latch=page;
        Serial.printf("Page %d actual_latch: %d\n",page,bank_latch );
        buf_p = 0xc000;
        while (lhandle.available()) {
            writebyte(buf_p,lhandle.read());
            buf_p++;
        }
      }
    }

    lhandle.close();

    video_latch=bitRead(tmp_port,3);
    rom_latch=bitRead(tmp_port,4);
    paging_lock=bitRead(tmp_port,5);
    bank_latch=tmp_latch;

    word retaddr = retaddr_l + retaddr_h * 0x100;

    _zxCpu.pc = retaddr;


#pragma GCC diagnostic ignored "-Wall"
    if (cfg_slog_on) {
        Serial.printf("%s SNA: %u\n", MSG_FREE_HEAP_AFTER, ESP.getFreeHeap());
        Serial.printf("Ret address: %x Stack: %x AF: %x Border: %x bank_latch: %x rom_latch %x video_latch: %x\n", retaddr, _zxCpu.registers.word[Z80_SP],
                      _zxCpu.registers.word[Z80_AF], borderTemp, bank_latch,rom_latch,video_latch);
    }
#pragma GCC diagnostic warning "-Wall"
    KB_INT_START;
}


void load_rom(String rom_file) {
    KB_INT_STOP;
    //File rom_f = open_read_file(rom_file);
    File rom_f = open_read_file("/rom/PLUS2A/ZX/0.rom");
    for (int i = 0; i < rom_f.size(); i++) {
        rom0[i] = (byte)rom_f.read();
    }
    rom_f.close();
    rom_f = open_read_file("/rom/PLUS2A/ZX/1.rom");
    for (int i = 0; i < rom_f.size(); i++) {
        rom1[i] = (byte)rom_f.read();
    }
    rom_f.close();
    vTaskDelay(2);
    KB_INT_START;
}

// Dump actual config to FS
void IRAM_ATTR config_save() {
    KB_INT_STOP;
    Serial.printf("Saving config file '%s'...", DISK_BOOT_FILENAME);
    File f = SPIFFS.open(DISK_BOOT_FILENAME, FILE_WRITE);
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
    Serial.println("OK");
    KB_INT_START;
}

// Get all sna files
String getSnaFileList() {
    KB_INT_STOP;
    Serial.printf("Reading dir: %s\n", DISK_SNA_DIR);
    String filelist;
    File snadir = SPIFFS.open(DISK_SNA_DIR);
    if (!snadir || !snadir.isDirectory()) {
        errorHalt(ERR_SNA_DIR_FAIL);
    }
    File file = snadir.openNextFile();
    while (file) {
        Serial.printf("Found sna: %s\n", file.name());
        if (!file.isDirectory()) {
            String filename = file.name();
            filelist += filename.substring(filename.lastIndexOf("/") + 1);
            filelist += "\n";
        }
        file = snadir.openNextFile();
    }
    return filelist;
}

// Read config from FS
void config_read() {
    String line;
    File cfg_f;

    //if (cfg_slog_on)
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

    cfg_sna_file_list = (String)MENU_SNA_TITLE + "\n" + getSnaFileList();

    KB_INT_START;
}
