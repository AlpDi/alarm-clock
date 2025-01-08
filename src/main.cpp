#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>
#include <vector>
#include <atomic>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
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
  display.display(); 
}

void loop() {
  
}