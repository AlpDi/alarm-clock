#pragma once
#include <Arduino.h>
#include "AlarmManager.h"

class buttonManager{
public: 
    using ButtonCallback = std::function<void(const Alarm&)>;

    static buttonManager& getInstance();

    bool readButtonDebounced(uint8_t pin, uint32_t debouceTime = 50);

private:
    buttonManager() = default;
    ButtonCallback onButtonPressed;
};