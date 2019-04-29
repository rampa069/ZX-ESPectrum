// OSD Headers

#include <FS.h>

#define MENU_REDRAW true
#define MENU_UPDATE false

const unsigned short SCR_W = 360; // X
const unsigned short SCR_H = 200; // Y
const unsigned short OSD_W = 248; // X
const unsigned short OSD_H = 152; // Y
const byte OSD_MARGIN = 4;
const byte OSD_FONT_W = 6;
const byte OSD_FONT_H = 8;
const char ASCII_NL = 10;

extern boolean checkAndCleanKey(byte scancode);
extern boolean isKeymapChanged();
extern void updateKeymap();
extern File open_read_file(String filename);

const String main_menu = "Main Menu\nChange ROM\nChange RAM\nReset\nReturn\n";
const String reset_menu = "Reset Menu\nSoft reset\nHard reset\nCancel\n";

// X origin to center an element with pixel_width
unsigned short scrAlignCenterX(unsigned short pixel_width) { return (SCR_W / 2) - (pixel_width / 2); }
// Y origin to center an element with pixel_height
unsigned short scrAlignCenterY(unsigned short pixel_height) { return (SCR_H / 2) - (pixel_height / 2); }

byte osdMaxRows() { return (OSD_H - (OSD_MARGIN * 2)) / OSD_FONT_H; }
byte osdMaxCols() { return (OSD_W - (OSD_MARGIN * 2)) / OSD_FONT_W; }
unsigned short osdInsideX() { return scrAlignCenterX(OSD_W) + OSD_MARGIN; }
unsigned short osdInsideY() { return scrAlignCenterY(OSD_H) + OSD_MARGIN; }

// Menu row count
unsigned short menuRowCount(String menu) {
    unsigned short count = 0;
    for (unsigned short i = 0; i < menu.length(); i++) {
        if (menu.charAt(i) == ASCII_NL) {
            count++;
        }
    }
    return count;
}

byte menuVirtualRows(String menu) {
    unsigned short rows = menuRowCount(menu);
    if (rows > 25) {
        return 25;
    }
    return rows;
}

// Menu columns
byte menuColMax(String menu) {
    byte max = 18;
    unsigned short col_count = 0;
    for (unsigned short i = 0; i < menu.length(); i++) {
        if (menu.charAt(i) == ASCII_NL) {
            if (col_count > max)
                max = col_count;
            col_count = 0;
        }
        col_count++;
    }
    return max;
}

// Menu get a row
String menuGetRow(String menu, unsigned short row) {
    unsigned short count = 0;
    unsigned short last = 0;
    for (unsigned short i = 0; i < menu.length(); i++) {
        if (menu.charAt(i) == ASCII_NL) {
            if (count == row) {
                return menu.substring(last, i);
            }
            count++;
            last = i + 1;
        }
    }
    return "MENU ERROR!";
}

unsigned short menuPixelWidth(char *menu) { return (menuColMax(menu) * OSD_FONT_W) + 2; }
unsigned short menuPixelHeight(char *menu) { return (menuRowCount(menu) * OSD_FONT_H) + 2; }
