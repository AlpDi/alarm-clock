#pragma once
#include <time.h>

class TimeManager {
public: 
    struct TimeInfo{
        int hour;
        int minute;
        int second;
        int weekDay;
    };
    static void init();
    static TimeInfo getCurrentTime();
    static bool compareTime(const TimeInfo& time1, const TimeInfo& time2);
    static bool isTimeToTriggerAlarm(uint8_t alarmDays, int alarmHour, int alarmMinute);
};

