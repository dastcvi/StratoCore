/*
 *  StratoGroundPort.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file implements functions for logging debug information
 *  to a connected serial terminal, as well as for receiving
 *  commands from the terminal.
 */

#include "StratoGroundPort.h"

// definitions of functions for internal GroundPort use only
void print_log(LOG_LEVEL_t log_level, const char * log_info);
void get_user_param(void);
bool get_user_command(void);
void handle_user_command(void);
void print_menu(void);
ZephyrMessage_t execute_command(void);

// variables for internal GroundPort use only
char current_command = '\0';
bool command_ready = false;
bool waiting_on_input = false;

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
        Serial.print("DBG: ");
        break;
    case LOG_NOMINAL:
        Serial.print("NOM: ");
        break;
    case LOG_ERROR:
        Serial.print("ERR: ");
        break;
    default:
        break;
    }

    Serial.println(log_info);
}

// Command functions --------------------------------------
ZephyrMessage_t ground_port()
{
    if (command_ready) {
        return execute_command();
    } else if (waiting_on_input) {
        get_user_param();
    } else if (get_user_command()) {
        handle_user_command();
    }

    return NONE;
}

void get_user_param()
{
    ;
}

bool get_user_command()
{
    if (!Serial.available()) return false;

    current_command = Serial.read();

    // clear the buffer
    while (Serial.available()) Serial.read();

    return true;
}

// sets flags to determine for each command if it is ready to execute or needs input
void handle_user_command()
{
    switch (current_command) {
    case 'b': case 'B': // standby
        waiting_on_input = false;
        command_ready = true;
        break;
    case 'f': case 'F': // flight
        waiting_on_input = false;
        command_ready = true;
        break;
    case 'l': case 'L': // low power
        waiting_on_input = false;
        command_ready = true;
        break;
    case 's': case 'S': // safety
        waiting_on_input = false;
        command_ready = true;
        break;
    case 'e': case 'E': // end of flight
        waiting_on_input = false;
        command_ready = true;
        break;
    case 'p': case 'm': case 'P': case 'M':
        waiting_on_input = false;
        command_ready = true;
        break;
    default:
        log_error("Invalid GroundPort command");
        current_command = '\0';
        break;
    }
}

void print_menu()
{
    Serial.println("-- Command Menu --");
    Serial.println("'p'|'m': re-print menu");
    Serial.println("------ Modes -----");
    Serial.println("'b': standby");
    Serial.println("'f': flight");
    Serial.println("'l': low power");
    Serial.println("'s': safety");
    Serial.println("'e': end of flight");
    Serial.println("------------------");
}

ZephyrMessage_t execute_command()
{
    ZephyrMessage_t message = NONE;

    switch (current_command) {
    case 'b': case 'B': // standby
        message = IM;
        reader_mode = STANDBY;
        break;
    case 'f': case 'F': // flight
        message = IM;
        reader_mode = FLIGHT;
        break;
    case 'l': case 'L': // low power
        message = IM;
        reader_mode = LOWPOWER;
        break;
    case 's': case 'S': // safety
        message = IM;
        reader_mode = SAFETY;
        break;
    case 'e': case 'E': // end of flight
        message = IM;
        reader_mode = ENDOFFLIGHT;
        break;
    case 'p': case 'm': case 'P': case 'M':
        print_menu();
        break;
    default:
        log_error("Invalid GroundPort command");
        current_command = '\0';
        break;
    }

    current_command = '\0';
    command_ready = false;
    return message;
}