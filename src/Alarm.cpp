#include "Alarm.h"

std::atomic<int> Alarm::s_id(0);

Alarm::Alarm(uint8_t d, int h, int m, bool e)
      : days(d), hour(h), minute(m), enabled(e), id(++s_id){}

void Alarm::toJSON (JsonObject& json)const{
    json["id"] = id;
    json["days"] = days;
    json["hour"] = hour;
    json["minute"] = minute;
    json["enabled"] = enabled;
}

Alarm Alarm::fromJSON(const JsonObject& json){

}

void Alarm::setEnabled(bool val){
    enabled = val;
}

