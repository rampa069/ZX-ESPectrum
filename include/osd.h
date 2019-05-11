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
#define MENU_MAX_ROWS 24

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
// Menu row count
unsigned short menuRowCount(String menu);
byte menuVirtualRows(String menu);
byte menuColMax(String menu);
String menuGetRow(String menu, unsigned short row);
unsigned short menuPixelWidth(char *menu);
unsigned short menuPixelHeight(char *menu);
void menuPrintRow(String line, byte cols);
void drawMenu(String menu, byte focus, boolean new_draw);
String getArchMenu();
String getRomsetMenu(String arch);
unsigned short do_Menu(String menu);
// SNA Management
void changeSna(String sna_filename);
void setDemoMode(boolean on, unsigned short every);
// ULA Management
void stopULA();
void startULA();
void stepULA();
