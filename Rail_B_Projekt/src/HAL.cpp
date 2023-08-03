#include "HAL.h"

// empty constructor for declaration
HAL::HAL() {
    ;
}

void HAL::setup(uint8_t in_pins[8], uint8_t out_pins[8], uint8_t error_pin) {
    // inizialise zero arrays for setting up memory
    bool bool_zero_array[] = {0,0,0,0,0,0,0,0};
    unsigned long timers[] = {0,0,0,0,0,0,0,0};

    // coping the zero values in the memory of class values
    memcpy(this->input_pins, in_pins, sizeof(this->input_pins));
    memcpy(this->output_pins, out_pins, sizeof(this->input_pins));
    memcpy(this->inputs, bool_zero_array, sizeof(this->inputs));
    memcpy(this->temp_inputs, bool_zero_array, sizeof(this->temp_inputs));
    memcpy(this->outputs, bool_zero_array, sizeof(this->outputs));
    memcpy(this->timers, timers, sizeof(this->timers));
    memcpy(this->invert_inputs, bool_zero_array, sizeof(this->invert_inputs));
    memcpy(this->invert_outputs, bool_zero_array, sizeof(this->invert_outputs));
    
    this->enable = true;
    this->debounce_time = 50; // milliseconds

    // set pinMode for all pins
    pinMode(this->error_pin, INPUT);
    for (int i = 0; i < 8; i++) {
        pinMode(this->input_pins[i], INPUT);
        pinMode(this->output_pins[i], OUTPUT);
    }
}

void HAL::input() {
    bool pin_state;

    // loop trough all input pins
    for (int i = 0; i < 8; i++) {
        // read pin state
        pin_state = digitalRead(this->input_pins[i]);
        // record time between state changes
        if (pin_state != this->temp_inputs[i]) {
            this->timers[i] = millis();
        }
        // if time larger than debounce time accept temp value
        if ((millis() - this->timers[i]) > this->debounce_time) {
            // invert input if true in the invert_inputs array
            if (this->invert_inputs[i]) {
                this->inputs[i] = !pin_state;
            } else {
                this->inputs[i] = pin_state;
            }   
        }
        // update temp value
        this->temp_inputs[i] = pin_state;
    }
}

void HAL::output() {
    //if (!digitalRead(this->error_pin)) this->enable = false;

    // enable does not work due to trouble with the electronics
    if (this->enable) {
        // loop trough all output pins
        for (int i = 0; i < 8; i++) {
            // invert output if true in the invert_outputs array
            if (this->invert_outputs[i]) {
                digitalWrite(this->output_pins[i], !this->outputs[i]);
            } else {
                digitalWrite(this->output_pins[i], this->outputs[i]);
            }
        }
    } else {
        for (int i = 0; i < 8; i++) {
            digitalWrite(this->output_pins[i], false);
        }
    }
}

// HAL will invert all inputs with true in corresponding array element
void HAL::set_invert_inputs(const bool invert_bits[8]) {
    // copy the values from method argument to memory of objects invert_inputs array
    memcpy(this->invert_inputs, invert_bits, sizeof(this->invert_inputs));
}

// HAL will invert all outputs with true in corresponding array element
void HAL::set_invert_outputs(const bool invert_bits[8]) {
    // copy the values from method argument to memory of objects invert_outputs array
    memcpy(this->invert_outputs, invert_bits, sizeof(this->invert_outputs));
}
