//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2017 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <smile/types.h>

// Shared Win32 support code for OS-specific data sources.

#if (SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Get the current date-and-time, in the current (local) timezone, as a
// Unix-style timestamp (seconds since midnight Jan 1 1970).
double Os_GetDateTime(void)
{
#	define WINDOWS_TICKS_PER_SECOND 10000000
#	define EPOCH_DIFFERENCE 11644473600LL

	FILETIME filetime;
	double nanoseconds;

	GetSystemTimeAsFileTime(&filetime);

	nanoseconds = (double)(((UInt64)filetime.dwHighDateTime << 32) + filetime.dwLowDateTime);

	return (nanoseconds / WINDOWS_TICKS_PER_SECOND - EPOCH_DIFFERENCE);
}

// Get the offset of the current (local) timezone, in minutes relative to UTC.
int Os_GetTimeZoneOffset(void)
{
	TIME_ZONE_INFORMATION timeZoneInformation;
	DWORD result = GetTimeZoneInformation(&timeZoneInformation);
	if (result == TIME_ZONE_ID_UNKNOWN || result == TIME_ZONE_ID_INVALID)
		return 0;
	return (int)timeZoneInformation.Bias;
}

// Copy the name of the current (local) timezone into 'buffer', up to 'bufSize' characters,
// including a trailing '\0'.  Returns the number of characters written (not including the
// trailing nul), or a negative number if the buffer isn't big enough.
int Os_GetTimeZoneName(char *buffer, int bufSize)
{
	TIME_ZONE_INFORMATION timeZoneInformation;
	WCHAR *src;
	DWORD result;
	char *dest;
	
	result = GetTimeZoneInformation(&timeZoneInformation);
	if (result == TIME_ZONE_ID_UNKNOWN || result == TIME_ZONE_ID_INVALID)
		return 0;

	if (result == TIME_ZONE_ID_STANDARD)
		src = timeZoneInformation.StandardName;
	else if (result == TIME_ZONE_ID_DAYLIGHT)
		src = timeZoneInformation.DaylightName;
	else {
		if (bufSize > 0) *buffer = '\0';
		return 0;
	}

	for (dest = buffer; bufSize > 0; bufSize--, dest++, src++) {
		*dest = (char)*src;
		if (*src == '\0') {
			return dest - buffer;
		}
	}
	if (bufSize > 0) *buffer = '\0';
	return -1;
}

// Get the ID of the current process (0 on OSes that don't support multiprocessing).
int Os_GetProcessId(void)
{
	return (int)GetCurrentProcessId();
}

// Get the ID of the currently-logged-in user.  Windows has no such concept, so we return
// the same meaningless ID that the WSL environment returns for all users: 1000.
int Os_GetUserId(void)
{
	return (int)1000;
}

// Copy the name of the currently-logged-in user into 'buffer', up to 'bufSize' characters,
// including a trailing '\0'.  Returns the number of characters written (not including the
// trailing nul), or a negative number if the buffer isn't big enough.
int Os_GetUserName(char *buffer, int bufSize)
{
#	define TEMP_SIZE (65536)	// Probably big enough?

	struct passwd pwd;
	struct passwd *result;
	uid_t uid;
	const char *src;
	char *dest;
	char *temp;

	if (bufSize <= 0) return -1;

	temp = (char *)malloc(TEMP_SIZE);

	uid = geteuid();
	getpwuid_r(uid, &pwd, temp, TEMP_SIZE, &result);
	if (result == NULL) {
		if (bufSize > 0) *buffer = '\0';
		free(temp);
		return -1;
	}
	else {
		for (src = pwd.pw_name, dest = buffer; bufSize > 0; bufSize--, dest++, src++) {
			*dest = *src;
			if (*src == '\0') {
				free(temp);
				return dest - buffer;
			}
		}
		if (bufSize > 0) *buffer = '\0';
		free(temp);
		return -1;
	}
}

// Copy random bytes from the OS's entropy source/random-data-source into 'buffer',
// of exactly 'bufSize' bytes.  This should only be called for "small" sizes of buffers,
// such as 256 bytes or less; "large" requests may fail on some platforms.  This function
// can be "slow," so you shouldn't call it often.  Returns '1' on success, '0' on failure.
int Os_GetRandomData(void *buffer, int bufSize)
{
	if (bufSize <= 0) return 0;

	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) return 0;

	ssize_t result = read(fd, buffer, (size_t)bufSize);
	if (result < 0) return 0;

	return 1;
}

#endif
