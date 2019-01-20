
#include <smile/numeric/random.h>
#include <smile/crypto/sha2.h>
#include <smile/atomic.h>
#include <smile/platform/platform.h>

/// <summary>
/// Initialize the given random-number generator with entropy provided by the OS.
/// This can be "slow," so you shouldn't do it too often.
/// </summary>
/// <param name="random">The random-number generator to seed.</param>
void Random_Init(Random random)
{
	Byte hashResult[32];

	struct StateStruct {
		UInt64 entropy;			// Entropy from the CPU counter.
		Float64 now;			// Current date-and-time, as a Unix-style seconds-since-1970 timestamp.
		UInt32 pid;				// Process ID of the current process.
		PtrInt aslrPtr;			// A pointer to this code, which hopefully is positioned via OS ASLR.
		Byte osRandom[64];		// 64 bytes of random data from the OS, if the OS has a native randomness source
	} state;

	// Ask the OS for the current baseline entropy value (CPU clock ticks).
	state.entropy = GetBaselineEntropy();

	// Ask the OS for the current date-and-time.
	state.now = Os_GetDateTime();

	// Ask the OS for the process ID of this process.
	state.pid = Os_GetProcessId();

	// Get a pointer to something in this process, which is hopefully somewhat-randomized by OS ASLR.
	state.aslrPtr = (PtrInt)Random_Init;

	// Ask the OS for some good entropy.
	Os_GetRandomData(state.osRandom, sizeof(state.osRandom));

	// Use a secure hash to transform that random state into unpredictable bits.
	Sha256(hashResult, (Byte *)&state, sizeof(struct StateStruct));

	// Use the first 64 of those bits to initialize the RNG.
	Random_InitWithSeed(random,
		( ((Int64)hashResult[0]      )
		| ((Int64)hashResult[1] <<  8)
		| ((Int64)hashResult[2] << 16)
		| ((Int64)hashResult[3] << 24)
		| ((Int64)hashResult[4] << 32)
		| ((Int64)hashResult[5] << 40)
		| ((Int64)hashResult[6] << 48)
		| ((Int64)hashResult[7] << 56) ));
}

#define LCG_MULTIPLIER	6364136223846793005ULL
#define LCG_INCREMENT	1442695040888963407ULL

/// <summary>
/// Initialize the given random-number generator with the given seed value.  This uses Melissa O'Neill's
/// excellent PCG 32-bit random-number generator, which is simple, fast, and proven, with a period of 2^64.
/// </summary>
/// <param name="random">The random-number generator to seed.</param>
/// <param name="seed">A seed value that will be used to initialize that random-number generator's state.</param>
void Random_InitWithSeed(Random random, UInt64 seed)
{
	random->state = 0U;
	random->state = random->state * LCG_MULTIPLIER + LCG_INCREMENT;
	random->state += seed;
	random->state = random->state * LCG_MULTIPLIER + LCG_INCREMENT;
}

/// <summary>
/// Retrieve a 32-bit uniformly-distributed random number from the given random-number generator.
/// This uses Melissa O'Neill's excellent PCG 32-bit random-number generator, which is simple, fast,
/// and proven.  This has a period of 2^64.  This function is thread-safe (but it does not attempt to
/// provide thread fairness guarantees or avoid thread starvation).  This sacrifices a little
/// performance for thread safety, but is overall still very fast.
/// </summary>
/// <param name="random">The random-number generator providing the random values.</param>
/// <returns>A 32-bit uniformly-distributed random number.</returns>
UInt32 Random_UInt32(Random random)
{
	UInt64 state, nextState;
retry:
	state = random->state;
	nextState = state * LCG_MULTIPLIER + LCG_INCREMENT;
	if (!Atomic_CompareAndSwapInt64((UInt64 *)&random->state, (UInt64)state, (UInt64)nextState))
		goto retry;
	return Smile_RotateRight32(((state >> 18) ^ state) >> 27, state >> 59);
}

/// <summary>
/// Retrieve a 64-bit uniformly-distributed random number from the given random-number generator
/// in the range of 0 to max, inclusive.  This works hard to ensure the result has a uniform
/// distribution, so it is slower than a simple modulus, but more accurate.
///
/// This uses the bitmask-with-rejection method, which is Apple's preferred method, and is
/// among the fastest overall solutions.  For more details, see Melissa O'Neill's blog posting on
/// bounded random numbers:  http://www.pcg-random.org/posts/bounded-rands.html
/// </summary>
/// <param name="random">The random-number generator providing the random values.</param>
/// <returns>A 64-bit uniformly-distributed random number in the range of 0 to max, inclusive.</returns>
UInt64 Random_ZeroToUInt64(Random random, UInt64 max)
{
	UInt64 x;
	UInt64 mask = UInt64Max;

#	if SMILE_COMPILER == SMILE_COMPILER_GCC || SMILE_COMPILER == SMILE_COMPILER_CLANG
		mask >>= __builtin_clzll(max | 1);
#	else
		UInt64 temp = max;
		for (temp |= 1; temp; temp >>= 1)
			mask >>= 1;
#	endif

	do {
		x = Random_UInt64(random) & mask;
	} while (x > max);

	return x;
}

/// <summary>
/// Retrieve a 32-bit uniformly-distributed random number from the given random-number generator
/// in the range of 0 to max, inclusive.  This works hard to ensure the result has a uniform
/// distribution, so it is slower than a simple modulus, but more accurate.
///
/// This uses the bitmask-with-rejection method, which is Apple's preferred method, and is
/// among the fastest overall solutions.  For more details, see Melissa O'Neill's blog posting on
/// bounded random numbers:  http://www.pcg-random.org/posts/bounded-rands.html
/// </summary>
/// <param name="random">The random-number generator providing the random values.</param>
/// <returns>A 32-bit uniformly-distributed random number in the range of 0 to max, inclusive.</returns>
UInt32 Random_ZeroToUInt32(Random random, UInt32 max)
{
	UInt32 x;
	UInt32 mask = UInt32Max;

#	if SMILE_COMPILER == SMILE_COMPILER_GCC || SMILE_COMPILER == SMILE_COMPILER_CLANG
		mask >>= __builtin_clz(max | 1);
#	else
		UInt32 temp = max;
		for (temp |= 1; temp; temp >>= 1)
			mask >>= 1;
#	endif

	do {
		x = Random_UInt32(random) & mask;
	} while (x > max);

	return x;
}