//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

// Include headers from the host so we have access to time and I/O stuff.
#if defined(_MSC_VER)
	#define WIN32_LEAN_AND_MEAN
	#pragma warning(push)
	#pragma warning(disable:4081)
	#pragma warning(disable:4255)
	#pragma warning(disable:4668)
	#include <windows.h>
	#pragma warning(pop)
#elif defined(__GNUC__)
	#include <sys/time.h>
#else
	#error Unknown compiler; please configure "init.c" for your compiler and environment.
#endif

/// <summary>
/// The number of system ticks at the time that Smile_Init() was called.  This can be used
/// to determine how long the interpreter has been running.
/// </summary>
UInt64 Smile_StartTicks = 0;

#if defined(_MSC_VER)
	/// <summary>
	/// This is the return value of calling QueryPerformanceFrequency(), cached by Init() for reference.
	/// </summary>
	static UInt64 WindowsTicksPerSecond;
	static double WindowsTicksPerSecondFloat;
#endif

/// <summary>
/// Initialize the high-performance tick-tracking timers.
/// </summary>
void Smile_InitTicks(void)
{
	#if defined(_MSC_VER)
		LARGE_INTEGER performanceFrequency;
	#endif

	Smile_StartTicks = Smile_GetTicks();

	#if defined(_MSC_VER)
		// Precalculate the Windows ticks-per-second value.
		if (!QueryPerformanceFrequency(&performanceFrequency)) {
			WindowsTicksPerSecond = 1;
			WindowsTicksPerSecondFloat = 1.0;
		}
		else {
			WindowsTicksPerSecond = (UInt64)performanceFrequency.QuadPart;
			WindowsTicksPerSecondFloat = (double)WindowsTicksPerSecond;
		}
	#endif
}

/// <summary>
/// Get the current count of system ticks (an arbitrary high-precision measure of time).
/// These will be specific to the current processor and OS.
/// </summary>
UInt64 Smile_GetTicks(void)
{
	#if defined(_MSC_VER)
		LARGE_INTEGER performanceCount;
		if (!QueryPerformanceCounter(&performanceCount)) return 0ULL;
		else return (UInt64)performanceCount.QuadPart;
	#elif defined(__GNUC__)
		struct timeval tv;
		struct timezone tz;
		if (gettimeofday(&tv, &tz)) return 0ULL;
		else return (UInt64)tv.tv_sec * 1000000ULL + (UInt64)tv.tv_usec;
	#else
		#error Unknown compiler; please configure "init.c" for your compiler and environment.
	#endif
}

/// <summary>
/// Convert a count of ticks into a fractional count of seconds.
/// </summary>
double Smile_TicksToSeconds(UInt64 ticks)
{
	#if defined(_MSC_VER)
		return (double)ticks / WindowsTicksPerSecondFloat;
	#elif defined(__GNUC__)
		return (double)ticks / 1000000.0;
	#else
		#error Unknown compiler; please configure "init.c" for your compiler and environment.
	#endif
}

/// <summary>
/// Convert a count of ticks into an integer count of milliseconds.
/// </summary>
UInt64 Smile_TicksToMilliseconds(UInt64 ticks)
{
	#if defined(_MSC_VER)
		if (ticks < UInt64Max / 1000ULL)
			return ticks * 1000ULL / WindowsTicksPerSecond;
		else
			return ticks / WindowsTicksPerSecond * 1000ULL;
	#elif defined(__GNUC__)
		return ticks / 1000;
	#else
		#error Unknown compiler; please configure "init.c" for your compiler and environment.
	#endif
}

/// <summary>
/// Convert a count of ticks into an integer count of milliseconds.
/// </summary>
UInt64 Smile_TicksToMicroseconds(UInt64 ticks)
{
	#if defined(_MSC_VER)
		if (ticks < UInt64Max / 1000000ULL)
			return ticks * 1000000ULL / WindowsTicksPerSecond;
		else
			return ticks / WindowsTicksPerSecond * 1000000ULL;
	#elif defined(__GNUC__)
		return ticks;
	#else
		#error Unknown compiler; please configure "init.c" for your compiler and environment.
	#endif
}
