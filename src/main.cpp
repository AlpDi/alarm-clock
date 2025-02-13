#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>
#include <vector>
#include <atomic>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>


#include "secrets.h"

#define HOSTNAME "alarmclock"

#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3" //Berlin timezone

#define TRACE(...) Serial.printf(__VA_ARGS__)


//----------Alarm Logic Setup--------------


using std::string;
using std::vector;

class Alarm {
  public:
    int id;
    uint8_t days;
    int hour;
    int minute;
    bool enabled;


    Alarm(uint8_t d, int h, int m, bool e = true)
      : days(d), hour(h), minute(m), enabled(e), id(++s_id){}



    void toJSON(JsonObject& json){
      json["id"] = id;
      json["days"] = days;
      json["hour"] = hour;
      json["minute"] = minute;
      json["enabled"] = enabled;
    }

  protected:
    static std::atomic<int> s_id;

    static void setIdCounter(int value){
      s_id = value;
    }
    
    static int getIdCounter(){
      return s_id;
    }

};
std::atomic<int> Alarm::s_id;

vector<Alarm> alarms;

void addAlarm(uint8_t d, int h, int m, bool e){
  alarms.push_back(Alarm(d, h, m, e));
  TRACE("%u,%d,%d,%i",d,h,m,e);
}

//Creates JsonDocument / JsonArray, jsonifies all Alarm Objects and stores them in alarmsArray, then returns the serialized jsonArray
string getAlarmsAsJson(){
  JsonDocument doc;
  JsonArray alarmsArray = doc.to<JsonArray>();

  for (auto& alarm : alarms){
    JsonObject alarmJson = alarmsArray.add<JsonObject>();
    alarm.toJSON(alarmJson);
  }

  string output;
  serializeJson(doc, output);
  return output;
}

//-----------Pull Push Modify alarms on Spiff--------------------


void saveAlarmsToSpiffs(){
  JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();

  for(Alarm &alarm : alarms){
    JsonObject obj = array.createNestedObject();
    alarm.toJSON(obj);
  }

  File file = SPIFFS.open("/alarms.json", "w");
  if(!file){
    TRACE("failed to open file for writing");
    return;
  }

  serializeJson(doc, file);
  file.close();
  TRACE("Alarm saved to SPIFFS");
}


void loadAlarmsFromSpiffs(){
  if(!SPIFFS.exists("/alarms.json")){
    TRACE("No Alarms found");
    return;
  }

  File file = SPIFFS.open("/alarms.json", "r");
  if(!file){
    TRACE("Failed to open file for reading");
    return;
  }
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if(error){
    TRACE("failed to parse json");
    file.close();
    return;
  }

  alarms.clear();

  for (JsonObject obj : doc.as<JsonArray>()){
    addAlarm(obj["days"], obj["hour"], obj["minute"], obj["enabled"]);
  }

  file.close();

  return;
}

//-----------WebServer Setup----------------

WebServer server(80);

void setCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}


void handleGetAlarms(){
  setCorsHeaders();
  string json = getAlarmsAsJson();
  server.send(200, "application/json", json.c_str());
}

void handleOptions() {
  setCorsHeaders();
  server.send(204);
}
void handleAddAlarm(){
  setCorsHeaders();
  
  if (server.hasArg("plain")){
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if(!error){
      addAlarm(doc["days"], doc["hour"], doc["minute"], doc["enabled"]);
      server.send(200, "application/json", "{\"status\":\"success\"}");
    }
  }
  saveAlarmsToSpiffs();
}

void handleDeleteAlarm() {
  setCorsHeaders();
  
  TRACE("Delete alarm request received\n");

  if(server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }

  if(!server.hasArg("plain")) {
    TRACE("No plain argument found\n");
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
    return;
  }

  String data = server.arg("plain");
  TRACE("Received data: %s\n", data.c_str()); 

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);

  if(error) {
    TRACE("JSON parsing failed: %s\n", error.c_str()); 
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    return;
  }

  if(!doc.containsKey("ids")) {
    TRACE("No ids field in JSON\n"); 
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing ids field\"}");
    return;
  }

  JsonArray idsArray = doc["ids"];
  if(idsArray.size() == 0) {
    TRACE("Empty ids array\n"); 
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No ids provided\"}");
    return;
  }

  for(JsonVariant id : idsArray) {
    TRACE("Processing id: %d\n", id.as<int>()); 
    auto it = std::find_if(alarms.begin(), alarms.end(), 
      [id](const Alarm &obj) { return obj.id == id.as<int>(); });
    
    if(it != alarms.end()) {
      alarms.erase(it);
      TRACE("Alarm deleted: %d\n", id.as<int>()); 
    } else {
      TRACE("Alarm not found: %d\n", id.as<int>()); 
    }
  }

  saveAlarmsToSpiffs();
  server.send(200, "application/json", "{\"status\":\"success\"}");
}




//-----------Display Setup------------------
/* #define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); */


void setup() {
  //wait for serial monitor
  delay(3000);

  Serial.begin(115200);


  //----------------initialize server------------
  if(!SPIFFS.begin(true)){
    Serial.println("error SPIFFS");
     return;
  }

  loadAlarmsFromSpiffs();


  WiFi.setHostname(HOSTNAME);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passPhrase);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    TRACE(".");
  }

  TRACE("connected.");
  Serial.println(WiFi.localIP());

  TRACE("Setup ntp...\n");
  configTzTime(TIMEZONE, "pool.ntp.org");


  server.on("/", [](){
    File file = SPIFFS.open("/Page.html", "r");
    if (!file){
      server.send(400, "text/plain", "File not found");
      return;
    }
    server.sendHeader("Access-Control-Allow-Origin", "*"); 
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");


    String html = file.readString();
    server.send(200, "text/html", html);
    file.close();
  });

  server.on("/", HTTP_OPTIONS, handleOptions);

  server.on("/styles.css", HTTP_GET, [](){
    File file = SPIFFS.open("/styles.css", "r");
    if (!file){
      server.send(400, "text/plain", "File not found");
      return;
    }
    setCorsHeaders();
    String css = file.readString();
    server.send(200, "text/css", css);
  });


  server.on("/", HTTP_OPTIONS, handleOptions);
  server.on("/alarms", HTTP_OPTIONS, handleOptions);
  server.on("/alarms/delete", HTTP_OPTIONS, [](){
    setCorsHeaders();
    server.send(204);
  }); 


  server.on("/alarms", HTTP_GET, handleGetAlarms);
  server.on("/alarms", HTTP_POST, handleAddAlarm);
  server.on("/alarms/delete", HTTP_POST, handleDeleteAlarm);

  server.begin();

}

void loop() {
  server.handleClient();
}

//todo alarm logic