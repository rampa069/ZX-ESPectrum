#ifdef VGA32V14

#define SPEAKER_PIN 25
#define EAR_PIN 34
#define MIC_PIN 39

#define KEYBOARD_DATA 32
#define KEYBOARD_CLK 33

#define COLOUR_16

// 16b pins
#define RED_PINS 21, 21, 22, 22, 22
#define GREEN_PINS 18, 18, 19, 19, 19
#define BLUE_PINS 4, 4, 5, 5

// VGA sync pins
#define HSYNC_PIN 23
#define VSYNC_PIN 15

#else

#define SPEAKER_PIN 5
#define EAR_PIN 34
#define MIC_PIN 0

#define KEYBOARD_DATA 25
#define KEYBOARD_CLK 26

// #define COLOUR_8
#define COLOUR_16

// 8bit pins
#define RED_PIN 14
#define GREEN_PIN 19
#define BLUE_PIN 27

// 16b pins
#define RED_PINS 2, 2, 14, 14, 14
#define GREEN_PINS 15, 15, 19, 19, 19
#define BLUE_PINS 21, 21, 27, 27

// VGA sync pins
#define HSYNC_PIN 32
#define VSYNC_PIN 33


#endif

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
#define BLACK 0xc000
#define BLUE 0xf000
#define RED 0xc018
#define MAGENTA 0xf018
#define GREEN 0xc300
#define CYAN 0xf300
#define YELLOW 0xc318
#define WHITE 0xf318
#endif
