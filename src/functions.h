// Eliminate Some Noise
void Calc_Brightness() {
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
  if (digitalRead(ilum_entry_pin) == HIGH) {
    entry_state = 1;
    analogWrite(ledpin, entry_brightness);
    Serial.println("illuminated entry");
  }
  // Otherwise Write the Dimmer Brightness to the LED
  else {
    entry_state = 0;
    if (led_brightness <= min_brightness) {
      digitalWrite(ledpin, LOW);
    } else {
      Serial.println(map(led_brightness, 0, 1023, 0, max_brightness));
      analogWrite(ledpin, map(led_brightness, 0, 1023, 0, max_brightness));
    }
  }
}

// void Check_ESP() {
//   if ((currenttime - ESP_timer) >= ESP_reset_threshold) {
//     ESP.restart();
//   }
//   ESP_timer = currenttime;
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

