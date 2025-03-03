#pragma once
#include <WebServer.h>
#include "AlarmManager.h"

class AlarmWebServer{
public:
    AlarmWebServer();
    void begin();
    void handleClient();

private:
    WebServer server;
    void setupRoutes();
    void setCorsHeaders();

    void handleGetAlarms();
    void handleAddAlarm();
    void handleDeleteAlarm();
    void handleToggleAlarm();
    void handleOptions();
    void handleRoot();
    void handleStyles();
};