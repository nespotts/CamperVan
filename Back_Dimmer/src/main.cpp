#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ArduinoOTA.h>
#include "OTA.h"

#include <ESP8266HTTPClient.h>
// #include <WiFiClient.h>

// #include <ESP8266WiFiMulti.h>
// ESP8266WiFiMulti WiFiMulti;

//Your IP address or domain name with URL path
const char* serverNameBrightness = "http://192.168.8.158/brightness";
const char* serverNameEntry = "http://192.168.8.158/illuminated_entry";

long requesttimer = 0;
long request_interval = 1000;
long timeout = 200;

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
int ledpin = D6;
int min_brightness = 0;
int max_brightness = 1023; // max = 1023

String brightness;
String illum_entry;
int illum_entry_state;
// int ilum_entry_pin = D6;
int entry_brightness = 150;

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


#include "functions.h"
#include "wifi.h"

void setup() {
  Serial.begin(115200);
  pinMode(potpin, INPUT);
  pinMode(ledpin, OUTPUT); 
  currenttime = millis();
  ESP_timer = millis();
  analogWriteFreq(20);
  // OTA_Functions();
  // ConnectWifi();
  // printWifiStatus();
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
  // } else {
    
  //   if ((currenttime - requesttimer) >= request_interval) {
  //     // brightness = httpGETRequest(serverNameBrightness);
  //     illum_entry = httpGETRequest();

  //     // Serial.println("Brightness: " + brightness + " - Entry State: " + illum_entry);
  //     requesttimer = currenttime;
  //   }
  // }

}





