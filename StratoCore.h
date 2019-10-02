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
#include "StratoSD.h"
#include "XMLReader_v5.h"
#include "XMLWriter_v5.h"
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
#define MODE_ERROR      253
#define MODE_SHUTDOWN   254
#define MODE_EXIT       255

// maximum amount onboard time can be off from Zephyr GPS time without being updated
#define MAX_TIME_DRIFT  (2) // seconds

// number of seconds without zephyr comms after which safety mode is entered
#define ZEPHYR_TIMEOUT  3600 // 3600 s = 2 hrs

class StratoCore {
public:
    // constructors/destructors
    StratoCore(Stream * zephyr_serial, Instrument_t instrument);
    ~StratoCore() { };

    // public interface functions
    void InitializeCore();
    void KickWatchdog();
    void RunMode();
    void RunRouter();
    void RunScheduler();

    // Pure virtual function definition for the instrument setup function, called publicly before the loop begins
    virtual void InstrumentSetup() = 0;

    // Pure virtual function definition for the instrument loop function, called at the end of each loop publicly
    virtual void InstrumentLoop() = 0;

protected: // available to StratoCore and instrument classes
    XMLWriter zephyrTX;
    XMLReader zephyrRX;

    // Scheduler
    StratoScheduler scheduler;

    // Set to determine the substate within a mode (always set to MODE_ENTRY when a mode is started)
    uint8_t inst_substate;

    // Set once the onboard time has been set from a Zephyr GPS message
    bool time_valid;

    // for a critical error, log to the terminal and send as telemetry
    void ZephyrLogFine(const char * log_info);
    void ZephyrLogWarn(const char * log_info);
    void ZephyrLogCrit(const char * log_info);

    // write the current TM buffer to a file on the SD card
    bool WriteFileTM(const char * file_prefix);

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

    // Pure virtual function definition for the instrument action handler
    virtual void ActionHandler(uint8_t action) = 0;

    enum ACK_t {
        NAK,
        ACK,
        NO_ACK
    };

    // ack status flags to be checked and cleared by the instrument for each corresponding message
    ACK_t RA_ack_flag;
    ACK_t S_ack_flag;
    ACK_t TM_ack_flag;

    // Type for action flags used by instruments
    struct ActionFlag_t {
        bool flag_value;
        uint8_t stale_count;
    };

private: // available only to StratoCore
    void InitializeWatchdog();
    void RouteRXMessage(ZephyrMessage_t message);
    void UpdateTime();

    time_t last_zephyr;

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