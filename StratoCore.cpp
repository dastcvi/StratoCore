/*
 *  StratoCore.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file implements an Arduino library (C++ class) with core functionality
 *  for LASP Strateole payload interface boards
 */

#include "StratoCore.h"
#include "TimeLib.h"

StratoCore::StratoCore(Print * zephyr_serial, Instrument_t instrument)
    : zephyrTX(zephyr_serial, &Serial)
    , zephyrRX(zephyr_serial, &Serial, instrument)
{
    inst_mode = STANDBY; // always boot to standby
    new_inst_mode = STANDBY;
    inst_substate = MODE_ENTRY; // substate starts as mode entry

    switch (instrument) {
    case FLOATS:
        zephyrTX.setDevId("FLOATS");
        break;
    case RACHUTS:
        zephyrTX.setDevId("RACHUTS");
        break;
    case LPC:
        zephyrTX.setDevId("LPC");
        break;
    default:
        break;
    }
}

void StratoCore::Initialize()
{
    ; // currently nothing to initialize
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

    // todo: thread safety! consider handling multiple messages per loop
    // todo: make reader more robust! ensure all bad messages are ignored
    while (PARSING == zephyrRX.getNew());
    if (zephyrRX.dataValid()) {
        RouteRXMessage(zephyr_message); // global from XMLReader
    }
}

void StratoCore::RouteRXMessage(ZephyrMessage_t message)
{
    switch (message) {
    case IM:
        if (instAck) { // message ok
            new_inst_mode = reader_mode;
        }
        zephyrTX.IMAck(instAck);
        break;
    case GPS:
        UpdateTime();
        break;
    case SW:
        // set the substate to shutdown and let the mode functions handle it
        inst_substate = MODE_SHUTDOWN;
        break;
    case TC:
        // todo: finish instrument TC handler design
        zephyrTX.TCAck(TCHandler(zephyr_tc));
        break;
    case SAck:
    case RAAck:
    case TMAck:
        // todo: ack handler
        break;
    case NONE:
        // nothing to do
        break;
    default:
        log_error("Unknown message to route");
        break;
    }
}


void StratoCore::TakeZephyrByte(uint8_t rx_char)
{
    // todo: ensure thread safety!
    zephyrRX.putChar(rx_char);
}

void StratoCore::UpdateTime()
{
    // hours, minutes, seconds, date, month years
    String temp_str = "";
    temp_str += (char)Time[0];
    temp_str += (char)Time[1];
    uint16_t hours = temp_str.toInt();
    temp_str = "";
    temp_str += (char)Time[2];
    temp_str += (char)Time[3];
    uint16_t minutes = temp_str.toInt();
    temp_str = "";
    temp_str += (char)Time[4];
    temp_str += (char)Time[5];
    uint16_t seconds = temp_str.toInt();
    temp_str = "";
    temp_str += (char)Date[6];
    temp_str += (char)Date[7];
    uint16_t days = temp_str.toInt();
    temp_str = "";
    temp_str += (char)Date[4];
    temp_str += (char)Date[5];
    uint16_t months = temp_str.toInt();
    temp_str = "";
    temp_str += (char)Date[0];
    temp_str += (char)Date[1];
    temp_str += (char)Date[2];
    temp_str += (char)Date[3];
    uint16_t years = temp_str.toInt();
    noInterrupts();
    setTime(hours, minutes, seconds, days, months, years);
    interrupts();

    temp_str = " " + String(hours) + ":" + String(minutes) + ":" + String(seconds);
    temp_str += ", " + String(months) + "/" + String(days) + "/" + String(years);
    log_nominal(temp_str.c_str());
}