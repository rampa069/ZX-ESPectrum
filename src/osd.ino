#include "OSD/osd.h"

// On Screen Display

// OSD Main Loop
void do_OSD() {
    static byte last_sna_row = 0;
    static boolean demo_mode_on = false;
    static unsigned int last_demo_ts = millis() / 1000;
    static unsigned int demo_every = 300;
    boolean cycle_sna = false;
    if (checkAndCleanKey(KEY_F12)) {
        cycle_sna = true;
    } else if (checkAndCleanKey(KEY_F1)) {
        // Main menu
        stopULA(); // ULA Stopped
        byte opt = do_Menu(MENU_MAIN);
        if (opt == 1) {
            // Change ROM
            String arch_menu = getArchMenu();
            byte arch_num = do_Menu(arch_menu);
            if (arch_num > 0) {
                String romset_menu = getRomsetMenu(menuGetRow(arch_menu, arch_num));
                byte romset_num = do_Menu(romset_menu);
                if (romset_num > 0) {
                    cfg_arch = menuGetRow(arch_menu, arch_num);
                    cfg_rom_set = menuGetRow(romset_menu, romset_num);
                    load_rom(cfg_arch, cfg_rom_set);
                    config_save();
                    zx_reset();
                }
            }
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
                if (cfg_ram_file != (String)NO_RAM_FILE)
                    load_ram("/sna/" + cfg_ram_file);
            } else if (opt2 == 2) {
                // Hard
                cfg_ram_file = (String)NO_RAM_FILE;
                config_save();
                zx_reset();
            }
        } else if (opt == 4) {
            // Demo mode
            byte opt2 = do_Menu(MENU_DEMO);
            if (opt2 == 1) {
                demo_mode_on = false;
                osdCenteredMsg(OSD_DEMO_MODE_OFF, LEVEL_WARN);
            } else {
                demo_mode_on = true;
                last_demo_ts = millis() / 1000;
                osdCenteredMsg(OSD_DEMO_MODE_ON, LEVEL_OK);
                switch (opt2) {
                case 2:
                    demo_every = 60;
                    break;
                case 3:
                    demo_every = 180;
                    break;
                case 4:
                    demo_every = 300;
                    break;
                case 5:
                    demo_every = 900;
                    break;
                case 6:
                    demo_every = 1800;
                    break;
                case 7:
                    demo_every = 3600;
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

    if (cycle_sna || (demo_mode_on && ((millis() / 1000) - last_demo_ts) > demo_every)) {
        // Cycle over snapshots
        last_sna_row++;
        if (last_sna_row > menuRowCount(cfg_sna_file_list) - 1) {
            last_sna_row = 1;
        }
        changeSna(menuGetRow(cfg_sna_file_list, last_sna_row));
        last_demo_ts = millis() / 1000;
        cycle_sna = false;
    }
}
