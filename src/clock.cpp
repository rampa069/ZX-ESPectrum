
#include "Arduino.h"
#include "Z80.h"


#define CPU_SPEED 3580000 


byte Z80_RDMEM(uint16_t A);
void Z80_WRMEM(uint16_t A,byte V);
void Z80_GetRegs (Z80_Regs *Regs);
extern int Z80_IPeriod;
extern int Z80_ICount;
int IFreq = 100;
int CPUSpeed=500;


void measure_clock()
{
  Z80_Regs i;
  Z80_Reset();
  Z80_WRMEM (0x4000 ,0xc3); //c3
  Z80_WRMEM (0x4001 ,0x00);
  Z80_WRMEM (0x4002 ,0x40);
  i.R=0;
  i.PC.D=0x4000;
  Z80_SetRegs (&i);
  
  Z80_ICount=12;
  uint32_t tstates =0;
  uint32_t ts1 = micros();
  Z80_Execute();
  uint32_t ts2 = micros();
  tstates=Z80_ICount;
  Z80_GetRegs(&i);
  Serial.printf("clock frequency = %5.3f Mhz\n", ((float) tstates/(ts2-ts1)));
  
  Serial.printf("Measured time: %d\n",ts2-ts1);
  Serial.printf("T-States executed: %d\n",tstates);
  Serial.printf("Z80_Count: %d\n",Z80_ICount);
  }

void setup_cpuspeed() {
   
  Z80_IPeriod = 20000; //Z80_IPeriod=(3579545*CPUSpeed)/(IFreq*1000);;
  Z80_ICount =  60000;

}
