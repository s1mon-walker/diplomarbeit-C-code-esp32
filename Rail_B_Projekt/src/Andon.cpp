#include "Andon.h"

AndonLine::AndonLine() {
    this->state = 0;
}

AndonLine::AndonLine(String loc, bool *input, bool *green, bool *orange, bool *red) {
    this->state = 0;
    this->online_state = 0;
    this->location = loc;
    this->topic = "Andon/" + this->location + "/State";
    this->switch_cooldown = 1000;
    this->cooldown_timer = 0;
    this->state_send_delay = 5000;
    this->state_send_timer = 0;
    this->blink_timer = 0;
    this->blink_interval = 500; //ms
    this->blink_bit = false;
    this->old_switch_state = 0;
    this->state_switch = input;
    this->green_light = green;
    this->orange_light = orange;
    this->red_light = red;
}

void AndonLine::loop() {
    // change init state to off
    if (this->state == 0) {
        Serial.println("ANDON: INIT -> OFF");
        this->state = 1;
    }

    // detect positive edge of switch and change state
    if (*this->state_switch == true && this->old_switch_state == false) {
        // only switch state after a minimum cooldown since last switch
        if ((unsigned long)(millis() - this->cooldown_timer) > this->switch_cooldown) {
            this->cooldown_timer = millis();
            
            switch (this->state) {
                case 1: // Aus
                    this->state = 2;
                    break;
                case 2: // Produktion
                    this->state = 3;
                    break;
                case 3: // QIP
                    this->state = 4;
                    break;
                case 4: // STOP
                    this->state = 1;
                    break;

            // memorise time of change
            this->state_send_timer = millis();
            }
        }
    }
    this->old_switch_state = *this->state_switch;

    // publish state if it not matches the already sent state
    if (this->online_state != this->state) {
        // only publish if a mimimum time has passed since change
        if ((unsigned long)(millis() - this->state_send_timer) > this->state_send_delay) {
            publish(this->topic, String(this->state).c_str());
            this->online_state = this->state;
        }
    }

    // flip blink_bit after in blink interval
    if ((unsigned long)(millis() - this->blink_timer) > this->blink_interval) {
        this->blink_bit = !this->blink_bit;
        this->blink_timer = millis();
    }

    // turn on lights according to the state
    switch (this->state) {
        case 1: // Aus
            *this->green_light = false;
            *this->orange_light = false;
            *this->red_light = false;
            break;
        case 2: // Produktion
            *this->green_light = true;
            *this->orange_light = false;
            *this->red_light = false;
            break;
        case 3: // QIP
            *this->green_light = false;
            *this->orange_light = true;
            *this->red_light = false;
            break;
        case 4: // STOP
            *this->green_light = false;
            *this->orange_light = false;
            *this->red_light = this->blink_bit;
            break;
    }
}

FifoShelf::FifoShelf() {
    // empty
}

FifoShelf::FifoShelf(String loc, bool *sensor_input, bool *light) {
    this->state = 0;
    this->old_state = 0;
    this->online_state = 0;
    this->state_send_timer = 0;
    this->state_send_delay = 5000; //ms
    this->location = loc;
    this->topic = "Fifo/" + this->location + "/State";
    this->sensor = sensor_input;
    this->light = light;
}

void FifoShelf::loop() {
    // set state based on sensor
    this->state = int(*this->sensor);

    // switch light based on state
    *this->light = (bool)this->state;

    // memorise time of state change
    if (this->state != this->old_state) state_send_timer = millis();

    // publish state if it not matches the already sent state
    if (this->online_state != this->state) {
        // only publish if a mimimum time has passed since change
        if ((unsigned long)(millis() - this->state_send_timer) > this->state_send_delay) {
            publish(this->topic, String(this->state).c_str());
            this->online_state = this->state;
        }
    }

    this->old_state = this->state;
}

ScrapContainer::ScrapContainer() {
    //empty
}

ScrapContainer::ScrapContainer(String loc, bool *sensor_half, bool *sensor_full) {
    this->state = 0;
    this->old_state = 0;
    this->online_state = 0;
    this->state_send_timer = 0;
    this->state_send_delay = 5000; //ms
    this->location = loc;
    this->topic = "Scrap/" + this->location + "/State";
    this->sensor_half = sensor_half;
    this->sensor_full = sensor_full;
}

void ScrapContainer::loop() {
    // set state based on sensors
    this->state = 0;
    if (*this->sensor_half) this->state = 1;
    if (*this->sensor_full) this->state = 2;

    // memorise time of state change
    if (this->state != this->old_state) this->state_send_timer = millis();

    // publish state if it not matches the already sent state
    if (this->online_state != this->state) {
        // only publish if a mimimum time has passed since change
        if ((unsigned long)(millis() - this->state_send_timer) > this->state_send_delay) {
            publish(this->topic, String(this->state).c_str());
            this->online_state = this->state;
        }
    }

    this->old_state = this->state;
}

RailCounter::RailCounter() {
    //empty
}

RailCounter::RailCounter(String loc, int max_n, bool *in_sensor1, bool *in_sensor2, bool *out_sensor1, bool *out_sensor2) {
    this->input_counter = 0;
    this->output_counter = 0;
    this->cart_counter = 0;
    this->count_old = 0;
    this->count_online = 0;
    this->state_send_timer = 0;
    this->state_send_delay = 5000; //ms
    this->max_count = max_n;
    this->location = loc;
    this->topic = "Rail/" + this->location + "/Count";
    this->in_sensor1 = in_sensor1;
    this->in_sensor2 = in_sensor2;
    this->out_sensor1 = out_sensor1;
    this->out_sensor2 = out_sensor2;
    this->in_sensor1_old = *this->in_sensor1;
    this->in_sensor2_old = *this->in_sensor2;
    this->out_sensor1_old = *this->out_sensor1;
    this->out_sensor2_old = *this->out_sensor2;

    // get localy saved data from flash memory
    this->preferences.begin(this->location.c_str());
    this->input_counter = this->preferences.getInt("input_counter", 0);
    this->output_counter = this->preferences.getInt("output_counter", 0);
    this->preferences.end();
}

void RailCounter::loop() {

    //Encoder logic decodeing input sensors
    if (!this->in_sensor1_old && *this->in_sensor1) { if (*this->in_sensor2) {this->input_counter++;} else {this->input_counter--;} }
    if (this->in_sensor1_old && !*this->in_sensor1) { if (*this->in_sensor2) {this->input_counter--;} else {this->input_counter++;} }
    if (!this->in_sensor2_old && *this->in_sensor2) { if (*this->in_sensor1) {this->input_counter--;} else {this->input_counter++;} }
    if (this->in_sensor2_old && !*this->in_sensor2) { if (*this->in_sensor1) {this->input_counter++;} else {this->input_counter--;} }
    this->in_sensor1_old = *this->in_sensor1;
    this->in_sensor2_old = *this->in_sensor2;

    //Encoder logic decodeing output sensors
    if (!this->out_sensor1_old && *this->out_sensor1) { if (*this->out_sensor2) {this->output_counter++;} else {this->output_counter--;} }
    if (this->out_sensor1_old && !*this->out_sensor1) { if (*this->out_sensor2) {this->output_counter--;} else {this->output_counter++;} }
    if (!this->out_sensor2_old && *this->out_sensor2) { if (*this->out_sensor1) {this->output_counter--;} else {this->output_counter++;} }
    if (this->out_sensor2_old && !*this->out_sensor2) { if (*this->out_sensor1) {this->output_counter++;} else {this->output_counter--;} }
    this->out_sensor1_old = *this->out_sensor1;
    this->out_sensor2_old = *this->out_sensor2;

    // calculating carts from counters
    int wheels_input = this->input_counter / 4;  // inputcounter raising by 4 indicates +1 wheel in rail
    int carts_input = (wheels_input/2) + (wheels_input%2);  // round up so 1-2 wheels indicate 1 cart

    int wheels_output = this->output_counter / 4;  // outputcounter raising by 4 inicates -1 wheel in rail
    int carts_output = wheels_output/2;  // no round up so the cars at output only count when both wheels gone

    this->cart_counter = carts_input - carts_output;  // set cartcounter according to calculations

    Serial.print("In: ");
    Serial.print(this->input_counter);
    Serial.print("  Out: ");
    Serial.print(this->output_counter);
    Serial.print("  Carts: ");
    Serial.println(this->cart_counter);

    // keep cart_counter in bounds
    if (this->cart_counter < 0) this->cart_counter = 0;
    if (this->cart_counter > this->max_count) this->cart_counter = this->max_count;

    // reset counters whenever they are equal
    if (this->input_counter == this->output_counter) {
        this->input_counter = 0;
        this->output_counter = 0;
    }

    // memorise time of count change
    if (this->cart_counter != this->count_old) this->state_send_timer = millis();

    // publish count if it not matches the already sent count
    if (this->count_online != this->cart_counter) {
        // only publish if a mimimum time has passed since change
        if ((unsigned long)(millis() - this->state_send_timer) > this->state_send_delay) {
            publish(this->topic, String(this->cart_counter).c_str());
            this->count_online = this->cart_counter;

            // save new counter state to local flash memory
            this->preferences.begin(this->location.c_str());
            this->preferences.putInt("input_counter", this->input_counter);
            this->preferences.putInt("output_counter", this->output_counter);
            this->preferences.end();
        }
    }

    this->count_old = this->cart_counter;
}

void RailCounter::set_count(String value) {
    int count = atoi(value.c_str());  // new cart count
    this->input_counter = count * 8;  // 1 cart is equivalent to 8 counter increments
    this->output_counter = 0;
}
