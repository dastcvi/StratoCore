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
#include <queue>
#include <vector>
#include <functional>

#define NO_SCHEDULED_ACTION 0
#define MAX_SCHEDULE_SIZE   32

using namespace std;

class StratoScheduler {
public:
    // empty constructors and destructors
    StratoScheduler() { };
    ~StratoScheduler() { };

    // returns 0 if no action ready, or the id if one is ready
    uint8_t CheckSchedule();

    // overloaded method for scheduling actions, returns based on schedule push success
    bool AddAction(uint8_t action, time_t seconds_from_now);
    bool AddAction(uint8_t action, TimeElements exact_time);

    // todo: consider adding adjustment function for when GPS time update is greater than xx seconds
    // in order to ensure that the schedule isn't messed up

    // called after every mode switch
    void ClearSchedule();

private:
    // define a private struct for use only as a container for scheduled actions
    struct ScheduleItem_t {
        uint8_t action;
        time_t time;
        
        // constructor to allow use of priority_queue emplace function
        ScheduleItem_t(uint8_t a, time_t t) : action(a), time(t) { }
    };

    // this private struct serves only to provide the operator that allows the priority queue to sort actions
    struct CompareItems { 
        bool operator()(ScheduleItem_t const& i1, ScheduleItem_t const& i2) {
            return i1.time > i2.time; 
        }
    }; 

    // define a priority queue for sorting and keeping all of the scheduled items
    priority_queue<ScheduleItem_t, vector<ScheduleItem_t>, CompareItems> schedule_queue;
};

#endif