/*
 *  StratoCore.h
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file declares an Arduino library (C++ class) with core functionality
 *  for LASP Strateole payload interface boards
 */

#ifndef STRATOCORE_H
#define STRATOCORE_H

#include "StratoGroundPort.h"
#include "StratoScheduler.h"
#include "XMLReader_v3.h"
#include "XMLWriter_v4.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "WProgram.h"
#include <stdint.h>

// an instrument can define up to 256 states (uint8_t) for each mode, provided that
// state 0 is entry, state 254 is the shutdown warning state, 255 is exit, and the
// protected inst_substate variable is used. On a mode switch, StratoCore will run
// the old mode once in MODE_EXIT before starting the new mode in MODE_ENTRY. In the
// case that a shutdown warning is received, StratoCore will set the state to MODE_SHUTDOWN
#define MODE_ENTRY      0
#define MODE_SHUTDOWN   254
#define MODE_EXIT       255

class StratoCore {
public:
    // constructors/destructors
    StratoCore(Print * zephyr_serial, Instrument_t instrument);
    ~StratoCore() { };

    // public interface functions
    void InitializeWatchdog();
    void KickWatchdog();
    void RunMode();
    void RunRouter();
    void RunScheduler();
    void TakeZephyrByte(uint8_t rx_char);

protected: // available to StratoCore and instrument classes
    XMLWriter_v4 zephyrTX;
    XMLReader_v3 zephyrRX;

    // Scheduler
    StratoScheduler scheduler;

    // Set to determine the substate within a mode (always set to MODE_ENTRY when a mode is started)
    uint8_t inst_substate;

    // Pure virtual mode functions (implemented entirely in instrument classes)
    // Using these, the StratoCore can call the mode functions of derived classes, but the
    // derived classes (other instruments) must implement them themselves
    virtual void StandbyMode() = 0;
    virtual void FlightMode() = 0;
    virtual void LowPowerMode() = 0;
    virtual void SafetyMode() = 0;
    virtual void EndOfFlightMode() = 0;

    // Pure virtual function definition for the instrument telecommand handler - returns ACK/NAK
    virtual bool TCHandler(Telecommand_t telecommand) = 0;
    virtual void ActionHandler(uint8_t action) = 0;

private: // available only to StratoCore
    void RouteRXMessage(ZephyrMessage_t message);
    void UpdateTime();
    
    // Only the Zephyr can change mode, unless 2 hr pass without comms (REQ461) -> Safety
    // InstMode_t defined in XMLReader
    InstMode_t inst_mode;
    InstMode_t new_inst_mode; // set this to change mode, StratoCore handles the rest

    // Array of mode functions indexed by InstMode_t enum in XMLReader (don't change order)
	void (StratoCore::*mode_array[NUM_MODES])(void) = {
		&StratoCore::StandbyMode,
		&StratoCore::FlightMode,
		&StratoCore::LowPowerMode,
		&StratoCore::SafetyMode,
		&StratoCore::EndOfFlightMode
	};
};

#endif /* STRATOCORE_H */