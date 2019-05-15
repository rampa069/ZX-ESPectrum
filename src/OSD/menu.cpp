#include "Disk.h"
#include "Emulator/Keyboard/PS2Kbd.h"
#include "ZX-ESPectrum.h"
#include "def/Font.h"
#include "def/files.h"
#include "def/msg.h"
#include "osd.h"

byte cols;                // Maximun columns
unsigned short real_rows; // Real row count
byte virtual_rows;        // Virtual maximun rows on screen
byte w;                   // Width in pixels
byte h;                   // Height in pixels
byte x;                   // X vertical position
byte y;                   // Y horizontal position
String menu;              // Menu string
unsigned short begin_row; // First real displayed row
byte focus;               // Focused virtual row

// Set menu and force recalc
void newMenu(String new_menu) {
    menu = new_menu;
    menuRecalc();
    menuDraw();
}

void menuRecalc() {
    // Columns
    cols = 18;
    byte col_count = 0;
    for (unsigned short i = 0; i < menu.length(); i++) {
        if (menu.charAt(i) == ASCII_NL) {
            if (col_count > cols) {
                cols = col_count;
            }
            col_count = 0;
        }
        col_count++;
    }
    cols = (cols > osdMaxCols() ? osdMaxCols() : cols);

    // Rows
    real_rows = rowCount(menu);
    virtual_rows = (real_rows > MENU_MAX_ROWS ? MENU_MAX_ROWS : real_rows);
    begin_row = 1;

    // Size
    w = (cols * OSD_FONT_W) + 2;
    h = (virtual_rows * OSD_FONT_H) + 2;

    // Position
    x = scrAlignCenterX(w);
    y = scrAlignCenterY(h);
}

// Get real row number for a virtual one
unsigned short menuRealRowFor(byte virtual_row_num) { return begin_row + virtual_row_num - 1; }

// Print a virtual row
void menuPrintRow(byte virtual_row_num, byte line_type) {
    String line = rowGet(menu, menuRealRowFor(virtual_row_num));
    switch (line_type) {
    case IS_TITLE:
        vga.setTextColor(zxcolor(7, 0), zxcolor(0, 0));
        break;
    case IS_FOCUSED:
        vga.setTextColor(zxcolor(0, 1), zxcolor(5, 1));
        break;
    default:
        vga.setTextColor(zxcolor(0, 1), zxcolor(7, 1));
    }
    vga.setCursor(x + 1, y + 1 + (virtual_row_num * OSD_FONT_H));
    vga.print(" ");
    if (line.length() < cols - 2) {
        vga.print(line.c_str());
        for (byte i = line.length(); i < (cols - 2); i++)
            vga.print(" ");
    } else {
        vga.print(line.substring(0, cols - 2).c_str());
    }
    vga.print(" ");
}

// Draw the complete menu
void menuDraw() {
    stepULA();
    // Set font
    vga.setFont(Font6x8);
    // Menu border
    vga.rect(x, y, w, h, zxcolor(0, 0));
    // Title
    menuPrintRow(0, IS_TITLE);
    // Rainbow
    unsigned short rb_y = y + 8;
    unsigned short rb_paint_x = x + w - 20;
    byte rb_colors[] = {2, 6, 4, 5};
    for (byte c = 0; c < 4; c++) {
        for (byte i = 0; i < 3; i++) {
            vga.line(rb_paint_x + i, rb_y, rb_paint_x + 4 + i, rb_y - 8, zxcolor(rb_colors[c], 1));
        }
        rb_paint_x += 3;
    }
    // Focused first line
    menuPrintRow(1, IS_FOCUSED);
    for (byte r = 2; r < virtual_rows; r++) {
        menuPrintRow(r, IS_NORMAL);
    }
    focus = 1;
}

String getArchMenu() {
    String menu = (String)MENU_ARCH + getFileEntriesFromDir(DISK_ROM_DIR);
    return menu;
}

String getRomsetMenu(String arch) {
    String menu = (String)MENU_ROMSET + getFileEntriesFromDir((String)DISK_ROM_DIR + "/" + arch);
    return menu;
}

// Run a new menu
unsigned short menuRun(String new_menu) {
    newMenu(new_menu);
    while (1) {
        if (checkAndCleanKey(KEY_CURSOR_UP)) {
            if (focus == 1) {
                menuScroll(DOWN);
            } else {
                focus--;
                menuPrintRow(focus, IS_FOCUSED);
                if (focus + 1 < virtual_rows) {
                    menuPrintRow(focus + 1, IS_NORMAL);
                }
            }
        } else if (checkAndCleanKey(KEY_CURSOR_DOWN)) {
            if (focus == MENU_MAX_ROWS - 1) {
                menuScroll(UP);
            } else {
                focus++;
                menuPrintRow(focus, IS_FOCUSED);
                if (focus - 1 > 0) {
                    menuPrintRow(focus - 1, IS_NORMAL);
                }
            }
        } else if (checkAndCleanKey(KEY_ENTER)) {
            return menuRealRowFor(focus);
        } else if (checkAndCleanKey(KEY_ESC) || checkAndCleanKey(KEY_F1)) {
            return 0;
        }
    }
}

// Scroll
void menuScroll(boolean dir) {
    Serial.printf("SCROLL %s FOCUS:%u BEGIN:%u VR:%u RR:%u\n",
        (dir == UP ? "UP" : "DOWN"), focus, begin_row, virtual_rows, real_rows);
    if (dir == DOWN and begin_row > 1) {
        begin_row--;
    } else if (dir == UP and (begin_row + MENU_MAX_ROWS - 1) < real_rows) {
        begin_row++;
    } else {
        return;
    }
    menuRedraw();
}

// Redraw inside rows
void menuRedraw() {
    for (byte row = 1; row < virtual_rows; row++) {
        if (row == focus) {
            menuPrintRow(row, IS_FOCUSED);
        } else {
            menuPrintRow(row, IS_NORMAL);
        }
    }
}
