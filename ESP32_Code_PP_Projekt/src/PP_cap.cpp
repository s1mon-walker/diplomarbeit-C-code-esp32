#include "PP_cap.h"

LIDARLite_v4LED LIDAR;
Preferences preferences;

// empty constructor for declaration
PPlace::PPlace() {
    //empty
}

// constructor for PPlace object
PPlace::PPlace(String loc, int r, int g, int b) {
    preferences.begin("PP_data", false);

    this->debug = false;
    this->state = 0;
    this->old_state = 0;
    this->location = loc;
    this->teach_distance = preferences.getFloat("teach_dist", 100);
    this->teach_delay = 3000;
    this->detection_hight = 6; //cm
    this->red_pin = r;
    this->green_pin = g;
    this->blue_pin = b;
    this->led_red = false;
    this->led_green = false;
    this->led_blue = false;
    this->topic = "Storage_balette/" + this->location + "/Status";
    this->debug_topic = "Storage_balette/" + this->location + "/Debug";

    preferences.end();

    pinMode(this->red_pin, OUTPUT);
    pinMode(this->green_pin, OUTPUT);
    pinMode(this->blue_pin, OUTPUT);

    Wire.begin(21, 22);
    while (LIDAR.begin() == false) {
    Serial.println("LIDAR did not acknowledge! Retry...");
    delay(10);
    }
    Serial.println("LIDAR acknowledged!");
}

void PPlace::loop() {
    // check connection to LIDAR sensor
    while (LIDAR.isConnected() == false) {
        Serial.println("LIDAR connection lost! Reconnecting...");
        while (LIDAR.begin() == false)
        {
            Serial.println("LIDAR did not acknowledge! Retry...");
            delay(50);
        }
        Serial.println("Reconnect sucessfull");
    }

    // get distance reading from LIDAR sensor
    this->distance = LIDAR.getDistance();
    
    // sendig the distance as debug information in debug mode
    if (abs(this->distance - this->distance_old) > 3 && this->debug) {
        this->log("New Distance: " + String(this->distance));
        this->distance_old = this->distance;
    }

    // if the messured distance is smaller than the teach distance the place is occupied
    if (this->distance < this->teach_distance) {
        this->state = 1;
        if (EXPRESS) {
            this->led_red = false;
            this->led_green = false;
            this->led_blue = true;
        } else {
            this->led_red = true;
            this->led_green = false;
            this->led_blue = false;
        }
    } else {
        this->state = 0;
        this->led_red = false;
        this->led_green = true;
        this->led_blue = false;
    }

    // if the distance reading is nearly zero, start the teach_timer
    if (this->distance < 20) {
        if (!this->enter_teach_mode) this->teach_timer = millis();
        this->enter_teach_mode = true;
    } else {
        this->enter_teach_mode = false;
    }
    
    // enter the teach mode by holding the distance small for 3 seconds
    // after 5 more seconds the distance reading is taken to calculate the new teach_distance
    if (this->teach_mode) {
        delay(5000);
        this->teach_distance = LIDAR.getDistance() - this->detection_hight;
        preferences.begin("PP_data", false);
        preferences.putFloat("teach_dist", this->teach_distance);
        preferences.end();
        this->teach_mode = false;
    } else if (this->enter_teach_mode) {
        if ((unsigned long)(millis() - this->teach_timer) > this->teach_delay) {
            // indicate the active teach mode with white
            this->teach_mode = true;
            this->led_red = true;
            this->led_green = true;
            this->led_blue = true;
        } else {
        // indicate the entering of the teach mode with purble
        this->led_red = true;
        this->led_green = false;
        this->led_blue = true;
        }
    }

    digitalWrite(this->red_pin, this->led_red);
    digitalWrite(this->green_pin, this->led_green);
    digitalWrite(this->blue_pin, this->led_blue);

    if (this->state != this->old_state) {
        publish(this->topic, String(this->state));
    }

    this->old_state = this->state;
}

// possibility to setting the teach distance trough a MQTT message
void PPlace::set_teach_distance(String value) {
    this->log("Setting teach dist...");
    this->teach_distance = atof(value.c_str());
    preferences.begin("PP_data", false);
    preferences.putFloat("teach_dist", this->teach_distance);
    preferences.end();

    // allways send feedback if the operation worked
    if (this->debug) {
        this->log("Teach Dist alterd");
    } else {
        this->debug = true;
        this->log("Teach Dist alterd");
        this->debug = false;
    }
    
}

// possibility to turn on or off debug mode trough MQTT
void PPlace::set_debug_mode(String value) {
    bool new_debug = (value.c_str()[0] == "1"[0]);
    if (new_debug) {
        this->debug = true;
        this->log("Turned on Debug Mode");
    } else {
        this->log("Turning Debug Mode off...");
        this->debug = false;
    }
}

// Performs a serial println and a MQTT publish if in debug mode
void PPlace::log(String str) {
    Serial.println(str);
    if (this->debug) publish(this->debug_topic, str);
}
