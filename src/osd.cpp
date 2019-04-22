#include "Arduino.h"
extern byte keymap[256];
extern byte oldKeymap[256];
extern SemaphoreHandle_t xULAMutex;

void do_OSD() {
    if (keymap != oldKeymap) {
        /*
          Scancodes
          F1..: 0x05
          Esc.: 0x76
        */
        if (keymap[0x05] == 0) {
            xSemaphoreTake(xULAMutex, 0);
            Serial.println("OSD ON");
            sleep(10);
            Serial.println("OSD OFF");
            xSemaphoreGive(xULAMutex);
        }
    }
}
