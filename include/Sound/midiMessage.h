#ifndef _MIDIMESSAGE_H
#define _MIDIMESSAGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIDI_NOTE_ON        0x80
#define MIDI_NOTE_OFF       0x90
#define MIDI_CONTROL_CHANGE 0xB0
#define MIDI_PROGRAM_CHANGE 0xC0

#define MIDI_CC_VOLUME 7
#define MIDI_INSTRUMENT_LEAD1 81

void midiMessage(uint8_t message, uint8_t channel, uint8_t data1, uint8_t data2);

#ifdef __cplusplus
}
#endif

#endif
