/*
 *  StratoCore.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file implements an Arduino library (C++ class) with core functionality
 *  for LASP Strateole payload interface boards
 */

#include "StratoCore.h"

StratoCore::StratoCore()
{
    inst_mode = STANDBY; // always boot to standby
    new_inst_mode = STANDBY;
    inst_substate = MODE_ENTRY; // substate starts as mode entry
}

void StratoCore::RunMode()
{
    // check for a new mode
    if (inst_mode != new_inst_mode) {
        // call the last mode after setting the substate to exit
        inst_substate = MODE_EXIT;
        (this->*(mode_array[inst_mode]))();

        // update the mode and set the substate to entry
        inst_mode = new_inst_mode;
        inst_substate = MODE_ENTRY;
    }

    // run the current mode
    (this->*(mode_array[inst_mode]))();
}

void StratoCore::Router()
{
    // check for and route any message from the ground port
    ZephyrMessage_t message = ground_port();
    if (message != NONE) RouteRXMessage(message);

    // todo: check XMLReader for messages, send them to RouteRXMessage
}

void StratoCore::RouteRXMessage(ZephyrMessage_t message)
{
    switch (message) {
    case IM:
        // todo: send IMAck (here or elsewhere)
        new_inst_mode = reader_mode;
        break;
    case GPS:
        // to time/date handler
        break;
    case SW:
        // todo: what to do
        break;
    case TC:
        // to instrument TC handler
        break;
    case SAck:
        // todo: ack handler
        break;
    case RAAck:
        // todo: ack handler
        break;
    case TMAck:
        // todo: ack handler
        break;
    case NONE:
        break;
    default:
        log_error("Unknown message to route");
        break;
    }
}