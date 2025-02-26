#include "Logger.h"
#include "StorageManager.h"
#include "AlarmManager.h"
#include "Api.h"
#include "secrets.h"

#define HOSTNAME "alarmclock"

void setup(){
    Logger::init();
}