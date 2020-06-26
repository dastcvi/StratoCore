# StratoCore

This repository maintains core software for [LASP](https://lasp.colorado.edu/home/) instruments flying on the [Stratéole 2](https://strat2.org/) campaign with CNES. The core controls mode management and interfacing with the Zephyr gondola. StratoCore is an abstract base class from which the classes for each instrument are derived via inheritance. **Key to the design of StratoCore is the use of [pure virtual functions](https://www.geeksforgeeks.org/pure-virtual-functions-and-abstract-classes/) that allow StratoCore to run instrument-specific functions.**

StratoCore was designed in conjunction with [StratoPIB](https://github.com/dastcvi/StratoPIB) for the LASP RACHuTS instrument, so it serves as the best template for implementation. See also [StratoDIB](https://github.com/dastcvi/StratoDIB) and [StratoLPC](https://github.com/kalnajslab/LPC).

The goal of StratoCore is to provide a standard software core for LASP's three Stratéole 2 instruments that simplifies the development of reliable software for all three by handling the Zephyr gondola interface and mode management. Additionally, the goal is to provide useful core tools that make it easy to maintain and upgrade the instrument software over the campaign lifetime.

## Software Development Environment

All of the instruments use [Teensy 3.6](https://www.sparkfun.com/products/14057) Arduino-compatible MCU boards as the primary computer. Thus, this and all other Strateole 2 code is implemented for Arduino, meaning that all of this C++ code uses the Arduino drivers for the Teensy 3.6 and is compiled using the Arduino IDE with the [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html) plug-in.

*StratoCore is known to work with Arduino 1.8.4 and Teensyduino 1.39, as well as with Arduino 1.8.11 and Teensyduino 1.51*

## Testing

The [OBC Simulator](https://github.com/dastcvi/OBC_Simulator) is a piece of software developed specifically for LASP Stratéole 2 instrument testing using only the Teensy 3.6 USB port. It provides the full OBC interface to allow extensive testing. StratoCore must be configured (via its constructor) to use the `&Serial` pointer for both `zephyr_serial` and `debug_serial`, and the OBC Simulator will separately display Zephyr and debug messages, color-coded by severity.

## Requirements

StratoCore is designed to satisfy the requirements defined in `STR2-ZEPH-DCI-0-031_v01.pdf`

## Components

The diagram below shows a simplified view of the software components that StratoCore provides, as well as the components that it requires of deriving classes (implemented as pure virtual functions). Blue represents functions implemented in StratoCore, orange represents pure virtual functions that must be implemented in derived instrument classes.

<img src="/Documentation/component_diagram.png" alt="/Documentation/component_diagram.png" width="900"/>

## Control Structure

StratoCore uses a cyclic executive loop running at 1 Hz with best-effort timing. If software in the loop takes longer than one second, a loop cycle will be missed. StratoCore is tolerant to loops taking up to 10 seconds before the watchdog timer kicks in and resets the software, but best effort should be made to design software that takes less than one second per loop.

The diagram below shows the loop structure, which should be implemented in the Arduino file itself (see StratoPIB's main file for an example). Again, blue represents functions implemented in StratoCore, orange represents pure virtual functions that must be implemented in derived instrument classes.

<p align="center">
  <img src="/Documentation/control_loop.png" alt="/Documentation/control_loop.png" width="150"/>
</p>

## Router

At the heart of StratoCore is the message router. This part of the software is responsible for reading, checking, and routing the Zephyr XML messages. The [StrateoleXML](https://github.com/dastcvi/StrateoleXML) library implements all XML transactions. The XMLWriter in this library performs all XML writes, and is called asynchonously throughout StratoCore and derived instrument classes. XMLReader is called only from the StratoCore router. The XML message types are routed as follows:

* `IM` (instrument mode): router sets the `new_inst_mode` variable for the Mode Manager, and sends an ACK
* `GPS`: router calls the `UpdateTime()` function, which is the GPS/Time Keeper component
* `SW` (shutdown warning): router sets the `inst_substate` variable to `MODE_SHUTDOWN`, which must be handled in instrument mode functions
* `SAck`/`RAAck`/`TMAck` (ACKs): router sets corresponding ack flags accordingly
* `TC` (telecommand): see following description

### Telecommand Handling

StratoCore handles telecommands as defined in StrateoleXML. Zephyr `TC` messages can contain multiple commands for an instrument, and if this is the case, StratoCore parses and routes one telecommand per loop (in other words, one telecommand per second). In the case that more spacing is needed between commands, use the "null telecommand" (`0;`), which causes no action, but takes up a loop. Most telecommands are routed directly to instruments via the pure virtual `TCHandler` function. StratoCore handles the following telecommands independently:

* `TC-0   NULL_TELECOMMAND`: used for loop timing, no action taken
* `TC-200 RESET_INST`: will perform a software reset immediately
* `TC-202 GETTMBUFFER`: sends a TM with whatever is currently in the TM buffer as-is
* `TC-203 SENDSTATE`: sends a TM with the current instrument mode and substate

### Simple Telemetry Messages

Since it is good practice to send simple plain-text telemetry messages to the ground to note significant events, StratoCore provides functions that take only a character array pointer as an input (max 100 chars) and send it as a TM to the ground at one of the Zephyr-defined status levels. The functions are:

* `ZephyrLogFine`
* `ZephyrLogWarn`
* `ZephyrLogCrit`

### Ground Port

StratoCore also provides logging functions for ground test, designed to be sent over USB to a support computer. The functions are `log_debug`, `log_nominal`, and `log_error`. An instrument can place logging calls throughout its code and then adjust the log level to mute messages below a certain priority. The OBC simulator is designed to color-code these messages by severity.

## GPS/Time Keeper

GPS messages from the Zephyr are routed to the GPS/Time Keeper via the `UpdateTime` function. This function updates the instrument time if it has drifted by more than two seconds. GPS position and solar zenith angle are available to instrument derived classes directly from the XMLReader in a struct accessible as `zephyrRX.zephyr_gps`.

## Watchdog

StratoCore uses the microcontroller's onboard watchdog timer. The timer is reset every loop and the timer is configured to reset the instrument if more than 10 seconds have ellapsed. Thus, StratoCore is tolerant to loops that take up to 10 seconds, but best effort should still be made to keep loop software shorter than one second.

## Scheduler

The scheduler is a utility that provides the instrument the ability to schedule enumerated actions at variable times in the future. To schedule an action, the instrument calls `scheduler.AddAction()` passing as arguments the action ID number (an 8-bit unsigned integer) and the time. The time can be set as a relative time (e.g. 10 seconds from now), or an exact time using the `TimeElements` struct from the Teensy `TimeLib`.

Once each loop, the scheduler will check to see if it is time for any of the scheduled actions. For any actions that are ready, StratoCore will call the `ActionHandler` pure virtual function, and it is up to the instrument to handle it. See StratoPIB for an example of using flags that can timeout as a way to handle actions.

The schedule is maintained as a linked list. Schedule elements are statically allocated, and the maximum schedule must be defined at compile time. **The current maximum schedule size is set to 32**. If the schedule is full, `scheduler.AddAction()` will return `false`.

*Note that if the time keeper updates the time due to drift, actions scheduled using relative times will be updated to maintain their relative timing. Actions scheduled at an exact time will keep their exact time: if that time is now known to be in the past, they will be handled immediately.*

<img src="/Documentation/scheduler.png" alt="/Documentation/scheduler.png" width="900"/>

## Mode Manager

Stratéole 2 instruments must implement the following modes:

* `SB` (Standby)
* `FL` (Flight)
* `SA` (Safety)
* `LP` (Low Power)
* `EF` (End of Flight)

The Zephyr can change the instrument mode by sending the `IM` XML message. StratoCore maintains the mode of the instrument and has a pure virtual function definition for each mode that it calls in every loop. In addition, StratoCore maintains an `inst_substate` variable (an 8-bit unsigned integer) that allows the instrument to implement substates in each mode. To facilitate this, StratoCore defines the following four substates:

* `MODE_ENTRY` = 0
* `MODE_ERROR` = 253
* `MODE_SHUTDOWN` = 254
* `MODE_EXIT` = 255

When a mode switch occurs, the old mode gets one last function call with the `MODE_EXIT` substate set so that it can perform any cleanup required. Then, the new mode function is called with the `MODE_ENTRY` substate set.

If a shutdown warning is received, the `MODE_SHUTDOWN` substate is set and whatever mode the instrument is currently in will continue to be called once per loop in this substate until the instrument is shut down.

StratoCore doesn't set `MODE_ERROR`, but it is defined to encourage instruments to implement an error substate.

Instruments can define their own substates numbered 1-252 and move between them at will. They should at every mode function call, be checking for `MODE_ENTRY`, `MODE_SHUTDOWN`, and `MODE_EXIT`.

**The instrument mode functions are called once per loop and should be designed to take less than a second and be called continuously.**

## SD Manager

The SD manager `StratoSD` is a light wrapper for the existing `SdFat` Arduino library for Teensy 3.6's built-in SD card. StratoCore will initialize the SD card and provides a function to write a file. If there is an error with the SD card, StratoCore will send a TM to inform the ground and gracefully refuse to perform SD writes.