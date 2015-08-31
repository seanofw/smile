
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


//------------------------------------------------------------------------------------------------
//  Platform-specific type declarations.

#if defined(_MSC_VER)

	//------------------------------------------------------------------------------------------------
	//  Microsoft Visual C.

	// Portable fixed-size and fixed-sign types.
	typedef char SByte;
	typedef unsigned char Byte;
	typedef char Int8;
	typedef unsigned char UInt8;
	typedef short Int16;
	typedef unsigned short UInt16;
	typedef long Int32;
	typedef unsigned long UInt32;
	typedef __int64 Int64;
	typedef unsigned __int64 UInt64;
	typedef float Real32;
	typedef double Real64;

	// How to make functions behave as 'inline' in this compiler.
	#define Inline static __inline

	// How to export public functions outside SmileLib.
	#ifdef SMILELIB_BUILD
		#define SMILE_API extern __declspec(dllexport)
	#else
		#define SMILE_API extern __declspec(dllimport)
	#endif

	// Macros for performing bit-rotation on an integer.
	#define RotateLeft8(__n__, __m__) ((((UInt8)__n__) << (__m__)) | (((UInt8)__n__) >> (8 - (__m__))))
	#define RotateRight8(__n__, __m__) ((((UInt8)__n__) >> (__m__)) | (((UInt8)__n__) << (8 - (__m__))))

	#define RotateLeft16(__n__, __m__) ((((UInt16)__n__) << (__m__)) | (((UInt16)__n__) >> (16 - (__m__))))
	#define RotateRight16(__n__, __m__) ((((UInt16)__n__) >> (__m__)) | (((UInt16)__n__) << (16 - (__m__))))

	#define RotateLeft32(__n__, __m__) ((((UInt32)__n__) << (__m__)) | (((UInt32)__n__) >> (32 - (__m__))))
	#define RotateRight32(__n__, __m__) ((((UInt32)__n__) >> (__m__)) | (((UInt32)__n__) << (32 - (__m__))))

	#define RotateLeft64(__n__, __m__) ((((UInt64)__n__) << (__m__)) | (((UInt64)__n__) >> (64 - (__m__))))
	#define RotateRight64(__n__, __m__) ((((UInt64)__n__) >> (__m__)) | (((UInt64)__n__) << (64 - (__m__))))

	#if defined(_M_IX86)

		typedef UInt32 PtrInt;		// An unsigned integer type that is the same size as a pointer.
		typedef Int32 Int;			// A signed integer type that matches the native platform's "best" register size.
		typedef UInt32 UInt;		// An unsigned integer type that matches the native platform's "best" register size.

		#define SizeofPtrInt 4
		#define SizeofInt 4

		// Get a reasonable degree of entropy from wherever this platform keeps it, as quickly as possible.
		// This doesn't need to be crypto-secure; it just needs to be suitably semi-random.
        Inline UInt64 GetBaselineEntropy(void)
        {
            UInt64 tsc;
            __asm {
                cpuid
                rdtsc
                mov dword ptr [tsc+0], eax
                mov dword ptr [tsc+4], edx
            }
            return tsc;
        }

	#elif defined(_M_X64)

		typedef UInt64 PtrInt;		// An unsigned integer type that is the same size as a pointer.
		typedef Int64 Int;			// A signed integer type that matches the native platform's "best" register size.
		typedef UInt64 UInt;		// An unsigned integer type that matches the native platform's "best" register size.

		#define SizeofPtrInt 8
		#define SizeofInt 8

		extern UInt64 __rdtsc(void);
        #pragma intrinsic(__rdtsc)

		// Get a reasonable degree of entropy from wherever this platform keeps it, as quickly as possible.
		// This doesn't need to be crypto-secure; it just needs to be suitably semi-random.
        Inline UInt64 GetBaselineEntropy(void)
        {
            return __rdtsc();
        }

	#else
		#error Unknown Microsoft Visual C environment; please configure "types.h" for your environment.
	#endif

#elif defined(__GNUC__)

	//------------------------------------------------------------------------------------------------
	//  GCC

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
	typedef float Real32;
	typedef double Real64;

	// How to make functions behave as 'inline' in this compiler.
	#define Inline static __inline__

	// How to export public functions outside SmileLib.
	#define SMILE_API extern

	// Macros for performing bit-rotation on an integer.
	#define RotateLeft8(__n__, __m__) ((((UInt8)__n__) << (__m__)) | (((UInt8)__n__) >> (8 - (__m__))))
	#define RotateRight8(__n__, __m__) ((((UInt8)__n__) >> (__m__)) | (((UInt8)__n__) << (8 - (__m__))))

	#define RotateLeft16(__n__, __m__) ((((UInt16)__n__) << (__m__)) | (((UInt16)__n__) >> (16 - (__m__))))
	#define RotateRight16(__n__, __m__) ((((UInt16)__n__) >> (__m__)) | (((UInt16)__n__) << (16 - (__m__))))

	#define RotateLeft32(__n__, __m__) ((((UInt32)__n__) << (__m__)) | (((UInt32)__n__) >> (32 - (__m__))))
	#define RotateRight32(__n__, __m__) ((((UInt32)__n__) >> (__m__)) | (((UInt32)__n__) << (32 - (__m__))))

	#define RotateLeft64(__n__, __m__) ((((UInt64)__n__) << (__m__)) | (((UInt64)__n__) >> (64 - (__m__))))
	#define RotateRight64(__n__, __m__) ((((UInt64)__n__) >> (__m__)) | (((UInt64)__n__) << (64 - (__m__))))

	#if defined(__i386__)

		typedef UInt32 PtrInt;		// An unsigned integer type that is the same size as a pointer.
		typedef Int32 Int;			// A signed integer type that matches the native platform's "best" register size.
		typedef UInt32 UInt;		// An unsigned integer type that matches the native platform's "best" register size.

		#define SizeofPtrInt 4
		#define SizeofInt 4

		// Get a reasonable degree of entropy from wherever this platform keeps it, as quickly as possible.
		// This doesn't need to be crypto-secure; it just needs to be suitably semi-random.
		Inline UInt64 GetBaselineEntropy(void)
		{
			UInt64 x;
			__asm__ __volatile__ (".byte 0x0f, 0x31" : "=A" (x));
			return x;
		}

	#elif defined(__x86_64__)

		typedef UInt64 PtrInt;		// An unsigned integer type that is the same size as a pointer.
		typedef Int64 Int;			// A signed integer type that matches the native platform's "best" register size.
		typedef UInt64 UInt;		// An unsigned integer type that matches the native platform's "best" register size.

		#define SizeofPtrInt 8
		#define SizeofInt 8

		// Get a reasonable degree of entropy from wherever this platform keeps it, as quickly as possible.
		// This doesn't need to be crypto-secure; it just needs to be suitably semi-random.
		Inline UInt64 GetBaselineEntropy(void)
		{
			UInt32 hi, lo;
			__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
			return ((UInt64)lo) | (((UInt64)hi) << 32);
		}

	#else

		#error Unknown GCC platform; please configure "types.h" for your platform.

	#endif

#else

	//------------------------------------------------------------------------------------------------
	//  Unknown platforms.

	#error Unknown compiler; please configure "types.h" for your compiler and environment.

#endif

// Minima for the signed integer types.
#define Int8Min ((Int8)(((UInt8)1) << 7))
#define Int16Min ((Int16)(((UInt16)1) << 15))
#define Int32Min ((Int32)(((UInt32)1) << 31))
#define Int64Min ((Int64)(((UInt64)1) << 63))
#define IntMin ((Int)(((UInt)1) << (sizeof(UInt) * 8 - 1)))

// Maxima for the signed integer types.
#define Int8Max ((Int8)((((UInt8)1) << 7) - 1))
#define Int16Max ((Int16)((((UInt16)1) << 15) - 1))
#define Int32Max ((Int32)((((UInt32)1) << 31) - 1))
#define Int64Max ((Int64)((((UInt64)1) << 63) - 1))
#define IntMax ((Int)((((UInt)1) << (sizeof(UInt) * 8 - 1)) - 1))

// Maxima for the unsigned integer types.
#define UInt8Max (~(UInt8)0)
#define UInt16Max (~(UInt16)0)
#define UInt32Max (~(UInt32)0)
#define UInt64Max (~(UInt64)0)
#define UIntMax (~(UInt)0)

// Global functions for the core runtime.
SMILE_API void Smile_Init(void);
SMILE_API void Smile_End(void);
SMILE_API void Smile_Abort_OutOfMemory(void);

SMILE_API UInt32 Smile_FnvHash(const void *buffer, Int length);
SMILE_API UInt64 Smile_SipHash(const void *buffer, Int length, UInt64 secret1, UInt64 secret2);

SMILE_API UInt32 Smile_HashOracle;

#ifndef Smile_Hash
	/// <summary>
	/// Compute a 32 bit hash for a buffer.  The hash is guaranteed to always be the
	/// same value for the same sequence of bytes within the current process, and approximates
	/// a random distribution for that sequence (quickly!).<br />
	/// <br />
	/// <strong>NOTE:  NOT CRYPTOGRAPHICALLY SECURE.</strong>  If you need crypto-safe hashes, use a SHA-2 hash.
	/// </summary>
	/// <param name="buffer">Start of buffer to hash.</param>
	/// <param name="length">Length of buffer in bytes.</param>
	/// <returns>32 bit hash of the buffer.</returns>
	#define Smile_Hash(__buffer__, __length__) \
		((UInt32)Smile_SipHash((__buffer__), (__length__), Smile_HashOracle, Smile_HashOracle))
#endif

#ifndef Smile_Hash64
	/// <summary>
	/// Compute a 64 bit hash for a buffer.  The hash is guaranteed to always be the
	/// same value for the same sequence of bytes within the current process, and approximates
	/// a random distribution for that sequence (quickly!).<br />
	/// <br />
	/// <strong>NOTE:  NOT CRYPTOGRAPHICALLY SECURE.</strong>  If you need crypto-safe hashes, use a SHA-2 hash.
	/// </summary>
	/// <param name="buffer">Start of buffer to hash.</param>
	/// <param name="length">Length of buffer in bytes.</param>
	/// <returns>64 bit hash of the buffer.</returns>
	#define Smile_Hash64(__buffer__, __length__) \
		(Smile_SipHash((__buffer__), (__length__), Smile_HashOracle, Smile_HashOracle))
#endif

#endif
