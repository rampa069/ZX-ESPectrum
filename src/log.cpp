#include "Arduino.h"

extern boolean cfg_slog_on;

void log(String text) {
    if (cfg_slog_on) {
        Serial.println(text);
    }
}
