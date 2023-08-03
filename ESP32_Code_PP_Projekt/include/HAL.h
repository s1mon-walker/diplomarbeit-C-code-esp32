#ifndef _HAL_H_
#define _HAL_H_

class HAL {
    public:
        HAL(void);
        void setup(uint8_t in_pins[8], uint8_t out_pins[8]);
        void input(void);
        void output(void);

        uint8_t input_pins[8];
        uint8_t output_pins[8];
        bool inputs[8];
        bool temp_inputs[8];
        bool outputs[8];
        unsigned long timers[8];
        unsigned long debounce_time;
};

#endif