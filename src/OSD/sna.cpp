#include "Disk.h"
#include "Emulator/z80main.h"
#include "def/files.h"
#include "def/msg.h"
#include "osd.h"

// Change running snapshot
void changeSna(String sna_filename) {
    osdCenteredMsg((String)MSG_LOADING + ": " + sna_filename, LEVEL_INFO);
    zx_reset();
    Serial.printf("Loading sna: %s\n", sna_filename.c_str());
    load_ram((String)DISK_SNA_DIR + "/" + sna_filename);
    osdCenteredMsg(MSG_SAVE_CONFIG, LEVEL_WARN);
    cfg_ram_file = sna_filename;
    config_save();
}

// Demo mode on off
void setDemoMode(boolean on, unsigned short every) {
    cfg_demo_mode_on = on;
    cfg_demo_every = (every > 0 ? every : 60);
    if (on) {
        osdCenteredMsg(OSD_DEMO_MODE_ON, LEVEL_OK);
    } else {
        osdCenteredMsg(OSD_DEMO_MODE_OFF, LEVEL_WARN);
    }
    Serial.printf("DEMO MODE %s every %u seconds.", (cfg_demo_mode_on ? "ON" : "OFF"), cfg_demo_every);
    vTaskDelay(200);
    osdCenteredMsg(MSG_SAVE_CONFIG, LEVEL_WARN);
    config_save();
}
