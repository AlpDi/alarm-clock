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

#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"

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

};
std::atomic<int> Alarm::s_id;

vector<Alarm> alarms;

void addAlarm(uint8_t d, int h, int m, bool e){
  alarms.push_back(Alarm(d, h, m, e));
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

//-----------WebServer Setup----------------

WebServer server(80);

void handleGetAlarms(){
  string json = getAlarmsAsJson();
  server.send(200, "application/json", json.c_str());
}


void handleAddAlarm(){
  server.sendHeader("Access-Control-Allow-Origin", "*");
  if (server.hasArg("plain")){
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if(!error){
      addAlarm(doc["days"], doc["hour"], doc["minutes"], doc["enabled"]);
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
    String html = file.readString();
    server.send(200, "text/html", html);
    file.close();
  });

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

  server.begin();

}

void loop() {
  server.handleClient();
}