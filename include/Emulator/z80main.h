#ifndef __ZXMAIN_INCLUDED__
#define __ZXMAIN_INCLUDED__

#include "Sound/ay3-8912-state.h"
#include "z80user.h"

extern Sound_AY::Ay3_8912_state _ay3_8912;
extern Z80_STATE _zxCpu;

void zx_setup();
int32_t zx_loop();
void zx_reset();

extern "C" uint8_t readbyte(uint16_t addr);
extern "C" void writebyte(uint16_t addr, uint8_t data);
extern "C" uint16_t readword(uint16_t addr);
extern "C" void writeword(uint16_t addr, uint16_t data);


#endif
