# Minion Light Controller
This is an Arduino-based physical lighting interaction prototype.
It uses a light sensor and an ultrasonic distance sensor, together
with Wi-Fi and MQTT, to control the Vespera lighting installation
in the laboratory.

**Final version: V2. Please use the code in [code/V2](code/V2).** 
![Final Minion Light Controller prototype](image/prototype.jpg)

## 1. Project Overview and Design Inspiration
The inspiration for this project came from a streetlight sensor I spotted by the roadside. It appeared to be designed to detect ambient light levels, which gave me an idea: could I build my own device that automatically adjusts its brightness according to the surrounding light conditions?

Building on this, I added an ultrasonic distance sensor, hoping that the light would not only respond to changes in its surroundings but also alert me to nearby objects approaching by changing colour.As the final prototype of the controller resembled the Minions from the films, I named the device the ‘Minion Light Controller’.

## 2. Functions and Operation
The two sensors control different lighting attributes independently.
Both responses can occur at the same time.
### Ambient Light Control
![Light control](image/Light_control.png)
### Distance Control
![Distance control](image/Distance_control.png)

## 3. Key Components and Wiring
- Arduino MKR WiFi 1010
- VEML7700 light sensor
- HC-SR04 ultrasonic distance sensor
- Breadboard and jumper wires
- Voltage-dividing resistors
- USB cable
- Cardboard enclosure