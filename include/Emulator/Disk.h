// Defines
#define KB_INT_START attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING)
#define KB_INT_STOP detachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK))

// Headers
#include "Emulator/Keyboard/PS2Kbd.h"
#include "Emulator/machines.h"
#include "Emulator/z80emu/z80emu.h"
#include "dirdefs.h"
#include "machinedefs.h"
#include "msg.h"
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

// Extern vars
extern byte *bank0;
extern byte borderTemp;
extern Z80_STATE _zxCpu;
extern boolean cfg_slog_on;

// Globals
byte specrom[16384];

// Config
byte cfg_machine_type = MACHINE_ZX48;
String cfg_ram_file = NO_RAM_FILE;
String cfg_rom_file = "0.rom";
String cfg_rom_set = "ZX";
String cfg_sna_file_list;
boolean cfg_slog_on = true;
boolean cfg_demo_on = false;
unsigned short cfg_demo_every = 300000;

// External methods
void errorHalt(String errormsg);
void IRAM_ATTR kb_interruptHandler(void);
void zx_reset();

// Types
typedef int32_t dword;
typedef signed char offset;
