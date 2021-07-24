// Remote Transmitter Address
const byte rem_tx[6] = "soltx";
// Remote Receiver Address
const byte rem_rx[6] = "solrx";
int last_mode = 1;

struct RECEIVE_DATA_STRUCTURE {
    // 0=auto, 1=home, 2=manual
    int mode;
    int LC_manual;
    int TC_manual;
    int home_count;
    bool request_data;
    bool center_sun_tracking;
};

RECEIVE_DATA_STRUCTURE receive_data;

struct SEND_DATA_STRUCTURE {
    // 0=auto, 1=home, 2=manual
    int16_t mode;
    bool at_home;
};

SEND_DATA_STRUCTURE send_data;

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
    radio.openWritingPipe(rem_rx);
    radio.openReadingPipe(0, rem_tx);
}

void Send_Data() {
    delay(10);
    radio.stopListening();
    // radio.openWritingPipe(rem_rx);
    send_data.mode = receive_data.mode;
    send_data.at_home = at_home;
    radio.write(&send_data, sizeof(SEND_DATA_STRUCTURE));
    delay(5);
}

void Receive_Data() {
    radio.startListening();
    // radio.openReadingPipe(0, rem_tx);
    if (radio.available()) {
        radio.read(&receive_data, sizeof(receive_data));

        // Serial.print(receive_data.mode); Serial.print("\t"); Serial.print(receive_data.LC_manual); 
        // Serial.print("\t"); Serial.print(receive_data.TC_manual); Serial.print("\t"); Serial.print(receive_data.home_count);
        // Serial.print("\t"); Serial.println(receive_data.request_data);

        if (receive_data.home_count == 1) {
            home_count = count;
        }

        if (receive_data.mode != last_mode) {
            last_mode = receive_data.mode;
            if (receive_data.mode == 1) {
                active = true;
            } else if (receive_data.mode == 0) {
                track_sun_freedom_timer = millis();
            }
        }
        if (receive_data.request_data) {
            receive_data.request_data = false;
            Send_Data();
        }
        if (receive_data.center_sun_tracking) {
            center_sun = true;
        } else {
            center_sun = false;
        }


    } else {
        // Serial.println("Radio Not Available");
    }
}


void PerceiveNightTime() {
    // Sum all photoresistor readings
    sensor_sum  = analogRead(A0) + analogRead(A1) + analogRead(A2) + analogRead(A3);
    // if above threshold for a certain amount of time, it is daytime, start tracking
    if (sensor_sum >= day_night_th && mode != 2) {
        if (started_timer == false) {
            // start timer
            timer = millis();
            started_timer = true;
        }
        if ((millis() - timer) >= time_delay) {
            // It is now daylight, start tracking
            mode = 2;
            started_timer = false;

        }
    }
    // if below threshold for a certain amount of time, it is nightime, go home or freeze()
    else if (sensor_sum < day_night_th && mode != 1) {
        if (started_timer == false) {
            // start timer
            timer = millis();
            started_timer = true;
        }
        if ((millis() - timer) >= time_delay) {
            // It is now nighttime, go home (or freeze)
            mode = 1;
            started_timer = false;

        }
    } else {
        started_timer = false;
    }
}

