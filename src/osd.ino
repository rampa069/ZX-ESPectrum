// On Screen Display

static int osd_x = 40;
static int osd_y = 30;
static int osd_w = 275;
static int osd_h = 120;
static int osd_bg = vga.RGB(0, 100, 200);
static int osd_fg = vga.RGB(200, 200, 200);

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
            vga.fillRect(40, 30, 275, 120, vga.RGB(0, 100, 200));
            vga.rect(40, 30, 275, 120, vga.RGB(200, 200, 200));
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
    vga.fillRect(osd_x, osd_y, osd_w, osd_h, osd_bg);
    vga.rect(osd_x, osd_y, osd_w, osd_h, osd_fg);
}
