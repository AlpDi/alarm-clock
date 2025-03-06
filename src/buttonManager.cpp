#include "buttonManager.h"

#include <Arduino.h>

#include <map>

#include "AlarmManager.h"
#include "AlarmTrigger.h"
#include "Logger.h"

buttonManager& buttonManager::getInstance() {
    static buttonManager instance;
    return instance;
}

void buttonManager::begin(uint8_t stopButtonPin, uint8_t snoozeButtonPin, uint8_t simButtonPin) {
    stopPin = stopButtonPin;
    snoozePin = snoozeButtonPin;
    simPin = simButtonPin;

    pinMode(stopPin, INPUT_PULLUP);
    pinMode(snoozePin, INPUT_PULLUP);
    pinMode(simPin, INPUT_PULLUP);
}

// wait 50ms after each state change to mitigate noise in button input
bool buttonManager::readButtonDebounced(uint8_t pin, uint32_t debounceTime) {
    struct ButtonState {
        uint32_t lastDebounceTime = 0;
        bool lastButtonState = false;
        bool buttonState = false;
    };

    static std::map<uint8_t, ButtonState> buttonStates;

    auto& state = buttonStates[pin];

    bool rawState = !digitalRead(pin);

    if (rawState != state.lastButtonState) {
        state.lastDebounceTime = millis();
        state.lastButtonState = rawState;
    }

    if ((millis() - state.lastDebounceTime) > debounceTime) {
        if (state.buttonState != rawState) {
            state.buttonState = rawState;
        }
    }

    return state.buttonState;
}

void buttonManager::handleStopButtonPress() {
    static uint32_t lastPressTime = 0;
    const uint32_t MIN_PRESS_INTERVAL = 250;

    // Logger::trace("Stop button raw state: %d", !digitalRead(stopPin));

    if (readButtonDebounced(stopPin)) {
        uint32_t curTime = millis();
        if (curTime - lastPressTime >= MIN_PRESS_INTERVAL) {
            Logger::trace("check Stop Button");
            for (auto& alarmId : AlarmTrigger::getInstance().getActiveAlarms()) {
                AlarmTrigger::getInstance().stopAlarm(alarmId);
            }
            lastPressTime = curTime;
        }
    }
}

void buttonManager::handleSnoozeButtonPress() {
    static uint32_t lastPressTime = 0;
    const uint32_t MIN_PRESS_INTERVAL = 250;

    if (readButtonDebounced(snoozePin)) {
        uint32_t curTime = millis();
        if (curTime - lastPressTime >= MIN_PRESS_INTERVAL) {
            Logger::trace("check Snooze Button");
            for (auto& alarmId : AlarmTrigger::getInstance().getActiveAlarms()) {
                AlarmTrigger::getInstance().snoozeAlarm(alarmId, 10);
            }
            lastPressTime = curTime;
        }
    }
}

void buttonManager::handleSimButtonPress() {
    static uint32_t lastPressTime = 0;
    const uint32_t MIN_PRESS_INTERVAL = 250;

    if (readButtonDebounced(simPin)) {
        uint32_t curTime = millis();
        if (curTime - lastPressTime >= MIN_PRESS_INTERVAL) {
            Logger::trace("check Sim Button");
            AlarmManager::getInstance().simulateAlarm();
            lastPressTime = curTime;
        }
    }
}