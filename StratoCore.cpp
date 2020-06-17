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

StratoCore::StratoCore(Stream * zephyr_serial, Instrument_t instrument, Stream * dbg_serial)
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

    debug_serial = dbg_serial; // located in StratoGroundPort

    last_zephyr = now();
}

void StratoCore::InitializeCore()
{
    if (!StartSD()) {
        log_error("StratoCore unable to start SD card");
    } else {
        log_nominal("StratoCore started SD card");
    }

    InitializeWatchdog();
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
                // if the TC is a reset request, ack, then perform the reset, otherwise route to instrument
                if (RESET_INST == zephyrRX.zephyr_tc) {
                    zephyrTX.TCAck(true);
                    delay(100);
                    SCB_AIRCR = 0x5FA0004; // write the reset key and bit to the ARM AIRCR register
                } else if (GETTMBUFFER == zephyrRX.zephyr_tc) {
                    SendTMBuffer();
                    tc_success = true;
                } else {
                    tc_success &= TCHandler(zephyrRX.zephyr_tc);
                }
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
    if (NULL == log_info) return;

    debug_serial->print("Zephyr-FINE: ");
    debug_serial->println(log_info);
    zephyrTX.TM_String(FINE, log_info);
    TM_ack_flag = NO_ACK;
}

void StratoCore::ZephyrLogWarn(const char * log_info)
{
    if (NULL == log_info) return;

    debug_serial->print("Zephyr-WARN: ");
    debug_serial->println(log_info);
    zephyrTX.TM_String(WARN, log_info);
    TM_ack_flag = NO_ACK;
}

void StratoCore::ZephyrLogCrit(const char * log_info)
{
    if (NULL == log_info) return;

    debug_serial->print("Zephyr-CRIT: ");
    debug_serial->println(log_info);
    zephyrTX.TM_String(CRIT, log_info);
    TM_ack_flag = NO_ACK;
}

void StratoCore::SendTMBuffer()
{
    // use only the first flag to report the motion
    zephyrTX.setStateDetails(1, "TM buffer as requested");
    zephyrTX.setStateFlagValue(1, FINE);
    zephyrTX.setStateFlagValue(2, NOMESS);
    zephyrTX.setStateFlagValue(3, NOMESS);

    TM_ack_flag = NO_ACK;
    zephyrTX.TM();
}

bool StratoCore::WriteFileTM(const char * file_prefix)
{
    char filename[64] = {0};
    uint8_t * tm_buffer = NULL;
    uint16_t tm_size = 0;

    if (NULL == file_prefix) return false;

    // prep the filename and check it was created correctly
    if (63 < snprintf(filename, 64, "%s_%ld.dat", file_prefix, now())) return false;

    // get a pointer to the TM buffer and its size
    tm_size = zephyrTX.getTmBuffer(&tm_buffer);

    bool success = FileWrite(filename, (const char *) tm_buffer, tm_size);

    return success;
}

void StratoCore::UpdateTime()
{
    int32_t before, new_time, difference;
    TimeElements new_time_elements;
    char gps_string[100] = {0};

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

    snprintf(gps_string, 100, "%u:%u:%u %u/%u/%u, SZA: %f", new_time_elements.Hour, new_time_elements.Minute, new_time_elements.Second,
             new_time_elements.Month, new_time_elements.Day, new_time_elements.Year + 1970, zephyrRX.zephyr_gps.solar_zenith_angle);

    log_nominal(gps_string);
}