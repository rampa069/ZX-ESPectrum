#include "Arduino.h"
//#include "Z80.h"

#define CPU_SPEED 3580000
#define CPU_SPEED_MHZ 3.58
#define FRAME_PERIOD_MS 20

extern boolean cfg_slog_on;
extern void log(String);

void setup_cpuspeed() {
    int calculated_count = CPU_SPEED_MHZ * FRAME_PERIOD_MS * 1000;
    int calculated_period = FRAME_PERIOD_MS * 1000;
    //Z80_IPeriod = calculated_period * 2;
    //Z80_ICount = calculated_count * 2;
    log("Calculated ICount: " + (String)calculated_count);
    log("Calculated IPeriod:" + (String)calculated_period);
}
