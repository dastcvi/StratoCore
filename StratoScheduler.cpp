/*
 *  StratoScheduler.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file implements a class to perform scheduling through the
 *  StratoCore based on GPS time from the Zephyr.
 */

#include "StratoScheduler.h"
#include "StratoGroundPort.h"

uint8_t StratoScheduler::CheckSchedule()
{
    uint8_t action = NO_SCHEDULED_ACTION;

    // if it's time for the top action, set it and remove it from the queue
    if (schedule_queue.size() > 0 && schedule_queue.top().time <= now()) {
        action = schedule_queue.top().action;
        schedule_queue.pop();
    }

    return action;
}

bool StratoScheduler::AddAction(uint8_t action, time_t seconds_from_now)
{
    // enforce a software size limit to avoid accidentally filling memory
    if (schedule_queue.size() > MAX_SCHEDULE_SIZE) {
        log_error("Schedule queue full");
        return false;
    }

    // calculate the time_t value given the current time, place on the queue
    schedule_queue.emplace(action, now() + seconds_from_now);
    return true;
}

bool StratoScheduler::AddAction(uint8_t action, TimeElements exact_time)
{
    // enforce a software size limit to avoid accidentally filling memory
    if (schedule_queue.size() > MAX_SCHEDULE_SIZE) {
        log_error("Schedule queue full");
        return false;
    }

    // place on the queue
    schedule_queue.emplace(action, makeTime(exact_time));
    return true;
}

void StratoScheduler::ClearSchedule()
{
    while (schedule_queue.size() > 0) {
        schedule_queue.pop();
    }
}