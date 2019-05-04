#define SOUND_PIN 5

#define KEYBOARD_DATA 25
#define KEYBOARD_CLK 26

#define  COLOUR_8
//#define COLOUR_16

#ifdef COLOUR_8
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 32;
const int vsyncPin = 33;

#define BLACK 0x08
#define BLUE 0x0c
#define RED 0x09
#define MAGENTA 0x0d
#define GREEN 0x0a
#define CYAN 0x0e
#define YELLOW 0x0b
#define WHITE 0x0f

#endif

#ifdef COLOUR_16
const int redPins[] = {2, 4, 12, 13, 14};
const int greenPins[] = {15, 16, 17, 18, 19};
const int bluePins[] = {21, 22, 23, 27};
const int hsyncPin = 32;
const int vsyncPin = 33;

#define BLACK 0xc000
#define BLUE 0xf000
#define RED 0xc018
#define MAGENTA 0xf018
#define GREEN 0xc300
#define CYAN 0xf300
#define YELLOW 0xc318
#define WHITE 0xf318

#endif
