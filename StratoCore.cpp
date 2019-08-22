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

StratoCore::StratoCore(Stream * zephyr_serial, Instrument_t instrument)
    : zephyrTX(zephyr_serial, instrument)
    , zephyrRX(zephyr_serial, instrument)
{
    inst_mode = MODE_STANDBY; // always boot to standby
    new_inst_mode = MODE_STANDBY;
    inst_substate = MODE_ENTRY; // substate starts as mode entry

    RA_ack_flag = NO_ACK;
    S_ack_flag = NO_ACK;
    TM_ack_flag = NO_ACK;

    time_valid = false;

    last_zephyr = now();
}

// initialize the watchdog using the 1kHz LPO clock source to achieve a 10s WDOG
void StratoCore::InitializeWatchdog()
{
    if ((RCM_SRS0 & RCM_SRS0_WDOG) != 0) {
        ZephyrLogCrit("Reset caused by watchdog");
    }

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
    // process as many messages as are available
    while (zephyrRX.GetNewMessage()) {
        RouteRXMessage(zephyrRX.zephyr_message);
    }

    // check for Zephyr no contact timeout
    if (now() > last_zephyr + ZEPHYR_TIMEOUT) {
        ZephyrLogCrit("Zephyr comm loss timeout");
        new_inst_mode = MODE_SAFETY;
    }
}

void StratoCore::RouteRXMessage(ZephyrMessage_t message)
{
    TCParseStatus_t tc_status = NO_TCs;
    bool tc_success = true;

    switch (message) {
    case IM:
        new_inst_mode = zephyrRX.zephyr_mode;
        zephyrTX.IMAck(true);
        break;
    case GPS:
        UpdateTime();
        break;
    case SW:
        // set the substate to shutdown and let the mode functions handle it
        inst_substate = MODE_SHUTDOWN;
        break;
    case TC:
        tc_status = zephyrRX.GetTelecommand();
        while (NO_TCs != tc_status) {
            if (READ_TC == tc_status) {
                tc_success &= TCHandler(zephyrRX.zephyr_tc);
            } else {
                tc_success = false;
            }
            tc_status = zephyrRX.GetTelecommand();
        }
        zephyrTX.TCAck(tc_success);
        break;
    case SAck:
        S_ack_flag = (zephyrRX.zephyr_ack == 1) ? ACK : NAK;
        break;
    case RAAck:
        RA_ack_flag = (zephyrRX.zephyr_ack == 1) ? ACK : NAK;
        break;
    case TMAck:
        TM_ack_flag = (zephyrRX.zephyr_ack == 1) ? ACK : NAK;
        break;
    case NO_ZEPHYR_MSG:
        break;
    default:
        log_error("Unknown message to route");
        break;
    }

    last_zephyr = now();
}

void StratoCore::RunScheduler()
{
    uint8_t scheduled_action = scheduler.CheckSchedule();

    while (scheduled_action != NO_SCHEDULED_ACTION) {
        ActionHandler(scheduled_action);
        scheduled_action = scheduler.CheckSchedule();
    }
}

void StratoCore::ZephyrLogFine(const char * log_info)
{
    Serial.print("Zephyr-FINE: ");
    Serial.println(log_info);
    zephyrTX.TM_String(FINE, log_info);
}

void StratoCore::ZephyrLogWarn(const char * log_info)
{
    Serial.print("Zephyr-WARN: ");
    Serial.println(log_info);
    zephyrTX.TM_String(WARN, log_info);
}

void StratoCore::ZephyrLogCrit(const char * log_info)
{
    Serial.print("Zephyr-CRIT: ");
    Serial.println(log_info);
    zephyrTX.TM_String(CRIT, log_info);
}

// TODO: Make more efficient (get rid of String class usage)
void StratoCore::UpdateTime()
{
    int32_t before, new_time, difference;
    TimeElements new_time_elements;
    String temp_str = "";

    new_time_elements.Hour = zephyrRX.zephyr_gps.hour;
    new_time_elements.Minute = zephyrRX.zephyr_gps.minute;
    new_time_elements.Second = zephyrRX.zephyr_gps.second;
    new_time_elements.Day = zephyrRX.zephyr_gps.day;
    new_time_elements.Month = zephyrRX.zephyr_gps.month;
    new_time_elements.Year = (uint8_t) (zephyrRX.zephyr_gps.year - 1970);

    before = now();
    new_time = makeTime(new_time_elements);
    difference = new_time - before;

    // if the time difference is greater than the configured maximum, update
    if (difference > MAX_TIME_DRIFT || difference < -MAX_TIME_DRIFT) {
        log_nominal("Correcting time drift");

        noInterrupts();
        setTime(new_time);
        interrupts();

        scheduler.UpdateScheduleTime(difference);
    }

    time_valid = true;

    temp_str = " " + String(new_time_elements.Hour) + ":" + String(new_time_elements.Minute) + ":" + String(new_time_elements.Second);
    temp_str += ", " + String(new_time_elements.Month) + "/" + String(new_time_elements.Day) + "/" + String(new_time_elements.Year + 1970);
    log_nominal(temp_str.c_str());
}