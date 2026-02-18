Arduino IR Temp-Controlled Fan


This is a fan controller I built using an Arduino. It has two modes: Auto (temperature-based) and Manual (IR remote control).
It reads the room temperature using a thermistor. As it gets hotter, the fan speeds up. As for the manual version, you can use an IR remote to change speeds, reverse direction, or shut it off completely.

Features


Auto Mode: Fan kicks in at 24.5°C. As it gets hotter (up to 35°C), the fan spins faster.
Manual Override: Use a remote to force the fan on, change speed (PWM), or swap motor direction.
Visuals: I added a 74HC595 shift register to drive an LED bar graph. It lights up more LEDs as the fan speed increases.
Hysteresis: There's a 1-degree buffer so the fan doesn't annoyingly toggle on and off if the temperature is hovering right around the threshold.

Hardware List
Arduino (Uno)
DC Motor + Driver 
IR Receiver + Remote
NTC Thermistor (10k) + 10k Resistor
74HC595 Shift Register + 8 LEDs

Pinout
Motor & Sensors
| Pin  | Component |
| :--- | :--- |
|  D11 | IR Receiver |
|  A0  | Thermistor (Divider) |
|  D5  | Motor PWM (Speed) |
|  D9  | Motor IN1 |
|  D10 | Motor IN2 |

Shift Register (LEDs)
| Pin | 74HC595 Pin |
| :--- | :--- |
|  D2  | Data (DS) |
|  D3  | Clock (SHCP) |
|  D4  | Latch (STCP) |
