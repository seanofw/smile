//-----------------------------------------------------------------------------
//
// This file is based on: https://github.com/mjosaarinen/tiny_sha3
//
// sha3.c and sha3.h
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>
//
// Revised 07-Aug-15 to match with official release of FIPS PUB 202 "SHA3"
// Revised 03-Sep-15 for portability + OpenSSL - style API
// Revised 10-Nov-18 to match Smile naming conventions
//
//-----------------------------------------------------------------------------
//
// Original license as follows:
//
// The MIT License(MIT)
// 
// Copyright(c) 2015 Markku - Juhani O.Saarinen
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//-----------------------------------------------------------------------------
//
// Implementation note:
//
// Why this SHA-3 implementation and not one of the other ones?  Because:
//   - This is simple and readable.
//   - This is small, fits in just one source file, and has no dependencies.
//   - This has a permissive MIT license that plays well with Smile's Apache license.
//   - This is Fast Enough when being invoked from a high-level language.
//
//-----------------------------------------------------------------------------

#include <smile/crypto/sha3.h>

#ifndef KECCAKF_ROUNDS
#define KECCAKF_ROUNDS 24
#endif

#ifndef ROTL64
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
#endif

// update the state with given number of rounds

void Sha3_Keccakf(UInt64 st[25])
{
	// constants
	const UInt64 keccakf_rndc[24] = {
		0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
		0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
		0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
		0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
		0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
		0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
		0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
		0x8000000000008080, 0x0000000080000001, 0x8000000080008008
	};
	const int keccakf_rotc[24] = {
		1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
		27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
	};
	const int keccakf_piln[24] = {
		10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
		15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
	};

	// variables
	int i, j, r;
	UInt64 t, bc[5];

#if SMILE_ENDIAN == SMILE_ENDIAN_BIG
	Byte *v;

	// endianess conversion. this is redundant on little-endian targets
	for (i = 0; i < 25; i++) {
		v = (Byte *)&st[i];
		st[i] = ((UInt64)v[0]) | (((UInt64)v[1]) << 8) |
			(((UInt64)v[2]) << 16) | (((UInt64)v[3]) << 24) |
			(((UInt64)v[4]) << 32) | (((UInt64)v[5]) << 40) |
			(((UInt64)v[6]) << 48) | (((UInt64)v[7]) << 56);
	}
#endif

	// actual iteration
	for (r = 0; r < KECCAKF_ROUNDS; r++) {

		// Theta
		for (i = 0; i < 5; i++)
			bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

		for (i = 0; i < 5; i++) {
			t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
			for (j = 0; j < 25; j += 5)
				st[j + i] ^= t;
		}

		// Rho Pi
		t = st[1];
		for (i = 0; i < 24; i++) {
			j = keccakf_piln[i];
			bc[0] = st[j];
			st[j] = ROTL64(t, keccakf_rotc[i]);
			t = bc[0];
		}

		//  Chi
		for (j = 0; j < 25; j += 5) {
			for (i = 0; i < 5; i++)
				bc[i] = st[j + i];
			for (i = 0; i < 5; i++)
				st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
		}

		//  Iota
		st[0] ^= keccakf_rndc[r];
	}

#if SMILE_ENDIAN == SMILE_ENDIAN_BIG
	// endianess conversion. this is redundant on little-endian targets
	for (i = 0; i < 25; i++) {
		v = (uint8_t *)&st[i];
		t = st[i];
		v[0] = t & 0xFF;
		v[1] = (t >> 8) & 0xFF;
		v[2] = (t >> 16) & 0xFF;
		v[3] = (t >> 24) & 0xFF;
		v[4] = (t >> 32) & 0xFF;
		v[5] = (t >> 40) & 0xFF;
		v[6] = (t >> 48) & 0xFF;
		v[7] = (t >> 56) & 0xFF;
	}
#endif
}

// Initialize the context for SHA3

int Sha3_Init(Sha3Context *c, int mdlen)
{
	int i;

	for (i = 0; i < 25; i++)
		c->st.q[i] = 0;
	c->mdlen = mdlen;
	c->rsiz = 200 - 2 * mdlen;
	c->pt = 0;

	return 1;
}

// update state with more data

int Sha3_Update(Sha3Context *c, const void *data, size_t len)
{
	size_t i;
	int j;

	j = c->pt;
	for (i = 0; i < len; i++) {
		c->st.b[j++] ^= ((const Byte *)data)[i];
		if (j >= c->rsiz) {
			Sha3_Keccakf(c->st.q);
			j = 0;
		}
	}
	c->pt = j;

	return 1;
}

// finalize and output a hash

int Sha3_Final(void *md, Sha3Context *c)
{
	int i;

	c->st.b[c->pt] ^= 0x06;
	c->st.b[c->rsiz - 1] ^= 0x80;
	Sha3_Keccakf(c->st.q);

	for (i = 0; i < c->mdlen; i++) {
		((Byte *)md)[i] = c->st.b[i];
	}

	return 1;
}

// compute a SHA-3 hash (md) of given byte length from "in"

void *Sha3(const void *in, size_t inlen, void *md, int mdlen)
{
	Sha3Context sha3;

	Sha3_Init(&sha3, mdlen);
	Sha3_Update(&sha3, in, inlen);
	Sha3_Final(md, &sha3);

	return md;
}

// SHAKE128 and SHAKE256 extensible-output functionality

void Shake_Xof(Sha3Context *c)
{
	c->st.b[c->pt] ^= 0x1F;
	c->st.b[c->rsiz - 1] ^= 0x80;
	Sha3_Keccakf(c->st.q);
	c->pt = 0;
}

void Shake_Out(Sha3Context *c, void *out, size_t len)
{
	size_t i;
	int j;

	j = c->pt;
	for (i = 0; i < len; i++) {
		if (j >= c->rsiz) {
			Sha3_Keccakf(c->st.q);
			j = 0;
		}
		((Byte *)out)[i] = c->st.b[j++];
	}
	c->pt = j;
}
