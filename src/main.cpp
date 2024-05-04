#include <Arduino.h>
#include "WiFi.h"
#include "SPIFFS.h"
#include "Arduino_JSON.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
void initSPIFFS() {
  if (SPIFFS.begin(true) == false) {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}
JSONVar readings;

String getSensorReadings(){
//宣告這個函式回傳的種類是什麽
    
  readings["temperature"] = String(dht.readTemperature(false));
  // 設定變數和數值
    
  readings["humidity"] =  String(dht.readHumidity());
  //設定變數和數值
    
  String jsonString = JSON.stringify(readings);
  //將這個dictionary轉換成一個字串，這樣才能把數值正確回傳給函式
    
  return jsonString;
  //回傳 string
}



AsyncEventSource events("/events");
AsyncWebServer server(80);


void setup() {
  dht.begin();
  initSPIFFS();
  Serial.begin(9600);
  server.serveStatic("/", SPIFFS, "/");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin("NKFW2024", "nkfw2024");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  // put your setup code here, to run once:
    server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    // "application"
    request->send(200, "application/json", json);
    json = String();
  });

  server.addHandler(&events);
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  events.send(getSensorReadings().c_str(),"new_readings" ,millis());
}