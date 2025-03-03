#include "Buzzer.h"
#include <Arduino.h>

Buzzer& Buzzer::getInstance(){
    static Buzzer instance;
    return instance;
}

void Buzzer::begin(int pin){
    buzzerPin = pin;
    pinMode(buzzerPin, OUTPUT);
}

void Buzzer::play(){
    if(isPlaying){
        return;
    }

    isPlaying = true;
    digitalWrite(buzzerPin, HIGH);
}

void Buzzer::stop(){
    if(!isPlaying){
        return;
    }
    isPlaying = false;
    digitalWrite(buzzerPin, LOW);   
}

