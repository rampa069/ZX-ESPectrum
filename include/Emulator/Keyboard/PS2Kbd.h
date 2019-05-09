#include "def/hardware.h"
#include "def/keys.h"
#include <Arduino.h>

#define KB_INT_START attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING)
#define KB_INT_STOP detachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK))

extern byte lastcode;

void IRAM_ATTR kb_interruptHandler(void);
void kb_begin();
boolean isKeymapChanged();
boolean checkAndCleanKey(byte scancode);
