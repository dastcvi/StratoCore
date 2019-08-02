# StratoCore

This repository maintains core software for LASP instruments flying on the Strateole 2 campaign with CNES. The core controls mode management and interfacing with the Zephyr gondola. StratoCore is an abstract base class from which the classes for each instrument inherit.

## Hardware

All of the instruments use Teensy 3.6 Arduino-compatible MCU boards as the primary computer. Thus, this and all other Strateole 2 code is implemented for Arduino, meaning that all of this C++ code uses the Arduino drivers for the Teensy 3.6 and is compiled using the Arduino IDE with the Teensy plug-in.
