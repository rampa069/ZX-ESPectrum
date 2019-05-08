#include "osd.h"
#include "ZX-ESPectrum.h"
#include "def/files.h"
#include "def/msg.h"

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
    osdHome();
}
