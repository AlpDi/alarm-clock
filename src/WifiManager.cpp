#include "WifiManager.h"
#include "config.h"
#include "secrets.h"
#include "Logger.h"

bool WifiManager::connect(const char* ssid, const char* password){
    WiFi.setHostname(hostname);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passPhrase);

    while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Logger::trace(".");
    }
}