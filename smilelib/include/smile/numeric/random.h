#ifndef __SMILE_NUMERIC_RANDOM_H__
#define __SMILE_NUMERIC_RANDOM_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_NUMERIC_REAL32_H__
#include <smile/numeric/real32.h>
#endif
#ifndef __SMILE_NUMERIC_REAL64_H__
#include <smile/numeric/real64.h>
#endif

typedef struct RandomStruct {
	UInt64 state;
} *Random;

SMILE_API_DATA Random Random_Shared;

SMILE_API_FUNC void Random_Init(Random random);
SMILE_API_FUNC void Random_InitWithSeed(Random random, UInt64 seed);
SMILE_API_FUNC UInt32 Random_UInt32(Random random);
SMILE_API_FUNC UInt32 Random_ZeroToUInt32(Random random, UInt32 max);
SMILE_API_FUNC UInt64 Random_ZeroToUInt64(Random random, UInt64 max);
SMILE_API_FUNC Float32 Random_Float32(Random random);
SMILE_API_FUNC Float64 Random_Float64(Random random);

/// <summary>
/// Retrieve a 64-bit uniformly-distributed random number from the given random-number generator.
/// </summary>
/// <param name="random">The random-number generator providing the random values.</param>
/// <returns>A 64-bit uniformly-distributed random number.</returns>
Inline UInt64 Random_UInt64(Random random)
{
	UInt32 low = Random_UInt32(random);
	UInt32 hi = Random_UInt32(random);
	return ((UInt64)hi << 32) | low;
}

/// <summary>
/// Retrieve a 64-bit uniformly-distributed random number from the given random-number generator
/// in the range of min to max, inclusive.  This works hard to ensure the result has a uniform
/// distribution, so it is slower than a simple modulus, but more accurate.
/// </summary>
/// <param name="random">The random-number generator providing the random values.</param>
/// <returns>A 64-bit uniformly-distributed random number in the range of min to max, inclusive.</returns>
Inline Int64 Random_IntRange64(Random random, Int64 min, Int64 max)
{
	if (max < min)
		return 0;
	else if (min == Int64Min && max == Int64Max)
		return (Int64)Random_UInt64(random);
	else
		return (Int64)Random_ZeroToUInt64(random, (UInt64)(max - min + 1)) - min;
}

/// <summary>
/// Retrieve a 32-bit uniformly-distributed random number from the given random-number generator
/// in the range of min to max, inclusive.  This works hard to ensure the result has a uniform
/// distribution, so it is slower than a simple modulus, but more accurate.
/// </summary>
/// <param name="random">The random-number generator providing the random values.</param>
/// <returns>A 32-bit uniformly-distributed random number in the range of min to max, inclusive.</returns>
Inline Int32 Random_IntRange32(Random random, Int32 min, Int32 max)
{
	if (max < min)
		return 0;
	else if (min == Int32Min && max == Int32Max)
		return (Int32)Random_UInt64(random);
	else
		return (Int32)Random_ZeroToUInt32(random, (UInt32)(max - min + 1)) - min;
}

/// <summary>
/// Retrieve a 32-bit uniformly-distributed random number from the given random-number generator
/// in the range of [0, max).  This works hard to ensure the result has a uniform
/// distribution, so it is slower than a simple modulus, but more accurate.
/// </summary>
/// <param name="random">The random-number generator providing the random values.</param>
/// <returns>A 32-bit uniformly-distributed random number in the range of 0 to max, exclusive of max.</returns>
Inline Int64 Random_ZeroToInt64(Random random, Int64 max)
{
	return max > 0 ? (Int64)Random_ZeroToUInt64(random, (UInt64)max) : 0;
}

/// <summary>
/// Retrieve a 64-bit uniformly-distributed random number from the given random-number generator
/// in the range of [0, max).  This works hard to ensure the result has a uniform
/// distribution, so it is slower than a simple modulus, but more accurate.
/// </summary>
/// <param name="random">The random-number generator providing the random values.</param>
/// <returns>A 64-bit uniformly-distributed random number in the range of 0 to max, exclusive of max.</returns>
Inline Int32 Random_ZeroToInt32(Random random, Int32 max)
{
	return max > 0 ? (Int32)Random_ZeroToUInt32(random, (UInt32)max) : 0;
}

/// <summary>
/// Generate a random Real32 value in [0, 1).  This is a cheesy implementation, but it works.
/// </summary>
Inline Real32 Random_Real32(Random random)
{
	return Real32_FromFloat32(Random_Float32(random));
}

/// <summary>
/// Generate a random Real64 value in [0, 1).  This is a cheesy implementation, but it works.
/// </summary>
Inline Real64 Random_Real64(Random random)
{
	return Real64_FromFloat64(Random_Float64(random));
}

#endif