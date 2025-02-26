#include "StorageManager.h"
#include "Logger.h" 

bool StorageManager::init(){
    if(!SPIFFS.begin(true)){
        return false;
    }
    else return true;
}


