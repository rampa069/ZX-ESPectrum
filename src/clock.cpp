
#include "Arduino.h"
#include "Z80.h"


#define CPU_SPEED 3580000
#define CPU_SPEED_MHZ 3.58
#define FRAME_PERIOD_MS 20 // or 17??


int IFreq = 100;
int CPUSpeed=500;




void setup_cpuspeed() {


  Z80_IPeriod = 20000; //Z80_IPeriod=(3579545*CPUSpeed)/(IFreq*1000);;
  Z80_ICount =  60000;
  int calculated_count = CPU_SPEED_MHZ * FRAME_PERIOD_MS * 1000;
  int calculated_period = FRAME_PERIOD_MS * 1000;
  Z80_IPeriod=calculated_period;
  Z80_ICount= calculated_count;
  Serial.printf ("calculated ICount: %d\n",calculated_count);
  Serial.printf ("calculated IPeriod: %d\n",calculated_period);

}
