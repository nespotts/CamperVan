void printWifiStatus() {
  ESP.wdtFeed();
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  //  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// Attempt to connect to WiFi
void ConnectWifi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WiFi.SSID());
  WiFi.persistent(true);  // May be able to set to true disables automatic saving of wifi credentials
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pass);

  int t1 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    int t2 = millis();
    ESP.wdtFeed();
    delay(50);
    Serial.print(".");  
    if ((t2-t1) > wifiTimeout) {
      ESP.wdtDisable();
      while (true){};
    }
    yield();
  }
}




// // Import required libraries
// #include <ESP8266WiFi.h>
// #include "ESPAsyncWebServer.h"

// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>

// // Set your access point network credentials
// const char* ssid = "ESP8266-Access-Point";
// const char* password = "123456789";

// /*#include <SPI.h>
// #define BME_SCK 18
// #define BME_MISO 19
// #define BME_MOSI 23
// #define BME_CS 5*/

// Adafruit_BME280 bme; // I2C
// //Adafruit_BME280 bme(BME_CS); // hardware SPI
// //Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

// // Create AsyncWebServer object on port 80
// AsyncWebServer server(80);

// String readTemp() {
//   return String(bme.readTemperature());
//   //return String(1.8 * bme.readTemperature() + 32);
// }

// String readHumi() {
//   return String(bme.readHumidity());
// }

// String readPres() {
//   return String(bme.readPressure() / 100.0F);
// }

// void setup(){
//   // Serial port for debugging purposes
//   Serial.begin(115200);
//   Serial.println();
  
//   // Setting the ESP as an access point
//   Serial.print("Setting AP (Access Point)…");
//   // Remove the password parameter, if you want the AP (Access Point) to be open
//   WiFi.softAP(ssid, password);

//   IPAddress IP = WiFi.softAPIP();
//   Serial.print("AP IP address: ");
//   Serial.println(IP);

//   server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
//     request->send_P(200, "text/plain", readTemp().c_str());
//   });
  
//   bool status;

//   // default settings
//   // (you can also pass in a Wire library object like &Wire2)
//   status = bme.begin(0x76);  
//   if (!status) {
//     Serial.println("Could not find a valid BME280 sensor, check wiring!");
//     while (1);
//   }
  
//   // Start server
//   server.begin();
// }
 
// void loop(){

// }