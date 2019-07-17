/* Define the FABGL macro to use FABGL boards pinout.
      Speaker and AY sound ports  are in the PS2 mouse connections.
   Define the BITLUNI macro to use bitluni VGA board v01
   Define the MARTIANOIDS macro to use the martianoids V1.0 board
*/

//#define FABGL 1
//#define BITLUNI 1
#define MARTIANOIDS 1


/*
   Define ZX2PS2 to anything to use the ZX2PS2 zx matrix to ps2 converter.
   it only changes the cursor keys to the zx spectrum 5678 and 0 keys.
   for F1 press SS+CS 1
*/

//#define ZX2PS2 1

#ifdef BITLUNI
 #define PINCONFIG pinConfig(-1, -1, -1, 2, 14,  -1, -1, -1, 15, 19,  -1, -1, 21, 27,  32, 33,  -1);
 #define KEYBOARD_DATA 25
 #define KEYBOARD_CLK 26
 #define SPEAKER_PIN 5
 #define EAR_PIN 34
 #define MIC_PIN 0
 #define AY_PIN -1
 #define AY_SOUND 0
#endif
#ifdef FABGL
 #define PINCONFIG pinConfig(-1, -1, -1, 21, 22,  -1, -1, -1, 18, 19,  -1, -1, 4, 5,  23, 15,  -1);
 #define KEYBOARD_DATA 32
 #define KEYBOARD_CLK 33
 #define SPEAKER_PIN 27
 #define EAR_PIN -1
 #define MIC_PIN -1
 #define AY_PIN 26
 #define AY_SOUND 1
 #undef BOARD_HAS_PSRAM
#endif
#ifdef MARTIANOIDS
 #define PINCONFIG pinConfig(-1, -1, -1, 16, 17,  -1, -1, -1, 4, 2,  -1, -1, 15, 13,  22, 21,  -1);
 #define KEYBOARD_DATA 32
 #define KEYBOARD_CLK 33
 #define SPEAKER_PIN 27
 #define EAR_PIN -1
 #define MIC_PIN -1
 #define AY_PIN 26
 #define AY_SOUND 1
 //#define BOARD_HAS_PSRAM 1
 #define HAS_JOYSTICK 1
 #define JOY_UP   8
 #define JOY_DOWN 7
 #define JOY_LEFT 6
 #define JOY_RIGHT 11
 #define JOY_FIRE 10
 #define JOY_0 14
 #define JOY_1 12
#endif


//#define COLOUR_8
#define COLOUR_16


#ifdef COLOUR_8
#define BLACK 0x08
#define BLUE 0x0c
#define RED 0x09
#define MAGENTA 0x0d
#define GREEN 0x0a
#define CYAN 0x0e
#define YELLOW 0x0b
#define WHITE 0x0f
#else
#define BLACK 0xc0
#define BLUE 0xe0
#define RED 0xc2
#define MAGENTA 0xe2
#define GREEN 0xc8
#define CYAN 0xe8
#define YELLOW 0xca
#define WHITE 0xea
#endif
