#pragma once
#include <Arduino.h>

class Logger {
public:
    template<typename... Args>
    static void trace(const char* format, Args... args){
        Serial.printf(format, args...);
        Serial.printf("\n");
    }

    static void init() {
        Serial.begin(115200);
        delay(3000);
    }
};