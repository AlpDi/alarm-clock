#pragma once
#include <SPIFFS.h>

class StorageManager {
public:
    static bool init();
    static bool writeFile(const char* path, const std::string& content);
    static std::string readFile(const char* path);
    static bool exists(const char* path);
};