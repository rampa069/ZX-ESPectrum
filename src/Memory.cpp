#include "Arduino.h"
#include "msg.h"
#include "sdkconfig.h"
#include "esp_attr.h"

void errorHalt(String);



byte *rom0;
byte *rom1;
byte *rom2;
byte *rom3;

byte *ram0;
byte *ram1;
byte *ram2;
byte *ram3;
byte *ram4;
byte *ram5;
byte *ram6;
byte *ram7;

volatile byte bank_latch=0;
volatile byte video_latch=0;
volatile byte rom_latch=0;
volatile byte paging_lock=0;
volatile byte sp3_mode=0;
volatile byte sp3_rom=0;
volatile byte rom_in_use =0;
