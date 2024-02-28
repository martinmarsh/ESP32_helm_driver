#include <Arduino.h>

#include <Wire.h>  // This is for i2C

#include <SPI.h>
#include <stdlib.h>

#include "Rudder_angle.h"
#include "UdpComms.h"
#include "Motor.h"


//The wifi passwords are stored in net_config.h which is not commited to public repo
//for security of wifi passwords etc
//Your must rename example_net_config.h to net_config.h when your passwords are set
//and comment in next line and comment out the example_net_config.h
#include "net_config.h"
//#include "example_net_config.h"


// define ESP32 Doit Dev board v1 to STK681-360-E motor driver
// max PWM = 20khz 10us min pulse
#define MOTOR_IN1 26
#define MOTOR_IN2 27
#define MOTOR_PWM 14

// Setting PWM properties
#define PWM_FREQ 15000
#define PWM_CHANNEL 0
#define PWM_RESOLUTION 8  // power is expressed 0 to 100 - resolution shoulb be > 7 bits 



#define LOOP_DELAY 33      //Each loop is 3*loop_delay ms= fast_update period
  
RudderAngle rudderAngle;

 
float g_gain = 100;
float g_pi = 100;
float g_pd = 100;

bool g_start = true;
bool g_auto_on = false;

UdpComms udpComms(SSID_A, PASSWORD_A, SSID_B, PASSWORD_B, BROADCAST_PORT, LISTEN_PORT, RETRY_PASSWORD);
Motor motor(MOTOR_IN1, MOTOR_IN2, MOTOR_PWM, PWM_FREQ, PWM_CHANNEL, PWM_RESOLUTION, 80, 20);
 
int update_counter = 0;
int slow_update_counter = 0;
int loop_phase = 0;      //Ensures only one routine runs per loop

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  motor.setup();

  Serial.begin(115200);
  Serial.println("Starting");

  rudderAngle.setBase(1, 0);

  digitalWrite(LED_BUILTIN, HIGH);

  delay(1000);  // Pause for 1 seconds

  digitalWrite(LED_BUILTIN,LOW);

  Wire.begin();            // start i2C
  Wire.setClock(400000L);  // fast clock

  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
}

void loop() {
  unsigned long start_time;
  unsigned long end_time;
 
  unsigned long exec_time =  LOOP_DELAY - end_time - start_time;
  if (exec_time < 0){
     Serial.printf("Shorter than 0 delay required: %i setting to 0\n", exec_time);
     exec_time = 0;   // copes with extra long functions or overflow on roll over;
  }
  
  delay(exec_time);
  start_time = millis();
   switch(loop_phase){
    case 0:
        fast_update();
      break;
    case 1:
        if (++update_counter > 4){
          update();
          update_counter = 0;
        }
      break;
    case 2:
      if (++slow_update_counter > 16){
          slow_update();
          slow_update_counter = 0;
        }
      break;
    default:
      Serial.printf("Phase Error in loop: %i\n", loop_phase);
  }
  if (++loop_phase > 2 ) loop_phase = 0;
  end_time = millis();
}


void update() {

   
}

void slow_update() {
  rudderAngle.checkAS5600Setup();  // check the magnet is present)
  udpComms.stateMachine();
}


void fast_update() {
  rudderAngle.read();
  motor.position(rudderAngle.getRotation());

  
}

void addCheckSum(char* buf){
	int check_sum = 0;
  int i;

	for (i = 1; i < strlen(buf); i++) {
		check_sum ^= (int)(buf[i]);
	}
  sprintf(&buf[i], "*%02X\n", check_sum);
}

