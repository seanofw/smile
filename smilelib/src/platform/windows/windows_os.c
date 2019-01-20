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
#include <smile/string.h>

// Shared Win32 support code for OS-specific data sources.

#if (SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <lmcons.h>
#include <NTSecAPI.h>

// Determine the length of the given WCHAR string by scanning it for its trailing NUL word,
// or stopping at maxLen characters.
static Int String_Utf16Length(const UInt16 *src, Int maxLen)
{
	const UInt16 *start = src;
	const UInt16 *end = src + maxLen;

	while (src < end && *src) src++;

	return src - start;
}

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

// Retrieve the name of the current local timezone.
String Os_GetTimeZoneName(void)
{
	TIME_ZONE_INFORMATION timeZoneInformation;
	WCHAR *src;
	DWORD result;
	
	result = GetTimeZoneInformation(&timeZoneInformation);
	if (result == TIME_ZONE_ID_UNKNOWN || result == TIME_ZONE_ID_INVALID)
		return 0;

	if (result == TIME_ZONE_ID_STANDARD)
		src = timeZoneInformation.StandardName;
	else if (result == TIME_ZONE_ID_DAYLIGHT)
		src = timeZoneInformation.DaylightName;
	else
		return String_Empty;

	return String_FromUtf16(src, String_Utf16Length(src, 32));
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

// Retrieve the name of the currently-logged-in user.
String Os_GetUserName(void)
{
	UInt16 username[UNLEN + 1];
	UInt32 usernameLen = UNLEN + 1;

	if (!GetUserNameW(username, &usernameLen))
		return String_Empty;

	if (usernameLen > UNLEN)
		usernameLen = UNLEN;

	return String_FromUtf16(username, (Int)usernameLen);
}

// Copy random bytes from the OS's entropy source/random-data-source into 'buffer',
// of exactly 'bufSize' bytes.  This should only be called for "small" sizes of buffers,
// such as 256 bytes or less; "large" requests may fail on some platforms.  This function
// can be "slow," so you shouldn't call it often.  Returns '1' on success, '0' on failure.
Bool Os_GetRandomData(void *buffer, int bufSize)
{
	if (bufSize <= 0) return 0;

	return RtlGenRandom(buffer, (ULONG)bufSize) != 0;
}

#endif
