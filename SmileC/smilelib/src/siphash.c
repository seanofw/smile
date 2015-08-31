/*
SipHash C implementation

Copyright (c) 2012-2014 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>

To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

Tweaked slightly for the needs of Smile in 2015 by Sean Werkema.
*/
#include <smile/types.h>

/* default: SipHash-2-4 */
#define cROUNDS 2
#define dROUNDS 4

#define ROTL(x,b) (UInt64)( ((x) << (b)) | ( (x) >> (64 - (b))) )

#define U32TO8_LE(p, v)                                         \
  (p)[0] = (Byte)((v)      ); (p)[1] = (Byte)((v) >>  8); \
  (p)[2] = (Byte)((v) >> 16); (p)[3] = (Byte)((v) >> 24);

#define U64TO8_LE(p, v)                        \
  U32TO8_LE((p),     (UInt32)((v)      ));   \
  U32TO8_LE((p) + 4, (UInt32)((v) >> 32));

#define U8TO64_LE(p)            \
  (((UInt64)((p)[0])      ) | \
   ((UInt64)((p)[1]) <<  8) | \
   ((UInt64)((p)[2]) << 16) | \
   ((UInt64)((p)[3]) << 24) | \
   ((UInt64)((p)[4]) << 32) | \
   ((UInt64)((p)[5]) << 40) | \
   ((UInt64)((p)[6]) << 48) | \
   ((UInt64)((p)[7]) << 56))

#define SIPROUND                                            \
    {                                                       \
		v0 += v1; v1=ROTL(v1,13); v1 ^= v0; v0=ROTL(v0,32); \
		v2 += v3; v3=ROTL(v3,16); v3 ^= v2;                 \
		v0 += v3; v3=ROTL(v3,21); v3 ^= v0;                 \
		v2 += v1; v1=ROTL(v1,17); v1 ^= v2; v2=ROTL(v2,32); \
    }

UInt64 Smile_SipHash(const Byte *in, Int inlen, UInt64 secret1, UInt64 secret2)
{
	/* "somepseudorandomlygeneratedbytes" */
	UInt64 v0 = 0x736f6d6570736575ULL;
	UInt64 v1 = 0x646f72616e646f6dULL;
	UInt64 v2 = 0x6c7967656e657261ULL;
	UInt64 v3 = 0x7465646279746573ULL;
	UInt64 b;
	UInt64 m;
	Int i;
	const Byte *end = in + inlen - (inlen % sizeof(UInt64));
	const Int left = inlen & 7;

	b = ((UInt64)inlen) << 56;

	v3 ^= secret2;
	v2 ^= secret1;
	v1 ^= secret2;
	v0 ^= secret1;

	for (; in != end; in += 8) {
		m = U8TO64_LE(in);
		v3 ^= m;

		for (i = 0; i < cROUNDS; ++i) SIPROUND

		v0 ^= m;
	}

	switch (left) {
		case 7: b |= ((UInt64)in[6]) << 48;
		case 6: b |= ((UInt64)in[5]) << 40;
		case 5: b |= ((UInt64)in[4]) << 32;
		case 4: b |= ((UInt64)in[3]) << 24;
		case 3: b |= ((UInt64)in[2]) << 16;
		case 2: b |= ((UInt64)in[1]) << 8;
		case 1: b |= ((UInt64)in[0]);
		case 0: break;
	}

	v3 ^= b;

	for (i = 0; i < cROUNDS; ++i) SIPROUND

	v0 ^= b;
	v2 ^= 0xff;

	for (i = 0; i < dROUNDS; ++i) SIPROUND

	b = v0 ^ v1 ^ v2 ^ v3;

	return b;
}
