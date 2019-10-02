/*
 *  StratoSD.cpp
 *  Author:  Alex St. Clair
 *  Created: June 2019
 *
 *  This file implements functions for use running the SD card
 */

#include "StratoSD.h"
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

	Serial.print("Filename: "); Serial.println(filename);

	if (!file) {
		Serial.println("File not ok");
		return false;
	}

	bytes_written = file.write(buffer, buffer_size);

    file.close();

	Serial.print("File write ok, bytes: "); Serial.println(bytes_written);

	return (bytes_written == buffer_size);
}