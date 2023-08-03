#include <Arduino.h>
#include "HAL.h"

HAL::HAL() {
    ;
}

void HAL::setup(uint8_t in_pins[8], uint8_t out_pins[8]) {
    bool inputs[] = {0,0,0,0,0,0,0,0};
    bool temp_inputs[] = {0,0,0,0,0,0,0,0};
    bool outputs[] = {0,0,0,0,0,0,0,0};
    unsigned long timers[] = {0,0,0,0,0,0,0,0};
    memcpy(this->input_pins, in_pins, sizeof(this->input_pins));
    memcpy(this->output_pins, out_pins, sizeof(this->input_pins));
    memcpy(this->inputs, inputs, sizeof(this->inputs));
    memcpy(this->temp_inputs, temp_inputs, sizeof(this->temp_inputs));
    memcpy(this->outputs, outputs, sizeof(this->outputs));
    memcpy(this->timers, timers, sizeof(this->timers));
    this->debounce_time = 100; // milliseconds

    for (int i = 0; i < 8; i++) {
        pinMode(this->input_pins[i], INPUT);
        pinMode(this->output_pins[i], OUTPUT);
    }
}

void HAL::input() {
    // loop trough all input pins
    bool pin_state;
    
    for (int i = 0; i < 8; i++) {
        // read pin state
        pin_state = digitalRead(this->input_pins[i]);
        // record time between state changes
        if (pin_state != this->temp_inputs[i]) {
            this->timers[i] = millis();
        }
        // if time larger than debounce time accept temp value
        if ((millis() - this->timers[i]) > this->debounce_time) {
            this->inputs[i] = pin_state;
        }
        // updtae temp value
        this->temp_inputs[i] = pin_state;
    }
}

void HAL::output() {
    for (int i = 0; i < 8; i++) {
        digitalWrite(this->output_pins[i], !this->outputs[i]);
    }
}

