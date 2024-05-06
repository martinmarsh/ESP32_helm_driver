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
#define PWM_FREQ 5000
#define PWM_CHANNEL 1
#define PWM_RESOLUTION 8  // power is expressed 0 to 100 - resolution shoulb be > 7 bits 

// Set number of turns of position sensor lock to lock;  1 for direct drive on rudder stock
#define HELM_GEAR_RATIO 13



#define LOOP_DELAY 33      //Each loop is 3*loop_delay ms= fast_update period
  
RudderAngle rudderAngle;

 
float g_gain = 100;
float g_pi = 100;
float g_pd = 100;
float g_last_heading = -1;
float p_integral = 0;

bool g_start = true;
bool g_auto_on = false;


UdpComms udpComms(SSID_A, PASSWORD_A, SSID_B, PASSWORD_B, BROADCAST_PORT, LISTEN_PORT, RETRY_PASSWORD);
Motor motor(MOTOR_IN1, MOTOR_IN2, MOTOR_PWM, PWM_FREQ, PWM_CHANNEL, PWM_RESOLUTION, 80, 20);
 
int update_counter = 0;
int slow_update_counter = 0;
int loop_phase = 0;      //Ensures only one routine runs per loop

 struct mdata {
    bool check_ok;
    bool active;
    bool set_base;
    float helm_pos;
    float heading;
    float desired_heading;
    char compass_status[6];
    int check_sum;
    float gain;
    float pi;
    float pd;
    float error;
    float change;
  };


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  motor.setup();

  Serial.begin(115200);
  Serial.println("Starting");

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
  digitalWrite(LED_BUILTIN, HIGH);
  if (exec_time < 0){
     Serial.printf("Shorter than 0 delay required: %i setting to 0\n", exec_time);
     exec_time = 0;   // copes with extra long functions or overflow on roll over;
  }
  delay(exec_time);
  digitalWrite(LED_BUILTIN, LOW);

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


bool process_message_1( char* s, int field, mdata& md){
  bool ok = true;
  switch (field){
    case 1:
      if (s[0] == '1'){
        md.active = true;
      } 
      break;
    case 2:
      md.heading = atof(s);
      break;
    case 3:
      //M
      break;
    case 4:
      md.desired_heading = atof(s);
      break;
    case 5:
      //M
      break;
    case 6:
      strcpy(md.compass_status, s);
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
      //Serial.print(s);
      md.gain = atof("100");
      break;
    case 11:
      md.pi = atof("100");
      break;
    case 12:
      md.pd = atof(s);
      // as we are at the last value process sentence if check_ok
      if (md.check_ok){
        if (md.active){
          g_auto_on = true;
          g_gain = md.gain;
          g_pi = md.pi;
          g_pd = md.pd;
          md.change = 0;
          md.error = relative180(md.heading - md.desired_heading);
          if (g_last_heading >= 0) {
            md.change = relative180(md.heading - g_last_heading);
          }
          p_integral += md.error *  g_pi/3000.0;
          if (p_integral > 10){
            p_integral = 10;
          } else if (p_integral < -10){
            p_integral = -10;
          }
          g_last_heading = md.heading;
          //helm_pos = (md.error + md.change * g_pd/100 + p_integral) * g_gain/50.0;
          md.helm_pos = md.error * g_gain * 10;
          //if (md.helm_pos > 30){
          //  md.helm_pos = 30;
          //} else if (md.helm_pos < -30){
          //  md.helm_pos = -30;
          //}

          Serial.printf("active helm head %.1f, desired: %.1f helm_pos %.2f error %.2f integral %.3f change %.1f\n", md.heading, md.desired_heading, md.helm_pos, md.error, p_integral, md.change);
          Serial.printf("GAIN %.1f - %.1f, PI:  %.1f - %.1f, PD: %.1f - %.1f\n", md.gain, g_gain, md.pi, g_pi, md.pd, g_pd);
 
          motor.moveto(int(md.helm_pos));
        } else {

          // not active
          g_last_heading = -1;
          p_integral = 0;
          g_auto_on = false;
          //Serial.printf("got inactive compass = %.2f, desired: %.2f\n", md.heading,  md.desired_heading);
          motor.break_stop();
          rudderAngle.setBase(HELM_GEAR_RATIO, 0);
        }

      } else {
          Serial.printf("invalid message1 checksum field = %d\n", field);
      }
      ok = false;
      break;
  }        

  return ok;
}


bool process_message_2( char* s, int field, mdata& md){
  bool ok = true;
  switch (field){
     case 1:
      if (s[0] == '1'){
        md.set_base = true;
      } 
      break;
    case 2:
      if (md.set_base){
        rudderAngle.setBase(HELM_GEAR_RATIO, 0);
      }
      md.helm_pos = atof(s);
      motor.moveto(md.helm_pos);
      Serial.printf("helm_pos %.2f set_base %d\n",md.helm_pos, md.set_base);   
      break;
  }
  return ok;
}

void update_2() {
  char* mess;
  bool ok = true;
  int message_no = 0;
  int im;
  int is;
  int field;
  char s[64];
  mdata message_data;
  
  if (udpComms.messageAvailable()){
    mess = udpComms.receivedMessage;
    ok = true;
    message_data.active = false;
    message_data.set_base = false;
    message_data.check_ok = false;
    im = 1;
    is = 0;
    field = 0;
    message_no = 0;
    // $PXXS1,auto,hdm,'M',set_hdm,'M',compass_status,pitch,roll,compass_temp,auto_gain,auto_pi,auto_pd*checksum
    // $PXXS2, set_base(1 or 0), helm_pos
    Serial.printf("Processing message = %s\n", mess);

    if (mess[0] != '$'){
      ok = false;
    }
    message_data.check_sum = 0;
    while (mess[im] != '\0' && ok) {

      if (mess[im] == '*') {
        char cs[4];
        mess[im] = ',';  //ensure last value is processed
        sprintf(&cs[0], "%02X\0",  message_data.check_sum);
        if (cs[0] == mess[im+1] && cs[1] == mess[im+2]){
           message_data.check_ok = true;
        } else {
          Serial.printf("invalid checksum expected = %s got %c%c\n", cs,mess[im+1], mess[im+2]);
        }
      }
      message_data.check_sum ^= (int)(mess[im]);

      
      if (mess[im] != ',') {
        s[is] = mess[im]; 
      } else {
        s[is] = '\0';
        if (field == 0 ){
           if (strcmp(s, "PXXS1") == 0){
              message_no = 1;
            } else if  (strcmp(s, "PXXS2") == 0){
              message_no = 2;
            } else {
              ok = false;
            }
        } else if (message_no == 1){
          ok = process_message_1(s, field, message_data); 
        } else if (message_no == 2){
          ok = process_message_2(s, field, message_data); 
        }
        ++field;
        is = -1;
      }
      if (im > 82 or is > 60){
        ok = false;   // abort if message parts are too large
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
  Serial.print("update 1\n"); 
  rudderAngle.read();
  if (g_auto_on){
    motor.position(rudderAngle.getRotation());
  }
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


