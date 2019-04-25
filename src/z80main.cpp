#include <string.h>
#include <stdio.h>
#include "startup.h"


#include "Emulator/z80main.h"
#include "Emulator/z80Input.h"
#include "Emulator/Keyboard/PS2Kbd.h"

//#define BEEPER
#define CYCLES_PER_STEP 69888
#define RAM_AVAILABLE 0xC000

Sound::Ay3_8912_state _ay3_8912;
uint8_t RamBuffer[RAM_AVAILABLE];
Z80_STATE _zxCpu;

extern byte *bank0;
extern byte specrom[16394];
extern byte borderTemp;
extern byte z80ports_in[128];


CONTEXT _zxContext;
static uint16_t _attributeCount;
int _total;
int _next_total = 0;
static uint8_t zx_data = 0;
static uint8_t frames = 0;
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
    memset(z80ports_in, 0xFF, 128);
    borderTemp=7;
    Z80Reset(&_zxCpu);
}

int32_t zx_loop()
{
    int32_t result = -1;

    _total += Z80Emulate(&_zxCpu, _next_total - _total, &_zxContext);

    if (_total >= _next_total)
    {
        _next_total += CYCLES_PER_STEP;

        Z80Interrupt(&_zxCpu, 0xff, &_zxContext);

        // delay
        //while (_spectrumScreen->_frames < _ticks)
        //{
        //}

		//_ticks = _spectrumScreen->_frames + 1;
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
    if (portLow == 0xFE)
    {
    	// Keyboard

        switch (portHigh)
        {
        case 0xFE:
        case 0xFD:
        case 0xFB:
        case 0xF7:
        case 0xEF:
        case 0xDF:
        case 0xBF:
        case 0x7F:
            return z80ports_in[portHigh - 0x7F];
        case 0x00:
			{
				uint8_t result = z80ports_in[0xFE - 0x7F];
				result &= z80ports_in[0xFD - 0x7F];
				result &= z80ports_in[0xFB - 0x7F];
				result &= z80ports_in[0xF7 - 0x7F];
				result &= z80ports_in[0xEF - 0x7F];
				result &= z80ports_in[0xDF - 0x7F];
				result &= z80ports_in[0xBF - 0x7F];
				result &= z80ports_in[0x7F - 0x7F];
				return result;
			}
        }
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
        uint8_t borderColor = (data & 0x07);
    	if ((z80ports_in[0x20] & 0x07) != borderColor)
    	{
            borderTemp=borderColor;
    	}

#ifdef BEEPER
        uint8_t sound = (data & 0x10);
    	if ((z80ports_in[0x20] & 0x10) != sound)
    	{
			if (sound)
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
			}
    	}
#endif

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
}
