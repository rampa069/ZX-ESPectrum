// On Screen Display

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
            vga.clear(7);
            vga.rect(20, 20, 240, 190, vga.RGB(0, 192, 192));
            vga.show();
            keymap[0x05] = 1;
            while (keymap[0x05] != 0) {
                log("OSD active...");
            }
            log("OSD OFF");
            keymap[0x05] = 1;
            xULAStop = false;
        }
    }
}
