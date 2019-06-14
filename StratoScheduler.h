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

#define NO_SCHEDULED_ACTION 0

#include <TimeLib.h>
#include <stdint.h>
#include <queue>
#include <vector>
#include <functional>

using namespace std;

class StratoScheduler {
public:
    // returns 0 if no action ready, or the id if one is ready
    uint8_t CheckSchedule();

    // overloaded method for scheduling actions
    void AddAction(uint8_t action, uint32_t seconds);
    void AddAction(uint8_t action, TimeElements tm);

private:
    // define a struct for internal use only as a container for scheduled actions
    struct ScheduleItem_t {
        uint8_t action;
        uint32_t time;
        // need to define how to perform > operator on this type
        bool operator>(const ScheduleItem_t& rhs) { return time > rhs.time; }
    };

    // define a priority queue for sorting and keeping all of the scheduled items
    priority_queue<ScheduleItem_t, vector<ScheduleItem_t>, greater<ScheduleItem_t>> schedule_queue;
};

#endif