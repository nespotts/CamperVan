// Eliminate Some Noise
void Calc_Brightness() {
  ESP.wdtFeed();
  yield();
  potvals[pot_index] = potval;
  for(int i=0; i < num_vals; i++) {
    pot_sum += potvals[i];
  }
  led_brightness = pot_sum/num_vals;
  pot_sum = 0;

  if (pot_index < num_vals-1) {
    pot_index++;
  } else {
    pot_index = 0;
  }
}

  // Determine if the illuminated Entry light should turn on
void Control_Light() {

  if (illum_entry == "0") {
    illum_entry_state = 0;
  } else if (illum_entry == "1") {
    illum_entry_state = 1;
  }

  if (illum_entry_state == 1) {
    analogWrite(ledpin, entry_brightness);
    Serial.println("illuminated entry");
  }
  // Otherwise Write the Dimmer Brightness to the LED
  else {
    if (led_brightness <= min_brightness) {
      digitalWrite(ledpin, LOW);
    } else {
      Serial.println(map(led_brightness, 0, 1023, min_brightness, max_brightness));
      // Serial.println(potval);
      if (led_brightness < 250) {
          digitalWrite(ledpin,LOW);
      } else if (led_brightness >= 250 && led_brightness < 600) {
        analogWrite(ledpin, map(led_brightness, 250, 600, min_brightness, 25));
      } else {
        analogWrite(ledpin, map(led_brightness, 600, 1023, 25, max_brightness));
      }
    }
  }
}





// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiClient.h>

// #include <ESP8266WiFiMulti.h>
// ESP8266WiFiMulti WiFiMulti;

// const char* ssid = "ESP8266-Access-Point";
// const char* password = "123456789";

// //Your IP address or domain name with URL path
// const char* serverNameTemp = "http://192.168.4.1/temperature";
// const char* serverNameHumi = "http://192.168.4.1/humidity";
// const char* serverNamePres = "http://192.168.4.1/pressure";

// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 64 // OLED display height, in pixels

// // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// #define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// String temperature;
// String humidity;
// String pressure;

// unsigned long previousMillis = 0;
// const long interval = 5000; 

// void setup() {
//   Serial.begin(115200);
//   Serial.println();
  
//   // Address 0x3C for 128x64, you might need to change this value (use an I2C scanner)
//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
//     Serial.println(F("SSD1306 allocation failed"));
//     for(;;); // Don't proceed, loop forever
//   }
//   display.clearDisplay();
//   display.setTextColor(WHITE);
 
//   WiFi.mode(WIFI_STA);
//   WiFiMulti.addAP(ssid, password);
//   while((WiFiMulti.run() == WL_CONNECTED)) { 
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("Connected to WiFi");
// }

// void loop() {
//   unsigned long currentMillis = millis();
  
//   if(currentMillis - previousMillis >= interval) {
//      // Check WiFi connection status
//     if ((WiFiMulti.run() == WL_CONNECTED)) {
//       temperature = httpGETRequest(serverNameTemp);
//       humidity = httpGETRequest(serverNameHumi);
//       pressure = httpGETRequest(serverNamePres);
//       Serial.println("Temperature: " + temperature + " *C - Humidity: " + humidity + " % - Pressure: " + pressure + " hPa");
      
//       display.clearDisplay();
      
//       // display temperature
//       display.setTextSize(2);
//       display.setCursor(0,0);
//       display.print("T: ");
//       display.print(temperature);
//       display.print(" ");
//       display.setTextSize(1);
//       display.cp437(true);
//       display.write(248);
//       display.setTextSize(2);
//       display.print("C");
      
//       // display humidity
//       display.setTextSize(2);
//       display.setCursor(0, 25);
//       display.print("H: ");
//       display.print(humidity);
//       display.print(" %"); 
      
//       // display pressure
//       display.setTextSize(2);
//       display.setCursor(0, 50);
//       display.print("P:");
//       display.print(pressure);
//       display.setTextSize(1);
//       display.setCursor(110, 56);
//       display.print("hPa");
           
//       display.display();
      
//       // save the last HTTP GET Request
//       previousMillis = currentMillis;
//     }
//     else {
//       Serial.println("WiFi Disconnected");
//     }
//   }
// }

// String httpGETRequest(const char* serverName) {
//   WiFiClient client;
//   HTTPClient http;
    
//   // Your IP address with path or Domain name with URL path 
//   http.begin(client, serverName);
  
//   // Send HTTP POST request
//   int httpResponseCode = http.GET();
  
//   String payload = "--"; 
  
//   if (httpResponseCode>0) {
//     Serial.print("HTTP Response code: ");
//     Serial.println(httpResponseCode);
//     payload = http.getString();
//   }
//   else {
//     Serial.print("Error code: ");
//     Serial.println(httpResponseCode);
//   }
//   // Free resources
//   http.end();

//   return payload;
// }





// void Send_Data() {
//     char intToPrint[5];
//   itoa(millis(), intToPrint, 10); //integer to string conversion for OLED library
 
//   // Use WiFiClient class to create TCP connections
//   WiFiClient client;
//   const char * host = "192.168.8.1";
//   const int httpPort = 80;
 
//   if (!client.connect(host, httpPort)) {
//     Serial.println("connection failed");
//     return;
//   }
 
//   // We now create a URI for the request
//   String url = "/data/";
//   url += "?sensor_reading=";
//   url += intToPrint;
 
//   Serial.print("Requesting URL: ");
//   Serial.println(url);
 
//   // This will send the request to the server
//   client.print(String("GET ") + url + " HTTP/1.1\r\n" +
//                "Host: " + host + "\r\n" +
//                "Connection: close\r\n\r\n");
//   unsigned long timeout = millis();
//   while (client.available() == 0) {
//     if (millis() - timeout > 5000) {
//       Serial.println(">>> Client Timeout !");
//       client.stop();
//       return;
//     }
//   }
// }