#include "Disk.h"
#include "def/speed.h"
#include <Arduino.h>

int CalcTStates() {
    if (cfg_arch == "48K") {
        return CPU_SPEED_MHZ_ZX48 * FRAME_PERIOD_MS * 1000;
    } else {
        return CPU_SPEED_MHZ_ZX128 * FRAME_PERIOD_MS * 1000;
    }
}
