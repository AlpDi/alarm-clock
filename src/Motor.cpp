#include "Motor.h"

#include <Arduino.h>

#include "Logger.h"

Motor& Motor::getInstance() {
    static Motor instance;
    return instance;
}

void Motor::begin(int pin) {
    MotorPin = pin;
    pinMode(MotorPin, OUTPUT);
    lastToggleTime = 0;
    isPlaying = false;
    motorState = false;
}

void Motor::play() {
    if (isPlaying) {
        return;
    }
    Logger::trace("vrommmmm");
    isPlaying = true;
    lastToggleTime = millis();
    motorState = true;
    digitalWrite(MotorPin, HIGH);
}

void Motor::stop() {
    if (!isPlaying) {
        return;
    }
    isPlaying = false;
    motorState = false;
    digitalWrite(MotorPin, LOW);
    Logger::trace("not vrooming anymore");
}

void Motor::update() {
    if (!isPlaying) {
        return;
    }

    unsigned long curTime = millis();

    if (motorState && curTime - lastToggleTime > ON_DURATION) {
        digitalWrite(MotorPin, LOW);
        motorState = false;
        lastToggleTime = curTime;
        Logger::trace("Motor cycle: off");
    } else if (!motorState && curTime - lastToggleTime > OFF_DURATION) {
        digitalWrite(MotorPin, HIGH);
        motorState = true;
        lastToggleTime = curTime;
        Logger::trace("Motor cycle: on");
    }
}
