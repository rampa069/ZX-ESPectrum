#include "ay3-8912-state.h"
#include "pitchtonote.h"
#include "volume.h"
#include "midimessage.h"

namespace Sound
{

void Ay3_8912_state::updated()
{
	int8_t oldChannelNote[3];
	int8_t oldChannelVolume[3];

	for (int8_t channel = 0; channel < 3; channel++)
	{
		oldChannelNote[channel] = this->channelNote[channel];
		oldChannelVolume[channel] = this->channelVolume[channel];
	}

	int16_t pitch;

	switch (this->selectedRegister)
	{
	case 0:
	case 1:
		pitch = ((this->coarsePitchChannelA << 8) + this->finePitchChannelA) & 0x0FFF;
		this->channelNote[0] = pitchToNote[pitch];
		break;
	case 2:
	case 3:
		pitch = ((this->coarsePitchChannelB << 8) + this->finePitchChannelB) & 0x0FFF;
		this->channelNote[1] = pitchToNote[pitch];
		break;
	case 4:
	case 5:
		pitch = ((this->coarsePitchChannelC << 8) + this->finePitchChannelC) & 0x0FFF;
		this->channelNote[2] = pitchToNote[pitch];
		break;
	case 6:
		// noisePitch - ignored for now
		break;
	case 7:
		this->channelVolume[0] = (this->mixer & 0x01) ? 0 : volume[this->volumeChannelA & 0x0F];
		this->channelVolume[1] = (this->mixer & 0x02) ? 0 : volume[this->volumeChannelB & 0x0F];
		this->channelVolume[2] = (this->mixer & 0x04) ? 0 : volume[this->volumeChannelC & 0x0F];
		break;
	case 8:
		this->channelVolume[0] = (this->mixer & 0x01) ? 0 : volume[this->volumeChannelA & 0x0F];
		break;
	case 9:
		this->channelVolume[1] = (this->mixer & 0x02) ? 0 : volume[this->volumeChannelB & 0x0F];
		break;
	case 10:
		this->channelVolume[2] = (this->mixer & 0x04) ? 0 : volume[this->volumeChannelC & 0x0F];
		break;
	case 11:
		// envelopeFineDuration - ignored for now
		break;
	case 12:
		// envelopeCoarseDuration - ignored for now
		break;
	case 13:
		// envelopeShape - ignored for now
		break;
	case 14:
		// ioPortA - ignored for now
		break;
	}

	for (int8_t channel = 0; channel < 3; channel++)
	{
		if (this->channelVolume[channel] != oldChannelVolume[channel])
		{
			if (this->channelVolume[channel] == 0)
			{
				midiMessage(MIDI_NOTE_OFF, 0, oldChannelNote[channel], 0);
			}
			else
			{
				midiMessage(MIDI_PROGRAM_CHANGE, channel, MIDI_INSTRUMENT_LEAD1, 0);
				midiMessage(MIDI_CONTROL_CHANGE, channel, MIDI_CC_VOLUME, this->channelVolume[channel]);
			}
		}

		if (this->channelVolume[channel] == 0)
		{
			continue;
		}

		if (this->channelNote[channel] != oldChannelNote[channel])
		{
			midiMessage(MIDI_NOTE_ON, 0, this->channelNote[channel], 0);
		}
	}
}

void Ay3_8912_state::selectRegister(uint8_t registerNumber)
{
	this->selectedRegister = registerNumber;
}

void Ay3_8912_state::setRegisterData(uint8_t data)
{
	switch (this->selectedRegister)
	{
	case 0:
		this->finePitchChannelA = data;
		break;
	case 1:
		this->coarsePitchChannelA = data;
		break;
	case 2:
		this->finePitchChannelB = data;
		break;
	case 3:
		this->coarsePitchChannelB = data;
		break;
	case 4:
		this->finePitchChannelC = data;
		break;
	case 5:
		this->coarsePitchChannelC = data;
		break;
	case 6:
		this->noisePitch = data;
		break;
	case 7:
		this->mixer = data;
		break;
	case 8:
		this->volumeChannelA = data;
		break;
	case 9:
		this->volumeChannelB = data;
		break;
	case 10:
		this->volumeChannelC = data;
		break;
	case 11:
		this->envelopeFineDuration = data;
		break;
	case 12:
		this->envelopeCoarseDuration = data;
		break;
	case 13:
		this->envelopeShape = data;
		break;
	case 14:
		this->ioPortA = data;
		break;
	default:
		// invalid register - do nothing
		return;
	}

	this->updated();
}

uint8_t Ay3_8912_state::getRegisterData()
{
	switch (this->selectedRegister)
	{
	case 0:
		return this->finePitchChannelA;
	case 1:
		return this->coarsePitchChannelA;
	case 2:
		return this->finePitchChannelB;
	case 3:
		return this->coarsePitchChannelB;
	case 4:
		return this->finePitchChannelC;
	case 5:
		return this->coarsePitchChannelC;
	case 6:
		return this->noisePitch;
	case 7:
		return this->mixer;
	case 8:
		return this->volumeChannelA;
	case 9:
		return this->volumeChannelB;
	case 10:
		return this->volumeChannelC;
	case 11:
		return this->envelopeFineDuration;
	case 12:
		return this->envelopeCoarseDuration;
	case 13:
		return this->envelopeShape;
	case 14:
		return this->ioPortA;
	default:
		return 0;
	}
}

}
