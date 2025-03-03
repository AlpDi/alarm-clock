//TODO All
#include "Api.h"
#include <ArduinoJson.h>
#include "AlarmManager.h"
#include "SPIFFS.h"





AlarmWebServer::AlarmWebServer()
    : server(80){
        setupRoutes();
    }

void AlarmWebServer::begin(){
    server.begin();
}

void AlarmWebServer::handleClient(){
    server.handleClient();
}

void AlarmWebServer::handleGetAlarms(){
    setCorsHeaders();
    std::string json = AlarmManager::getInstance().getAlarmsAsJson();
    server.send(200, "application/json", json.c_str());
}

void AlarmWebServer::handleAddAlarm(){
    setCorsHeaders();

    if(server.hasArg("plain")){
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));

        if(!error){
            AlarmManager::getInstance().addAlarm(doc["days"], doc["hour"], doc["minute"], doc["enabled"]);
            server.send(200, "application/json", "{\"status\":\"success\"}");

            AlarmManager::getInstance().saveToStorage();
        }
    }
    
}

void AlarmWebServer::handleDeleteAlarm(){
    if(server.method() == HTTP_OPTIONS) {
        server.send(204);
        return;
    }

    if(!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
        return;
    }

    String data = server.arg("plain");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);

    if(error){
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        return;
    }

    if(!doc.containsKey("ids")) {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing ids field\"}");
        return;
    }

  JsonArray idsArray = doc["ids"];
  if(idsArray.size() == 0) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No ids provided\"}");
    return;
  }

  for(JsonVariant id : idsArray) {
    AlarmManager::getInstance().deleteAlarm(id.as<int>());
  }

    AlarmManager::getInstance().saveToStorage();
  
  server.send(200, "application/json", "{\"status\":\"success\"}");
}

void AlarmWebServer::handleToggleAlarm() {
    setCorsHeaders();
    
    if(!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if(error) {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        return;
    }

    if(!doc.containsKey("id") || !doc.containsKey("enabled")) {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing id or enabled field\"}");
        return;
    }

    int id = doc["id"];
    bool enabled = doc["enabled"];

    if(enabled) {
        AlarmManager::getInstance().enableAlarm(id);
    } else {
        AlarmManager::getInstance().disableAlarm(id);
    }

    AlarmManager::getInstance().saveToStorage();
    server.send(200, "application/json", "{\"status\":\"success\"}");
}

void AlarmWebServer::handleOptions(){
    setCorsHeaders();
    server.send(204);
}


void AlarmWebServer::setupRoutes(){

    server.on("/", HTTP_GET, [this]() { this->handleRoot();});
    server.on("/styles.css", HTTP_GET, [this]() { this->handleStyles(); });
    server.on("/alarms", HTTP_GET, [this]() { this->handleGetAlarms(); });
    server.on("/alarms", HTTP_POST, [this]() { this->handleAddAlarm(); });
    server.on("/alarms/delete", HTTP_POST, [this]() { this->handleDeleteAlarm(); });
    server.on("/alarms/toggle", HTTP_POST, [this]() { this->handleToggleAlarm(); });

    server.on("/", HTTP_OPTIONS, [this]() { this->handleOptions(); });
    server.on("/alarms", HTTP_OPTIONS, [this]() { this->handleOptions(); });
    server.on("/alarms/delete", HTTP_OPTIONS, [this]() { this->handleOptions(); });
    server.on("/alarms/toggle", HTTP_OPTIONS, [this]() { this->handleOptions(); });
}


void AlarmWebServer::setCorsHeaders() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void AlarmWebServer::handleRoot() {
    File file = SPIFFS.open("/Page.html", "r");
    if (!file) {
        server.send(400, "text/plain", "File not found");
        return;
    }
    
    setCorsHeaders();
    String html = file.readString();
    server.send(200, "text/html", html);
    file.close();
}

void AlarmWebServer::handleStyles() {
    File file = SPIFFS.open("/styles.css", "r");
    if (!file) {
        server.send(400, "text/plain", "File not found");
        return;
    }
    
    setCorsHeaders();
    String css = file.readString();
    server.send(200, "text/css", css);
    file.close();
}