// On Screen Display
#include "osd.h";

unsigned short int osdX(unsigned short int rel_x) { return OSD_X + OSD_MARGIN + rel_x; }

unsigned short int osdY(unsigned short int rel_y) { return OSD_Y + OSD_MARGIN + rel_y; }

void osdHome() { vga.setCursor(osdX(0), osdY(0)); }

// Text 17x45 with 6x8 font
void osdAt(int row, int col) {
    if (row > OSD_MAX_ROW)
        row = 0;
    if (col > OSD_MAX_COL)
        col = 0;
    vga.setCursor(osdX(col * 6), osdY(row * 8));
    row_pos = row;
    col_pos = col;
}

void osdCharSq(int lines, int cols) {
    const int start_row = row_pos;
    const int start_col = col_pos;
    for (int l = 1; l <= lines; l++) {
        for (int c = 1; c <= cols; c++) {
            vga.print(" ");
        }
        osdAt(row_pos + 1, start_col);
    }
    osdAt(start_row, start_col);
}

void do_OSD() {
    if (keymap != oldKeymap) {
        /*
          Scancodes
          F1..: 0x05
          Esc.: 0x76
        */
        if (keymap[0x05] == 0) {
            xULAStop = true;
            while (!xULAStopped) {
                delay(5);
            }
            keymap[0x05] = 1;
            keymap[0x76] = 1;
            while (keymap[0x05] != 0 && keymap[0x76] != 0) {
                int opt = osdMainMenu();
            }
            keymap[0x05] = 1;
            xULAStop = false;
        }
    }
}

void osdSquare() {
    vga.fillRect(OSD_X, OSD_Y, OSD_W, OSD_H, zxcolor(1, 0));
    vga.rect(OSD_X, OSD_Y, OSD_W, OSD_H, zxcolor(0, 0));
    vga.rect(OSD_X + 1, OSD_Y + 1, OSD_W - 2, OSD_H - 2, zxcolor(7, 0));
    osdHome();
    vga.setTextColor(zxcolor(0, 0), zxcolor(5, 1));
    vga.setFont(Font6x8);
    vga.println("** ZX-ESPectrum v1.0 by Rampa & Queru 2019 **");
    osdAt(16, 0);
    vga.setTextColor(zxcolor(0, 0), zxcolor(5, 0));
    vga.println(" Cursor: Move | Enter: Select | F1/Esc: Exit ");
}

void menuLine(String line, int row) {
    vga.setCursor(menu_x + 1, menu_y + 1 + (row * 8));
    if (row == 0) {
        vga.setTextColor(zxcolor(7, 1), zxcolor(0, 1));
    } else {
        vga.setTextColor(zxcolor(0, 1), zxcolor(7, 1));
    }
    vga.print(" ");
    vga.print(line.c_str());
    for (int i = 1; i < (menu_cols - line.length()); i++) {
        vga.print(" ");
    }
}

// int menuMaxLen(char menu) {
//     int max = 0;
//     for (int i = 0; i >= sizeof(menu); i++) {
//         if (sizeof(menu[i]) > max)
//             max = sizeof(menu[i]);
//     }
//     return max;
// }

void menuFocusRow(int row) {
    if (row == 0)
        row = 1;
    vga.setCursor(menu_x + 1, menu_y + 1 + (row * 8));
}

void menuSquare(String title, int rows, int cols) {
    // Menu calc relative to OSD
    menu_cols = cols;
    menu_rows = rows;
    menu_w = cols * 6 + 2;
    menu_h = rows * 8 + 4;
    menu_y = (OSD_H / 2) - (menu_h / 2) + OSD_Y;
    menu_x = (OSD_W / 2) - (menu_w / 2) + OSD_X;
    // White background
    vga.fillRect(menu_x, menu_y, menu_w, menu_h, zxcolor(7, 1));
    // Black border
    vga.rect(menu_x, menu_y, menu_w, menu_h, zxcolor(0, 0));
    // Text title
    menuLine(title, 0);
    menuRainbow();
}

void menuRainbow() {
    // Rainbow
    int rb_x = menu_x + menu_w - 20;
    int rb_y = menu_y + 8;
    int rb_colors[] = {2, 6, 4, 5};
    int rb_paint_x = rb_x;
    for (int c = 0; c < 4; c++) {
        for (int i = 0; i < 3; i++) {
            vga.line(rb_paint_x + i, rb_y, rb_paint_x + 4 + i, rb_y - 8, zxcolor(rb_colors[c], 0));
        }
        rb_paint_x += 3;
    }
}

int osdMainMenu() {
    Serial.printf("Menu Max Size: %d", 20);
    osdSquare();
    menuSquare("Main Menu", 4, 20);
    menuLine("Change ROM", 1);
    menuLine("Change RAM", 2);
    menuLine("Reset ESPectrum", 3);
    // osdAt(4, 16);
    // vga.setTextColor(zxcolor(0, 0), zxcolor(7, 0));
    // osdCharSq(6, menu_w);
    // vga.setTextColor(zxcolor(7, 1), zxcolor(0, 1));
    // vga.println(menuLine("Main Menu", menu_w).c_str());
    // vga.setTextColor(zxcolor(0, 1), zxcolor(7, 0));
    // vga.println(menuLine("Change ROM", menu_w).c_str());
    // vga.println(menuLine("Change RAM", menu_w).c_str());
    // vga.println(menuLine("Reset", menu_w).c_str());
    vga.show();
    while (1)
        delay(50);
    return 1;
}
