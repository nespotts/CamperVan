#include "Arduino.h"

int lvd_relay_pin = 6;
int fan_pin = 5;
int ssr_pin = 4;
int base_pin = 7;
int power_pin = A4;
int temp_pin = A0;

// Fan Temperature Controller Variables
int sensorval;
float voltage;
float temperatureC;
float temperatureF;
float temp_sum;
float avg_temp;
int temp_count = 0;
float temp_threshold = 80; // degrees fahrenheit
float temp_offset = 1.5; // degrees fahrenheit
int fan_state = 0;  // 0=off, 1=on

int lvd_reading = 1;

// State Variables
int state = 0;
int lvd_state = 1;
int toggle = 0;
bool toggle_once = false;
bool lvd_flag = false;
int power_state = false;

bool turning_on = false;
bool turning_off = false;
bool toggle_again = false;

// Timers
long currenttime = 0;
long toggle_timer = 0;
long toggle_duration = 1200;  // milliseconds

long lvd_change_duration = 5; // seconds
long lvd_timer = 0;

long on_timer = 0;
long off_timer = 0;
long off_duration = 10000;
long on_duration = 5000;

long temp_timer = 0;
long temp_interval = 50; // milliseconds


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(lvd_relay_pin, INPUT);
  pinMode(base_pin, OUTPUT);
  pinMode(power_pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ssr_pin, OUTPUT);
  pinMode(fan_pin, OUTPUT);
  pinMode(temp_pin, INPUT);
  
  // initialize value of states
  lvd_state = digitalRead(lvd_relay_pin);
  state = lvd_state;
  power_state = digitalRead(power_pin);
  if (state == 1) {
    digitalWrite(ssr_pin, HIGH);
  }
  else {
    digitalWrite(ssr_pin, LOW);
  }
  Serial.println("Restarted");
}

void loop() {
  currenttime = millis();
  lvd_reading = digitalRead(lvd_relay_pin);
//  power_state = analogRead(power_pin);
  sensorval = analogRead(temp_pin);
  voltage = (sensorval / 1024.0) * 5.0;
  temperatureC = (voltage - 0.5) * 100;
  temperatureF = (temperatureC * 9.0/5.0) + 32.0;

  Serial.print("state: "); Serial.print(state); Serial.print(" lvd_state: "); Serial.print(lvd_state); Serial.print(" Temp: "); Serial.println(avg_temp);

  if (lvd_reading == LOW) {
    lvd_state = 0;
  } else {
    lvd_state = 1;
  }

  if (state == 0 && lvd_state == 1){
    // need to turn on
    state = 2;
    on_timer = currenttime;
  }
  else if (state == 1 && lvd_state == 0) {
    // need to turn off
    state = 3;
    off_timer = currenttime;
  }
  else if (state == 2) {
    if (lvd_state == 0) {
      state = 0;
    }
    else if ((currenttime - on_timer) >= on_duration) {
      digitalWrite(ssr_pin, HIGH);
      state = 1;
    }
  }
  else if (state == 3) {
    if (lvd_state == 1) {
      state = 1;
    }
    else if ((currenttime - off_timer) >= off_duration) {
      digitalWrite(ssr_pin, LOW);
      Serial.println("Turned OFF");
      state = 0;
    }
  }



  temp_count += 1;
  temp_sum += temperatureF;

  if ((currenttime - temp_timer) >= temp_interval) {
    avg_temp = temp_sum/ (float)temp_count;
  //  Serial.println(avg_temp);
    if (avg_temp >= (temp_threshold + temp_offset) && fan_state == 0) {
      digitalWrite(fan_pin, HIGH);
      fan_state = 1;
    } else if (avg_temp <= (temp_threshold - temp_offset) && fan_state == 1) {
      digitalWrite(fan_pin, LOW);
      fan_state = 0;
    }
    temp_count = 0;
    temp_sum = 0;
    temp_timer = currenttime;
  }

  
  delay(500);
}
