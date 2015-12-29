
#ifndef __SMILE_PLATFORM_PLATFORM_H__
#define __SMILE_PLATFORM_PLATFORM_H__

//------------------------------------------------------------------------------------------------
//  Platform-specific type declarations.

#if (defined(_WIN64) || defined(_WIN32)) && defined(_MSC_VER) && defined(_M_IX86)
	#include "windows-msvc-x86/platform.h"

#elif (defined(_WIN64) || defined(_WIN32)) && defined(_MSC_VER) && defined(_M_X64)
	#include "windows-msvc-x64/platform.h"

// Cygwin can pretend to be __linux__, so it has to be tested before real Linux.
#elif defined(__CYGWIN__) && defined(__GNUC__) && defined(__i386__)
	#include "cygwin-gcc-x86/platform.h"

#elif defined(__CYGWIN__) && defined(__GNUC__) && defined(__x86_64__)
	#include "cygwin-gcc-x64/platform.h"

#elif defined(__linux__) && defined(__GNUC__) && defined(__i386__)
	#include "linux-gcc-x86/platform.h"

#elif defined(__linux__) && defined(__GNUC__) && defined(__x86_64__)
	#include "linux-gcc-x64/platform.h"

#else
	#error Unsupported OS/compiler/architecture; please configure <smile/platform/platform.h> for your environment.

#endif

#endif
