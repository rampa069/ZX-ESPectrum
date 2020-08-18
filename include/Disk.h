// Defines
#define ASCII_NL 10
#define ON true
#define OFF false

// Headers
#include <Arduino.h>
#include <FS.h>

// Declared vars
extern boolean cfg_demo_mode_on;
extern unsigned short cfg_demo_every;
extern String cfg_sna_file_list;
extern String cfg_arch;
extern String cfg_ram_file;
extern String cfg_rom_set;
extern String cfg_sna_file_list;
extern boolean cfg_slog_on;
extern boolean cfg_wconn;
extern String cfg_wssid;
extern String cfg_wpass;

// Declared methods
void IRAM_ATTR mount_spiffs();
String getAllFilesFrom(const String path);
void listAllFiles();
File IRAM_ATTR open_read_file(String filename);
void IRAM_ATTR load_ram(String sna_file);
void IRAM_ATTR save_ram(String sna_file);
String getFileEntriesFromDir(String path);
unsigned short countFileEntriesFromDir(String path);
void load_rom(String arch, String romset);
String getSnaFileList();
void config_read();
void IRAM_ATTR config_save();
