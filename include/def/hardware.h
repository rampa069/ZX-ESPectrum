
/* Define the FABGL macro to use FABGL boards pinout. Keep it undefined
   to use bitluni VGA board v01
   Speaker and mic are in the PS2 mouse connections. ear on GPIO 35
*/

//#define FABGL 1

#ifndef FABGL
 #define PINCONFIG pinConfig(-1, -1, -1, 14, 2,  -1, -1, -1, 19, 15,  -1, -1, 27, 21,  32, 33,  -1);
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
#define BLUE 0xe0
#define RED 0xc2
#define MAGENTA 0xe2
#define GREEN 0xc8
#define CYAN 0xe8
#define YELLOW 0xca
#define WHITE 0xea
#endif