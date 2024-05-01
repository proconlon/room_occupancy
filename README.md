# Room Occupancy Monitor

Team:
- James Conlon
- Phyliss Darko
- Allen Fraiman
- Hayden Robinson

[Demo Video](https://youtu.be/HA0_gBuAFfs)

## Final Report
See the Final Report.pdf for more details.

## Overview
This Arduino system utilizes two HC-SR04 ultrasonic sensors to monitor and control room occupancy based on a predefined maximum capacity. The system is designed to determine the direction of movement (entry or exit) by detecting which sensor is triggered first. This project includes debouncing mechanisms, dynamic threshold calibration, mechanical indicators, and an LCD display to show current occupancy count.

## Features
- **Directional Detection:** Utilizes two ultrasonic sensors positioned to detect the direction of entry and exit.
- **Debouncing:** Ensures accurate detection by requiring a minimum time between successive entries or exits.
- **Dynamic Threshold Calibration:** Automatically adjusts the entry threshold based on the detected distance at startup, accommodating different doorway sizes.
- **Visual/Mechanical Indicators:** Includes LEDs and a mechanical arm to signal when the room reaches maximum capacity.
- **LCD Display:** Shows the current number of occupants and indicates whether more people can enter or if the room is full.

## Hardware
- 2 x HC-SR04 Ultrasonic Sensors
- Arduino Uno or similar microcontroller
- LEDs (Red and Green)
- Buzzer
- Stepper Motor
- LiquidCrystal_I2C LCD Display

## Setup and Installation
- **Calibration:** Upon first setup, allow the system to calibrate the sensors by ensuring the doorway is clear during startup.

## Configuration
Adjust the following parameters in the code to suit your specific environment:
- `MAX_OCCUPANCY`: Maximum number of people allowed in the room.
- `ENTRY_THRESHOLD`: Minimum distance for the sensors to detect presence.
- `DEBOUNCE_TIME`: Minimum interval between detected entries/exits. (not recommended to modify default)
- `SENSOR_DELAY`: Time to wait for a second sensor to trigger after the first is activated. (not recommended)

## Usage
Once set up, the system will continuously monitor the entry and exit of individuals. The LCD display will update in real-time, and the system will alert through visual and auditory signals if maximum capacity is reached.

