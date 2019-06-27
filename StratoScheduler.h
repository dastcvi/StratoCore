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
    ScheduleItem_t(uint8_t act, time_t t, ScheduleItem_t * p, ScheduleItem_t * n) : action(act), time(t), prev(p), next(n) { }

    // data
    uint8_t action;
    time_t time;
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

    // todo: consider adding adjustment function for when GPS time update is greater than xx seconds
    // in order to ensure that the schedule isn't messed up when time changes underneath it
    // void UpdateScheduleTime();

    // called after every mode switch
    void ClearSchedule();

//private:

    bool SchedulePush(uint8_t action, time_t schedule_time); // add to schedule
    ScheduleItem_t * SchedulePeek(); // look at first item without removing
    void SchedulePop(); // remove (and delete!) the first item

    uint8_t schedule_size; // num items in schedule
    ScheduleItem_t * schedule_top; // pointer to first element
};

#endif

//   -- test code -- 
 
//   delay(1000);
//   ScheduleItem_t * tmp;
  
//   Serial.println("beginning");
//   Serial.println(strato.scheduler.schedule_size);
//   strato.scheduler.SchedulePop();
//   Serial.println(strato.scheduler.schedule_size);
//   strato.scheduler.SchedulePush(0, 10);
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePush(0, 11);
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePush(0, 9);
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   strato.scheduler.SchedulePush(0, 12);
//   strato.scheduler.SchedulePush(0, 12);
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePush(0, 8);
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePush(0, 13);
//   strato.scheduler.SchedulePush(0, 13);
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePush(0, 7);
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   tmp = strato.scheduler.SchedulePeek();
//   Serial.println(tmp->time);
//   strato.scheduler.SchedulePop();
//   Serial.println(strato.scheduler.schedule_size);
//   for (int i = 0; i < 35; i++) {
//     if (i == 4) {
//       strato.scheduler.SchedulePush(0,1);
//       continue;
//     } else if (i == 6) {
//       strato.scheduler.SchedulePush(0,3);
//       continue;
//     }
//     if (!strato.scheduler.SchedulePush(0,2)) Serial.println(i);
//   }
//   strato.scheduler.ClearSchedule();
//   Serial.println(strato.scheduler.schedule_size);
//   // -- 