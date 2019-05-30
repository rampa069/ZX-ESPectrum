#include "Disk.h"
#include "WiFi.h"
#include "def/msg.h"
#include "osd.h"

void wifiConn() {
    if (cfg_wconn) {
        WiFi.mode(WIFI_STA);
        Serial.println((String)MSG_WIFI_CONN_BEGIN + " " + cfg_wssid + "//" + cfg_wpass + "...BEGIN");
        WiFi.begin(cfg_wssid.c_str(), cfg_wpass.c_str());
        byte count = 0;
        while (WiFi.status() != WL_CONNECTED && count <= 300) {
            Serial.printf("Waiting for WiFi connecction...%u\n", count);
            delay(1000);
            count++;
        }
        Serial.printf("WiFi Status: %s\n", (WiFi.status() == WL_CONNECTED ? "CONNECTED" : "FAIL"));
    } else {
        Serial.print("WiFi is disabled");
    }
}
