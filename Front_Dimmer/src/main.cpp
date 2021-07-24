#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ArduinoOTA.h>
#include "OTA.h"


#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// *************************** WiFi Setup **********************************************
// Campervan WiFi
#define wifi_ssid  "GL-AR750S-bf8"
#define wifi_pass  "goodlife"
// Phone Hotspot
// #define wifi_ssid "natespott"
// #define wifi_pass "Nspotts42194"

int wifiTimeout = 8000;

// *************************************************************************************

int potpin = A0;
int ledpin = D5;
int min_brightness = 0;
int max_brightness = 512; // max = 1023

int ilum_entry_pin = D6;
int entry_brightness = 150;
int entry_state;

int pot_index = 0;
int potval = 0;
int potvals[200];
double num_vals = 200;
double pot_sum = 0;
long count = 0;
int led_brightness = 0;

long currenttime = 0;
long ESP_timer = 0;
long ESP_reset_threshold = 1000; //millis

#include "wifi.h"
#include "functions.h"


String readBright() {
  return String(potval);
}

String readEntry() {
  return String(entry_state);
}

void setup() {
  Serial.begin(115200);
  pinMode(potpin, INPUT);
  pinMode(ledpin, OUTPUT); 
  currenttime = millis();
  ESP_timer = millis();
  // OTA_Functions();
  // ConnectWifi();
  // printWifiStatus();

  // server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send_P(200, "text/plain", readBright().c_str());
  // });
  // server.on("/illuminated_entry", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send_P(200, "text/plain", readEntry().c_str());
  // });

  // // Start server
  // server.begin();
}


void loop() {
  // ArduinoOTA.handle();
  currenttime = millis();
  potval = analogRead(potpin);

  Calc_Brightness();
  Control_Light();

  // if (WiFi.status() != WL_CONNECTED) {
  //   ConnectWifi();
  //   printWifiStatus();
  // }
}