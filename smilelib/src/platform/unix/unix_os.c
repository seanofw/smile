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

// Shared Unix/Linux/MacOS X support code for OS-specific data sources.

#if (SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY

#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>

// Get the current date-and-time, in the current (local) timezone, as a
// Unix-style timestamp (seconds since midnight Jan 1 1970).
double Os_GetDateTime(void)
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL)) return 0;
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

// Get the offset of the current (local) timezone, in minutes relative to UTC.
int Os_GetTimeZoneOffset(void)
{
	struct timeval tv;
	struct timezone tz;
	if (gettimeofday(&tv, &tz)) return 0;
	return tz.tz_minuteswest;
}

#define TEMP_SIZE (65536)	// Probably big enough?

// Get the name of the current (local) timezone.
String Os_GetTimeZoneName()
{
	struct tm t;
	int bufLen;
	String timeZoneName;
	char *temp;

	temp = (char *)malloc(TEMP_SIZE);
	if (temp == NULL)
		return String_Empty;

	bufLen = strftime(temp, TEMP_SIZE - 1, "%z", &t);

	if (bufLen <= 0)
		timeZoneName = String_Empty;
	else {
		temp[TEMP_SIZE - 1] = '\0';
		timeZoneName = String_FromC(temp);
	}

	free(temp);
	return timeZoneName;
}

// Get the ID of the current process (0 on OSes that don't support multiprocessing).
int Os_GetProcessId(void)
{
	return (int)getpid();
}

// Get the ID of the currently-logged-in user (0 on single-user OSes).
int Os_GetUserId(void)
{
	return (int)geteuid();
}

// Get the name of the currently-logged-in user.
String Os_GetUserName()
{
	struct passwd pwd;
	struct passwd *result;
	uid_t uid;
	char *temp;
	String userName;

	if (bufSize <= 0)
		return String_Empty;

	temp = (char *)malloc(TEMP_SIZE);
	if (temp == NULL)
		return String_Empty;

	pwd.pw_name = NULL;

	uid = geteuid();
	getpwuid_r(uid, &pwd, temp, TEMP_SIZE, &result);
	if (result == NULL) {
		userName = String_Empty;
	}
	else if (pwd.pw_name == NULL || pwd.pw_name < temp || pwd.pw_name >= temp + TEMP_SIZE)
		userName = String_Empty;
	else {
		temp[TEMP_SIZE - 1] = '\0';
		userName = String_FromC(pwd.pw_name);
	}

	free(temp);
	return userName;
}

// Copy random bytes from the OS's entropy source/random-data-source into 'buffer',
// of exactly 'bufSize' bytes.  This should only be called for "small" sizes of buffers,
// such as 256 bytes or less; "large" requests may fail on some platforms.  This function
// can be "slow," so you shouldn't call it often.  Returns '1' on success, '0' on failure.
Bool Os_GetRandomData(void *buffer, int bufSize)
{
	if (bufSize <= 0) return 0;

	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) return 0;

	ssize_t result = read(fd, buffer, (size_t)bufSize);
	if (result < 0) return 0;

	return 1;
}

#endif
