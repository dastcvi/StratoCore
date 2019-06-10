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

#include "Arduino.h"
#include "HardwareSerial.h"
#include "WProgram.h"
#include <stdint.h>

// display all logs at or above this level
#define LOG_LEVEL   LOG_DEBUG

// importance levels in increasing order
enum LOG_LEVEL_t {
    LOG_DEBUG   = 0,
    LOG_NOMINAL = 1,
    LOG_ERROR   = 2,
    LOG_NONE    = 3
};

void log_debug(const char * log_info);
void log_nominal(const char * log_info);
void log_error(const char * log_info);
void print_log(LOG_LEVEL_t log_level, const char * log_info);

#endif /* STRATOGROUNDPORT_H */