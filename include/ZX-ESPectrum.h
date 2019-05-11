#pragma once

#include "MartianVGA.h"

// Declared vars
#ifdef COLOUR_8
extern VGA3Bit vga;
#else
extern VGA14Bit vga;
#endif
extern byte borderTemp;
extern byte keymap[256];
extern byte oldKeymap[256];

// Declared methods
unsigned int zxcolor(int c, int bright);
void do_keyboard();
