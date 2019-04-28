#pragma once
#pragma GCC diagnostic ignored "-Wall"
#include "Arduino.h"
#include <FS.h>
#include <SPIFFS.h>
#pragma GCC diagnostic warning "-Wall"
#include "msg.h"

extern boolean cfg_slog_on;

void mount_spiffs() {
    while (!SPIFFS.begin()) {
        if (cfg_slog_on)
            Serial.println(MSG_MOUNT_FAIL);
        delay(500);
    }
}

File open_read_file(String filename) {
    File f;
    mount_spiffs();
    if (cfg_slog_on)
        Serial.println(MSG_LOADING + filename);
    f = SPIFFS.open(filename, FILE_READ);
    while (!f) {
        if (cfg_slog_on)
            Serial.println(MSG_READ_FILE_FAIL + filename);
        sleep(10);
        f = SPIFFS.open(filename, FILE_READ);
    }
    return f;
}

String getDirAsMenu(String title, String path) {
    String menu = title + "\n";
    return menu;
}
