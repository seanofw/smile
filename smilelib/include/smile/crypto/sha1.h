
#ifndef __SMILE_CRYPTO_SHA1_H__
#define __SMILE_CRYPTO_SHA1_H__

#include <smile/types.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LibSha1
//
//  Implementation of SHA1 hash function.
//  Original author:  Steve Reid <sreid@sea-to-sky.net>
//  Contributions by: James H. Brown <jbrown@burgoyne.com>, Saul Kravitz <Saul.Kravitz@celera.com>,
//  and Ralph Giles <giles@ghostscript.com>
//  Modified by WaterJuice retaining Public Domain license.
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SHA1_HASH_SIZE           ( 160 / 8 )

typedef struct {
	UInt32        State[5];
	UInt32        Count[2];
	Byte          Buffer[64];
} Sha1Context;

SMILE_API_FUNC void Sha1_Init(Sha1Context *context);
SMILE_API_FUNC void Sha1_Update(Sha1Context *context, const Byte *buffer, UInt32 length);
SMILE_API_FUNC void Sha1_Finish(Sha1Context *context, Byte *sha1);

SMILE_API_FUNC void Sha1(Byte *result, const Byte *buffer, Int size);

#endif
