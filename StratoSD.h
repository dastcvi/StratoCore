/*
 *  StratoSD.h
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file declares functions for use running the SD card
 */

#ifndef STRATOSD_H
#define STRATOSD_H

bool StartSD();

bool FileWrite(const char * filename, const char * buffer, int buffer_size);

#endif /* STRATOSD_H */