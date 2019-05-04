#include "Arduino.h"
#include "msg.h"
#include "sdkconfig.h"
#include "esp_attr.h"

void errorHalt(String);
volatile byte *rom0;

volatile byte *rom1;

volatile byte *ram0 ;
volatile byte *ram1 ;
volatile byte *ram2 ;
volatile byte *ram3 ;
volatile byte *ram4 ;
volatile byte *ram5 ;
volatile byte *ram6 ;
volatile byte *ram7 ;

byte bank_latch=0;
byte video_latch=0;
byte rom_latch=0;
byte paging_lock=0;
