// Defines
#define KB_INT_START attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING)
#define KB_INT_STOP detachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK))
#define DISK_BOOT_FILENAME "/boot.cfg"
#define DISK_ROM_DIR "/rom"
#define DISK_SNA_DIR "/sna"

// Headers
#include "Emulator/Keyboard/PS2Kbd.h"
#include "Emulator/machines.h"
#include "Emulator/msg.h"
#include "Emulator/z80emu/z80emu.h"
#include "paledefs.h"
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

// Extern vars
extern byte borderTemp;
extern Z80_STATE _zxCpu;
extern boolean cfg_slog_on;

// Globals

byte cfg_machine_type = MACHINE_ZX48;
String cfg_ram_file = "noram";
String cfg_rom_file = "0.rom";
String cfg_rom_set = "ZX";
String cfg_sna_file_list;
boolean cfg_mode_sna = false;
boolean cfg_debug_on = false;
boolean cfg_slog_on = true;

// External methods
void errorHalt(String errormsg);
void IRAM_ATTR kb_interruptHandler(void);
void zx_reset();
void IRAM_ATTR load_ram_128(String sna_file);


// Types
typedef int32_t dword;
typedef signed char offset;
