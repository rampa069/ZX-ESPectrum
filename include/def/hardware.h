#define SPEAKER_PIN 27
#define EAR_PIN 34
#define MIC_PIN 0

#define KEYBOARD_DATA 25
#define KEYBOARD_CLK 26

#define COLOUR_8
//#define COLOUR_16

// 8bit pins
#define RED_PIN 13
#define GREEN_PIN 12
#define BLUE_PIN 14

// 16b pins
#define RED_PINS 2, 2, 14, 14, 14
#define GREEN_PINS 15, 15, 19, 19, 19
#define BLUE_PINS 21, 21, 27, 27

// VGA sync pins
#define HSYNC_PIN 32
#define VSYNC_PIN 33

#ifdef COLOUR_8           //       BGR 
#define BLACK   0x08      // 0000 1000
#define BLUE    0x0c      // 0000 1100
#define RED     0x09      // 0000 1001
#define MAGENTA 0x0d      // 0000 1101
#define GREEN   0x0a      // 0000 1010
#define CYAN    0x0e      // 0000 1110
#define YELLOW  0x0b      // 0000 1011
#define WHITE   0x0f      // 0000 1111
#else                     //   BB --GG ---R R---
#define BRI_BLACK   0xC000    // 1100 0000 0000 0000
#define BRI_BLUE    0xF000    // 1111 0000 0000 0000
#define BRI_RED     0xC018    // 1100 0000 0001 1000
#define BRI_MAGENTA 0xF018    // 1111 0000 0001 1000
#define BRI_GREEN   0xC300    // 1100 0011 0000 0000
#define BRI_CYAN    0xF300    // 1111 0011 0000 0000
#define BRI_YELLOW  0xC318    // 1100 0011 0001 1000
#define BRI_WHITE   0xF318    // 1111 0011 0001 1000
                              //   BB --GG ---R R---
#define BLACK       0xC000    // 1100 0000 0000 0000
#define BLUE        0xE000    // 1110 0000 0000 0000
#define RED         0xC010    // 1100 0000 0001 0000
#define MAGENTA     0xE010    // 1110 0000 0001 0000
#define GREEN       0xC200    // 1100 0010 0000 0000
#define CYAN        0xE200    // 1110 0010 0000 0000
#define YELLOW      0xC210    // 1100 0010 0001 0000
#define WHITE       0xE210    // 1110 0010 0001 0000

#endif
