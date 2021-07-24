void ReadButtons() {
  TC_cw = digitalRead(TCpin_cw);
  TC_ccw = digitalRead(TCpin_ccw);
  LC_up = digitalRead(LCpin_up);
  LC_dn = digitalRead(LCpin_dn);
  mode1 = digitalRead(modepin1);
  mode2 = digitalRead(modepin2);
  // mode3 = digitalRead(modepin3);
  home = analogRead(home_pin);
  center_sun = digitalRead(centerSunPin);
}

// auto=0, home=1, manual=2
void SetModeLEDs(int mode, bool at_home) {
  // auto
  if (mode == 0) {
    digitalWrite(A1, HIGH);
    digitalWrite(A2, LOW);
    digitalWrite(A3, LOW);
  }
  // home
  if (mode == 1) {
      if(at_home) {
        led.Update();
        digitalWrite(A3, LOW);
        digitalWrite(A1, LOW);  
      } else {
        digitalWrite(A2, HIGH);
        digitalWrite(A3, LOW);
        digitalWrite(A1, LOW);
      }
  }
  // manual
  if (mode == 2) {
    digitalWrite(A3, HIGH);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
  } 
}

void Button_Logic() {
  if (mode1 == LOW && mode2 == HIGH) {
    mode = 0;
  } else if (mode1 == HIGH && mode2 == LOW) {
    mode = 2;
  } else if (mode1 == HIGH && mode2 == HIGH) {
    mode = 1;
  }

  if (LC_up == LOW) {
    send_data.LC_manual = -1;
  } else if (LC_dn == LOW) {
    send_data.LC_manual = 1;
  } else {
    send_data.LC_manual = 0;
  }

  if (TC_cw == LOW) {
    send_data.TC_manual = 1;
  } else if (TC_ccw == LOW) {
    send_data.TC_manual = -1;
  } else {
    send_data.TC_manual = 0;
  }

  if (home <= 500) {
    send_data.home_count = 1;
  } else {
    send_data.home_count = 0;
  } 

  if (center_sun == HIGH) {
    send_data.center_sun_tracking = false;
  } else {
    send_data.center_sun_tracking = true;
  }
}

void SendData() {
  radio.stopListening();
  send_data.mode = mode;
  Serial.println(send_data.center_sun_tracking);
  radio.write(&send_data, sizeof(SEND_DATA_STRUCTURE));
}

void ReceiveData() {
  radio.startListening();
  long t1 = millis();
  
  // only wait 1 second to get response
  while(!radio.available()) {
    long t2 = millis();
    if (t2 - t1 >= 1000) {
      break;
    }
    Serial.println("Waiting for Response");
  }
    
  if (radio.available()) {
    radio.read(&receive_data, sizeof(receive_data));
    Serial.println(receive_data.mode);
    send_data.mode = receive_data.mode; // not sure if this will be ok

    SetModeLEDs(receive_data.mode, receive_data.at_home);
  } else {
    Serial.println("Radio Not Available");
  }
}

void Radio_Setup(int PA_level) {
    radio.begin();
    if (PA_level == 1) {
        radio.setPALevel(RF24_PA_MIN);
    } else if (PA_level == 2) {
        radio.setPALevel(RF24_PA_LOW);
    } else if (PA_level == 3) {
        radio.setPALevel(RF24_PA_HIGH);
    } else {
        radio.setPALevel(RF24_PA_MAX);
    }
    radio.openWritingPipe(rem_tx);
    radio.openReadingPipe(0, rem_rx);
}