#include "Arduino.h"

extern byte keymap[256];
extern byte oldKeymap[256];
extern SemaphoreHandle_t xULAMutex;
extern void log(String);

void do_OSD() {
    if (keymap != oldKeymap) {
        /*
          Scancodes
          F1..: 0x05
          Esc.: 0x76
        */
        if (keymap[0x05] == 0) {
            xSemaphoreTake(xULAMutex, 0);
            keymap[0x05] = 1;
            log("OSD ON");
            while (keymap[0x05] != 0) {
                log("OSD Active");
            }
            log("OSD OFF");
            keymap[0x05] = 1;
            xSemaphoreGive(xULAMutex);
        }
    }
}
