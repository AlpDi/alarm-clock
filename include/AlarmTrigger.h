#pragma once
#include "TimeManager.h"
#include "AlarmManager.h"

class AlarmTrigger{
public:
    using TriggerCallback = std::function<void(const Alarm&)>;

    static AlarmTrigger& getInstance();

    void setTriggerCallback(TriggerCallback callback){
        onAlarmTriggered = callback;
    }
    void checkAlarms();
    void snoozeAlarm(int alarmId);
    void stopAlarm(int alarmId);

private:
    AlarmTrigger() = default;
    TriggerCallback onAlarmTriggered;
    std::vector<int> activeAlarms;
    std::vector<std::pair<int,time_t>> snoozedAlarms;

    bool shouldTriggerAlarm(const Alarm& alarm, const TimeManager::TimeInfo& currentTime);
    bool isAlarmActive(int alarmId) const;
    void markAlarmActive(int alarmId);
    void clearAlarmActive(int alarmId);
    bool isAlarmSnoozed(int alarmId) const;
    time_t getSnoozedUntil(int alarmId) const;
};