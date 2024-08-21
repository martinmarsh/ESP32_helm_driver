#ifndef Motor_h
#define Motor_h
#include <Arduino.h>

#define MAX_MOTOR 18000

class Motor {
    public:

    Motor(int in1, int in2, uint8_t pwm, int pwm_feq, uint8_t pwm_channel, uint8_t pwm_resolution, int max_duty, int min_duty);
    void setup();
    void standby();
    void moveto(int position);
    void position(int position);
    bool targetReached();
    void printStatus();

    
    private:
    void run();
    void action();
    void forward();
    void reverse();
    void break_stop();
    uint32_t dutyCycle;
    int max_duty;
    int min_duty;
    bool pwm_active;
    bool power_off;
    int power;
    float distance;
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