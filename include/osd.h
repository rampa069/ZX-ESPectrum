// OSD Headers
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
// Rows
unsigned short rowCount(String menu);
String rowGet(String menu, unsigned short row_number);
// SNA Management
void changeSna(String sna_filename);
void setDemoMode(boolean on, unsigned short every);
// ULA
void stepULA();
