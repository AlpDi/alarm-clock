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

    buzzer.begin(23);

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

void loop(){
    server.handleClient();
}
