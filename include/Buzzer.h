#pragma once

class Buzzer{
public:
    static Buzzer& getInstance();

    void begin(int pin);
    void play();
    void stop();

private:
    Buzzer() = default;
    int buzzerPin;
    bool isPlaying = false;
    
};