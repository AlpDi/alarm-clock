#pragma once
#include <Arduino.h>
#include "AlarmManager.h"

class buttonManager{
public: 
    static buttonManager& getInstance();

    void begin(uint8_t stopPin, uint8_t snoozePin, uint8_t simPin);

    bool readButtonDebounced(uint8_t pin, uint32_t debouceTime = 50);


    void handleStopButtonPress();
    void handleSnoozeButtonPress();
    void handleSimButtonPress();

private:
    buttonManager() = default;
};