/*
 *  StratoGroundPort.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *
 *  This file implements functions for logging debug information
 *  to a connected serial terminal
 */

#include "StratoGroundPort.h"

Stream * debug_serial = &Serial;

// definitions of functions for internal GroundPort use only
void print_log(LOG_LEVEL_t log_level, const char * log_info);

// Log functions ------------------------------------------
void log_debug(const char * log_info)
{
    print_log(LOG_DEBUG, log_info);
}

void log_nominal(const char * log_info)
{
    print_log(LOG_NOMINAL, log_info);
}

void log_error(const char * log_info)
{
    print_log(LOG_ERROR, log_info);
}

void print_log(LOG_LEVEL_t log_level, const char * log_info)
{
    if (log_level < LOG_LEVEL) return;

    switch (log_level) {
    case LOG_DEBUG:
        debug_serial->print("DBG: ");
        break;
    case LOG_NOMINAL:
        debug_serial->print("NOM: ");
        break;
    case LOG_ERROR:
        debug_serial->print("ERR: ");
        break;
    default:
        break;
    }

    debug_serial->println(log_info);
}
