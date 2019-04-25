// On Screen Display

#include "msg.h"
#include "osd.h"
#include <Ressources/Font6x8.h>

unsigned short int osdX(unsigned short int rel_x) { return OSD_X + OSD_MARGIN + rel_x; }

unsigned short int osdY(unsigned short int rel_y) { return OSD_Y + OSD_MARGIN + rel_y; }

void osdHome() { vga.setCursor(osdX(0), osdY(0)); }

void osdAt(int x, int y) { vga.setCursor(osdX(x * 6), osdY(y * 8)); }

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
            log("OSD ON");
            osdSquare();
            vga.show();
            keymap[0x05] = 1;
            while (keymap[0x05] != 0) {
                delay(5);
            }
            log("OSD OFF");
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
    vga.setTextColor(zxcolor(0, 0), zxcolor(5, 0));
    vga.setFont(Font6x8);
    // vga.println("** ZX-ESPectrum v1.0 by Rampa & Queru 2019 **");
    for (int i = 0; i < 17; i++) {
        osdAt(0, i);
        char line[50];
        sprintf(line, "%2u:456789|123456789|123456789|123456789|12345", i);
        vga.print(line);
    }
    // vga.println("Cursor: Move | Enter: Select | F1/Esc: Exit");
}
