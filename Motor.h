#ifndef Motor_h
#define Motor_h
#include <Arduino.h>


class Motor {
    public:

    Motor(int in1, int in2, int pwm, int pwm_feq, int pwm_channel, int pwm_resolution, int max_duty, int min_duty);
    void setup();
    void standby();
    void forward(int power);
    void reverse(int power);
    void power(int power);
    void break_stop();
    void moveto(float position);
    void position(float position);

    
    private:
    int dutyCycle;
    int max_duty;
    int min_duty;
    bool pwm_active;
    int in1;
    int in2;
    int pwm;
    int pwm_freq;
    int pwm_channel;
    int pwm_resolution;
    int max_pwm;

};


#endif