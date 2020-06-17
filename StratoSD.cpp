/*
 *  StratoSD.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *
 *  This file implements functions for use running the SD card
 */

#include "StratoSD.h"
#include "StratoGroundPort.h"
#include <SdFat.h>
#include <SdFatConfig.h>

// globals for internal use
bool sd_state = false;
File file;
SdFatSdio SD;

bool StartSD()
{
    if (SD.begin()) {
        sd_state = true;
    } else {
        sd_state = false;
    }

    return sd_state;
}

bool FileWrite(const char * filename, const char * buffer, int buffer_size)
{
	int bytes_written = 0;

	if (NULL == filename || NULL == buffer || 0 == buffer_size) return false;

	if (!sd_state) {
		return false;
	}

	file = SD.open(filename, FILE_WRITE);

	debug_serial->print("Filename: "); debug_serial->println(filename);

	if (!file) {
		debug_serial->println("File not ok");
		return false;
	}

	bytes_written = file.write(buffer, buffer_size);

    file.close();

	debug_serial->print("File write ok, bytes: "); debug_serial->println(bytes_written);

	return (bytes_written == buffer_size);
}