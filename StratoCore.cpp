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

    RA_ack_flag = NO_ACK;
    S_ack_flag = NO_ACK;
    TM_ack_flag = NO_ACK;

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

// initialize the watchdog using the 1kHz LPO clock source to achieve a 10s WDOG
void StratoCore::InitializeWatchdog()
{
    noInterrupts(); // disable interrupts

    // unlock
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ1; // unlock access to WDOG registers
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
    delayMicroseconds(1);

    WDOG_PRESC = 0; // no prescaling of clock

    WDOG_TOVALH = 0x0000; // upper bits set to 0
    WDOG_TOVALL = 0x2710; // 10000 counter at 1 kHz => 10s WDOG period

    // in one write, enable the watchdog using the 1kHz LPO clock source 
    WDOG_STCTRLH = 0x01D1;

    interrupts(); // enable interrupts
}

void StratoCore::KickWatchdog()
{
    noInterrupts();
    WDOG_REFRESH = 0xA602;
    WDOG_REFRESH = 0xB480;
    interrupts();
}

void StratoCore::RunMode()
{
    // check for a new mode
    if (inst_mode != new_inst_mode) {
        // call the last mode after setting the substate to exit
        inst_substate = MODE_EXIT;
        (this->*(mode_array[inst_mode]))();

        // clear any scheduled items from the old mode
        scheduler.ClearSchedule();

        // update the mode and set the substate to entry
        inst_mode = new_inst_mode;
        inst_substate = MODE_ENTRY;
    }

    // run the current mode
    (this->*(mode_array[inst_mode]))();
}

void StratoCore::RunRouter()
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
    // ignore bad messages and send NAKs when applicable
    if (!instAck) {
        if (message == IM) {
            zephyrTX.IMAck(instAck);
        }
        return;
    }

    switch (message) {
    case IM:
        new_inst_mode = reader_mode;
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
        S_ack_flag = (zephAck == 1) ? ACK : NAK;
        break;
    case RAAck:
        RA_ack_flag = (zephAck == 1) ? ACK : NAK;
        break;
    case TMAck:
        TM_ack_flag = (zephAck == 1) ? ACK : NAK;
        break;
    case NONE:
        break;
    default:
        log_error("Unknown message to route");
        break;
    }

    // todo: log zephyr comms
}

void StratoCore::RunScheduler()
{
    uint8_t scheduled_action = scheduler.CheckSchedule();

    while (scheduled_action != NO_SCHEDULED_ACTION) {
        ActionHandler(scheduled_action);
        scheduled_action = scheduler.CheckSchedule();
    }
}

void StratoCore::TakeZephyrByte(uint8_t rx_char)
{
    // todo: ensure thread safety!
    zephyrRX.putChar(rx_char);
}

void StratoCore::UpdateTime()
{
    int32_t before, new_time, difference;
    TimeElements new_time_elements;

    // hours, minutes, seconds, date, month years
    String temp_str = "";
    temp_str += (char)Time[0];
    temp_str += (char)Time[1];
    new_time_elements.Hour = (uint8_t) temp_str.toInt();
    temp_str = "";
    temp_str += (char)Time[2];
    temp_str += (char)Time[3];
    new_time_elements.Minute = (uint8_t) temp_str.toInt();
    temp_str = "";
    temp_str += (char)Time[4];
    temp_str += (char)Time[5];
    new_time_elements.Second = (uint8_t) temp_str.toInt();
    temp_str = "";
    temp_str += (char)Date[6];
    temp_str += (char)Date[7];
    new_time_elements.Day = (uint8_t) temp_str.toInt();
    temp_str = "";
    temp_str += (char)Date[4];
    temp_str += (char)Date[5];
    new_time_elements.Month = (uint8_t) temp_str.toInt();
    temp_str = "";
    temp_str += (char)Date[0];
    temp_str += (char)Date[1];
    temp_str += (char)Date[2];
    temp_str += (char)Date[3];
    new_time_elements.Year = (uint8_t) (temp_str.toInt() - 1970);

    before = now();
    new_time = makeTime(new_time_elements);
    difference = new_time - before;
    
    // if the time difference is greater than 3s, update
    if (difference > 3 || difference < -3) {
        log_nominal("Correcting time drift");

        noInterrupts();
        setTime(new_time);
        interrupts();

        scheduler.UpdateScheduleTime(difference);
    }
    
    temp_str = " " + String(new_time_elements.Hour) + ":" + String(new_time_elements.Minute) + ":" + String(new_time_elements.Second);
    temp_str += ", " + String(new_time_elements.Month) + "/" + String(new_time_elements.Day) + "/" + String(new_time_elements.Year + 1970);
    log_nominal(temp_str.c_str());
}