
#ifndef __SMILE_CRYPTO_SHA3_H__
#define __SMILE_CRYPTO_SHA3_H__

// See sha3.c for license and other info.

#include <smile/types.h>

// state context
typedef struct {
	union {                                 // state:
		Byte b[200];                        // 8-bit bytes
		UInt64 q[25];                       // 64-bit words
	} st;
	int pt, rsiz, mdlen;                    // these don't overflow
} Sha3Context;

// Compression function.
SMILE_API_FUNC void Sha3_Keccakf(UInt64 st[25]);

// OpenSSL-like interface.
SMILE_API_FUNC int Sha3_Init(Sha3Context *c, int mdlen);    // mdlen = hash output in bytes
SMILE_API_FUNC int Sha3_Update(Sha3Context *c, const void *data, size_t len);
SMILE_API_FUNC int Sha3_Final(void *md, Sha3Context *c);    // digest goes to md

// compute a sha3 hash (md) of given byte length from "in"
SMILE_API_FUNC void *Sha3(const void *in, size_t inlen, void *md, int mdlen);

Inline void Sha3_224(Byte *result, const Byte *buffer, Int size) { Sha3(buffer, (size_t)size, result, 224 / 8); }
Inline void Sha3_256(Byte *result, const Byte *buffer, Int size) { Sha3(buffer, (size_t)size, result, 256 / 8); }
Inline void Sha3_384(Byte *result, const Byte *buffer, Int size) { Sha3(buffer, (size_t)size, result, 384 / 8); }
Inline void Sha3_512(Byte *result, const Byte *buffer, Int size) { Sha3(buffer, (size_t)size, result, 512 / 8); }

// SHAKE128 and SHAKE256 extensible-output functions
#define Shake128_Init(c) Sha3_Init(c, 16)
#define Shake256_Init(c) Sha3_Init(c, 32)
#define Shake_Update Sha3_Update

SMILE_API_FUNC void Shake_Xof(Sha3Context *c);
SMILE_API_FUNC void Shake_Out(Sha3Context *c, void *out, size_t len);

#endif
