
#ifndef __SMILE_PLATFORM_LINUX_GCC_X64_PLATFORM_H__
#define __SMILE_PLATFORM_LINUX_GCC_X64_PLATFORM_H__

//------------------------------------------------------------------------------------------------
//  OS: Linux.  Compiler: GCC.  Architecture: Intel x64.

#ifdef SMILE_PLATFORM_IS_DEFINED
#error Only one platform can be selected at a time!
#endif
#define SMILE_PLATFORM_IS_DEFINED

#define SMILE_OS	SMILE_OS_LINUX
#define SMILE_CPU	SMILE_CPU_X64
#define SMILE_COMPILER	SMILE_COMPILER_GCC
#define SMILE_ENDIAN	SMILE_ENDIAN_LITTLE

//------------------------------------------------------------------------------------------------
//  Portable type definitions.

// Portable fixed-size and fixed-sign types.
typedef char SByte;
typedef unsigned char Byte;
typedef char Int8;
typedef unsigned char UInt8;
typedef short Int16;
typedef unsigned short UInt16;
typedef int Int32;
typedef unsigned int UInt32;
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
typedef struct __attribute__((aligned(4))) { UInt32 value; } Real32;
typedef struct __attribute__((aligned(8))) { UInt64 value; } Real64;
typedef struct __attribute__((aligned(16))) { UInt64 value[2]; } Real128;

// Needed for Linux to support 64-bit system calls.
#define _LARGEFILE64_SOURCE

// If we have support for __int128, enable it.
#if __SIZEOF_INT128__ == 16
#	undef SMILE_COMPILER_HAS_INT128
#	define SMILE_COMPILER_HAS_INT128 1
	typedef __int128 Int128;

#	undef SMILE_COMPILER_HAS_UINT128
#	define SMILE_COMPILER_HAS_UINT128 1
	typedef unsigned __int128 UInt128;
#endif

//------------------------------------------------------------------------------------------------
//  GC needs to know what platform we're on.

#define GC_THREADS
#define GC_LINUX_THREADS

//------------------------------------------------------------------------------------------------
//  Declaration prefixes.

// How to make functions behave as 'inline' in this compiler.
#undef Inline
#define Inline static __inline__

// How to declare thread-local data.
#define SMILE_HAS_THREAD_LOCAL True
#define SMILE_THREAD_LOCAL __thread

// How to align data structures in memory.
#undef SMILE_ALIGN
#define SMILE_ALIGN(__n__) __attribute__((aligned(n)))

#if __GNUC__ >= 4
	#undef SMILE_API_FUNC
	#define SMILE_API_FUNC extern
	#undef SMILE_API_DATA
	#define SMILE_API_DATA extern

	#undef SMILE_INTERNAL_FUNC
	#define SMILE_INTERNAL_FUNC __attribute__ ((visibility ("hidden")))
	#undef SMILE_INTERNAL_DATA
	#define SMILE_INTERNAL_DATA __attribute__ ((visibility ("hidden")))
#endif

// Compatibility macros.
#undef SMILE_NO_RETURN
#define SMILE_NO_RETURN __attribute((noreturn))
#define SMILE_IGNORE_UNUSED_VARIABLES _Pragma("GCC diagnostic ignored \"-Wunused-variable\"")
#undef SMILE_UNREACHABLE
#define SMILE_UNREACHABLE __builtin_unreachable();

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

//------------------------------------------------------------------------------------------------
//  Optimized bit-search functions.

Inline UInt32 UInt32_CountLeadingZeros(UInt32 value)
{
	return value ? (UInt32)__builtin_clz((unsigned int)value) : 32;
}

Inline UInt32 UInt32_CountTrailingZeros(UInt32 value)
{
	return value ? (UInt32)__builtin_ctz((unsigned int)value) : 32;
}

Inline UInt64 UInt64_CountLeadingZeros(UInt64 value)
{
	return value ? (UInt64)__builtin_clzll((unsigned long long)value) : 64;
}

Inline UInt64 UInt64_CountTrailingZeros(UInt64 value)
{
	return value ? (UInt64)__builtin_ctzll((unsigned long long)value) : 64;
}

#endif
