#pragma once
#include <Arduino.h>

void kb_begin();
extern byte lastcode;
extern char keymap[256];
extern char oldKeymap[256];
