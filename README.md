# StratoCore

This repository maintains core software for [LASP](https://lasp.colorado.edu/home/) instruments flying on the [Stratéole 2](https://strat2.org/) campaign with CNES. The core controls mode management and interfacing with the Zephyr gondola. StratoCore is an abstract base class from which the classes for each instrument are derived via inheritance.

An example instrument derived class is the [StratoPIB](https://github.com/dastcvi/StratoPIB) for the LASP RACHuTS instrument.

The goal of StratoCore is to provide a standard software core for LASP's three Stratéole 2 instruments that simplifies the development of reliable software for all three by handling the Zephyr gondola interface and mode management. Additionally, the goal is to provide useful core tools that make it easy to maintain and upgrade the instrument software over the campaign lifetime.

## Software Development Environment

All of the instruments use [Teensy 3.6](https://www.sparkfun.com/products/14057) Arduino-compatible MCU boards as the primary computer. Thus, this and all other Strateole 2 code is implemented for Arduino, meaning that all of this C++ code uses the Arduino drivers for the Teensy 3.6 and is compiled using the Arduino IDE with the [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html) plug-in.

*StratoCore is known to work with Arduino 1.8.4 and Teensyduino 1.39, as well as with Arduino 1.8.11 and Teensyduino 1.51*

## Components

<img src="/Documentation/component_diagram.png" alt="/Documentation/component_diagram.png" width="900"/>

## Control Structure

StratoCore uses a cyclic executive loop running at 1 Hz with best-effort timing. The diagram below shows the loop structure. Blue represents methods implemented in StratoCore, orange represents pure virtual methods that must be implemented in derived instrument classes.

<p align="center">
  <img src="/Documentation/control_loop.png" alt="/Documentation/control_loop.png" width="150"/>
</p>

## Scheduler

<img src="/Documentation/scheduler.png" alt="/Documentation/scheduler.png" width="900"/>

