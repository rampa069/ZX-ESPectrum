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
                if (cfg_ram_file != NO_RAM_FILE)
                    load_ram(cfg_ram_file);
            } else if (opt2 == 2) {
                // Hard
                cfg_ram_file = NO_RAM_FILE;
                config_save();
                zx_reset();
            }
        } else if (opt == 4) {
            // Demo mode
            byte opt2 = do_Menu(MENU_DEMO);
            if (opt2 == 1) {
                cfg_demo_on = false;
                osdCenteredMsg(OSD_DEMO_MODE_OFF, LEVEL_WARN);
            } else {
                cfg_demo_on = true;
                osdCenteredMsg(OSD_DEMO_MODE_ON, LEVEL_OK);
                switch (opt2) {
                case 2:
                    cfg_demo_every = 60000;
                    break;
                case 3:
                    cfg_demo_every = 180000;
                    break;
                case 4:
                    cfg_demo_every = 300000;
                    break;
                case 5:
                    cfg_demo_every = 900000;
                    break;
                case 6:
                    cfg_demo_every = 1800000;
                    break;
                case 7:
                    cfg_demo_every = 3600000;
                    break;
                }
            }
            vTaskDelay(500);
        } else if (opt == 5) {
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
