#include "StorageManager.h"
#include "Logger.h" 

bool StorageManager::init(){
    if(!SPIFFS.begin(true)){
        Logger::trace("error SPIFFS");
        return;
    }
}


