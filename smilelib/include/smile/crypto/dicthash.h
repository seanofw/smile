
#ifndef __SMILE_CRYPTO_DICTHASH_H__
#define __SMILE_CRYPTO_DICTHASH_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

//-----------------------------------------------------------------------------
// Smile dictionary hashing (NOT SECURE).

#define SMILE_HASHTABLE_BITS (14)
#define SMILE_HASHTABLE_SIZE (1 << SMILE_HASHTABLE_BITS)

SMILE_API_DATA UInt32 Smile_HashOracle;
SMILE_API_DATA UInt32 Smile_HashTable[SMILE_HASHTABLE_SIZE];

SMILE_API_FUNC void Smile_InitHashTable(UInt32 hashBasis);

SMILE_API_FUNC UInt32 FnvHash(const Byte *buffer, Int length);
SMILE_API_FUNC UInt64 SipHash(const Byte *buffer, Int length, UInt64 secret1, UInt64 secret2);

/// <summary>
/// "Randomize" the input value x in a predictable way, based on the current hash oracle,
/// as quickly as possible.  This algorithm is designed to be more fast than secure, but it's
/// still pretty good for our needs, like making dictionary bucket indexes from pointers
/// and symbol IDs.  Note that this ignores/discards the top 8 bits of the 64-bit value when
/// calculating the hash; 
/// </summary>
Inline UInt32 Smile_ApplyHashOracle(UInt64 x)
{
	x += x >> 28;		// Fold high 28 bits of 56-bit value down to low 28 bits.
	x += x >> 14;		// Fold high 14 bits of 28-bit value down to low 14 bits.
	return Smile_HashTable[x & 0x3FFF];
}

/// <summary>
/// Compute a 32 bit hash for a buffer.  The hash is guaranteed to always be the
/// same value for the same sequence of bytes within the current process, and approximates
/// a random distribution for that sequence (quickly!).<br />
/// <br />
/// <strong>NOTE:  NOT CRYPTOGRAPHICALLY SECURE.</strong>  If you need crypto-safe hashes, use a SHA-2 or SHA-3 hash.
/// </summary>
/// <param name="buffer">Start of buffer to hash.</param>
/// <param name="length">Length of buffer in bytes.</param>
/// <returns>32 bit hash of the buffer.</returns>
Inline UInt32 Smile_Hash(const Byte *buffer, Int length)
{
	return (UInt32)SipHash(buffer, length, Smile_HashOracle, Smile_HashOracle);
}

/// <summary>
/// Compute a 64 bit hash for a buffer.  The hash is guaranteed to always be the
/// same value for the same sequence of bytes within the current process, and approximates
/// a random distribution for that sequence (quickly!).<br />
/// <br />
/// <strong>NOTE:  NOT CRYPTOGRAPHICALLY SECURE.</strong>  If you need crypto-safe hashes, use a SHA-2 or SHA-3 hash.
/// </summary>
/// <param name="buffer">Start of buffer to hash.</param>
/// <param name="length">Length of buffer in bytes.</param>
/// <returns>64 bit hash of the buffer.</returns>
Inline UInt64 Smile_Hash64(const Byte *buffer, Int length)
{
	return SipHash(buffer, length, Smile_HashOracle, Smile_HashOracle);
}

#endif
