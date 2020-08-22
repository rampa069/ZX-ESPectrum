#pragma once

// General
#define MSG_LOADING "Loading file"
#define MSG_SAVE_CONFIG "Saving config file"
#define MSG_CHIP_SETUP "Chip setup"
#define MSG_VGA_INIT "Initalizing VGA"
#define MSG_FREE_HEAP_BEFORE "Free heap before "
#define MSG_FREE_HEAP_AFTER "Free heap after "
#define MSG_Z80_RESET "Reseting Z80 CPU"
#define MSG_EXEC_ON_CORE "Executing on core #"
#define ULA_ON "ULA ON"
#define ULA_OFF "ULA OFF"

// WiFi
#define MSG_WIFI_CONN_BEGIN "Connecting to WiFi"

// Error
#define ERROR_TITLE "  !!!   ERROR - CLIVE MEDITATION   !!!  "
#define ERROR_BOTTOM "  Sir Clive is smoking in the Rolls...  "
#define ERR_READ_FILE "Cannot read file!"
#define ERR_BANK_FAIL "Failed to allocate RAM bank "
#define ERR_MOUNT_FAIL "Cannot mount internal memory!"
#define ERR_DIR_OPEN "Cannot open directory!"

// OSD
#define OSD_TITLE  "* ZX-ESPectrum - ESP32 - VGA - Wiimote *"
#define OSD_BOTTOM "      SCIENCE  LEADS  TO  PROGRESS      "
#define OSD_ON "OSD ON"
#define OSD_OFF "OSD OFF"
#define OSD_DEMO_MODE_ON "Demo Mode ON"
#define OSD_DEMO_MODE_OFF "Demo Mode OFF"
#define OSD_PAUSE "--=[PAUSED]=--"

#define OSD_QSNA_NOT_AVAIL "No Quick Snapshot Available"
#define OSD_QSNA_LOADING "Loading Quick Snapshot..."
#define OSD_QSNA_SAVING "Saving Quick Snapshot..."
#define OSD_QSNA_SAVE_ERR "ERROR Saving Quick Snapshot"
#define OSD_QSNA_LOADED "Quick Snapshot Loaded"
#define OSD_QSNA_LOAD_ERR "ERROR Loading Quick Snapshot"
#define OSD_QSNA_SAVED "Quick Snapshot Saved"

#define OSD_PSNA_NOT_AVAIL "No Persist Snapshot Available"
#define OSD_PSNA_LOADING "Loading Persist Snapshot..."
#define OSD_PSNA_SAVING "Saving Persist Snapshot..."
#define OSD_PSNA_SAVE_ERR "ERROR Saving Quick Snapshot"
#define OSD_PSNA_LOADED "Persist Snapshot Loaded"
#define OSD_PSNA_LOAD_ERR "ERROR Loading Quick Snapshot"
#define OSD_PSNA_SAVED "Persist Snapshot Saved"

#define MENU_SNA_TITLE "Select Snapshot"
#define MENU_MAIN \
    "Main Menu\n"\
    "Load Snapshot to RAM\n"\
    "Select ROM\n"\
    "Quick Save (F2)\n"\
    "Quick Load (F3)\n"\
    "Persist Save (F4)\n"\
    "Persist Load (F5)\n"\
    "Reset\n"\
    "About...\n"\
    "Return\n"
#define MENU_RESET \
    "Reset Menu\n"\
    "Soft reset\n"\
    "Hard reset\n"\
    "Cancel\n"
#define MENU_DEMO "Demo mode\nOFF\n 1 minute\n 3 minutes\n 5 minutes\n15 minutes\n30 minutes\n 1 hour\n"
#define MENU_ARCH "Select Arch\n"
#define MENU_ROMSET "Select Rom Set\n"
#define OSD_HELP \
    "Developed in 2019 by Rampa & Queru\n"\
    "Modified in 2020 by DCrespo\n"\
    "for using Wiimote as input device.\n\n" \
    "    Home   for main menu\n"\
    "    Cursor to move.\n"\
    "    A/1/2  to select.\n"\
    "    Home   to exit.\n"
#define MENU_TEST getTestMenu(200)
