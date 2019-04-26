#pragma once
#pragma GCC diagnostic ignored "-Wall"
#include <Arduino.h>
#pragma GCC diagnostic warning "-Wall"

const byte KEY_F1 = 0x05;
const byte KEY_ESC = 0x76;

void kb_begin();
extern byte lastcode;
extern char keymap[];
extern char oldKeymap[];
