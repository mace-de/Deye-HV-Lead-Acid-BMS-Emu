# Deye-HV-Lead-Acid-BMS-Emulator

This Project is a quick and dirty BMS Emulator to use a high voltage lead acid battery on a Deye Hybrid Inverter with a bit more comfort as just with the "User defined battery" with it's voltage setpoints. The emulator reads the actual measured battery current and voltage form the inverter over a Modbus connection. It uses these values to calculate a SOC and to switch from bulk charge voltage to float charge when the battery is full. It transfers these calculated values over CAN-Bus back to the inverter using either BYD or PYLONTEC Protocol. It runs on the Liligo T-CAN485 Board. It supports a small 0.96" OLED-Display to display current values, a DS1302 to store SOC on power loss and a simple webserver to show actual values online. The DS1302 must be connected, the display can be connected. The on board LED shows the actual working state. Red = startup, Green = bulk charge, Blue = float charge, Blinking = Modbus communication ok
Configuration is done in config.h. Here the pins for the external peripherals can be found.

I used some code from Dala's great battery emulator project. https://github.com/dalathegreat/Battery-Emulator/tree/main

