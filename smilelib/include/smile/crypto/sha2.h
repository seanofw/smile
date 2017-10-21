
#ifndef __SMILE_CRYPTO_SHA2_H__
#define __SMILE_CRYPTO_SHA2_H__

#include <smile/types.h>

/*
* FILE:	sha2.h
* AUTHOR:	Aaron D. Gifford - http://www.aarongifford.com/
*
* Copyright (c) 2000-2001, Aaron D. Gifford
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
* $Id: sha2.h,v 1.1 2001/11/08 00:02:01 adg Exp adg $
*/

/*** SHA-256/384/512 Various Length Definitions ***********************/
#define SHA256_BLOCK_LENGTH		64
#define SHA256_DIGEST_LENGTH		32
#define SHA256_DIGEST_STRING_LENGTH	(SHA256_DIGEST_LENGTH * 2 + 1)
#define SHA384_BLOCK_LENGTH		128
#define SHA384_DIGEST_LENGTH		48
#define SHA384_DIGEST_STRING_LENGTH	(SHA384_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH		128
#define SHA512_DIGEST_LENGTH		64
#define SHA512_DIGEST_STRING_LENGTH	(SHA512_DIGEST_LENGTH * 2 + 1)

//-----------------------------------------------------------------------------
// SHA-256

typedef struct {
	UInt32 state[8];
	UInt64 bitcount;
	Byte buffer[SHA256_BLOCK_LENGTH];
} Sha256Context;

SMILE_API_FUNC void Sha256_Init(Sha256Context *context);
SMILE_API_FUNC void Sha256_Update(Sha256Context *context, const Byte *buffer, UInt32 length);
SMILE_API_FUNC void Sha256_Finish(Sha256Context *context, Byte *sha256);

SMILE_API_FUNC void Sha256(Byte *result, const Byte *buffer, Int size);

//-----------------------------------------------------------------------------
// SHA-512

typedef struct {
	UInt64 state[8];
	UInt64 bitcount[2];
	Byte buffer[SHA512_BLOCK_LENGTH];
} Sha512Context;

SMILE_API_FUNC void Sha512_Init(Sha512Context *context);
SMILE_API_FUNC void Sha512_Update(Sha512Context *context, const Byte *buffer, UInt32 length);
SMILE_API_FUNC void Sha512_Finish(Sha512Context *context, Byte *sha512);

SMILE_API_FUNC void Sha512(Byte *result, const Byte *buffer, Int size);

//-----------------------------------------------------------------------------
// SHA-384

typedef Sha512Context Sha384Context;

SMILE_API_FUNC void Sha384_Init(Sha384Context *context);
SMILE_API_FUNC void Sha384_Update(Sha384Context *context, const Byte *buffer, UInt32 length);
SMILE_API_FUNC void Sha384_Finish(Sha384Context *context, Byte *sha512);

SMILE_API_FUNC void Sha384(Byte *result, const Byte *buffer, Int size);

#endif
