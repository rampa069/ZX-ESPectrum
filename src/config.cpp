#include "Arduino.h"
#include "SPIFFS.h"

const String boot_filename = "/boot.cfg";

extern File open_read_file(String);
extern void log(String);

boolean cfg_mode_sna = false;
boolean cfg_debug_on = false;
boolean cfg_slog_on = true;
String cfg_ram_file = "none";
String cfg_rom_file = "/zx48.rom";

void config_read() {
    String line;
    File cfg_f;

    // Boot config file
    cfg_f = open_read_file(boot_filename);
    for (int i = 0; i < cfg_f.size(); i++) {
        char c = (char)cfg_f.read();
        if (c == '\n') {
            log(line);
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
}
