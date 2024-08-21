#include "esp32-hal-ledc.h"
#include "Motor.h"
#include "cmath"


Motor::Motor(int in1, int in2, uint8_t pwm, int pwm_feq, uint8_t pwm_channel,uint8_t pwm_resolution, int max_duty, int min_duty) {
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
    this->max_pwm = int(pow(2, pwm_resolution))-1;
    this->power_off = true;
    this->power = 0;
    this->distance = 0;
}

void Motor::setup(){

  // sets motor pins as outputs:
  pinMode(this->in1, OUTPUT);
  pinMode(this->in2, OUTPUT);

  // set motor to off state
  this->standby();

  // set up PWM
  ledcAttachPin(this->pwm, this->pwm_channel);
  ledcSetup(this->pwm_channel, this->pwm_freq, this->pwm_resolution);
  ledcWrite(this->pwm_channel, this->max_pwm); 
  
}

void Motor::standby(){
  digitalWrite(this->in1, HIGH);
  digitalWrite(this->in2, HIGH);
}

void Motor::forward(){
  digitalWrite(this->in1, HIGH);
  digitalWrite(this->in2, LOW);
  this->action();
}

void Motor::reverse(){
  digitalWrite(this->in1, LOW);
  digitalWrite(this->in2, HIGH);
  this->action();
}

void Motor::break_stop(){
  digitalWrite(this->in1, LOW);
  digitalWrite(this->in2, LOW);
  ledcWrite(this->pwm_channel, this->max_pwm); 
}

void Motor::action(){
  //duty is a percentage 100 = fully on, channel is 0 for full on and off is a number based on resolution 
  if (this->power < this->min_duty){ 
    this->break_stop();
    this->power_off = true;
    //Serial.printf("duty = %i, less than setting min duty cycle = %i,  applied duty: %i, motor off\n", duty,  this->min_duty, this->max_pwm);
  } else if (this->power > this->max_duty) {
    //Serial.printf("duty = %i, greater than setting max duty cycle = %i, applied duty: 0, motor fully on\n", duty, this->max_duty);
    ledcWrite(this->pwm_channel, 0); 
    this->power_off = false;
  } else {
    this->dutyCycle = ((100-this->power)*this->max_pwm)/100;
    //Serial.printf("duty = %i duty cycle = %i  min power = %i  max = 0\n", duty, this->dutyCycle, this->max_pwm);
    ledcWrite(this->pwm_channel, this->dutyCycle);
    this->power_off = false; 
  }
}


bool Motor::targetReached() {
  return this->power_off;
}

void Motor::moveto(int position){
  // this is based on multi turn angle
  
  if (position > MAX_MOTOR){
      position = MAX_MOTOR;
  } else if (position < -MAX_MOTOR){
      position = -MAX_MOTOR;
  }
  this->desired_position = position;
}

void Motor::position(int position){
  // must be called at regular intervals to start motor and stop at desired position
  // position is multi turn angle with offset applied
  if (position > MAX_MOTOR){
      position = MAX_MOTOR;
    } else if (position < -MAX_MOTOR){
      position = -MAX_MOTOR;
  }
  this->last_position = position;
  this->run();
}

void Motor::run(){
  float abs_distance;
  this->distance = float(this->last_position - this->desired_position);
  abs_distance = abs(this->distance);
  this->power = int (abs_distance / 40);
  if (this->distance > 0){
    this->forward();
  } else {
    this->reverse();
  }
  
}


void Motor::printStatus(){
   Serial.printf("Motor: distance = %.2f, desired = %i, position = %i, duty = %i, power = %i off = %d\n",
    this->distance, this->desired_position, this->last_position, this->dutyCycle, this->power, this->power_off);
 
}
