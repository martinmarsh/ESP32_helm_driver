# Helm Driver - UDP API, rudder angle sensor and helm motor control using ESP32 dev board

This is code is designed to run on a standard ESP32 dev board such as the Doit ESP32 version 1
board.  It is a component in a boat auto-helm system which allows the rudder angle to be controlled over Wifi.
The other major component in this system is another portable compass and control unit based on  the ESP32 fire beetle, compass senso and controller see repro
Compass_cmps12.

A rotary encoder AS5600 mounted on the auto-helm drive motor measures the boat's rudder angle and an H-Bridge motor provided bi-directional motor control.

The motor drives via internal gearing the steering wheel either directly or in my case via a belt.

The rudder angle could also be detected by measuring the steering wheel rotation or by measuring the deflection of the rudder direclty by mouting the sensor
on the rudder stock.   I opted for mounting the rotation sensor on the motor unit so that the ESP32, H-Bridge controller and roation sensor
could be built into one detachable unit requiring only a 12v supply and wifi connection. 

A wiFi base UPD based Api allows the Compass heading, required heading and PID contoller parameters to be recieved several times a second.

The ESP32 dev board is preferred for this application as it is smaller than the Fire-beetle board and can be powered via a pin rather than the usb socket.
The Fire-beetle board is better for self powered battery applications as it has LiPo charge controller and has preferred powering via the USB connector.
The Fire-beetle board is therefore preferred in the battery powerred portable compass controller.
