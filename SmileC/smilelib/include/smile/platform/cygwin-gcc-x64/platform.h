
#ifndef __SMILE_PLATFORM_CYGWIN_GCC_X64_PLATFORM_H__
#define __SMILE_PLATFORM_CYGWIN_GCC_X64_PLATFORM_H__

//------------------------------------------------------------------------------------------------
//  OS: Cygwin (Microsoft Windows).  Compiler: GCC.  Architecture: Intel x64.

#ifdef SMILE_PLATFORM_IS_DEFINED
#error Only one platform can be selected at a time!
#endif
#define SMILE_PLATFORM_IS_DEFINED

//------------------------------------------------------------------------------------------------
//  Portable type definitions.

// Portable fixed-size and fixed-sign types.
typedef char SByte;
typedef unsigned char Byte;
typedef char Int8;
typedef unsigned char UInt8;
typedef short Int16;
typedef unsigned short UInt16;
typedef long Int32;
typedef unsigned long UInt32;
typedef long long Int64;
typedef unsigned long long UInt64;

// Portable pointer-casting types.
typedef UInt64 PtrInt;		// An unsigned integer type that is the same size as a pointer.
typedef Int64 Int;			// A signed integer type that matches the native platform's "best" register size.
typedef UInt64 UInt;		// An unsigned integer type that matches the native platform's "best" register size.

#define SizeofPtrInt 8
#define SizeofInt 8

// Portable binary floating-point types.
typedef float Float32;
typedef double Float64;
typedef struct __attribute__((aligned(16))) { UInt64 value[2]; } Float128;

// Portable decimal floating-point types.
typedef struct { UInt32 value; } Real32;
typedef struct { UInt64 value; } Real64;
typedef struct __attribute__((aligned(16))) { UInt64 value[2]; } Real128;

//------------------------------------------------------------------------------------------------
//  Declaration prefixes.

// How to make functions behave as 'inline' in this compiler.
#undef Inline
#define Inline static __inline__

// How to declare thread-local data.
#define SMILE_HAS_THREAD_LOCAL True
#define SMILE_THREAD_LOCAL __thread

// How to export public functions and data outside SmileLib.
#undef SMILE_API
#ifdef SMILELIB_BUILD
	#define SMILE_API __declspec(dllexport)
#else
	#define SMILE_API extern __declspec(dllimport)
#endif

//------------------------------------------------------------------------------------------------
//  Entropy.

// Get a reasonable degree of entropy from wherever this platform keeps it, as quickly as possible.
// This doesn't need to be crypto-secure; it just needs to be suitably semi-random.
Inline UInt64 GetBaselineEntropy(void)
{
	UInt32 hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ((UInt64)lo) | (((UInt64)hi) << 32);
}

#endif