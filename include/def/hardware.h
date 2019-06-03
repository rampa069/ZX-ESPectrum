
//#define FABGL 1

#ifndef FABGL
 #define PINCONFIG pinConfig(-1, -1, -1, 2, 14,  -1, -1, -1, 15, 19,  -1, -1, 21, 27,  32, 33,  -1);
 #define KEYBOARD_DATA 25
 #define KEYBOARD_CLK 26
 #define SPEAKER_PIN 5
 #define EAR_PIN 34
 #define MIC_PIN 0

#else
 #define PINCONFIG pinConfig(-1, -1, -1, 21, 22,  -1, -1, -1, 18, 19,  -1, -1, 4, 5,  23, 15,  -1);
 #define KEYBOARD_DATA 32
 #define KEYBOARD_CLK 33
 #define SPEAKER_PIN 27
 #define EAR_PIN 35
 #define MIC_PIN 26
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
#define BLUE 0x20
#define RED 0x02
#define MAGENTA 0x22
#define GREEN 0x08
#define CYAN 0x28
#define YELLOW 0x0a
#define WHITE 0x2a
#endif
