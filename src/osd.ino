#include "OSD/osd.h"

// On Screen Display

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
                if (cfg_mode_sna)
                    load_ram(cfg_ram_file);
                break;
            case 2:
                // Hard
                zx_reset();
                cfg_mode_sna = false;
                cfg_ram_file = 'none';
                config_save();
                break;
            }
        case 4:
            // Help
            drawOSD();
            osdAt(2, 0);
            vga.setTextColor(zxcolor(7, 0), zxcolor(1, 0));
            vga.print(OSD_HELP);
            while (!checkAndCleanKey(KEY_F1) && !checkAndCleanKey(KEY_ESC) && !checkAndCleanKey(KEY_ENTER))
                vTaskDelay(5);
        }

        // Exit
        startULA();
    }
}
