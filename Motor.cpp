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
  ledcWrite(this->pwm_channel, this->max_pwm); 
}

void Motor::power(int duty){
  //duty is a percentage 100 = fully on, channel is 0 for full on and off is a number based on resolution 
  if (duty < this->min_duty){ 
    this->break_stop();
    //Serial.printf("duty = %i, less than setting min duty cycle = %i,  applied duty: %i, motor off\n", duty,  this->min_duty, this->max_pwm);
  } else if (duty > this->max_duty) {
    //Serial.printf("duty = %i, greater than setting max duty cycle = %i, applied duty: 0, motor fully on\n", duty, this->max_duty);
    ledcWrite(this->pwm_channel, 0); 
  } else {
    this->dutyCycle = ((100-duty)*this->max_pwm)/100;
    //Serial.printf("duty = %i duty cycle = %i  min power = %i  max = 0\n", duty, this->dutyCycle, this->max_pwm);
    ledcWrite(this->pwm_channel, this->dutyCycle); 

  }
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
  float distance = float(this->last_position - this->desired_position);
 
  float abs_distance = abs(distance);
  int power = int (abs_distance / 40);
  Serial.printf("motor abs distance = %.2f, distance =  %.2f, desired = %i, last motor = %i power = %i\n",  abs_distance, distance, this->desired_position, this->last_position, power);
  
  if (distance > 0){
    this->forward(power);
  } else {
    this->reverse(power);
  }
  
}
