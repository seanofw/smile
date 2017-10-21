
#ifndef __SMILE_TYPES_H__
#define __SMILE_TYPES_H__

//------------------------------------------------------------------------------------------------
//  Common type declarations

#include <stddef.h>		// Gives us NULL and size_t and offsetof() and stuff like that.
#include <stdarg.h>		// Gives us va_list and va_start() and va_arg() and va_end().

// A suitable Boolean type, which should be no larger than a single byte.  True and False must be
// defined as 1 and 0, respectively.
typedef unsigned char Bool;
#define True 1
#define False 0

// A macro to mark unused parameters as unused to avoid compiler warnings.
#define UNUSED(__x__) ((void)(__x__))

// Macros for performing bit-rotation on an integer (defined early to allow for optimized compiler substitutes).
#define Smile_RotateLeft8(__n__, __m__) ((((UInt8)__n__) << (__m__)) | (((UInt8)__n__) >> (8 - (__m__))))
#define Smile_RotateRight8(__n__, __m__) ((((UInt8)__n__) >> (__m__)) | (((UInt8)__n__) << (8 - (__m__))))

#define Smile_RotateLeft16(__n__, __m__) ((((UInt16)__n__) << (__m__)) | (((UInt16)__n__) >> (16 - (__m__))))
#define Smile_RotateRight16(__n__, __m__) ((((UInt16)__n__) >> (__m__)) | (((UInt16)__n__) << (16 - (__m__))))

#define Smile_RotateLeft32(__n__, __m__) ((((UInt32)__n__) << (__m__)) | (((UInt32)__n__) >> (32 - (__m__))))
#define Smile_RotateRight32(__n__, __m__) ((((UInt32)__n__) >> (__m__)) | (((UInt32)__n__) << (32 - (__m__))))

#define Smile_RotateLeft64(__n__, __m__) ((((UInt64)__n__) << (__m__)) | (((UInt64)__n__) >> (64 - (__m__))))
#define Smile_RotateRight64(__n__, __m__) ((((UInt64)__n__) >> (__m__)) | (((UInt64)__n__) << (64 - (__m__))))

// Compatibility macros.
#define SMILE_DECLARATION_STATIC_PROTOTYPE extern
#define SMILE_DECLARATION_EXTERN_OF_UNKNOWN_SIZE
#define SMILE_NO_RETURN
#define SMILE_UNREACHABLE

// How to export API functions and data.
#ifdef SMILELIB_BUILD
	#define SMILE_API_FUNC
	#define SMILE_API_DATA extern
	#define SMILE_INTERNAL_FUNC
	#define SMILE_INTERNAL_DATA
#else
	#define SMILE_API_FUNC extern
	#define SMILE_API_DATA extern
	#define SMILE_INTERNAL_FUNC
	#define SMILE_INTERNAL_DATA
#endif

// How to align data structures in memory.
//
// WARNING:  Due to bugs in MSVC, using this can be dangerous, often resulting in spurious
//   'error C2719' output.  Use with caution, and test on MSVC 32-bit AND 64-bit to be sure.
#define SMILE_ALIGN(__n__)

// How to make functions behave as 'inline', or as close to it as possible.
#define Inline static

// External debugger support is only present on some systems, so by default, its macros are no-ops.
#define SMILE_DEBUGGER_BREAK ((void)0)
#define SMILE_DEBUGGER_IS_ATTACHED (0)
#define SMILE_DEBUGGER_BREAK_IF_ATTACHED ((void)0)

//------------------------------------------------------------------------------------------------
//  Platform-specific type declarations and overrides.

#include "platform/platform.h"

//------------------------------------------------------------------------------------------------

// Minima for the signed integer types.
#define Int8Min  ((Int8)(UInt8)0x80)
#define Int16Min ((Int16)(UInt16)0x8000)
#define Int32Min ((Int32)(UInt32)0x80000000U)
#define Int64Min ((Int64)(UInt64)0x8000000000000000ULL)
#define IntMin ((Int)(((UInt)1) << (sizeof(UInt) * 8 - 1)))
#define ByteMin ((UInt8)0)

// Maxima for the signed integer types.
#define Int8Max  ((Int8)(UInt8)0x7F)
#define Int16Max ((Int16)(UInt16)0x7FFF)
#define Int32Max ((Int32)(UInt32)0x7FFFFFFFU)
#define Int64Max ((Int64)(UInt64)0x7FFFFFFFFFFFFFFFULL)
#define IntMax ((Int)((((UInt)1) << (sizeof(UInt) * 8 - 1)) - 1))
#define ByteMax ((UInt8)0xFF)

// Maxima for the unsigned integer types.
#define UInt8Max (~(UInt8)0)
#define UInt16Max (~(UInt16)0)
#define UInt32Max (~(UInt32)0)
#define UInt64Max (~(UInt64)0)
#define UIntMax (~(UInt)0)
#define PtrIntMax (~(PtrInt)0)

// Global functions for the core runtime.
SMILE_API_FUNC void Smile_Init(void);
SMILE_API_FUNC void Smile_End(void);
SMILE_API_FUNC void Smile_Abort_OutOfMemory(void);
SMILE_API_FUNC void Smile_Abort_FatalError(const char *message);

SMILE_API_DATA UInt64 Smile_StartTicks;
SMILE_API_FUNC UInt64 Smile_GetTicks(void);
SMILE_API_FUNC double Smile_TicksToSeconds(UInt64 ticks);
SMILE_API_FUNC UInt64 Smile_TicksToMilliseconds(UInt64 ticks);
SMILE_API_FUNC UInt64 Smile_TicksToMicroseconds(UInt64 ticks);

/// <summary>
/// Convert a numeric value to its string representation at compile time.
/// </summary>
#define TOSTRING_AT_COMPILE_TIME(__n__) TOSTRING_AT_COMPILE_TIME2(__n__)
#define TOSTRING_AT_COMPILE_TIME2(__n__) #__n__

/// <summary>
/// Assertions, which are useful for guaranteeing correct behavior (in debug builds).
/// </summary>
#define SMILE_ASSERT(__n__) \
	do { \
		if (!(__n__)) { \
			SMILE_DEBUGGER_BREAK_IF_ATTACHED; \
			Smile_Abort_FatalError(__FILE__ ":" TOSTRING_AT_COMPILE_TIME(__LINE__) ": " TOSTRING_AT_COMPILE_TIME(__n__)); \
		} \
	} while (0);

#endif
