#include "WifiManager.h"
#include "config.h"
#include "secrets.h"
#include "Logger.h"

bool WifiManager::connect(const char* ssid, const char* password){
    WiFi.setHostname(hostname);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passPhrase);

    int attempts = 0;
    const int maxAttempts = 20; // 10 seconds timeout

    while(WiFi.status() != WL_CONNECTED && attempts < maxAttempts){
        delay(500);
        Logger::trace(".");
        attempts++;
    }

    return WiFi.status() == WL_CONNECTED;
}