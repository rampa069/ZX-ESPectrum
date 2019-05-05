//extern byte *rom0;

extern volatile byte *rom0;
extern volatile byte *rom1;
extern volatile byte *rom2;
extern volatile byte *rom3;

extern volatile byte* ram0;
extern volatile byte* ram1;
extern volatile byte* ram2;
extern volatile byte* ram3;
extern volatile byte* ram4;
extern volatile byte* ram5;
extern volatile byte* ram6;
extern volatile byte* ram7;

extern byte rom_latch,bank_latch,video_latch,paging_lock;
extern byte sp3_mode, sp3_rom;
void alloc_memory();
