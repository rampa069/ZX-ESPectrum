#ifndef _Z80INPUT_H_
#define _Z80INPUT_H_

#include <stdint.h>

extern uint8_t indata[];

enum {
	ZX_KEY_SHIFT = 0,
	ZX_KEY_Z,
	ZX_KEY_X,
	ZX_KEY_C,
	ZX_KEY_V,

	ZX_KEY_A,
	ZX_KEY_S,
	ZX_KEY_D,
	ZX_KEY_F,
	ZX_KEY_G,

	ZX_KEY_Q,
	ZX_KEY_W,
	ZX_KEY_E,
	ZX_KEY_R,
	ZX_KEY_T,

	ZX_KEY_1,
	ZX_KEY_2,
	ZX_KEY_3,
	ZX_KEY_4,
	ZX_KEY_5,

	ZX_KEY_0,
	ZX_KEY_9,
	ZX_KEY_8,
	ZX_KEY_7,
	ZX_KEY_6,

	ZX_KEY_P,
	ZX_KEY_O,
	ZX_KEY_I,
	ZX_KEY_U,
	ZX_KEY_Y,

	ZX_KEY_ENTER,
	ZX_KEY_L,
	ZX_KEY_K,
	ZX_KEY_J,
	ZX_KEY_H,

	ZX_KEY_SPACE,
	ZX_KEY_SYM,
	ZX_KEY_M,
	ZX_KEY_N,
	ZX_KEY_B,

	ZX_KEY_LAST
};

extern const uint8_t keyaddr[ZX_KEY_LAST];
extern const uint8_t keybuf[ZX_KEY_LAST];

bool OnKey(uint32_t scanCode, bool isKeyUp);

#endif
