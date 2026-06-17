# micro-ROS ESP32 Project with UWB Simulation

This project is a template for micro-ROS on ESP32, featuring:
- Wi-Fi connection to a micro-ROS agent.
- Simulated UWB (Ultra-Wideband) data published on `/uwb_range`.
- Base Bluetooth initialization.
- Wokwi simulation support.

## Quick Start
1. Start the micro-ROS agent: `microros agent`
2. Build the project: `microros build`
3. Flash to hardware: `microros flash` (or `microros sim` for simulation)

## Configuration
Update `sdkconfig.defaults` or use `idf.py menuconfig` to change:
- Wi-Fi SSID and Password.
- micro-ROS Agent IP and Port.
