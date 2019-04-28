// On Screen Display

// Globals
byte row_pos = 0;
byte col_pos = 0;

// Cursor to OSD first row,col
void osdHome() {
    Serial.printf("OSD HOME X:%d Y:%d\n", osdInsideX(), osdInsideY());
    vga.setCursor(osdInsideX(), osdInsideY());
}

// Cursor positioning
void osdAt(byte row, byte col) {
    if (row > osdMaxRows() - 1)
        row = 0;
    if (col > osdMaxCols() - 1)
        col = 0;
    unsigned short y = (row * OSD_FONT_H) + osdInsideY();
    unsigned short x = (col * OSD_FONT_W) + osdInsideX();
    Serial.printf("OSD AT ROW:%d/%d-->%dpx COL:%d/%d-->%dpx\n", row, osdMaxRows(), y, col, osdMaxCols(), x);
    vga.setCursor(x, y);
}

void drawOSD() {
    Serial.println("Drawing OSD");
    unsigned short x = scrAlignCenterX(OSD_W);
    unsigned short y = scrAlignCenterY(OSD_H);
    Serial.printf("OSD X:%d Y:%d W:%d H:%d\n", x, y, OSD_W, OSD_H);
    vga.fillRect(x, y, OSD_W, OSD_H, zxcolor(1, 0));
    vga.rect(x, y, OSD_W, OSD_H, zxcolor(0, 0));
    vga.rect(x + 1, y + 1, OSD_W - 2, OSD_H - 2, zxcolor(7, 0));
    vga.setTextColor(zxcolor(0, 0), zxcolor(5, 1));
    vga.setFont(Font6x8);
    osdHome();
    vga.print("* ZX-ESPectrum v1.0|Rampa & Queru 2019 *");
    osdAt(17, 0);
    vga.setTextColor(zxcolor(0, 0), zxcolor(13, 0));
    vga.print("           LIVE FREE OR DIE!!           ");
}

void drawMenu(String menu, byte focus) {
    Serial.printf("Drawing menu, focus on: %d\n", focus);
    const byte cols = menuColMax(menu) + 2;
    const unsigned short real_rows = menuRowCount(menu);
    const byte rows = menuVirtualRows(menu);
    const byte w = (cols * OSD_FONT_W) + 2;
    const byte h = (rows * OSD_FONT_H) + 2;
    const unsigned short x = scrAlignCenterX(w);
    const unsigned short y = scrAlignCenterY(h);
    Serial.printf("%d cols, %d real_rows and %d rows\n", cols, real_rows, rows);
    // White background
    Serial.printf("White bg x:%d, y:%d, w:%d, h:%d\n", x, y, w, h);
    vga.fillRect(x, y, w, h, zxcolor(7, 1));
    // Black border
    Serial.printf("Black border x:%d, y:%d, w:%d, h:%d\n", x, y, w, h);
    vga.rect(x, y, w, h, zxcolor(0, 0));

    Serial.printf("Iterating %d rows\n", rows);
    for (byte row = 0; row < rows; row++) {
        Serial.printf("Row %d: ", row);
        vga.setCursor(x + 1, y + 1 + (row * OSD_FONT_H));
        if (row == 0) {
            Serial.println("Title row");
            vga.setTextColor(zxcolor(7, 1), zxcolor(0, 1));
        } else if (row == focus) {
            Serial.println("Focus row");
            vga.setTextColor(zxcolor(0, 1), zxcolor(5, 1));
        } else {
            Serial.println("Normal row");
            vga.setTextColor(zxcolor(0, 1), zxcolor(7, 1));
        }
        String line = menuGetRow(menu, row);
        Serial.printf("Printing row %d '%s' len:%d to cols:%d\n", row, line.c_str(), line.length(), cols);
        vga.print(" ");
        vga.print(line.c_str());
        for (byte i = line.length(); i < (cols - 1); i++)
            vga.print(" ");
    }

    // Rainbow
    unsigned short rb_y = y + 8;
    unsigned short rb_paint_x = x + w - 20;
    byte rb_colors[] = {2, 6, 4, 5};
    for (byte c = 0; c < 4; c++) {
        for (byte i = 0; i < 3; i++) {
            vga.line(rb_paint_x + i, rb_y, rb_paint_x + 4 + i, rb_y - 8, zxcolor(rb_colors[c], 0));
        }
        rb_paint_x += 3;
    }
}

unsigned short do_Menu(String menu) {
    const unsigned short real_rows = menuRowCount(menu);
    const byte rows = menuVirtualRows(menu);
    unsigned short focus = 1;
    unsigned short focus_new = 1;

    Serial.println("Draw OSD");
    drawOSD();
    Serial.println("Draw Menu");
    drawMenu(menu, focus);
<<<<<<< HEAD
    Serial.println("VGA Show");
    //vga.show();
    delay(50);
=======
>>>>>>> 487ef4699fd4b9a5f69906d14e64b376dd07a123
    while (1) {
        if (checkAndCleanKey(KEY_UP)) {
            focus_new = focus - 1;
            if (focus_new < 1)
                focus_new = rows - 1;
        } else if (checkAndCleanKey(KEY_DOWN)) {
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
            drawOSD();
            drawMenu(menu, focus);
            //vga.show();
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
        // ULA Stopped
        Serial.printf("Main menu rows: %d\n", menuRowCount(main_menu));
        Serial.printf("Main menu max col: %d\n", menuColMax(main_menu));
        switch (do_Menu(main_menu)) {
        case 1:
            // Change ROM
            break;
        case 2:
            // Change RAM
            break;
        case 3:
            // Reset
            switch (do_Menu(reset_menu)) {
            case 1:
                // Soft
                zx_setup();
                if (cfg_mode_sna)
                    load_ram(cfg_ram_file);
                break;
            case 2:
                // Hard
                zx_setup();
                break;
            }
        }

        // Exit
        xULAStop = false;
        while (xULAStopped) {
            delay(20);
        }
        Serial.println(ULA_ON);
    }
}
