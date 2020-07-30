#ifndef WIIMOTE2KEYS_H
#define WIIMOTE2KEYS_H

void initWiimote2Keys();

void loadKeytableForGame(const char* sna_fn);

void updateWiimote2Keys();      // normal operation

void updateWiimote2KeysOSD();   // OSD operation

#endif WIIMOTE2KEYS_H 