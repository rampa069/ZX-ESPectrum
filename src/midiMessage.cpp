#include "Sound/midiMessage.h"
#include "Arduino.h"

byte last_status;

void send_channel_message(byte command, byte channel, int num_data_bytes, byte *data) {
    // We are assuming that all of the inputs are in a valid range:
    //  Acceptable commands are 0x80 to 0xF0.
    // Channels must be between 1 and 16.
    // num_data_bytes should be either 1 or 2
    // data should be an array of 1 or 2 bytes, with each element constrained
    // to the 0x0 to 0x7f range.
    //
    // More realistic code might validate each input and return an error if out of range

    byte first;

    // Combine MS-nybble of command with channel nybble.
    first = (command & 0xf0) | ((channel - 1) & 0x0f);

    if (first != last_status) {
        Serial.write(first);
        last_status = first;
    }

    // Then send the right number of data bytes
    for (int i = 0; i < num_data_bytes; i++) {
        Serial.write(data[i]);
    }
}

void midiMessage(uint8_t message, uint8_t channel, uint8_t data1, uint8_t data2) {
    uint8_t data[2];
    data[0] = data1;
    data[1] = data2;

    // send_channel_message(message,channel,2,data);
    // Serial.printf("MSG: %u\n", data);
}
