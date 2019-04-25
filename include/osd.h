#include "msg.h"
#include <Arduino.h>
#include <Ressources/Font6x8.h>

const int OSD_X = 41;
const int OSD_Y = 28;
const int OSD_W = 278;
const int OSD_H = 144;
const int OSD_MARGIN = 4;
const int OSD_MAX_ROW = 16;
const int OSD_MAX_COL = 44;

extern void log(String text);

int row_pos = 0;
int col_pos = 0;
int menu_x = 0;
int menu_y = 0;
int menu_cols = 0;
int menu_rows = 0;
int menu_w = 0;
int menu_h = 0;

const char main_menu[4][20] = {"Main Menu", "Change ROM", "Change RAM", "Reset"};
