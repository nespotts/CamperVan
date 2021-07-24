
class Linear_Actuator {
    private:
        int top_photo_pin;
        int bottom_photo_pin;
        int en_pin;
        int in_1_pin;
        int in_2_pin;
        int endstop_pin;
        int angle_pin;

        int endstop_val;
        
        long dir_control_timer;

        long currenttime;

        int lo_level_state = 0;
        int hi_level_state = 0;

        int t_val_list[20];
        float t_val;
        int b_val_list[20];
        float b_val;
        int index = 0;

        float t_sum;
        float b_sum;

        long home_stop_timer = 0;

    public:
        int motor_speed;
        int photo_thresh;
        float prop_coef;
        float int_coef;
        double int_sum;
        float correction = -2.00;

        long constant;
        double motor_dir_ch_delay;
        int diff;
        int dir_req;
        int motor_dir;
        int dir_req2;
        int motor_dir2;

        int num_avg_pts = 20;

        float panel_angle;
        float panel_angle_list[50];
        int panel_index;
        float angle_sum;
        int panel_angle_threshold;

        bool homing;
        // bool at_home;
        int homing_disable_seconds = 2;

        Linear_Actuator(int top_photo_pin, int bottom_photo_pin, int en_pin, int in_1_pin, 
         int in_2_pin, int endstop_pin, int angle_pin, int panel_angle_threshold,
         int photo_thresh=20, float prop_coef=10, float int_coef=0.0, long constant=0,long motor_dir_ch_delay=1000) {
            this->top_photo_pin = top_photo_pin;
            this->bottom_photo_pin = bottom_photo_pin;
            this->en_pin = en_pin;
            this->in_1_pin = in_1_pin;
            this->in_2_pin = in_2_pin;
            this->endstop_pin = endstop_pin;
            this->angle_pin = angle_pin;
            this->photo_thresh = photo_thresh;
            this->panel_angle_threshold = panel_angle_threshold;
            this->prop_coef = prop_coef;
            this->int_coef = int_coef;
            this->constant = constant;
            // this->correction = correction;
            this->motor_dir_ch_delay = motor_dir_ch_delay;

            pinMode(top_photo_pin, INPUT);
            pinMode(bottom_photo_pin, INPUT);
            pinMode(en_pin, OUTPUT);
            pinMode(in_1_pin, OUTPUT);
            pinMode(in_2_pin, OUTPUT);
            pinMode(endstop_pin, INPUT);
            pinMode(angle_pin, INPUT);
         }

        void calculate_angle() {
            panel_angle_list[panel_index] = analogRead(angle_pin);

            if (panel_index < 50-1) {panel_index++;}
            else {panel_index = 0;}

            angle_sum = 0;
            for (int i=0; i<50; i++){
                angle_sum+=panel_angle_list[i];
            }

            panel_angle = angle_sum / 50.0;

            // start turret here
            if (panel_angle >= panel_angle_threshold) {
                above_angle_th = true;
            } else {
                above_angle_th = false;
            }

            // keep linear actuator going until here.
            if (panel_angle >= panel_angle_threshold + panel_angle_deadband) {
                above_angle_th2 = true;
            } else {
                above_angle_th2 = false;
            }
        }

        void determine_diff() {
            t_val_list[index] = analogRead(top_photo_pin);
            b_val_list[index] = analogRead(bottom_photo_pin);
            
            if (index < num_avg_pts-1) {index++;}
            else {index = 0;}

            t_sum = 0;
            b_sum = 0;
            for (int i=0; i<num_avg_pts; i++){
                t_sum+=t_val_list[i];
                b_sum+=b_val_list[i];
            }

            t_val = t_sum / (float)num_avg_pts;
            b_val = b_sum / (float)num_avg_pts;
            
            diff = t_val - b_val - correction;

            if (center_sun) {
                correction = diff + correction;
            }
        }

        void control_to_home() {
            if (TC_at_home) {
                endstop_val = digitalRead(endstop_pin);
                if (active) {
                    if (endstop_val == HIGH) {
                        dir_req = -1;
                        LC_at_home = false;
                        at_home = false;
                        home_stop_timer = 0;
                    } else {
                        if (home_stop_timer == 0) {
                            home_stop_timer = millis();
                        }
                        if (millis() - home_stop_timer >= homing_disable_seconds*1000) {
                            active = false;
                            home_stop_timer = 0;
                            at_home = true;
                        } else {
                            at_home = false;
                        }
                        dir_req = 0;
                        LC_at_home = true;
                    }
                }
            } else if (above_angle_th == false) {
                if (sun_position == 1) {
                    dir_req = 1;
                }
                LC_at_home = false;
            } else {
                dir_req = 0;
                LC_at_home = false;
            }
        }

        void request_direction() {
            // Determine Requested Direction if out of deadband
            if (sun_position == 0) {
                dir_req = 1;
                if (above_angle_th2) {
                    sun_position = 1;
                }
            } else {
                if (diff >= photo_thresh) {
                    // Extend
                    dir_req = 1;
                    int_sum += 1;
                }
                else if (diff <= -photo_thresh) {
                    // Retract if not at endstop
                    endstop_val = digitalRead(endstop_pin);
                    if (endstop_val == HIGH) {
                        at_home = false;
                        // Also need to add code to only retract past certain angle when in home position
                        if (panel_angle > panel_angle_threshold+panel_angle_deadband) {
                            dir_req = -1;
                            int_sum += 1;
                        // Logic to retract past threshold
                        } else {
                            // if TC at home, retract
                            if (TC_at_home && receive_data.mode != 0) {
                                int_sum += 1;
                                dir_req = -1;
                            // move TC to home, then retract
                            } else {
                                // if (sun_position != 0) {
                                //     // maybe add condition to increase diff threshold to change this
                                //     // sun_position = 2;
                                //     // control_to_home();
                                // } else {
                                    dir_req = 0;
                                    int_sum = 0;
                                // }
                            }     
                        }
                    } else {
                        dir_req = 0; 
                        int_sum = 0;
                    }
                }
                else {
                    // Stop
                    dir_req = 0;
                    int_sum = 0;
                }
            }
        }

        void motor_control_logic() {
            // Logic to control switching motor directions - This Works Very Well
            if (motor_dir != dir_req) {
                hi_level_state = 1;
            }

            if (hi_level_state == 1 && lo_level_state == 0) {
                if (dir_req == 0) {
                dir_control_timer = currenttime;
                motor_dir = 0;
                digitalWrite(in_1_pin, HIGH);
                digitalWrite(in_2_pin, HIGH);
                lo_level_state = 0;
                hi_level_state = 0;
                }
                else if (motor_dir == 0) {
                if ((currenttime - dir_control_timer) >= motor_dir_ch_delay) {
                    motor_dir = dir_req;
                    if (motor_dir > 0) {
                        digitalWrite(in_1_pin, LOW);
                        digitalWrite(in_2_pin, HIGH);
                    } else if (motor_dir < 0) {
                        digitalWrite(in_1_pin, HIGH);
                        digitalWrite(in_2_pin, LOW);
                    }
                    lo_level_state = 0;
                    hi_level_state = 0;
                } else {
                    lo_level_state = 2;
                }
                }
                else {
                dir_control_timer = currenttime;
                motor_dir = 0;
                lo_level_state = 2;
                }
            } 
            if (hi_level_state == 1 && lo_level_state == 2) {
                if ((currenttime - dir_control_timer) >= motor_dir_ch_delay) {
                motor_dir = dir_req;
                // Send Motor Direction Signal
                if (motor_dir > 0) {
                    digitalWrite(in_1_pin, LOW);
                    digitalWrite(in_2_pin, HIGH);
                } else if (motor_dir < 0) {
                    digitalWrite(in_1_pin, HIGH);
                    digitalWrite(in_2_pin, LOW);
                }
                lo_level_state = 0;
                hi_level_state = 0;
                }
            }
        }

        void calculate_motor_speed() {
            if (homing) {
                // make proportional with panel angle
                motor_speed = abs(motor_dir*((float)panel_angle/4.0));
            }
            else if (sun_position == 0) {
                motor_speed = abs(motor_dir*255);
            } else {
                motor_speed = abs(motor_dir*diff*prop_coef);
                if (motor_speed != 0) {
                    motor_speed = motor_speed + constant + int_sum*int_coef;
                    // digitalWrite(en_pin, HIGH);
                } else {
                    // digitalWrite(en_pin, LOW);
                }
            }

            if (motor_speed > 255) {
                motor_speed = 255;
            } else if (motor_speed < 15) {
                motor_speed = 0;
            }

            analogWrite(en_pin, motor_speed);

            dir_req2 = 100*dir_req;
            motor_dir2 = 70*motor_dir;
        }

        void track_sun() {
            homing = false;
            calculate_angle();
            determine_diff();
            request_direction();
            motor_control_logic();
            calculate_motor_speed();
        }

        void go_home() {
            homing = true;
            calculate_angle();
            control_to_home();
            motor_control_logic();
            calculate_motor_speed();           
        }

        void manual_mode() {
            homing = false;
            calculate_angle();
            determine_diff();
            if (receive_data.LC_manual == 1) {
                diff = 255;
            } else if (receive_data.LC_manual == -1) {
                diff = -100;
            } else {
                diff = 0;
            }
            request_direction();
            motor_control_logic();
            calculate_motor_speed();
        }

        void run() {
            currenttime = millis();

            if (receive_data.mode == 1) {
                go_home();
            } else if (receive_data.mode == 2) {
                manual_mode();
            } else if (receive_data.mode == 0) {
                if (mode == 2) {
                    track_sun();
                } else if (mode == 1) {
                    // go_home();
                    receive_data.LC_manual = -1;
                    manual_mode();
                }
            }
        }
};


class Turret_Controller {
    private:
        int left_photo_pin;
        int right_photo_pin;
        int pwm_speed_pin;
        int motor_dir_pin;
        
        long dir_control_timer;

        

        int lo_level_state = 0;
        int hi_level_state = 0;

        int l_val_list[20];
        float l_val;
        int r_val_list[20];
        float r_val;
        int index = 0;

        float l_sum;
        float r_sum;

    public:
        long currenttime;

        int motor_speed;
        int photo_thresh;
        float prop_coef;
        float int_coef;
        double int_sum;

        float I = 0.3; // 0.1 works pretty well;
        long I_count = 0;

        long constant;
        double motor_dir_ch_delay;
        float diff;
        int dir_req;
        int motor_dir;
        int dir_req2;
        int motor_dir2;

        int num_avg_pts = 20;

        float turret_gear_ratio = 6.4;
        long motor_gear_ratio = 165;
        long pulse_per_round = 9;
        long pulse_per_rev = turret_gear_ratio*motor_gear_ratio*pulse_per_round;
        long counts_to_home;
        bool homing;

        float correction = 2.10;
        int count_list[100];
        int last_count;
        int diff_list[100];
        double count_diff_sum;
        int correction_index = 0;

        double count_diff;

        bool Keep_Close_To_Home = false;
        long target_count = 0;
        long counts_to_target = 0;
        bool TC_at_target = true;
        float deg_over_center = 30.0;

        double counts_per_cycle = 0.0;
        double last_count2;
        float cycles = 0.0;
        int num_cycles = 500;
        float counts_per_cycle_threshold = 1.0;
        long delay_timer = 0;
        long delay_length = 5*60000;
        long track_sun_freedom_timer_length = 30000;

        Turret_Controller(int left_photo_pin, int right_photo_pin, int pwm_speed_pin, 
         int motor_dir_pin, int photo_thresh=20,
         float prop_coef=10, float int_coef=0.0, long constant=150, long motor_dir_ch_delay=1000) {
            this->left_photo_pin = left_photo_pin;
            this->right_photo_pin = right_photo_pin;
            this->pwm_speed_pin = pwm_speed_pin;
            this->motor_dir_pin = motor_dir_pin;
            this->photo_thresh = photo_thresh;
            this->prop_coef = prop_coef;
            this->int_coef = int_coef;
            this->constant = constant;
            this->motor_dir_ch_delay = motor_dir_ch_delay;

            pinMode(left_photo_pin, INPUT);
            pinMode(right_photo_pin, INPUT);
            pinMode(pwm_speed_pin, OUTPUT);
            pinMode(motor_dir_pin, OUTPUT);
         }

        void determine_count_home() {
            counts_to_home = count - home_count;
            if (counts_to_home >= -20 && counts_to_home <= 20) {
                TC_at_home = true;
            } else {
                TC_at_home = false;
            }
        }

        void determine_counts_per_cycle() {
            cycles++;
            if (cycles >= num_cycles) {
                counts_per_cycle = abs((count - last_count2)/cycles);
                last_count2 = count;
                cycles = 0.0;
            }

            if ((currenttime - track_sun_freedom_timer) >= track_sun_freedom_timer_length) {       
                if ((currenttime - delay_timer) <= delay_length) {
                    diff = 0;
                }
                if (counts_per_cycle > counts_per_cycle_threshold) {
                    diff = 0;
                    delay_timer = currenttime;
                }
            }
        }

        void control_to__home() {
            // only move if above angle threshold
            if (above_angle_th) {
                if (counts_to_home >= -20 && counts_to_home <= 20) {
                    TC_at_home = true;
                    dir_req = 0;
                    I_count = 0;
                } else if (counts_to_home > 0) {
                    TC_at_home = false;
                    at_home = false;
                    dir_req = -1;
                    I_count++;
                } else {
                    TC_at_home = false;
                    at_home = false;
                    dir_req = 1;
                    I_count++;
                }
            } else {
                dir_req = 0;
            }
        }

        void determine_diff() {
            l_val_list[index] = analogRead(left_photo_pin);
            r_val_list[index] = analogRead(right_photo_pin);
            
            if (index < num_avg_pts-1) {
                index++;
            }
            else {
                index = 0;
            }
            l_sum = 0;
            r_sum = 0;
            for (int i=0; i<num_avg_pts; i++){
                l_sum+=l_val_list[i];
                r_sum+=r_val_list[i];
            }

            l_val = l_sum / (float)num_avg_pts;
            r_val = r_sum / (float)num_avg_pts;
            
            diff = r_val - l_val - correction;

            if(center_sun) {
                correction = diff + correction;
            }

        }

        void calculate_correction() {
            for (int i=0; i<99; i++) {
                count_diff_sum += abs((float)diff_list[i] - (float)diff_list[i + 1]) / abs((float)count_list[i] - (float)count_list[i+1]);

            }

            count_diff = count_diff_sum / 99.0;
            
            if (count_diff <= 0.0002) { //0.0002
                correction = r_val - l_val;
            } else if (count_diff >= 0.01) { //0.01
                correction = 0;
            }

            count_diff_sum = 0;

            if (correction_index == 99) {
                correction_index = 0;
            } else {
                correction_index ++;
            }
        }

        void apply_correction() {
            if (count != last_count) {
                count_list[correction_index] = count;
                diff_list[correction_index] = diff;
                last_count = count;

                calculate_correction();
            }
        }

        void apply_correction_manual() {
            if (count != last_count) {
                l_val_list[index] = analogRead(left_photo_pin);
                r_val_list[index] = analogRead(right_photo_pin);
                
                if (index < num_avg_pts-1) {
                    index++;
                }
                else {
                    index = 0;
                }
                l_sum = 0;
                r_sum = 0;
                for (int i=0; i<num_avg_pts; i++){
                    l_sum+=l_val_list[i];
                    r_sum+=r_val_list[i];
                }

                l_val = l_sum / (float)num_avg_pts;
                r_val = r_sum / (float)num_avg_pts;

                count_list[correction_index] = count;
                diff_list[correction_index] = r_val - l_val - correction;

                calculate_correction();
                last_count = count;
            }
        }

        void Check_Counts_From_Home() {
            // if 200deg (180+20) rotated from home, then rotate toward home, 360deg.
            // needs to be mode that takes over when above is true
            if (TC_at_target == true) {
                if (abs(counts_to_home) >= ((180.0+deg_over_center)/360.0*pulse_per_rev)) {
                    TC_at_target = false;
                    if (counts_to_home < 0) {
                        target_count = count + 260.0/360.0*pulse_per_rev; //was 320.0
                    } else {
                        target_count = count - 260.0/360.0*pulse_per_rev; // was 320.0
                    }
                }
            }
        }  

        void Control_To_Other_Side() {
            counts_to_target = count - target_count;

            // only move if above angle threshold
            if (above_angle_th) {
                if (counts_to_target >= -20 && counts_to_target <= 20) {
                    TC_at_target = true;
                    dir_req = 0;
                    I_count = 0;
                } else if (counts_to_target > 0) {
                    TC_at_target = false;
                    dir_req = -1;
                    I_count++;
                } else {
                    TC_at_target = false;
                    dir_req = 1;
                    I_count++;
                }
            } else {
                dir_req = 0;
            }
        }


        void request_direction() {
            if (sun_position == 2) {
                // Not Used
                control_to__home();
            // *********** NEW ***********************
            } else if (TC_at_target == false) {
                Control_To_Other_Side();
            // ***************************************
            } else {
                // only move if above panel angle threshold
                if (above_angle_th) {
                    // sun_position = 1; !!!!!!!!!!
                    // Determine Requested Direction if out of deadband
                    if (diff >= photo_thresh) {
                        // Turn (CW)
                        dir_req = 1;
                        int_sum += 1;
                    }
                    else if (diff <= -photo_thresh) {
                        // Turn (CCW)
                        dir_req = -1;
                        int_sum += 1;
                    }
                    else {
                        // Stop
                        dir_req = 0;
                        int_sum = 0;
                    }
                } else {
                    if (abs(diff) >= 1*photo_thresh) {
                        sun_position = 0;
                    } else {
                        sun_position = 1;
                    }
                    dir_req = 0;
                }
            }
        }

        void motor_control_logic() {
            // Logic to control switching motor directions - This Works Very Well
            if (motor_dir != dir_req) {
                hi_level_state = 1;
            }

            if (hi_level_state == 1 && lo_level_state == 0) {
                if (dir_req == 0) {
                dir_control_timer = currenttime;
                motor_dir = 0;
                lo_level_state = 0;
                hi_level_state = 0;
                }
                else if (motor_dir == 0) {
                if ((currenttime - dir_control_timer) >= motor_dir_ch_delay) {
                    motor_dir = dir_req;
                    if (motor_dir > 0) {
                        digitalWrite(motor_dir_pin, LOW);
                        count_dir = 1;
                    } else if (motor_dir < 0) {
                        digitalWrite(motor_dir_pin, HIGH);
                        count_dir = -1;
                    }
                    lo_level_state = 0;
                    hi_level_state = 0;
                } else {
                    lo_level_state = 2;
                }
                }
                else {
                dir_control_timer = currenttime;
                motor_dir = 0;
                lo_level_state = 2;
                }
            } 
            if (hi_level_state == 1 && lo_level_state == 2) {
                if ((currenttime - dir_control_timer) >= motor_dir_ch_delay) {
                motor_dir = dir_req;
                // Send Motor Direction Signal
                if (motor_dir > 0) {
                    count_dir = 1;
                    digitalWrite(motor_dir_pin, LOW);
                } else if (motor_dir < 0) {
                    count_dir = -1;
                    digitalWrite(motor_dir_pin, HIGH);
                }
                lo_level_state = 0;
                hi_level_state = 0;
                }
            }
        }

        void calculate_motor_speed() {
            if (homing || sun_position == 2) {
                motor_speed = abs(motor_dir*0.5*counts_to_home);
                if (motor_speed >= 255){
                    I_count = 0;
                } else {
                    motor_speed += I*I_count;
                }
            } else if (TC_at_target==false) {
                motor_speed = abs(motor_dir*0.5*counts_to_target);
                if (motor_speed >= 255) {
                    I_count = 0;
                } else {
                    motor_speed += I*I_count;
                }
            
            } else {
                motor_speed = abs(motor_dir*diff*prop_coef);
                // if (motor_speed != 0) {
                if (abs(diff) >= photo_thresh && counts_per_cycle <= counts_per_cycle_threshold) {
                    motor_speed = motor_speed + constant + int_sum*int_coef;
                }
            }

            if (motor_speed > 255) {
                motor_speed = 255;
            }

            analogWrite(pwm_speed_pin, motor_speed);

            dir_req2 = 100*dir_req;
            motor_dir2 = 70*motor_dir;
        }

        void rotate_home() {
            homing = true;
            TC_at_target = true; // new
            determine_count_home();
            control_to__home();
            motor_control_logic();
            calculate_motor_speed();
        }
        
        void track_sun() {
            homing = false;
            determine_count_home();
            // ****  NEW!!!!!
            Check_Counts_From_Home();
            // ***************
            determine_diff();
            // apply_correction();
            determine_counts_per_cycle();
            request_direction();
            motor_control_logic();
            calculate_motor_speed();
        }

        void manual_mode() {
            homing = false;
            TC_at_target = true; // new
            determine_count_home();
            // apply_correction_manual();
            determine_diff();
            if (receive_data.TC_manual == -1) {
                diff = -300;
            } else if (receive_data.TC_manual == 1) {
                diff = 300;
            } else {
                diff = 0;
            }
            request_direction();
            motor_control_logic();
            calculate_motor_speed();
        }

        void run() {
            currenttime = millis();

            if (receive_data.mode == 1) {
                rotate_home();
            } else if (receive_data.mode == 2) {
                manual_mode();
            } else if (receive_data.mode == 0) {
                if (mode == 2) {
                    track_sun();
                } else if (mode == 1) {
                    rotate_home();
                    // receive_data.TC_manual = 0;
                    // manual_mode();
                }
            }
        }
};
