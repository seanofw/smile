
#ifndef __SMILE_PLATFORM_PLATFORM_H__
#define __SMILE_PLATFORM_PLATFORM_H__

//------------------------------------------------------------------------------------------------
//  Operating system IDs.

#define SMILE_OS_FAMILY	0xFF00

#define SMILE_OS_UNIX_FAMILY	0x0100
#define SMILE_OS_LINUX	0x0101	// GNU/Linux, to be precise :-)
#define SMILE_OS_FREEBSD	0x0102
#define SMILE_OS_MACOSX	0x0103
#define SMILE_OS_CYGWIN	0x0104
#define SMILE_OS_SUNOS	0x0105
#define SMILE_OS_SOLARIS	0x0106
#define SMILE_OS_ULTRIX	0x0107
#define SMILE_OS_HPUX	0x0108
#define SMILE_OS_AIX	0x0109
	
#define SMILE_OS_WINDOWS_FAMILY	0x0200
#define SMILE_OS_WINDOWS	0x0201	// Any flavor of Win32/Win64: WinNT or Win9x

//------------------------------------------------------------------------------------------------
//  Compiler IDs.

#define SMILE_COMPILER_GCC	1
#define SMILE_COMPILER_MSVC	2
#define SMILE_COMPILER_CLANG	3

//------------------------------------------------------------------------------------------------
//  CPU IDs.

#define SMILE_CPU_X86	0x0101
#define SMILE_CPU_X64	0x0102
#define SMILE_CPU_X86_FAMILY	0x01FF
	
#define SMILE_CPU_ARM32	0x0201
#define SMILE_CPU_ARM64	0x0202
#define SMILE_CPU_ARM_FAMILY	0x02FF
	
#define SMILE_CPU_POWERPC32	0x0301
#define SMILE_CPU_POWERPC64	0x0302
#define SMILE_CPU_POWERPC_FAMILY	0x03FF
	
#define SMILE_CPU_SPARC_FAMILY	0x04FF
	
#define SMILE_CPU_MIPS_FAMILY	0x05FF

//------------------------------------------------------------------------------------------------
//  Endianness IDs.

#define SMILE_ENDIAN_LITTLE	0
#define SMILE_ENDIAN_BIG	1

//------------------------------------------------------------------------------------------------
//  Compiler compatibility default settings.

#define SMILE_COMPILER_HAS_INT128 0

//------------------------------------------------------------------------------------------------
//  Platform-specific type declarations.
//
//  To port Smile to your platform, you will need to define a suitable 'platform.h' file,
//  and you will need to modify, patch, or implement the following:
//
//    -	The Boehm garbage collector may require modifications.
//    -	The Intel decimal floating point library may require modifications.
//    -	You will need to implement an 'atomic.c' to provide atomic CPU operations.
//    -	[future] The Oniguruma regex library may require modifications.
//    -	[future] You will need to implement a 'thread.c' to create and manage kernel threads.
//    -	[future] You will need to implement file and directory I/O suitable to your OS.
//    -	[future] You will need to implement network I/O suitable to your OS.

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

#elif defined(__APPLE__) && defined(__MACH__) && defined(__ppc__)
	#error OSX PPC support hasn`t been implemented yet.  If you want to implement it, we`d love to have it.

#elif defined(__APPLE__) && defined(__MACH__) && defined(__ppc64__)
	#error OSX PPC support hasn`t been implemented yet.  If you want to implement it, we`d love to have it.

#elif defined(__APPLE__) && defined(__MACH__) && defined(__i386__)
	#error We haven`t started i386 OSX support yet... 

#elif defined(__APPLE__) && defined(__MACH__) && defined(__x86_64__)
	#include "macosx-clang-x64/platform.h"
	
#else
	#error Unsupported OS/compiler/architecture; please configure <smile/platform/platform.h> for your environment.

#endif

//------------------------------------------------------------------------------------------------
// Platform-specific functions for low-level access to OS information.

// Get the current date-and-time, in the current (local) timezone, as a
// Unix-style timestamp (seconds since midnight Jan 1 1970).
SMILE_API_FUNC double Os_GetDateTime(void);

// Get the offset of the current (local) timezone, in minutes relative to UTC.
SMILE_API_FUNC int Os_GetTimeZoneOffset(void);

// Retrieve the name of the current (local) timezone.
SMILE_API_FUNC String Os_GetTimeZoneName(void);

// Get the ID of the current process (0 on OSes that don't support multiprocessing).
SMILE_API_FUNC int Os_GetProcessId(void);

// Get the ID of the currently-logged-in user (0 on single-user OSes).
SMILE_API_FUNC int Os_GetUserId(void);

// Retrieve the name of the currently-logged-in user.
SMILE_API_FUNC String Os_GetUserName(void);

// Copy random bytes from the OS's entropy source/random-data-source into 'buffer',
// of exactly 'bufSize' bytes.  This should only be called for "small" sizes of buffers,
// such as 256 bytes or less; "large" requests may fail on some platforms.  This function
// can be "slow," so you shouldn't call it often.  Returns '1' on success, '0' on failure.
SMILE_API_FUNC Bool Os_GetRandomData(void *buffer, int bufSize);

#endif
