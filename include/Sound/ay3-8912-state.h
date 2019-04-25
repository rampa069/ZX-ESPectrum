#ifndef _AY3_8912_STATE_H
#define _AY3_8912_STATE_H

#include <stdint.h>

namespace Sound
{

class Ay3_8912_state
{
public:
	// Registers
	uint8_t finePitchChannelA;
	uint8_t coarsePitchChannelA;
	uint8_t finePitchChannelB;
	uint8_t coarsePitchChannelB;
	uint8_t finePitchChannelC;
	uint8_t coarsePitchChannelC;
	uint8_t noisePitch;
	uint8_t mixer;
	uint8_t volumeChannelA;
	uint8_t volumeChannelB;
	uint8_t volumeChannelC;
	uint8_t envelopeFineDuration;
	uint8_t envelopeCoarseDuration;
	uint8_t envelopeShape;
	uint8_t ioPortA;

	// Status
	uint8_t selectedRegister = 0;
	int8_t channelNote[3] = { 0, 0, 0 };
	int8_t channelVolume[3] = { 0, 0, 0 };

	void selectRegister(uint8_t registerNumber);
	void setRegisterData(uint8_t data);
	uint8_t getRegisterData();

private:
	void updated();
};

}

#endif
