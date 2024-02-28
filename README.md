# Helm Driver - UDP API, rudder angle sensor and helm motor control using ESP32 dev board

This is code is designed to run on a standard ESP32 dev board such as the Doit ESP32 version 1
board.  It is a component in a boat auto-helm system which allows the rudder angle to be controlled over Wifi.
The other major component in this system is the ESP32 fire beetle based compass sensor and controller see repro
Compass_cmps12.

A rotary encoder AS5600 mounted on the rudder stock measures the boat's rudder angle and an H-Bridge motor controller connected to a auto-helm motor which can be connected to the steering wheel.

A wiFi base UPD based Api allows the rudder angle to be measured and can configure closed loop control of rudder position so that any rudder angle can be set via higher level logic.

The ESP32 dev board is preferred for this application as it is smaller than the Fire-beetle board and can be powered via a pin rather than the usb socket.  The Fire-beetle board is better for self powered battery applications as it has LiPo charge controller and preferred powering via the USB connector.
