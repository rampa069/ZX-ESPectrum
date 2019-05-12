#include "Disk.h"
#include "WiFi.h"
#include "def/msg.h"
#include "osd.h"

void wifiConn() {
    WiFi.begin(cfg_wssid.c_str(), cfg_wpass.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println((String)MSG_WIFI_CONN_BEGIN + " " + cfg_wssid + "...");
    }
}
