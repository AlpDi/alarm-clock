#include "Buzzer.h"

#include <Arduino.h>

#include "Logger.h"

Buzzer& Buzzer::getInstance() {
    static Buzzer instance;
    return instance;
}

void Buzzer::begin(int pin) {
    buzzerPin = pin;
    pinMode(buzzerPin, OUTPUT);
    lastToggleTime = 0;
    isPlaying = false;
    buzzerState = false;
}

void Buzzer::play() {
    if (isPlaying) {
        return;
    }
    Logger::trace("buzzz");
    isPlaying = true;
    lastToggleTime = millis();
    buzzerState = true;
    digitalWrite(buzzerPin, HIGH);
}

void Buzzer::stop() {
    if (!isPlaying) {
        return;
    }
    isPlaying = false;
    buzzerState = false;
    digitalWrite(buzzerPin, LOW);
    Logger::trace("not buzzing anymore");
}

void Buzzer::update() {
    if (!isPlaying) {
        return;
    }

    unsigned long curTime = millis();

    if (buzzerState && curTime - lastToggleTime > ON_DURATION) {
        digitalWrite(buzzerPin, LOW);
        buzzerState = false;
        lastToggleTime = curTime;
        Logger::trace("buzzer cycle: off");
    } else if (!buzzerState && curTime - lastToggleTime > OFF_DURATION) {
        digitalWrite(buzzerPin, HIGH);
        buzzerState = true;
        lastToggleTime = curTime;
        Logger::trace("buzzer cycle: on");
    }
}
