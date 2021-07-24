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

void ConnectWifi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WiFi.SSID());
  WiFi.persistent(true);  // May be able to set to true disables automatic saving of wifi credentials
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pass);


  int t1 = millis();
  while((WiFi.status() != WL_CONNECTED)) { 
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

String httpGETRequest() {
//   WiFiClient client;
  HTTPClient http;

  http.setTimeout(timeout);
    
  // Your IP address with path or Domain name with URL path 
  http.begin("http://192.168.8.158/illuminated_entry");
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 

  ESP.wdtFeed();
  yield();

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  ESP.wdtFeed();
  yield();
  return payload;
}




    // HTTPClient http;  //Declare an object of class HTTPClient
 
    // http.begin("http://jsonplaceholder.typicode.com/users/1");  //Specify request destination
    // int httpCode = http.GET();                                  //Send the request
 
    // if (httpCode > 0) { //Check the returning code
 
    //   String payload = http.getString();   //Get the request response payload
    //   Serial.println(payload);             //Print the response payload
 
    // }
 
    // http.end();   //Close connection



// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
 
// const char* ssid = "yourNetworkName";
// const char* password = "yourNetworkPassword";
 
// void setup () {
 
//   Serial.begin(115200);
 
//   WiFi.begin(ssid, password);
 
//   while (WiFi.status() != WL_CONNECTED) {
 
//     delay(1000);
//     Serial.print("Connecting..");
 
//   }
 
// }
 
// void loop() {
 
//   if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
//     HTTPClient http;  //Declare an object of class HTTPClient
 
//     http.begin("http://jsonplaceholder.typicode.com/users/1");  //Specify request destination
//     int httpCode = http.GET();                                  //Send the request
 
//     if (httpCode > 0) { //Check the returning code
 
//       String payload = http.getString();   //Get the request response payload
//       Serial.println(payload);             //Print the response payload
 
//     }
 
//     http.end();   //Close connection
 
//   }
 
//   delay(30000);    //Send a request every 30 seconds
// }