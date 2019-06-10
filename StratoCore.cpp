/*
 *  StratoCore.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file implements an Arduino library (C++ class) with core functionality
 *  for LASP Strateole payload interface boards
 */

#include "StratoCore.h"

StratoCore::StratoCore()
{
    inst_mode = MODE_SB; // always boot to standby
    inst_substate = MODE_ENTRY; // substate starts as mode entry
}

void StratoCore::RunMode()
{
    // todo: set inst_substate to MODE_ENTRY on every switch
    
    switch (inst_mode) {
    case MODE_SB:
        StandbyMode();
        break;
    case MODE_FL:
        FlightMode();
        break;
    case MODE_LP:
        LowPowerMode();
        break;
    case MODE_SA:
        SafetyMode();
        break;
    case MODE_EF:
        EndOfFlightMode();
        break;
    default:
        // todo: log error
        inst_mode = MODE_SB;
        StandbyMode();
        break;
    }
}