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

StratoScheduler::StratoScheduler()
{
    schedule_size = 0;
}

uint8_t StratoScheduler::CheckSchedule()
{
    uint8_t action = NO_SCHEDULED_ACTION;

    // if it's time for the top action, set it and remove it from the queue
    // if (schedule_queue.size() > 0 && schedule_queue.top().time <= now()) {
    //     action = schedule_queue.top().action;
    //     schedule_queue.pop();
    // }

    return action;
}

bool StratoScheduler::AddAction(uint8_t action, time_t seconds_from_now)
{
    // enforce a software size limit to avoid accidentally filling memory
    if (schedule_size >= MAX_SCHEDULE_SIZE) {
        log_error("Schedule queue full");
        return false;
    }

    // calculate the time_t value given the current time, place on the queue
    SchedulePush(action, now() + seconds_from_now);
    return true;
}

bool StratoScheduler::AddAction(uint8_t action, TimeElements exact_time)
{
    // enforce a software size limit to avoid accidentally filling memory
    if (schedule_size >= MAX_SCHEDULE_SIZE) {
        log_error("Schedule queue full");
        return false;
    }

    // place on the queue
    SchedulePush(action, makeTime(exact_time));
    return true;
}

void StratoScheduler::ClearSchedule()
{
    while (schedule_size > 0) {
        Serial.println(schedule_size);
        SchedulePop();
    }
}

// ------- schedule queue functions -------

bool StratoScheduler::SchedulePush(uint8_t action, time_t schedule_time)
{
    if (schedule_size >= MAX_SCHEDULE_SIZE) return false;

    // create the new action
    ScheduleItem_t * new_item = new ScheduleItem_t(action, schedule_time, NULL, NULL);

    // check that it's good
    if (new_item == NULL) return false;

    schedule_size++;

    // if empty, create in place
    if (schedule_top == NULL) {
        schedule_top = new_item;
        return true;
    }

    // create an iterator for moving through the queue, starting with the first element
    ScheduleItem_t * cur = schedule_top;

    // move through the queue until either the end is reach, or the insertion spot is found
    while (new_item->time > cur->time && cur->next != NULL) {
        cur = cur->next;
    }

    // place
    if (new_item->time <= cur->time) {
        if (cur->prev == NULL) {
            schedule_top = new_item;
            cur->prev = new_item;
            new_item->next = cur;
        } else {
            new_item->prev = cur->prev;
            cur->prev->next = new_item;
            cur->prev = new_item;
            new_item->next = cur;
        }
    } else {
        cur->next = new_item;
        new_item->prev = cur;
    }

    return true;
}

ScheduleItem_t * StratoScheduler::SchedulePeek()
{
    if (schedule_size == 0) {
        return NULL;
    } else {
        return schedule_top;
    }
}

void StratoScheduler::SchedulePop()
{
    if (schedule_size == 0) return;

    ScheduleItem_t * tmp = schedule_top;

    schedule_top = tmp->next;

    delete tmp;

    schedule_size--;
}