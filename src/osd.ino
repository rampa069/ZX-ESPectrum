#include "Emulator/msg.h"
#include "Emulator/osd.h"

// On Screen Display

// Globals
byte row_pos = 0;
byte col_pos = 0;

// Change running snapshot
void changeSna(String sna_filename) {
    cfg_ram_file = "/sna/" + sna_filename;
    cfg_mode_sna = true;
    zx_reset();
    load_ram(cfg_ram_file);
    config_save();
}

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
    unsigned short x = scrAlignCenterX(OSD_W);
    unsigned short y = scrAlignCenterY(OSD_H);
    vga.fillRect(x, y, OSD_W, OSD_H, zxcolor(1, 0));
    vga.rect(x, y, OSD_W, OSD_H, zxcolor(0, 0));
    vga.rect(x + 1, y + 1, OSD_W - 2, OSD_H - 2, zxcolor(7, 0));
    vga.setTextColor(zxcolor(0, 0), zxcolor(5, 1));
    vga.setFont(Font6x8);
    osdHome();
    vga.print(OSD_TITLE);
    osdAt(17, 0);
    vga.print(OSD_BOTTOM);
}

// Shows a red panel with error text
void errorPanel(String errormsg) {
    unsigned short x = scrAlignCenterX(OSD_W);
    unsigned short y = scrAlignCenterY(OSD_H);
    if (cfg_slog_on)
        Serial.println(errormsg);
    vga.fillRect(x, y, OSD_W, OSD_H, zxcolor(0, 0));
    vga.rect(x, y, OSD_W, OSD_H, zxcolor(7, 0));
    vga.rect(x + 1, y + 1, OSD_W - 2, OSD_H - 2, zxcolor(2, 1));
    vga.setFont(Font6x8);
    osdHome();
    vga.setTextColor(zxcolor(7, 1), zxcolor(2, 1));
    vga.print(ERROR_TITLE);
    osdAt(2, 0);
    vga.setTextColor(zxcolor(7, 1), zxcolor(0, 0));
    vga.println(errormsg.c_str());
    osdAt(17, 0);
    vga.setTextColor(zxcolor(7, 1), zxcolor(2, 1));
    vga.print(ERROR_BOTTOM);
}

// Error panel and infinite loop
void errorHalt(String errormsg) {
    xULAStop = true;
    while (!xULAStopped) {
        delay(20);
    }
    errorPanel(errormsg);
    while (1)
        delay(500);
}

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
        // Launch ULA for a while

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

unsigned short do_Menu(String menu) {
    const unsigned short real_rows = menuRowCount(menu);
    const byte rows = menuVirtualRows(menu);
    unsigned short focus = 1;
    unsigned short focus_new = 1;

    stepULA();
    drawMenu(menu, focus, MENU_REDRAW);
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
            drawMenu(menu, focus, MENU_UPDATE);
        }
        delay(50);
    }
}

// OSD Main Loop
void do_OSD() {
    static byte last_sna_row = 0;
    if (checkAndCleanKey(KEY_F12)) {
        // Cycle over snapshots
        last_sna_row++;
        if (last_sna_row > menuRowCount(cfg_sna_file_list) - 1) {
            last_sna_row = 1;
        }
        changeSna(menuGetRow(cfg_sna_file_list, last_sna_row));
    } else if (checkAndCleanKey(KEY_F1)) {
        // Main menu
        stopULA(); // ULA Stopped
        switch (do_Menu(MENU_MAIN)) {
        case 1:
            // Change ROM
            break;
        case 2: {
            // Change RAM
            unsigned short snanum = do_Menu(cfg_sna_file_list);
            if (snanum > 0) {
                changeSna(menuGetRow(cfg_sna_file_list, snanum));
            }
            break;
        }
        case 3:
            // Reset
            switch (do_Menu(MENU_RESET)) {
            case 1:
                // Soft
                zx_reset();
                //if (cfg_mode_sna)
                    load_ram_128("/sna/abadia.sna");

                    //load_ram(cfg_ram_file);
                break;
            case 2:
                // Hard
                zx_reset();
                cfg_mode_sna = false;
                cfg_ram_file = 'none';
                config_save();
                break;
            }
        }

        // Exit
        startULA();
    }
}
