#include "Disk.h"
#include "ZX-ESPectrum.h"
#include "def/Font.h"
#include "def/files.h"
#include "def/msg.h"
#include "osd.h"

// Shows a red panel with error text
void errorPanel(String errormsg) {
    unsigned short x = scrAlignCenterX(OSD_W);
    unsigned short y = scrAlignCenterY(OSD_H);

    if (cfg_slog_on)
        Serial.println(errormsg);

    stepULA();

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
    errorPanel(errormsg);
    while (1) {
        do_keyboard();
        do_OSD();
        delay(5);
    }
}

// Centered message
void osdCenteredMsg(String msg, byte warn_level) {
    const unsigned short w = (msg.length() + 2) * OSD_FONT_W;
    const unsigned short h = OSD_FONT_H * 3;
    const unsigned short x = scrAlignCenterX(w);
    const unsigned short y = scrAlignCenterY(h);
    unsigned short paper;
    unsigned short ink;

    switch (warn_level) {
    case LEVEL_OK:
        ink = zxcolor(7, 1);
        paper = zxcolor(4, 0);
        break;
    case LEVEL_ERROR:
        ink = zxcolor(7, 1);
        paper = zxcolor(2, 0);
        break;
    case LEVEL_WARN:
        ink = zxcolor(0, 0);
        paper = zxcolor(6, 0);
        break;
    default:
        ink = zxcolor(7, 0);
        paper = zxcolor(1, 0);
    }

    stepULA();

    vga.fillRect(x, y, w, h, paper);
    // vga.rect(x - 1, y - 1, w + 2, h + 2, ink);
    vga.setTextColor(ink, paper);
    vga.setFont(Font6x8);
    vga.setCursor(x + OSD_FONT_W, y + OSD_FONT_H);
    vga.print(msg.c_str());
}
