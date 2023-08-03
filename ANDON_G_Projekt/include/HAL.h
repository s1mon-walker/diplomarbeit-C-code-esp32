#ifndef _HAL_H_
#define _HAL_H_

#include <Arduino.h>

class HAL {
    public:
        HAL(void);
        void setup(uint8_t in_pins[8], uint8_t out_pins[8], uint8_t error_pin);
        void input(void);
        void output(void);
        void set_invert_inputs(const bool invert_bits[8]);
        void set_invert_outputs(const bool invert_bits[8]);
        bool inputs[8];
        bool outputs[8];
        
    private:
        uint8_t input_pins[8];
        uint8_t output_pins[8];
        uint8_t error_pin;
        bool enable;
        bool temp_inputs[8];
        bool invert_inputs[8];
        bool invert_outputs[8];
        unsigned long timers[8];
        unsigned long debounce_time;
};

#endif