
#ifndef __SMILE_CRYPTO_MD5_H__
#define __SMILE_CRYPTO_MD5_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

//-----------------------------------------------------------------------------
// MD5 Implementation (NOT SECURE ANYMORE).

/*
* This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
* MD5 Message-Digest Algorithm (RFC 1321).
*
* Homepage:
* http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
*
* Author:
* Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
*
* This software was written by Alexander Peslyak in 2001.  No copyright is
* claimed, and the software is hereby placed in the public domain.
* In case this attempt to disclaim copyright and place the software in the
* public domain is deemed null and void, then the software is
* Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
* general public under the following terms:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted.
*
* There's ABSOLUTELY NO WARRANTY, express or implied.
*
* See md5.c for more information.
*/

typedef struct {
	UInt32 lo, hi;
	UInt32 a, b, c, d;
	Byte buffer[64];
	UInt32 block[16];
} Md5Context;

SMILE_API_FUNC void Md5_Init(Md5Context *context);
SMILE_API_FUNC void Md5_Update(Md5Context *context, const Byte *data, UInt32 size);
SMILE_API_FUNC void Md5_Finish(Md5Context *context, Byte *result);

SMILE_API_FUNC void Md5(Byte *result, const Byte *buffer, Int size);

#endif
