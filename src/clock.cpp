
#include "Arduino.h"
#include "Z80.h"


#define CPU_SPEED 3580000


int IFreq = 100;
int CPUSpeed=500;




void setup_cpuspeed() {

  Z80_IPeriod = 20000; //Z80_IPeriod=(3579545*CPUSpeed)/(IFreq*1000);;
  Z80_ICount =  60000;

}
