#ifndef _Andon_h_
#define _Andon_h_

#include <Arduino.h>
#include <Preferences.h>
#include "Mqtt_client.h"

class AndonLine {
    public:
        AndonLine();
        AndonLine(String loc, bool *input, bool *green, bool *orange, bool *red);
        void loop();

    private:
        int state;
        int online_state;
        unsigned long switch_cooldown;
        unsigned long cooldown_timer;
        unsigned long state_send_delay;
        unsigned long state_send_timer;
        unsigned long blink_timer;
        unsigned long blink_interval;
        String location;
        String topic;
        bool blink_bit;
        bool old_switch_state;
        bool *state_switch;
        bool *green_light;
        bool *orange_light;
        bool *red_light;

};

class FifoShelf {
    public:
        FifoShelf();
        FifoShelf(String loc, bool *sensor_input, bool *light);
        void loop();

    private:
        int state;
        int old_state;
        int online_state;
        unsigned long state_send_timer;
        unsigned long state_send_delay;
        String location;
        String topic;
        bool *sensor;
        bool *light;

};

class ScrapContainer {
    public:
        ScrapContainer();
        ScrapContainer(String loc, bool *sensor_half, bool *sensor_full);
        void loop();

    private:
        int state;
        int old_state;
        int online_state;
        unsigned long state_send_timer;
        unsigned long state_send_delay;
        String location;
        String topic;
        bool *sensor_half;
        bool *sensor_full;
 
};

class RailCounter {
    public:
        RailCounter();
        RailCounter(String loc, int max_n, bool *in_sensor1, bool *in_sensor2, bool *out_sensor1, bool *out_sensor2);
        void loop();
        void set_count(String value);
        
    private:
        String location;
        String topic;
        int input_counter;
        int output_counter;
        int cart_counter;
        int count_old;
        int count_online;
        int max_count;
        unsigned long state_send_timer;
        unsigned long state_send_delay;
        bool *in_sensor1;
        bool *in_sensor2;
        bool *out_sensor1;
        bool *out_sensor2;
        bool in_sensor1_old;
        bool in_sensor2_old;
        bool out_sensor1_old;
        bool out_sensor2_old;
        Preferences preferences;
 
};

#endif