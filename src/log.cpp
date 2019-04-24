#include "Arduino.h"

extern boolean cfg_slog_on;

void log(String text) {
    // Serial init
    if (!Serial) {
        Serial.begin(115200);
        while (!Serial)
            sleep(1);
        Serial.println("Serial begin");
    }
    Serial.println(text);
}
