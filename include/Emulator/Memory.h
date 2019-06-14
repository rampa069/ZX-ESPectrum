#include <Arduino.h>

extern byte *rom0;
extern byte *rom1;
extern byte *rom2;
extern byte *rom3;

extern byte *ram0;
extern byte *ram1;
extern byte *ram2;
extern byte *ram3;
extern byte *ram4;
extern byte *ram5;
extern byte *ram6;
extern byte *ram7;

extern volatile byte rom_latch, bank_latch, video_latch, paging_lock;
extern volatile byte sp3_mode, sp3_rom, rom_in_use;
extern volatile boolean writeScreen;
extern volatile byte ula_bus;
