#pragma GCC diagnostic ignored "-Wall"
#include "Arduino.h"
#include "Z80.h"
#pragma GCC diagnostic warning "-Wall"

#define CPU_SPEED 3580000
#define CPU_SPEED_MHZ 3.58
#define FRAME_PERIOD_MS 20

extern boolean cfg_slog_on;

void setup_cpuspeed() {
    int calculated_count = CPU_SPEED_MHZ * FRAME_PERIOD_MS * 1000;
    int calculated_period = FRAME_PERIOD_MS * 1000;
    Z80_IPeriod = calculated_period * 2;
    Z80_ICount = calculated_count * 2;
    Serial.printf("CALC ICount:%d IPeriod:%d\n", calculated_count, calculated_period);
}
