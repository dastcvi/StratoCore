/*
 *  StratoScheduler.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file implements a class to perform scheduling through the
 *  StratoCore based on GPS time from the Zephyr.
 */

#include "StratoScheduler.h"

uint8_t StratoScheduler::CheckSchedule()
{
    return NO_SCHEDULED_ACTION;
}

void StratoScheduler::AddAction(uint8_t action, uint32_t seconds)
{
    ;
}

void StratoScheduler::AddAction(uint8_t action, TimeElements tm)
{
    ;
}