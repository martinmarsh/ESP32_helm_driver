#include "esp32-hal-ledc.h"
#include "Motor.h"
#include "cmath"


Motor::Motor(int in1, int in2, int pwm, int pwm_feq, int pwm_channel, int pwm_resolution, int max_duty, int min_duty) {
    this->dutyCycle = 0;
    this->pwm_active = false;
    this->in1 = in1;
    this->in2 = in2;
    this->pwm = pwm;
    this->pwm_freq = pwm_freq;
    this->pwm_channel = pwm_channel;
    this->pwm_resolution = pwm_resolution;
    this->max_duty = max_duty;
    this->min_duty = min_duty;
    this->max_pwm = int(pow(pwm_resolution, 2));
}

void Motor::setup(){

  // sets motor pins as outputs:
  pinMode(this->in1, OUTPUT);
  pinMode(this->in2, OUTPUT);
  pinMode(this->pwm, OUTPUT);

  // set motor to off state
  this->standby();

  // set up PWM
  ledcSetup(this->pwm_channel, this->pwm_freq, this->pwm_resolution);
  

}

void Motor::standby(){
  digitalWrite(this->in1, HIGH);
  digitalWrite(this->in2, HIGH);
  this->power(0);
}

void Motor::forward(int power){
  digitalWrite(this->in1, HIGH);
  digitalWrite(this->in2, LOW);
  this->power(power);

}

void Motor::reverse(int power){
  digitalWrite(this->in1, LOW);
  digitalWrite(this->in2, HIGH);
  this->power(power);
}

void Motor::break_stop(){
  digitalWrite(this->in1, LOW);
  digitalWrite(this->in2, LOW);
  this->power(0);
}

void Motor::power(int duty){
  if (duty >= this->min_duty && duty <= this->max_duty) {
    if (!this->pwm_active) { 
        ledcAttachPin(this->pwm, this->pwm_channel);
        this->pwm_active = true;
    }
    this->dutyCycle == duty*this->max_pwm/100;
    ledcWrite(this->pwm_channel, this->dutyCycle);  
  } else{
    if (this->pwm_active) { 
        ledcDetachPin(this->pwm);
        this->pwm_active = false;
    }
    if (duty <= this->min_duty) {
        digitalWrite(this->pwm, HIGH);
    }
    if (duty >= this->max_duty){
        digitalWrite(this->pwm, LOW);
    }

  }
 
}


void Motor::moveto(float position){

}

void Motor::position(float position){


}





 