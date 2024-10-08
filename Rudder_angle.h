#ifndef RudderAngle_h
#define RudderAngle_h
#include <Arduino.h>
#include <Wire.h>  // This is for i2C

#define AS5600 0x36  //Comment out if AS5600 not used otherwise set value of the status register (MD, ML, MH)

class RudderAngle  {
  
  public:
    RudderAngle();
    void read();
    void checkAS5600Setup();
    int getMagnetStatus();
    int getRotation();
    void setBase(bool full);
    void printRotation();
    
  private:
    bool button_pushed_;
    int angle_;                   // angle 4096 full circle
    int rotation_angle_;          // logical angle based on multi turns
    int rotations_;              // rotations Left -, + Right
    int last_angle_;
    int last_rotations_;
    bool AS5600Setup_; 
    int magnet_status_;
    int offset_ ;               // base rotation angle

    int last_angle_read_;         // used by rotation detector only

    void checkMagnetPresence_();
    void readRaw_();
    void computeRotations_();
 
};

#endif
