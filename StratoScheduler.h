/*
 *  StratoScheduler.h
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file declares a class to perform scheduling through the
 *  StratoCore based on GPS time from the Zephyr.
 */

#ifndef STRATOSCHEDULER_H
#define STRATOSCHEDULER_H

#include <TimeLib.h>
#include <stdint.h>

#define NO_SCHEDULED_ACTION 0
#define MAX_SCHEDULE_SIZE   ((uint8_t) 32) // must be 0-255

// define a struct for use only as a container for scheduled actions
struct ScheduleItem_t {
    // constructor
    ScheduleItem_t(uint8_t act, time_t t, bool e, ScheduleItem_t * p, ScheduleItem_t * n) 
        : action(act), time(t), exact_time (e), prev(p), next(n) { }

    // data
    uint8_t action;
    time_t time;
    bool exact_time; // scheduled exact or relative?
    ScheduleItem_t * prev;
    ScheduleItem_t * next;
};

class StratoScheduler {
public:
    // empty constructors and destructors
    StratoScheduler();
    ~StratoScheduler() { };

    // returns 0 if no action ready, or the id if one is ready
    uint8_t CheckSchedule();

    // overloaded method for scheduling actions, returns based on schedule push success
    bool AddAction(uint8_t action, time_t seconds_from_now);
    bool AddAction(uint8_t action, TimeElements exact_time);

    // changes the scheduled times according to a number of seconds to adjust
    void UpdateScheduleTime(int32_t seconds_adjustment);

    // called after every mode switch
    void ClearSchedule();

private:

    bool SchedulePush(uint8_t action, time_t schedule_time, bool exact);
    void SchedulePop(); // remove (and delete!) the first item

    uint8_t schedule_size; // num items in schedule
    ScheduleItem_t * schedule_top; // pointer to first element
};

#endif
