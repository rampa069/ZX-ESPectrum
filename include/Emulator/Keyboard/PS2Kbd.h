#pragma once
#pragma GCC diagnostic ignored "-Wall"
#include <Arduino.h>
#pragma GCC diagnostic warning "-Wall"

#define KEY_F1 0x05
#define KEY_F12 0x07
#define KEY_ESC 0x76
#define KEY_CURSOR_LEFT 0x6B
#define KEY_CURSOR_DOWN 0x72
#define KEY_CURSOR_RIGHT 0x74
#define KEY_CURSOR_UP 0x75
#define KEY_ALT_GR 0x11
#define KEY_ENTER 0x5A

void kb_begin();
extern byte lastcode;
