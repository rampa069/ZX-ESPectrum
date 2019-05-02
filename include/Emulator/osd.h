// OSD Headers
#pragma once

// Defines
#define MENU_REDRAW true
#define MENU_UPDATE false
#define OSD_ERROR true
#define OSD_NORMAL false
#define SCR_W 360
#define SCR_H 200
#define OSD_W 248
#define OSD_H 152
#define OSD_MARGIN 4
#define OSD_FONT_W 6
#define OSD_FONT_H 8
#define ASCII_NL 10
#define LEVEL_INFO 0
#define LEVEL_OK 1
#define LEVEL_WARN 2
#define LEVEL_ERROR 3

// Globals
boolean checkAndCleanKey(byte scancode);
boolean isKeymapChanged();
void updateKeymap();

// Ext var
extern boolean cfg_mode_sna;
extern String cfg_sna_file_list;
extern boolean xULAStop;
extern boolean xULAStopped;

// Ext method
void config_save();

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
            if (col_count > max) {
                max = col_count;
            }
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
    return "MENU ERROR! (Unknown row?)";
}

unsigned short menuPixelWidth(char *menu) { return (menuColMax(menu) * OSD_FONT_W) + 2; }
unsigned short menuPixelHeight(char *menu) { return (menuRowCount(menu) * OSD_FONT_H) + 2; }

// Stop ULA service
void stopULA() {
    xULAStop = true;
    while (!xULAStopped) {
        delay(5);
    }
}

// Start ULA service
void startULA() {
    xULAStop = false;
    while (xULAStopped) {
        delay(5);
    }
}

void stepULA() {
    startULA();
    stopULA();
}
