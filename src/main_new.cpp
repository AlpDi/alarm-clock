#include "Logger.h"
#include "StorageManager.h"
#include "AlarmManager.h"
#include "Api.h"
#include "secrets.h"
#include "WifiManager.h"
#include "Buzzer.h"
#include "AlarmTrigger.h"
#include "TimeManager.h"


#define HOSTNAME "alarmclock"

AlarmWebServer server;

AlarmTrigger& trigger = AlarmTrigger::getInstance();
Buzzer& buzzer = Buzzer::getInstance();

void setup(){
    TimeManager::init();
    
    buzzer.begin(26);

    Logger::init();

    if(!StorageManager::init()){
        Logger::trace("Failed to initialize storage");
        return;
    }

    if(!WifiManager::connect(ssid, passPhrase)){
        Logger::trace("Failed to connect to WiFi");
        return;
    }

    

    Logger::trace("Connected to WiFi");

    AlarmManager::getInstance().loadFromStorage();

    server.begin();


    trigger.setTriggerCallback([](const Alarm& alarm){
        Logger::trace("Alarm triggered");
        buzzer.play();
    });
}
unsigned long previousMillis = 0;
const long interval = 1000;

void loop(){
    server.handleClient();

    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= interval){
        previousMillis = currentMillis;
        trigger.checkAlarms();
        Logger::trace("checking alarms");
    }
}
