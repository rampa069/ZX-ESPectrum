#include "Disk.h"
#include "Emulator/Keyboard/PS2Kbd.h"
#include "ZX-ESPectrum.h"
#include "def/Font.h"
#include "def/files.h"
#include "def/msg.h"
#include "osd.h"

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

byte menuVirtualRows(String menu) { return (menuRowCount(menu) > MENU_MAX_ROWS ? MENU_MAX_ROWS : menuRowCount(menu)); }

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

void menuPrintRow(String line, byte cols) {
    vga.print(" ");
    vga.print(line.c_str());
    for (byte i = line.length(); i < (cols - 1); i++)
        vga.print(" ");
}

void drawMenu(String menu, byte focus, boolean new_draw) {
    Serial.printf("DRAW MENU focus:%u new_draw:%s\n", focus, (new_draw ? "true" : "false"));
    const byte cols = menuColMax(menu) + 2;
    Serial.printf("Max cols: %u\n", cols);
    const unsigned short real_rows = menuRowCount(menu);
    Serial.printf("Real rows: %u\n", real_rows);
    const byte rows = menuVirtualRows(menu);
    Serial.printf("Rows: %u\n", rows);
    const byte w = (cols * OSD_FONT_W) + 2;
    const byte h = (rows * OSD_FONT_H) + 2;
    const unsigned short x = scrAlignCenterX(w);
    const unsigned short y = scrAlignCenterY(h);
    Serial.printf("Menu x:%u, y:%u, w:%u, h:%u\n", x, y, w, h);

    if (new_draw) {
        delay(50);
        vga.setFont(Font6x8);
        // Menu border
        vga.rect(x, y, w, h, zxcolor(0, 0));
    }

    for (byte row = 0; row < rows; row++) {
        vga.setCursor(x + 1, y + 1 + (row * OSD_FONT_H));
        if (row == 0) {
            if (new_draw) {
                vga.setTextColor(zxcolor(7, 0), zxcolor(0, 0));
                menuPrintRow(menuGetRow(menu, row), cols);
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
            }
        } else if (row == focus) {
            vga.setTextColor(zxcolor(0, 1), zxcolor(5, 1));
            menuPrintRow(menuGetRow(menu, row), cols);
        } else {
            vga.setTextColor(zxcolor(0, 1), zxcolor(7, 1));
            menuPrintRow(menuGetRow(menu, row), cols);
        }
    }
}

String getArchMenu() {
    String menu = (String)MENU_ARCH + getFileEntriesFromDir(DISK_ROM_DIR);
    return menu;
}

String getRomsetMenu(String arch) {
    String menu = (String)MENU_ROMSET + getFileEntriesFromDir((String)DISK_ROM_DIR + "/" + arch);
    return menu;
}

unsigned short do_Menu(String menu) {
    const unsigned short real_rows = menuRowCount(menu);
    const byte rows = menuVirtualRows(menu);
    unsigned short focus = 1;
    unsigned short focus_new = 1;

    drawMenu(menu, focus, MENU_REDRAW);
    while (1) {
        if (checkAndCleanKey(KEY_CURSOR_UP)) {
            focus_new = focus - 1;
            if (focus_new < 1)
                focus_new = rows - 1;
        } else if (checkAndCleanKey(KEY_CURSOR_DOWN)) {
            focus_new = focus + 1;
            if (focus_new >= rows)
                focus_new = 1;
        } else if (checkAndCleanKey(KEY_ENTER)) {
            return focus;
        } else if (checkAndCleanKey(KEY_ESC) || checkAndCleanKey(KEY_F1)) {
            return 0;
        }

        if (focus_new != focus) {
            focus = focus_new;
            drawMenu(menu, focus, MENU_UPDATE);
        }
        delay(50);
    }
}
