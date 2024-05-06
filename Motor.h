#ifndef Motor_h
#define Motor_h
#include <Arduino.h>


class Motor {
    public:

    Motor(int in1, int in2, uint8_t pwm, int pwm_feq, uint8_t pwm_channel, uint8_t pwm_resolution, int max_duty, int min_duty);
    void setup();
    void standby();
    void forward(int power);
    void reverse(int power);
    void power(int power);
    void break_stop();
    void moveto(int position);
    void position(int position);

    
    private:
    void run();
    uint32_t dutyCycle;
    int max_duty;
    int min_duty;
    bool pwm_active;
    int in1;
    int in2;
    uint8_t pwm;
    int pwm_freq;
    uint8_t pwm_channel;
    uint8_t pwm_resolution;
    int max_pwm;
    int desired_position;
    int last_position;
};


#endif