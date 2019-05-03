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
        byte opt = do_Menu(MENU_MAIN);
        if (opt == 1) {
            // Change ROM
        } else if (opt == 2) {
            // Change RAM
            unsigned short snanum = do_Menu(cfg_sna_file_list);
            if (snanum > 0) {
                changeSna(menuGetRow(cfg_sna_file_list, snanum));
            }
        } else if (opt == 3) {
            // Reset
            byte opt2 = do_Menu(MENU_RESET);
            if (opt2 == 1) {
                // Soft
                zx_reset();
                if (cfg_mode_sna)
                    load_ram(cfg_ram_file);
            } else if (opt2 == 2) {
                // Hard
                cfg_mode_sna = false;
                cfg_ram_file = 'none';
                config_save();
                zx_reset();
            }
        } else if (opt == 4) {
            // Help
            drawOSD();
            osdAt(2, 0);
            vga.setTextColor(zxcolor(7, 0), zxcolor(1, 0));
            vga.print(OSD_HELP);
            while (!checkAndCleanKey(KEY_F1) && !checkAndCleanKey(KEY_ESC) && !checkAndCleanKey(KEY_ENTER)) {
                vTaskDelay(5);
            }
        }
        // Exit
        startULA();
    }
}
