#include "AlarmManager.h"
#include "AlarmTrigger.h"
#include "Api.h"
#include "Buzzer.h"
#include "Logger.h"
#include "Motor.h"
#include "StorageManager.h"
#include "TimeManager.h"
#include "WifiManager.h"
#include "buttonManager.h"
#include "secrets.h"

#define HOSTNAME "alarmclock"

AlarmWebServer server;

AlarmTrigger& trigger = AlarmTrigger::getInstance();

void setup() {
    TimeManager::init();

    Buzzer::getInstance().begin(26);

    Motor::getInstance().begin(19);

    Logger::init();

    if (!StorageManager::init()) {
        Logger::trace("Failed to initialize storage");
        return;
    }

    if (!WifiManager::connect(ssid, passPhrase)) {
        Logger::trace("Failed to connect to WiFi");
        return;
    }

    buttonManager::getInstance().begin(12, 13, 14);

    Logger::trace("Connected to WiFi");

    AlarmManager::getInstance().loadFromStorage();

    server.begin();

    trigger.setTriggerCallback([](const Alarm& alarm) {
        Logger::trace("Trigger callback executing for alarm %d", alarm.getId());
        Buzzer::getInstance().play();
        Motor::getInstance().play();
    });

    Logger::trace("Alarm trigger callback set");
}
unsigned long previousMillis = 0;
const long interval = 1000;

void loop() {
    server.handleClient();
    buttonManager::getInstance().handleSimButtonPress();
    buttonManager::getInstance().handleStopButtonPress();
    buttonManager::getInstance().handleSnoozeButtonPress();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        trigger.checkAlarms();
        // Logger::trace("checking alarms");

        if (trigger.getActiveAlarms().empty()) {
            Buzzer::getInstance().stop();
            Motor::getInstance().stop();
        }
    }
    Motor::getInstance().update();
    Buzzer::getInstance().update();
}
