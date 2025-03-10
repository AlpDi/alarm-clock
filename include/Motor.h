#pragma once

class Motor {
   public:
    static Motor& getInstance();

    void begin(int pin);
    void play();
    void stop();
    void update();

   private:
    Motor() = default;
    int MotorPin;
    bool isPlaying = false;
    bool motorState = false;
    unsigned long lastToggleTime = 0;

    static const unsigned long ON_DURATION = 4000;
    static const unsigned long OFF_DURATION = 1000;
};