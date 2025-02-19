#pragma once
#include <ArduinoJson.h>
#include <atomic>

class Alarm {
public:
     Alarm(uint8_t d, int h, int m, bool e = true);
     void toJSON(JsonObject& json) const;
     static Alarm fromJSON(const JsonObject& json);
     bool operator==(int otherId) const {return id == otherId;}


    int getId() const { return id; }
    uint8_t getDays() const { return days; }
    int getHour() const { return hour; }
    int getMinute() const { return minute; }
    bool isEnabled() const { return enabled; }

private:
    int id;
    uint8_t days;
    int hour;
    int minute;
    bool enabled;
    static std::atomic<int> s_id;
};
