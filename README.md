# Step Counter 
Atmega32u4 based counter of steps with accelometer, sd card and oled display

# Main features
- Program runs on AtMega32u4 which has a USB port. After flashing uC with Caterina bootloader over ICSP pins, AtMega32u4 can be programmed over USB. 
- LiPo 3,7 V battery, can be charged over USB 
- Accelerometer ADXL335 with analog outputs for X, Y and Z directions.
- OLED display for displaying data, can be turned off to maximise battery life.
- SD holder, used to store data during development. Currently not used but can be used to save data about user (Saved sessions, multiple profiles, ect.)
- For interfacing with SD Card, PetitFS library was used. If you want to know more you should check the [website](http://elm-chan.org/fsw/ff/00index_p.html) 

# Description of folders
- **StepCounter - code**: All the code regarding the software
- **Doumentation**: Schematic and PCB file. 