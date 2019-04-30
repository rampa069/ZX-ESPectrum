#pragma GCC diagnostic ignored "-Wall"
#include <Arduino.h>
#include <SPIFFS.h>
#pragma GCC diagnostic warning "-Wall"
#include "Emulator/machines.h"

const String boot_filename = "/boot.cfg";

extern File open_read_file(String);
extern String getAllFilesFrom(const String);

boolean cfg_mode_sna = false;
boolean cfg_debug_on = false;
boolean cfg_slog_on = true;
String cfg_ram_file = "noram";
String cfg_rom_file = "norom";
String cfg_rom_set = "noromset";
byte cfg_machine_type = MACHINE_ZX48;
String cfg_rom_file_list;
String cfg_sna_file_list;

// Dump actual config to FS
void config_save() {
    File f = SPIFFS.open("/boot.cfg", "w+");
    f.printf("machine:%u\n", cfg_machine_type);
    f.printf("romset:%s\n", cfg_rom_set);
    f.print("mode:");
    if (cfg_mode_sna) {
        f.print("sna\n");
    } else {
        f.print("basic\n");
    }
    f.print("ram:");
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
            Serial.printf("CFG LINE --> %s\n", line.c_str());
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
    cfg_sna_file_list = "Select snapshot to run\n";
    cfg_sna_file_list += getAllFilesFrom("/sna");
}
