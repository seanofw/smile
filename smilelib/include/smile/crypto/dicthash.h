
#ifndef __SMILE_CRYPTO_DICTHASH_H__
#define __SMILE_CRYPTO_DICTHASH_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

//-----------------------------------------------------------------------------
// Smile dictionary hashing (NOT SECURE).

SMILE_API_DATA UInt32 Smile_HashOracle;

SMILE_API_FUNC UInt32 FnvHash(const Byte *buffer, Int length);
SMILE_API_FUNC UInt64 SipHash(const Byte *buffer, Int length, UInt64 secret1, UInt64 secret2);

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
