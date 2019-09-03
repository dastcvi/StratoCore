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
    schedule_top = NULL;
}

uint8_t StratoScheduler::CheckSchedule()
{
    uint8_t action = NO_SCHEDULED_ACTION;

    // if it's time for the top action, set it and remove it from the queue
    if (schedule_size > 0 && schedule_top->time <= now()) {
        action = schedule_top->action;
        SchedulePop();
    }

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
    return SchedulePush(action, now() + seconds_from_now, false);
}

bool StratoScheduler::AddAction(uint8_t action, TimeElements exact_time)
{
    // enforce a software size limit to avoid accidentally filling memory
    if (schedule_size >= MAX_SCHEDULE_SIZE) {
        log_error("Schedule queue full");
        return false;
    }

    // place on the queue
    return SchedulePush(action, makeTime(exact_time), true);
}

void StratoScheduler::ClearSchedule()
{
    while (schedule_size > 0) {
        SchedulePop();
    }
}

void StratoScheduler::PrintSchedule()
{
    ScheduleItem_t * itr = schedule_top;

    // adjust each item
    while (itr != NULL) {
        Serial.print(itr->action);
        Serial.print(",");
        Serial.println(itr->time);
        itr = itr->next;
    }
}

void StratoScheduler::UpdateScheduleTime(int32_t seconds_adjustment)
{
    ScheduleItem_t * itr = schedule_top;

    // adjust each item
    while (itr != NULL) {
        // adjust only if scheduled relatively
        if (!itr->exact_time) {
            itr->time += seconds_adjustment;
        }
        itr = itr->next;
    }
}

// ------- schedule queue functions -------
ScheduleItem_t * StratoScheduler::GetFreeItem()
{
    // if we know the schedule is full, return NULL
    if (schedule_size == MAX_SCHEDULE_SIZE) return NULL;

    // iterate through and find a free item
    int i = 0;
    for (i = 0; i < MAX_SCHEDULE_SIZE; i++) {
        if (!item_array[i].in_use) break; // found a free one
    }

    // make sure there wasn't some unexpected error
    if (item_array[i].in_use) {
        return NULL;
    } else {
        return &(item_array[i]);
    }
}

bool StratoScheduler::SchedulePush(uint8_t action, time_t schedule_time, bool exact)
{
    if (schedule_size >= MAX_SCHEDULE_SIZE) return false;

    // create the new action
    ScheduleItem_t * new_item = GetFreeItem();

    // check that it's good
    if (new_item == NULL) return false;

    // set the values for the new item
    new_item->action = action;
    new_item->exact_time = exact;
    new_item->time = schedule_time;
    new_item->next = NULL;
    new_item->prev = NULL;
    new_item->in_use = true;

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

// remove and delete the first item
void StratoScheduler::SchedulePop()
{
    if (schedule_size == 0) return;

    ScheduleItem_t * tmp = schedule_top;

    schedule_top = tmp->next;

    if (NULL != schedule_top) schedule_top->prev = NULL;

    tmp->in_use = false;
    tmp->prev = NULL;
    tmp->next = NULL;

    schedule_size--;
}