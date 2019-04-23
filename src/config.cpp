#include "Arduino.h"
#include "SPIFFS.h"

#define BOOT_FILE "/boot.cfg"

extern void slog(String);

boolean cfg_mode_sna = false;
boolean cfg_debug_on = false;
boolean cfg_slog_on = true;
String cfg_ram_file = "none";
String cfg_rom_file = "/zx48.rom";

void config_read() {
    String line;
    File cfg_f;

    // Serial init
    if (cfg_slog_on) {
        Serial.begin(115200);
        while (!Serial)
            sleep(1);
    }

    // Boot config file
    if (cfg_slog_on)
        Serial.printf("Loading %s\n", BOOT_FILE);
    while (!SPIFFS.begin()) {
        if (cfg_slog_on)
            Serial.println("Internal memory Mount Failed");
        sleep(5);
    }
    cfg_f = SPIFFS.open(BOOT_FILE, FILE_READ);
    while (!cfg_f) {
        if (cfg_slog_on)
            Serial.printf("Cannot read %s\n", BOOT_FILE);
        sleep(10);
        cfg_f = SPIFFS.open(BOOT_FILE, FILE_READ);
    }
    for (int i = 0; i < cfg_f.size(); i++) {
        char c = (char)cfg_f.read();
        if (c == '\n') {
            if (cfg_slog_on)
                Serial.println(line);
            if (line.compareTo("debug:true") == 0) {
                cfg_debug_on = true;
            } else if (line.compareTo("slog:false") == 0) {
                cfg_slog_on = false;
                if (Serial)
                    Serial.end();
            } else if (line.compareTo("mode:sna") == 0) {
                cfg_mode_sna = true;
            } else if (line.startsWith("rom:")) {
                cfg_rom_file = "/" + line.substring(line.lastIndexOf(':') + 1);
            } else if (line.startsWith("ram:")) {
                cfg_ram_file = "/" + line.substring(line.lastIndexOf(':') + 1);
            }
            line = "";
        } else {
            line.concat(c);
        }
    }
    cfg_f.close();
    Serial.print("SNA-->");
    Serial.println(cfg_ram_file);
}
