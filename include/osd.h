// OSD Headers
<<<<<<< HEAD
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
extern String getDirAsMenu(String title, String path);
extern void listAllFiles();

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
=======
#include <Arduino.h>

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
#define LEVEL_INFO 0
#define LEVEL_OK 1
#define LEVEL_WARN 2
#define LEVEL_ERROR 3
#define ON true
#define OFF false
#define MENU_MAX_ROWS 23
// Line type
#define IS_TITLE 0
#define IS_FOCUSED 1
#define IS_NORMAL 2
// Scroll
#define UP true
#define DOWN false
// Sound
#define SND_CLICK_DURATION 50
#define SND_CLICK_SPACE 5

// OSD Interface
// Calc
unsigned short scrAlignCenterX(unsigned short pixel_width);
unsigned short scrAlignCenterY(unsigned short pixel_height);
byte osdMaxRows();
byte osdMaxCols();
unsigned short osdInsideX();
unsigned short osdInsideY();
// OSD
void osdHome();
void osdAt(byte row, byte col);
void drawOSD();
void do_OSD();
// Error
void errorPanel(String errormsg);
void errorHalt(String errormsg);
void osdCenteredMsg(String msg, byte warn_level);
// Menu
void newMenu(String new_menu);
void menuRecalc();
unsigned short menuRealRowFor(byte virtual_row_num);
void menuPrintRow(byte virtual_row_num, byte line_type);
void menuDraw();
void menuRedraw();
String getArchMenu();
String getRomsetMenu(String arch);
unsigned short menuRun(String new_menu);
void menuScroll(boolean up);
void menuAt(short int row, short int col);
void menuScrollBar();
String getTestMenu(unsigned short n_lines);
// Rows
unsigned short rowCount(String menu);
String rowGet(String menu, unsigned short row_number);
// SNA Management
void changeSna(String sna_filename);
void setDemoMode(boolean on, unsigned short every);
// ULA
void stepULA();
>>>>>>> osddev
