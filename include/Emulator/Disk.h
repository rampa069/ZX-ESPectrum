// Defines
#define KB_INT_START attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING)
#define KB_INT_STOP detachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK))
#define ASCII_NL 10

// Headers
#include "Emulator/Keyboard/PS2Kbd.h"
#include "Emulator/z80emu/z80emu.h"
#include "dirdefs.h"
#include "machinedefs.h"
#include "msg.h"
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

// Extern vars
extern byte borderTemp;
extern Z80_STATE _zxCpu;
extern boolean cfg_slog_on;

// Globals
String cfg_arch = "128K";
String cfg_ram_file = NO_RAM_FILE;
String cfg_rom_set = "SINCLAIR";
String cfg_sna_file_list;
boolean cfg_slog_on = true;

// External methods
void errorHalt(String errormsg);
void IRAM_ATTR kb_interruptHandler(void);
void zx_reset();
void IRAM_ATTR load_ram_128(String sna_file);

// Types
typedef int32_t dword;
typedef signed char offset;
