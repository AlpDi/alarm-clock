#pragma once
#include <vector>
#include "Alarm.h"

class AlarmManager {
public:
    static AlarmManager& getInstance();
    
    void addAlarm(uint8_t days, int hour, int minute, bool enabled);
    void deleteAlarm(int id);
    void deleteAlarms(const std::vector<int>& ids);
    const std::vector<Alarm>& getAlarms() const;
    std::string getAlarmsAsJson() const;
    
    void loadFromStorage();
    void saveToStorage();

    void disableAlarm(int id);
    void enableAlarm(int id);

private:
    AlarmManager() = default;
    std::vector<Alarm> alarms;

};