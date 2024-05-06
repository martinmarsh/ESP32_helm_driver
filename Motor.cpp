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
  //pinMode(this->pwm, OUTPUT);

  // set motor to off state
  this->standby();

  // set up PWM
  ledcAttachPin(14,1);
  ledcSetup(1, 5000, 8);
  
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
  //duty is 
  if (duty < this->min_duty){
    ledcWrite(1, 255); 
    Serial.printf("duty = %i, less than setting motor duty cycle = %i\n", duty, 255);
  } else if (duty > this->max_duty) {
    Serial.printf("duty = %i, greater than setting motor duty cycle = %i\n", duty, 0);
    ledcWrite(1, 0); 
  } else {
    this->dutyCycle = ((100-duty)*this->max_pwm)/100;
    Serial.printf("duty = %i duty cycle = %i  max = %i\n", duty, this->dutyCycle, this->max_pwm);
    ledcWrite(1, this->dutyCycle); 

  }

  /*
  if (duty >= this->min_duty && duty <= this->max_duty) {
    if (!this->pwm_active) { 
        ledcAttachPin(this->pwm, this->pwm_channel);
        this->pwm_active = true;
    }
    this->dutyCycle = (duty*this->max_pwm)/100;
    ledcWrite(this->pwm_channel, this->dutyCycle);  
    Serial.printf("motor duty = %i  max = %i\n", this->dutyCycle, this->max_pwm);
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
  */

}


void Motor::moveto(int position){
  // this is based on multi turn angle
  this->desired_position = position;
  this->run();

}

void Motor::position(int position){
  //  this is based on multi turn angle
  this->last_position = position;
  this->run();
}

void Motor::run(){
  float distance = float(this->desired_position - this->last_position);
 
  float abs_distance = abs(distance);
  int power = int (abs_distance / 20);
  Serial.printf("motor abs distance = %.2f, distance =  %.2f, desired = %i, last motor = %i power = %i\n",  abs_distance, distance, this->desired_position, this->last_position, power);
  
  if (abs_distance <= 500.0){
     power = 0;
  } 
  if (distance < 0){
    this->forward(power);
  } else {
    this->reverse(power);
  }


}





 