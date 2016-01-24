#pragma once

#include "atoms.h"
#include "windows.h"

void UnixTimeToFileTime(u32 unixTime, FILETIME& fileTime)
{
	u64 temp = Int32x32To64(unixTime, 10000000) + 116444736000000000;
	fileTime.dwLowDateTime = (u32)temp;
	fileTime.dwHighDateTime = temp >> 32;
}

void UnixTimeToSystemTime(u32 unixTime, SYSTEMTIME& systemTime)
{
	FILETIME fileTime;
	UnixTimeToFileTime(unixTime, fileTime);
	FileTimeToSystemTime(&fileTime, &systemTime);
}