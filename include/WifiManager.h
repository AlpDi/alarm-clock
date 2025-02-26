#pragma once
#include <WiFi.h>

class WifiManager{
public:
    static bool connect(const char* ssid, const char* password);
};