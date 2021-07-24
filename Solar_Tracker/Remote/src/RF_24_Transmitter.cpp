#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9,10); // CE, CSN
// Remote Transmitter Address
const byte rem_tx[6] = "soltx";
// Remote Receiver Address
const byte rem_rx[6] = "solrx";


#include <jled.h>
auto led = JLed(A2).Blink(500,500).Forever();

struct SEND_DATA_STRUCTURE{
  // 0=auto, 1=home, 2=manual
  int mode;
  int LC_manual;
  int TC_manual;
  int home_count;
  bool request_data;
  bool center_sun_tracking;
};

SEND_DATA_STRUCTURE send_data;

struct RECEIVE_DATA_STRUCTURE{
  // 0=auto, 1=home, 2=manual
  int16_t mode;
  bool at_home;
};

RECEIVE_DATA_STRUCTURE receive_data;

int mode;
int last_mode = 0;

// Setup pins
int TCpin_cw = 2;
int TCpin_ccw = 3;
int LCpin_up = 4;
int LCpin_dn = 5;
int modepin1 = 6;
int modepin2 = 7;
// int modepin3 = 8;
int home_pin = A0;
int autoledpin = A1;
int homeledpin = A2;
int manualledpin = A3;
int centerSunPin = A5;

// Setup Variables for button values
int TC_cw;
int TC_ccw;
int LC_up;
int LC_dn;
int mode1;
int mode2;
int mode3;
int home;
int center_sun;

#include "rem_functions.h"

long currenttime;
long lastreqesttime = 0;


void setup() {
  Serial.begin(115200);  

  pinMode(TCpin_cw, INPUT);
  pinMode(TCpin_ccw, INPUT);
  pinMode(LCpin_up, INPUT);
  pinMode(LCpin_dn, INPUT);
  pinMode(modepin1, INPUT);
  pinMode(modepin2, INPUT);
  // pinMode(modepin3, INPUT);
  pinMode(home_pin, INPUT);

  pinMode(autoledpin, OUTPUT);
  pinMode(homeledpin, OUTPUT);
  pinMode(manualledpin, OUTPUT);

  pinMode(centerSunPin, INPUT);

  Radio_Setup(3);

  send_data.request_data = true;
}

void loop() {
  currenttime = millis();
  ReadButtons();
  Button_Logic();

  if (mode != last_mode) {
    send_data.request_data = true;
    last_mode = mode;
  }
  if (receive_data.mode == 1 && (currenttime - lastreqesttime) > 1000) {
    send_data.request_data = true;
    lastreqesttime = currenttime;
  }

  SendData();

  if (send_data.request_data == true) {
    send_data.request_data = false;
    ReceiveData();
  } else {
    Serial.println("Not Available");
  }

  SetModeLEDs(receive_data.mode, receive_data.at_home);

  // Serial.println(mode);
}
