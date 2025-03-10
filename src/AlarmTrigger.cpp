#include "AlarmTrigger.h"

#include "Logger.h"

AlarmTrigger& AlarmTrigger::getInstance() {
    static AlarmTrigger instance;
    return instance;
}

void AlarmTrigger::checkAlarms() {
    auto& alarmManager = AlarmManager::getInstance();
    const auto& alarms = alarmManager.getAlarms();

    time_t now = time(nullptr);

    auto it = snoozedAlarms.begin();
    while (it != snoozedAlarms.end()) {
        if (now >= it->second) {
            int alarmId = it->first;
            it = snoozedAlarms.erase(it);

            for (const auto& alarm : alarms) {
                if (alarm.getId() == alarmId && alarm.isEnabled()) {
                    if (onAlarmTriggered) {
                        Logger::trace("Snoozed alarm %d reactivated", alarmId);
                        onAlarmTriggered(alarm);
                    }
                    markAlarmActive(alarmId);
                    Logger::trace("breakpoint");
                    break;
                }
            }
        } else {
            ++it;
        }
    }

    for (const auto& alarm : alarms) {
        Logger::trace("AlarmId: %d enabled: %d", alarm.getId(), alarm.isEnabled());
        if (isAlarmActive(alarm.getId()) || isAlarmSnoozed(alarm.getId())) {
            continue;
        }
        if (alarm.isEnabled() && TimeManager::isTimeToTriggerAlarm(alarm.getDays(), alarm.getHour(),
                                                                   alarm.getMinute())) {
            Logger::trace("Alarm triggered: %d\n", alarm.getId());

            if (onAlarmTriggered) {
                onAlarmTriggered(alarm);
            }
            markAlarmActive(alarm.getId());
        }
    }

    auto activeIt = activeAlarms.begin();
    while (activeIt != activeAlarms.end()) {
        bool shouldRemain = false;

        for (const auto& alarm : alarms) {
            if (alarm.getId() == *activeIt && alarm.isEnabled() &&
                TimeManager::isTimeToTriggerAlarm(alarm.getDays(), alarm.getHour(),
                                                  alarm.getMinute())) {
                shouldRemain = true;
                break;
            }
        }

        if (!shouldRemain) {
            Logger::trace("Alarm %d no longer active", *activeIt);
            for (const auto& alarm : alarms) {
                if (alarm.getId() == *activeIt && alarm.getDays() & 0b10000000) {
                    AlarmManager::getInstance().disableAlarm(alarm.getId());
                    AlarmManager::getInstance().saveToStorage();
                    Logger::trace("One-time Alarm %d disabled after triggering", alarm.getId());
                }
            }
            activeIt = activeAlarms.erase(activeIt);
        } else {
            ++activeIt;
        }
    }
}

void AlarmTrigger::stopAlarm(int alarmId) {
    clearAlarmActive(alarmId);

    auto it = std::find_if(
        snoozedAlarms.begin(), snoozedAlarms.end(),
        [alarmId](const std::pair<int, time_t> pair) { return pair.first == alarmId; });
    if (it != snoozedAlarms.end()) {
        snoozedAlarms.erase(it);
    }
    Logger::trace("Alarm %d stopped\n", alarmId);
}

void AlarmTrigger::snoozeAlarm(int alarmId, int snoozeMinutes) {
    clearAlarmActive(alarmId);

    time_t snoozeUntil = time(nullptr) + snoozeMinutes * 60;

    auto it = std::find_if(
        snoozedAlarms.begin(), snoozedAlarms.end(),
        [alarmId](const std::pair<int, time_t> pair) { return pair.first == alarmId; });

    if (it != snoozedAlarms.end()) {
        it->second = snoozeUntil;
    } else {
        snoozedAlarms.emplace_back(alarmId, snoozeUntil);
    }
    Logger::trace("Alarm %d snoozed for %d minutes", alarmId, snoozeMinutes);
}

bool AlarmTrigger::isAlarmActive(int alarmId) const {
    return std::find(activeAlarms.begin(), activeAlarms.end(), alarmId) != activeAlarms.end();
}

void AlarmTrigger::markAlarmActive(int alarmId) {
    if (!isAlarmActive(alarmId)) {
        activeAlarms.push_back(alarmId);
    }
}

void AlarmTrigger::clearAlarmActive(int alarmId) {
    auto it = std::find(activeAlarms.begin(), activeAlarms.end(), alarmId);
    if (it != activeAlarms.end()) {
        activeAlarms.erase(it);
    }
}

bool AlarmTrigger::isAlarmSnoozed(int alarmId) const {
    return std::find_if(snoozedAlarms.begin(), snoozedAlarms.end(),
                        [alarmId](const std::pair<int, time_t> pair) {
                            return pair.first == alarmId;
                        }) != snoozedAlarms.end();
}

time_t AlarmTrigger::getSnoozedUntil(int alarmId) const {
    auto it = std::find_if(
        snoozedAlarms.begin(), snoozedAlarms.end(),
        [alarmId](const std::pair<int, time_t> pair) { return pair.first == alarmId; });
    return (it != snoozedAlarms.end()) ? it->second : 0;
}

std::vector<int> AlarmTrigger::getActiveAlarms() { return activeAlarms; }
