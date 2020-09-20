#pragma once

#include "def/hardware.h"
#include "MartianVGA.h"

// Declared vars
#ifdef COLOR_3B
extern VGA3Bit vga;
#endif

#ifdef COLOR_6B
extern VGA6Bit vga;
#endif

#ifdef COLOR_14B
extern VGA14Bit vga;
#endif

extern byte borderTemp;
extern byte keymap[256];
extern byte oldKeymap[256];

// ULA Task
extern QueueHandle_t vidQueue;
extern TaskHandle_t videoTaskHandle;
extern volatile bool videoTaskIsRunning;

// Declared methods
unsigned int zxcolor(int c, int bright);
void do_keyboard();
