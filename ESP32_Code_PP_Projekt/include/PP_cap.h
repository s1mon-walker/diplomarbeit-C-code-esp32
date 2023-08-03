#ifndef _PP_cap_h_
#define _PP_cap_h_

#include <Arduino.h>
#include <Preferences.h>
#include "Mqtt_client.h"
#include "LIDARLite_v4LED.h"

class PPlace {
    public:
        PPlace();
        PPlace(String loc, int r, int g, int b);
        void loop();
        void set_teach_distance(String dist);
        void set_debug_mode(String value);

    private:
        int state;
        int old_state;
        String location;
        String topic;
        String debug_topic;
        float distance;
        float distance_old;
        float teach_distance;
        float detection_hight;
        int red_pin;
        int green_pin;
        int blue_pin;
        bool debug;
        bool led_red;
        bool led_green;
        bool led_blue;
        bool enter_teach_mode;
        bool teach_mode;
        unsigned long teach_timer;
        unsigned long teach_delay;
        void log(String str);
  
};

#endif