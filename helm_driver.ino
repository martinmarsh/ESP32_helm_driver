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
bool g_active = false;

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

  //rudderAngle.setBase(1, 0);    // As sensor is on rudder stock only 1 turn is possible typically +/- 40

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
  digitalWrite(LED_BUILTIN, LOW);
  if (exec_time < 0){
     Serial.printf("Shorter than 0 delay required: %i setting to 0\n", exec_time);
     exec_time = 0;   // copes with extra long functions or overflow on roll over;
  }
  delay(exec_time);
  digitalWrite(LED_BUILTIN, HIGH);

  start_time = millis();
   switch(loop_phase){
    case 0:
        update_1();
      break;
    case 1:
        update_2();
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


void update_2() {
  char* mess;
  bool ok = true;
  bool check_ok = false;
  bool active = false;
  int im;
  int is;
  int field;
  char s[64];
  float helm_pos = 0;
  float heading = 0;
  float desired_heading = 0;
  char compass_status[6];
  int check_sum = 0;
  float gain;
  float pi;
  float pd;

  if (udpComms.messageAvailable()){
    mess = udpComms.receivedMessage;
    ok = true;
    active = false;
    check_ok = false;
    im = 1;
    is = 0;
    field = 0;
    // $PXXS1,auto,hdm,'M',set_hdm,'M',compass_status,pitch,roll,compass_temp,auto_gain,auto_pi,auto_pd*checksum
    Serial.printf("Processing message = %s\n", mess);

    if (mess[0] != '$'){
      ok = false;
    }
    check_sum = 0;
    while (mess[im] != '\0' && ok) {
      if (mess[im] == '*') {
        char cs[4];
        mess[im] = ',';  //ensure last value is processed
        sprintf(&cs[0], "%02X\0", check_sum);
        if (cs[0] == mess[im+1] && cs[1] == mess[im+2]){
          check_ok = true;
        } else {
          Serial.printf("invalid checksum expected = %s got %c%c\n", cs,mess[im+1], mess[im+2]);
        }
      }
      check_sum ^= (int)(mess[im]);
      if (mess[im] != ',') {
        s[is] = mess[im]; 
      } else {
        s[is] += '\0';
        switch (field){
          case 0:
            if (strcmp(s, "PXXS1") != 0){
              ok = false;
            }
            break;
          case 1:
            if (s[0] == '1'){
              active = true;
            } 
            break;
          case 2:
            heading = atof(s);
            break;
          case 3:
            //M
            break;
          case 4:
            desired_heading = atof(s);
            break;
          case 5:
            //M
            break;
          case 6:
            strcpy(compass_status, s);
            break;
          case 7:
            //pitch
            break;
          case 8:
            //roll
            break;
          case 9:
            // temp
            break;
          case 10:
            gain = atof(s);
            break;
          case 11:
            pi = atof(s);
            break;
          case 12:
            pd = atof(s);
         
            // as we are at the last value process sentence if check_ok
            if (check_ok){
              if (active){
                g_active = true;
                Serial.printf("got active compass = %.2f, deired: %.2f\n",heading, desired_heading);
                float error = relative180(heading - desired_heading);
                helm_pos = error * gain/33.0;
                motor.moveto(helm_pos);
              } else {
                g_active = false;
                Serial.printf("got inactive compass = %.2f, deired: %.2f\n",heading, desired_heading);
                motor.break_stop();
                rudderAngle.setBase(1,0);
              }

            } else {
              Serial.printf("invalid checksum fields = %d\n", field);
            }
            ok = false;
            break;

        } 
        ++field;
        is = -1;
      }
      im++;
      is++;
    }
    udpComms.nextMessage(); 
  } 
}

void slow_update() {
  rudderAngle.checkAS5600Setup();  // check the magnet is present)
  udpComms.stateMachine();
}


void update_1() {
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

// relative180(heading - desired_heading) gives error in degrees
float relative180(float dif) {
  if (dif < -180.0) {
    dif += 360.0;
  }
  if (dif > 180.0) {
    dif -= 360.0;
  }
  return dif;
}


