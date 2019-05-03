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
byte row_pos = 0;
byte col_pos = 0;

// Ext var
extern boolean cfg_mode_sna;
extern String cfg_sna_file_list;
extern boolean xULAStop;
extern boolean xULAStopped;

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
// Error
void errorPanel(String errormsg);
void errorHalt(String errormsg);
void osdCenteredMsg(String msg, byte warn_level);
// Menu
unsigned short menuRowCount(String menu);
byte menuVirtualRows(String menu);
byte menuColMax(String menu);
String menuGetRow(String menu, unsigned short row);
unsigned short menuPixelWidth(char *menu);
unsigned short menuPixelHeight(char *menu);
void menuPrintRow(String line, byte cols);
void drawMenu(String menu, byte focus, boolean new_draw);
unsigned short do_Menu(String menu);
// SNA Management
void changeSna(String sna_filename);
// ULA Management
void stopULA();
void startULA();
void stepULA();

// Ext method
void config_save();

// Include rest of OSD code
#include "OSD/SNA_mng.cpp"
#include "OSD/ULA_mng.cpp"
#include "OSD/calc.cpp"
#include "OSD/error.cpp"
#include "OSD/menu.cpp"
#include "OSD/osd.cpp"
