# Lambda Shifter 2.0

## About this device
This device is intended for Moronic 2.4 ECUs equipped with a narrowband 02 sensor to adjust fuel in the cruise range at low-mid RPM and low load to improve driving characteristics. In the stock condition, the closed-loop air-fuel ratio is fixed at 14.7 to comply with environmental regulations. With this device, it can be set richer to about 13.7.  
The advantages of making it richer are as follows

- Stable idling
- Increased mid- and low-speed torque
- Improved throttle response in cruising range
- Lower engine temperature

All other functions of the device can be controlled by a smartphone or other wireless LAN-compatible device.  
This device corresponds to STEP 1 in the project. The entire project is described below.

## Areas requiring assistance
- Power supply design validation
- Knowledge of collaborative development with GitHUB
- Device box design
 ->We are currently using a generic product called FLT20190828J-0058.

## Project Outline
A/F Ratio Tuning/Visualization Device Development Project Summary

### Objective
To facilitate setting and visualization of the air-fuel ratio of a motorcycle.  
By visualization, we would like to understand the health condition of motorcycles and evaluate the appropriateness for customization of intake and exhaust systems.

### Background
Custom ECU chips, sensor spoofing, and other devices of questionable effectiveness have been sold in large numbers.  
However, users do not have the means to objectively evaluate the validity of these devices.

### Target.
- Motorcycle users with one or two Bosch narrowband O2 sensors
- BMW R1100S, R1150R, R1150RT, and R1150GS are targeted until STEP 1 described below.
  Because the basic specifications of the air-fuel ratio measurement and ECU are the same as those of the test machine (R1100S).

### GOAL
Develop and market a device that can set and visualize closed-loop and open-loop A/F ratios using a smartphone.

### Device Requirements
- The device must be mounted on the motorcycle itself.
- The device shall operate on motorcycle voltage.
- The device must be able to withstand surges and inrush currents.
- All operations must be completed by a smartphone.
- All operations must be completed by a smartphone, including firmware upload.
- Compact as possible.
　->Motorcycles have limited space for installation.
- All designs must be open source.
- The source must be available in English.
- The device should provide users with a manual on how to use the device.
- Cost-effectiveness must be superior to the functions of competing products.
- The accuracy of the air-fuel ratio measurement should be as high as possible.

### Sales channels and marketing
Amazon and Ebay are under consideration.  
Beta version to FaceBook community, publicity

### Development Steps
#### STEP1
 Develop a device that utilizes a stock narrow band O2 sensor
- Operation via smartphone  
- Open-loop map switching function by coding plug (optional)  

#### STEP2
 Development of device using commercially available Bosch broadband 02 sensor
 ->In short, a smartphone version of LC-2

#### STEP3 (undecided) 
 Development of a device with sub-controller function
 ->In essence, upward compatible with PowerCommander3.  
 ->Smartphone operation, visualization of air-fuel ratio, and log acquisition  

### Schedule
　Undecided

### System
　Undecided
