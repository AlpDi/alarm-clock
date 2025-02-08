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


    // takes reference of json object and filled with data
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
  StaticJsonDocument<1024> doc;
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
  if(SPIFFS.exists("/alarms.json")){
    TRACE("No Alarms found");
    return;
  }

  File file = SPIFFS.open("/alarms.json", "r");
  if(!file){
    TRACE("Failed to open file for reading");
    return;
  }
  StaticJsonDocument<1024> doc;
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

void handleGetAlarms(){
  string json = getAlarmsAsJson();
  server.send(200, "application/json", json.c_str());
  TRACE(json.c_str());
}

void handleOptions() {
  // Handle preflight OPTIONS request for CORS
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);  // No content response
}
void handleAddAlarm(){
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  if (server.hasArg("plain")){
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if(!error){
      addAlarm(doc["days"], doc["hour"], doc["minute"], doc["enabled"]);
      server.send(200, "application/json", "{\"status\":\"success\"}");
    }
  }
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

/*   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  addAlarm(0b00000001, 12,30,true);

  display.println(getAlarmsAsJson().c_str());
  display.display();  */

  //----------------initialize server------------
  if(!SPIFFS.begin(true)){
    Serial.println("error SPIFFS");
     return;
  }

  addAlarm(0b00000001, 12, 30, true);
  addAlarm(0b00110001, 13, 40, true);
  addAlarm(0b10101010, 14, 50, true);
  addAlarm(0b11111111, 15, 0, true);


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
    server.sendHeader("Access-Control-Allow-Origin", "*");  // Allow all origins
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
    String css = file.readString();
    server.send(200, "text/css", css);
  });

    server.on("/alarms", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204); // Send success with no content
});

  server.on("/alarms", HTTP_GET, handleGetAlarms);

    server.on("/alarms", HTTP_POST, handleAddAlarm);

  server.enableCORS(true);

  server.begin();

}

void loop() {
  server.handleClient();
}

//todo alarm logic