// On Screen Display

// Globals
byte row_pos = 0;
byte col_pos = 0;

void osdHome() { vga.setCursor(osdX(0), osdY(0)); }

// Text 17x45 with 6x8 font
void osdAt(byte row, byte col) {
    if (row > OSD_MAX_ROW)
        row = 0;
    if (col > OSD_MAX_COL)
        col = 0;
    vga.setCursor(osdX(col * 6), osdY(row * 8));
    row_pos = row;
    col_pos = col;
}

void osdCharSq(byte lines, byte cols) {
    const byte start_row = row_pos;
    const byte start_col = col_pos;
    for (byte l = 1; l <= lines; l++) {
        for (byte c = 1; c <= cols; c++) {
            vga.print(" ");
        }
        osdAt(row_pos + 1, start_col);
    }
    osdAt(start_row, start_col);
}

// Menu calc
byte menuCols(MENUPARAM) {
    byte max = 0;
    byte i = 0;
    while (menu[i][0] != 0) {
        if (strlen(menu[i]) > max)
            max = strlen(menu[i]);
        i++;
    }
    if (max < 20)
        max = 20;
    if (max > 140)
        max = 140;
    return max;
}

byte menuRows(MENUPARAM) {
    byte i = 0;
    while (menu[i][0] != 0)
        i++;
    return i;
}

// Menu max columns
byte menuW(MENUPARAM) { return (menuCols(menu) * 6) + 2; }
// Menu max rows
byte menuH(MENUPARAM) { return (menuRows(menu) * 8) + 4; }
// Menu width in pixels
unsigned short menuY(MENUPARAM) { return (OSD_H / 2) - (menuH(menu) / 2) + OSD_Y; }
// Menu height in pixels
unsigned short menuX(MENUPARAM) { return (OSD_W / 2) - (menuW(menu) / 2) + OSD_X; }

unsigned short osdX(unsigned short rel_x) { return (OSD_X + OSD_MARGIN + rel_x); }
unsigned short osdY(unsigned short rel_y) { return (OSD_Y + OSD_MARGIN + rel_y); }

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

void menuDraw(MENUPARAM, byte focus_row) {
    Serial.printf("MENU X:%u Y:%u W:%u H:%u\n", menuX(menu), menuY(menu), menuW(menu), menuH(menu));
    // White background
    vga.fillRect(menuX(menu), menuY(menu), menuW(menu), menuH(menu), zxcolor(7, 1));
    // Black border
    vga.rect(menuX(menu), menuY(menu), menuW(menu), menuH(menu), zxcolor(0, 0));

    byte row = 0;
    while (menu[row][0] != 0) {
        vga.setCursor(menuX(menu) + 1, menuY(menu) + 1 + (row * 8));
        if (row == 0) {
            vga.setTextColor(zxcolor(7, 1), zxcolor(0, 1));
        } else if (row == focus_row) {
            vga.setTextColor(zxcolor(0, 1), zxcolor(5, 1));
        } else {
            vga.setTextColor(zxcolor(0, 1), zxcolor(7, 1));
        }
        vga.print(" ");
        vga.print(menu[row]);
        for (byte i = 0; i < (menuCols(menu) - strlen(menu[row]) - 1); i++) {
            vga.print(" ");
        }
        row++;
    }
    // Rainbow
    unsigned short rb_y = menuY(menu) + 8;
    unsigned short rb_paint_x = menuX(menu) + menuW(menu) - 20;
    byte rb_colors[] = {2, 6, 4, 5};
    for (byte c = 0; c < 4; c++) {
        for (byte i = 0; i < 3; i++) {
            vga.line(rb_paint_x + i, rb_y, rb_paint_x + 4 + i, rb_y - 8, zxcolor(rb_colors[c], 0));
        }
        rb_paint_x += 3;
    }
}

// Menu run
byte osdMenu(MENUPARAM, byte focus_row) {
    Serial.println("OSD MENU BEGIN");
    Serial.printf("Menu Max COLS:%u ROWS:%u\n", menuCols(menu), menuRows(menu));
    osdSquare();
    menuDraw(menu, focus_row);
    vga.show();
    byte new_focus_row = focus_row;
    while (1) {
        if (checkAndCleanKey(KEY_UP)) {
            new_focus_row = focus_row - 1;
            if (new_focus_row < 1)
                new_focus_row = menuRows(menu) - 1;
        } else if (checkAndCleanKey(KEY_DOWN)) {
            new_focus_row = focus_row + 1;
            if (new_focus_row > menuRows(menu) - 1)
                new_focus_row = 1;
        } else if (checkAndCleanKey(KEY_ENTER)) {
            return focus_row;
        } else if (checkAndCleanKey(KEY_ESC) || checkAndCleanKey(KEY_F1)) {
            return 0;
        }

        if (new_focus_row != focus_row) {
            focus_row = new_focus_row;
            osdSquare();
            menuDraw(menu, focus_row);
            vga.show();
        }
        delay(50);
    }
}

// OSD Main Loop
void do_OSD() {
    if (checkAndCleanKey(KEY_F1)) {
        Serial.println(OSD_ON);
        xULAStop = true;
        while (!xULAStopped) {
            delay(20);
        }
        Serial.println(ULA_OFF);
        byte opt = osdMenu(main_menu, 1);
        Serial.printf("Menu option: %u --> %s", opt, main_menu[opt]);
        switch (opt) {
        case 3:
            // RESET
            Serial.println(MSG_Z80_RESET);
            Z80_Reset();
            break;
        }
        xULAStop = false;
        Serial.println(ULA_ON);
    }
}
