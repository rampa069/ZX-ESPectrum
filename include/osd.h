// OSD Headers

const unsigned short SCR_W = 360;
const unsigned short SCR_H = 200;
const unsigned short OSD_X = 41;
const unsigned short OSD_Y = 28;
const unsigned short OSD_W = 278;
const unsigned short OSD_H = 144;
const unsigned short OSD_MARGIN = 4;
const unsigned short OSD_MAX_ROW = 16;
const unsigned short OSD_MAX_COL = 44;
const unsigned short MENU_MAX_OPTS = 20;
const unsigned short MENU_MAX_LINE_LEN = 80;
const unsigned short MENU_FILE_MAX_OPTS = 20;
const unsigned short MENU_FILE_MAX_LINE_LEN = 20;
char main_menu[MENU_MAX_OPTS][MENU_MAX_LINE_LEN] = {"Main Menu", "Change ROM", "Change RAM", "Reset", "Return", 0};
char reset_menu[MENU_MAX_OPTS][MENU_MAX_LINE_LEN] = {"Reset Menu", "Soft reset", "Hard reset", "Cancel", 0};
char file_menu[MENU_FILE_MAX_OPTS][MENU_FILE_MAX_LINE_LEN] = {"Select file", 0};

extern boolean checkAndCleanKey(byte scancode);
extern boolean isKeymapChanged();
extern void updateKeymap();

#define MENUPARAM char menu[MENU_MAX_OPTS][MENU_MAX_LINE_LEN]
