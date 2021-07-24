#include "Arduino.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9,10); // CE, CSN


#include "Plotter.h"
Plotter p;

long count = 0;
long home_count = 0;
int count_dir;
bool TC_at_home;
bool LC_at_home;
bool above_angle_th;
bool above_angle_th2;
int panel_angle_deadband = 40;
bool high_sun;
bool low_sun = false;
int sun_position = 1;

bool active = true;

bool at_home = false;

bool center_sun = false;


long sensor_sum = 500;
bool started_timer = false;
long timer = 0;
int time_delay = 5000;
int day_night_th = 1200; //900
int mode = 2; // 1-night, 2-day

long track_sun_freedom_timer = 0;

#include "functions.h"
#include "class.h"

Turret_Controller TC = Turret_Controller(A2, A3, 3, 4, 1, 2.00F, 0.05F, 170L, 1000L);
//top pin, bottom_pin, en_pin, in_1_pin, in_2_pin, endstop_pin, angle_pin, panel_angle_threshold,photo_thresh=20, prop_coef=10, int_coef=0.0, constant=0, correction = 4, motor_dir_ch_delay=1000
Linear_Actuator LC = Linear_Actuator(A1, A0, 5, 6, 7, 8, A4, 240, 1, 2.0F, 0.05F, 70L, 1000L);

void isr() {
  if (count_dir == -1) {count--;}
  else {count++;}
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  p.Begin();
  p.AddTimeGraph("Data", 1000, "Difference", TC.diff, "Direction Request", 
    TC.dir_req2, "Motor Command", TC.motor_dir2,"Motor Speed", TC.motor_speed);
    // p.AddTimeGraph("Data",1000, "TC Correction", TC.correction, "LC Correction", LC.correction);
  // p.AddTimeGraph("Data", 1000, "Difference", LC.diff, "Direction Request", LC.dir_req2, 
  //   "Motor Command", LC.motor_dir2, "Motor Speed", LC.motor_speed);
  // // p.AddTimeGraph("Data", 10000, "Threshold", above_angle_th);
  // // p.AddTimeGraph("Data", 1000, "Panel Angle", LC.panel_angle);
  // p.AddTimeGraph("Data", 1000, "diff", TC.diff);
  // p.AddTimeGraph("Data2", 1000, "int_sum", LC.int_sum);
  // p.AddTimeGraph("Encoder", 1000, "Count", count, "Counts to home", TC.counts_to_home);
  // p.AddTimeGraph("Commands", 1000, "Direction Request", TC.dir_req, "Motor Direction", TC.motor_dir);
  //  p.AddTimeGraph("Correction", 1000, "Correction Value", TC.correction);
  //  p.AddTimeGraph("Count Difference", 1000, "Difference Per Count", TC.count_diff);
  //p.AddTimeGraph("Sum of Sensors", 1000, "Sum of Sensors", sensor_sum);
  //p.AddTimeGraph("Mode", 1000, "mode", mode
  // p.AddTimeGraph("Target", 1000, "TC At Target", TC.TC_at_target);
  // p.AddTimeGraph("Counts_Per_Cycle", 1000, "Data", TC.counts_per_cycle, "dir_req", TC.dir_req);

  attachInterrupt(digitalPinToInterrupt(2), isr, RISING);
  // 1-lowest power, 4-highest power
  Radio_Setup(3);
  receive_data.LC_manual = 0;
  receive_data.TC_manual = 0;
  receive_data.mode = 2;
}

void loop() {


  TC.run();
  LC.run();
  PerceiveNightTime(); // New
  Receive_Data();

  p.Plot();

  // Serial.print(TC.correction); Serial.print("\t"); Serial.println(LC.correction);
}
