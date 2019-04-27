#pragma once
#pragma GCC diagnostic ignored "-Wall"
#include <Arduino.h>
#pragma GCC diagnostic warning "-Wall"

const byte KEY_F1 = 0x05;
const byte KEY_ESC = 0x76;
const byte KEY_DOWN = 0x72;
const byte KEY_UP = 0x75;
const byte KEY_ENTER = 0x5A;

void kb_begin();
extern byte lastcode;
extern char keymap[];
extern char oldKeymap[];
