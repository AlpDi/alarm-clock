#include "AlarmManager.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "Logger.h"
#include "TimeManager.h"

AlarmManager& AlarmManager::getInstance(){
    static AlarmManager instance;
    return instance;
}

const std::vector<Alarm>& AlarmManager::getAlarms() const{
  return alarms;
}

void AlarmManager::addAlarm(uint8_t days, int hour, int minute, bool enabled){
    alarms.push_back(Alarm(days,hour,minute,enabled));
}

void AlarmManager::deleteAlarm(int id){
    auto it = std::find_if(alarms.begin(), alarms.end(), 
    [id](const Alarm &obj) {return obj.getId() == id;});

    if(it != alarms.end()){
        alarms.erase(it);
    }
}

void AlarmManager::deleteAlarms(const std::vector<int>& ids){
    for(int id : ids){
        deleteAlarm(id);
    }
}

std::string AlarmManager::getAlarmsAsJson() const{
    JsonDocument doc;
    JsonArray alarmsArray = doc.to<JsonArray>();

    for(auto& alarm : alarms){
        JsonObject alarmJson = alarmsArray.add<JsonObject>();
        alarm.toJSON(alarmJson);
    }

    std::string output;
    serializeJson(doc, output);
    return output;
}

void AlarmManager::loadFromStorage(){
      if(!SPIFFS.exists("/alarms.json")){
    Logger::trace("No Alarms found");
    return;
  }

  File file = SPIFFS.open("/alarms.json", "r");
  if(!file){
    Logger::trace("Failed to open file for reading");
    return;
  }
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if(error){
    Logger::trace("failed to parse json");
    file.close();
    return;
  }

  alarms.clear();

  for (JsonObject obj : doc.as<JsonArray>()){
    addAlarm(obj["days"], obj["hour"], obj["minute"], obj["enabled"]);
  }

  file.close();

  return;
}

void AlarmManager::saveToStorage(){
    JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();

  for(Alarm &alarm : alarms){
    JsonObject obj = array.createNestedObject();
    alarm.toJSON(obj);
  }

  File file = SPIFFS.open("/alarms.json", "w");
  if(!file){
    Logger::trace("failed to open file for writing");
    return;
  }

  serializeJson(doc, file);
  file.close();
  Logger::trace("Alarm saved to SPIFFS");
}

void AlarmManager::disableAlarm(int id){
    auto it = std::find_if(alarms.begin(), alarms.end(), 
    [id](const Alarm &obj) {return obj.getId() == id;});

    if(it != alarms.end()){
          it->setEnabled(false);
    }
}

void AlarmManager::enableAlarm(int id){
    auto it = std::find_if(alarms.begin(), alarms.end(), 
    [id](const Alarm &obj) {return obj.getId() == id;});

    if(it != alarms.end()){
        it->setEnabled(true);
    }
}

void AlarmManager::simulateAlarm(){
  TimeManager::TimeInfo currTime = TimeManager::getCurrentTime();
  addAlarm(128, currTime.hour, currTime.minute, true);
}

