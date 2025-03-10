#pragma once

class Buzzer {
   public:
    static Buzzer& getInstance();

    void begin(int pin);
    void play();
    void stop();
    void update();

   private:
    Buzzer() = default;
    int buzzerPin;
    bool isPlaying = false;
    bool buzzerState = false;
    unsigned int lastToggleTime = 0;

    static const unsigned long ON_DURATION = 1000;
    static const unsigned long OFF_DURATION = 1000;
};