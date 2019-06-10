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
#include "Arduino.h"
#include "HardwareSerial.h"
#include "WProgram.h"
#include <stdint.h>

// an instrument can define up to 256 states (uint8_t) for each mode, provided that
// state 0 is entry, state 255 is exit, and the protected inst_substate variable
// is used. StratoCore will set inst_substate=0 on every mode switch
#define MODE_ENTRY  0
#define MODE_EXIT   255

enum MODE_t {
    MODE_SB, // standby
    MODE_FL, // flight
    MODE_LP, // low power
    MODE_SA, // safety
    MODE_EF  // end of flight
};

class StratoCore {
public:
    // constructors/destructors
    StratoCore();
    ~StratoCore() { };

    // public interface
    void RunMode();
protected: // available to StratoCore and child classes
    // Mode and state control
    MODE_t inst_mode;
    uint8_t inst_substate;

    // Pure virtual mode functions (implemented entirely in child class)
    virtual void StandbyMode() = 0;
    virtual void FlightMode() = 0;
    virtual void LowPowerMode() = 0;
    virtual void SafetyMode() = 0;
    virtual void EndOfFlightMode() = 0;

private: // available only to StratoCore
};

#endif /* STRATOCORE_H */