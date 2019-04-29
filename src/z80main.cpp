#include <string.h>
#include <stdio.h>
#include "startup.h"
#include "paledefs.h"


#include "Emulator/z80main.h"
#include "Emulator/z80Input.h"
#include "Emulator/Keyboard/PS2Kbd.h"

#define CYCLES_PER_STEP 69888//71600
#define RAM_AVAILABLE 0xC000

Sound::Ay3_8912_state _ay3_8912;
Z80_STATE _zxCpu;

extern byte *bank0;
extern byte specrom[16394];
extern byte borderTemp;
extern byte z80ports_in[128];
extern byte tick;
extern boolean writeScreen;


CONTEXT _zxContext;
static uint16_t _attributeCount;
int _total;
int _next_total = 0;
static uint8_t zx_data = 0;
static uint32_t frames = 0;
static uint32_t _ticks = 0;

extern "C"
{
    uint8_t readbyte(uint16_t addr);
    uint16_t readword(uint16_t addr);
    void writebyte(uint16_t addr, uint8_t data);
    void writeword(uint16_t addr, uint16_t data);
    uint8_t input(uint8_t portLow, uint8_t portHigh);
    void output(uint8_t portLow, uint8_t portHigh, uint8_t data);
}

void zx_setup()
{

    _zxContext.readbyte = readbyte;
    _zxContext.readword = readword;
    _zxContext.writeword = writeword;
    _zxContext.writebyte = writebyte;
    _zxContext.input = input;
    _zxContext.output = output;


    zx_reset();
}

void zx_reset()
{
    memset(z80ports_in, 0x1F, 128);
    borderTemp=7;
    Z80Reset(&_zxCpu);
}

int32_t zx_loop()
{
    int32_t result = -1;
    byte tmp_color =0;
    uint32_t ts1,ts2;



    ts1=millis();
    _total += Z80Emulate(&_zxCpu, _next_total - _total, &_zxContext);
    ts2=millis();


    if ((ts2-ts1) < 20)
      delay(20-(ts2-ts1));

      //while (tick==0)
       //delayMicroseconds(1);

    if (_total >= _next_total)
    {
        _next_total += CYCLES_PER_STEP;

        //while (tick==0)
         //delayMicroseconds(1);

        Z80Interrupt(&_zxCpu, 0xff, &_zxContext);

    }
    return result;
}

extern "C" uint8_t readbyte(uint16_t addr)
{
    uint8_t res;
    if (addr >= (uint16_t)0x4000)
    {
      res = bank0[addr-0x4000];
    }
        else
    {
        res = specrom[addr];
    }
    return res;
}

extern "C" uint16_t readword(uint16_t addr)
{
    return ((readbyte(addr + 1) << 8) | readbyte(addr));
}

extern "C" void writebyte(uint16_t addr, uint8_t data)
{
    if (addr >= (uint16_t)0x4000 && addr <= (uint16_t)0x7FFF)
    {
          while (writeScreen)
          {
            delayMicroseconds(1);
          }

            bank0[addr-0x4000] = data;
    }
    if (addr >= (uint16_t)0x4000)
    {
            bank0[addr-0x4000] = data;
    }
}

extern "C" void writeword(uint16_t addr, uint16_t data)
{
    writebyte(addr, (uint8_t)data);
    writebyte(addr + 1, (uint8_t)(data >> 8));
}

extern "C" uint8_t input(uint8_t portLow, uint8_t portHigh)
{
    int16_t kbdarrno = 0;
    if (portLow == 0xFE)
    {
    	// Keyboard

        switch (portHigh)
        {

          case 0xfe: kbdarrno = 0;break;
          case 0xfd: kbdarrno = 1;break;
          case 0xfb: kbdarrno = 2;break;
          case 0xf7: kbdarrno = 3;break;
          case 0xef: kbdarrno = 4;break;
          case 0xdf: kbdarrno = 5;break;
          case 0xbf: kbdarrno = 6;break;
          case 0x7f: kbdarrno = 7;break;


          case 0x00:
                        {
                                uint8_t result = z80ports_in[7];
                                result &= z80ports_in[6];
                                result &= z80ports_in[5];
                                result &= z80ports_in[4];
                                result &= z80ports_in[3];
                                result &= z80ports_in[2];
                                result &= z80ports_in[1];
                                result &= z80ports_in[0];
                                Serial.printf("Port 0 value: %x\n", result );
                                return  result;

                        }


        }
        return (z80ports_in[kbdarrno]);
			}


    // Sound (AY-3-8912)
    if (portLow == 0xFD)
    {
        switch (portHigh)
        {
        case 0xFF:
        	return _ay3_8912.getRegisterData();
        }
    }

    uint8_t data = zx_data;
    data |= (0xe0); /* Set bits 5-7 - as reset above */
    data &= ~0x40;
    return data;
}

extern "C" void output(uint8_t portLow, uint8_t portHigh, uint8_t data)
{
    switch (portLow)
    {
    case 0xFE:
    {
        // border color (no bright colors)
      bitWrite(borderTemp,0,bitRead(data,0));
      bitWrite(borderTemp,1,bitRead(data,1));
      bitWrite(borderTemp,2,bitRead(data,2));




			digitalWrite(SOUND_PIN,bitRead(data,4));  //speaker


      //digitalWrite(SOUND_PIN,bitRead(data,3));  //tape_out

      z80ports_in[0x20] = data;
    }
    break;

    case 0xFD:
    {
        // Sound (AY-3-8912)
        switch (portHigh)
        {
        case 0xFF:
        	_ay3_8912.selectRegister(data);
        case 0xBF:
        	_ay3_8912.setRegisterData(data);
        }
    }
    break;

    default:
        zx_data = data;
        break;
    }
    //Serial.printf("Port: %x High: %x Value: %x\n",portLow,portHigh,data );
}
