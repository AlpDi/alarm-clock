#include "TimeManager.h"
#include "config.h"
#include "Arduino.h"
#include <time.h>

void TimeManager::init(){
    configTzTime(timezone, "pool.ntp.org");
}

TimeManager::TimeInfo TimeManager::getCurrentTime(){
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    TimeInfo result;
    result.hour = timeinfo.tm_hour;
    result.minute = timeinfo.tm_min;
    result.second = timeinfo.tm_sec;
    result.weekDay = timeinfo.tm_wday;

    return result;
}

bool TimeManager::compareTime(const TimeInfo& time1, const TimeInfo& time2){
    return(time1.hour == time2.hour && time1.minute == time2.minute);
}

bool TimeManager::isTimeToTriggerAlarm(uint8_t alarmDays, int alarmHour, int alarmMinute){
    TimeInfo currentTime = getCurrentTime();

    if(alarmDays & 0b10000000){
        return(currentTime.hour == alarmHour && 
               currentTime.minute == alarmMinute);
    }

    bool isCorrectDay = (alarmDays & (1 << currentTime.weekDay)) != 0;

    if(!isCorrectDay){
        return false;
    }

    return(currentTime.hour == alarmHour && 
           currentTime.minute == alarmMinute);
}
