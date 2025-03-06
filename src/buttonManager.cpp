#include "buttonManager.h"
#include "AlarmTrigger.h"
#include <Arduino.h>
#include "AlarmManager.h"

buttonManager& buttonManager::getInstance(){
    static buttonManager instance;
    return instance;
}

void buttonManager::begin(uint8_t stopButtonPin, uint8_t snoozeButtonPin, uint8_t simButtonPin){
    stopPin = stopButtonPin;
    snoozePin = snoozeButtonPin;
    simPin = simButtonPin;

    pinMode(stopPin, INPUT_PULLUP);
    pinMode(snoozePin, INPUT_PULLUP);
    pinMode(simPin, INPUT_PULLUP);
}

//wait 50ms after each state change to mitigate noise in button input
bool buttonManager::readButtonDebounced(uint8_t pin, uint32_t debounceTime) {
  static uint32_t lastDebounceTime = 0;
  static bool lastButtonState = false;
  static bool buttonState = false;
  

  bool rawState = !digitalRead(pin);

  if (rawState != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = rawState;
  }
  
  if ((millis() - lastDebounceTime) > debounceTime) {
    if (buttonState != rawState) {
      buttonState = rawState;
    }
  }
  
  return buttonState;
}

void buttonManager::handleStopButtonPress(){
    static uint32_t lastPressTime = 0;
    const uint32_t MIN_PRESS_INTERVAL = 250;

    if(readButtonDebounced(stopPin)){
        uint32_t curTime = millis();
        if(curTime - lastPressTime >= MIN_PRESS_INTERVAL){
            for(auto& alarmId: AlarmTrigger::getInstance().getActiveAlarms()){
                AlarmTrigger::getInstance().stopAlarm(alarmId);
            }
            lastPressTime = curTime;
        }
    }
}

void buttonManager::handleSnoozeButtonPress(){
    static uint32_t lastPressTime = 0;
    const uint32_t MIN_PRESS_INTERVAL = 250;

    if(readButtonDebounced(snoozePin)){
        uint32_t curTime = millis();
        if(curTime - lastPressTime >= MIN_PRESS_INTERVAL){
            for(auto& alarmId: AlarmTrigger::getInstance().getActiveAlarms()){
                AlarmTrigger::getInstance().snoozeAlarm(alarmId, 10);
            }
            lastPressTime = curTime;
        }
    }
}

void buttonManager::handleSimButtonPress(){
    static uint32_t lastPressTime = 0;
    const uint32_t MIN_PRESS_INTERVAL = 250;

    if(readButtonDebounced(simPin)){
        uint32_t curTime = millis();
        if(curTime - lastPressTime >= MIN_PRESS_INTERVAL){
            AlarmManager::getInstance().simulateAlarm();
            lastPressTime = curTime;
        }  
    }
}