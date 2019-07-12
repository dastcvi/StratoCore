/*
 *  StratoGroundPort.h
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file declares functions for logging debug information
 *  to a connected serial terminal, as well as for receiving
 *  commands from the terminal.
 */

#ifndef STRATOGROUNDPORT_H
#define STRATOGROUNDPORT_H

#include "XMLReader_v3.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "WProgram.h"
#include <stdint.h>

// display all logs at or above this level
#define LOG_LEVEL   LOG_NOMINAL

// importance levels in increasing order
enum LOG_LEVEL_t {
    LOG_DEBUG   = 0,
    LOG_NOMINAL = 1,
    LOG_ERROR   = 2,
    LOG_NONE    = 3
};

// functions for logging information to the terminal at different levels
// to add values to print, either use a char array and snprintf, or
// create a String, and log this way: log_xxxx(string_name.c_str())
void log_debug(const char * log_info);
void log_nominal(const char * log_info);
void log_error(const char * log_info);

// note: log_crit_error is defined/implemented in StratoCore for XMLWriter access

// get user commands from the terminal
ZephyrMessage_t ground_port(void);

#endif /* STRATOGROUNDPORT_H */