/*
 *  StratoSD.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *  
 *  This file implements functions for use running the SD card
 */

#include "StratoSD.h"
#include "SD.h"

// globals for internal use
bool sd_state = false;
File file;

bool StartSD()
{
    if (SD.begin(BUILTIN_SDCARD)) {
        sd_state = true;
    } else {
        sd_state = false;
    }

    return sd_state;
}

bool FileWrite(const char * filename, const char * buffer, int buffer_size)
{
	int bytes_written = 0;

	if (!sd_state) {
		return false;
	}

	file = SD.open(filename, FILE_WRITE);

	if (!file) {
		return false;
	}

	bytes_written = file.write(buffer, buffer_size);
	
    file.close();

	return (bytes_written == buffer_size);
}