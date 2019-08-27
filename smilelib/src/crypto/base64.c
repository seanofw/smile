//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <smile/gc.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/crypto/base64.h>

/*
 * This code is derived from:
 *
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * It's basically Jouni's code, but with some slight revisions to allow the
 * return types to match what Smile prefers, and to use GC buffers instead of malloc.
 */

static const unsigned char base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/// <summary>Base-64 encode the given buffer of data.</summary>
/// <param name="src">Data to be encoded.</param>
/// <param name="len">Length of the data to be encoded.</param>
/// <returns>Allocated String of encoded data.</returns>
String Base64Encode(const Byte *src, Int len, Bool wrapLines)
{
	String result;
	Byte *out, *pos;
	const Byte *end, *in;
	Int olen;
	Int line_len;

	olen = len * 4 / 3 + 4;		// 3-byte blocks to 4-byte
	if (wrapLines)
		olen += olen / 72;			// line feeds
	olen++;						// nul termination
	if (olen < len)
		Smile_Abort_OutOfMemory();	// integer overflow
	result = String_CreateInternal(olen);
	out = (Byte *)String_GetBytes(result);
	if (out == NULL)
		return NULL;

	end = src + len;
	in = src;
	pos = out;
	line_len = 0;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
		line_len += 4;
		if (wrapLines && line_len >= 72) {
			*pos++ = '\n';
			line_len = 0;
		}
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		}
		else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
				(in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
		line_len += 4;
	}

	if (wrapLines && line_len)
		*pos++ = '\n';

	*pos = '\0';

	// We likely over-allocated, so set the string's length to the correct count.
	result->_opaque.length = (Int)(pos - out);

	return result;
}

/// <summary>Base-64 decode the given string of encoded data.</summary>
/// <param name="text">Data to be decoded.</param>
/// <param name="out_len">This will be set to the number of bytes decoded.</param>
/// <returns>Allocated buffer of decoded data, or NULL if the input is invalid.</returns>
Byte *Base64Decode(String text, Int *out_len)
{
	Byte dtable[256], *out, *pos, block[4], tmp;
	Int i, count, olen;
	Int pad = 0;

	const Byte *src = String_GetBytes(text);
	Int len = String_Length(text);

	MemSet(dtable, 0x80, 256);
	for (i = 0; i < sizeof(base64_table) - 1; i++)
		dtable[base64_table[i]] = (Byte)i;
	dtable['='] = 0;

	count = 0;
	for (i = 0; i < len; i++) {
		if (dtable[src[i]] != 0x80)
			count++;
	}

	if (count == 0 || count % 4)
		return NULL;

	olen = count / 4 * 3;
	pos = out = GC_MALLOC_ATOMIC(olen);
	if (out == NULL)
		Smile_Abort_OutOfMemory();

	count = 0;
	for (i = 0; i < len; i++) {
		tmp = dtable[src[i]];
		if (tmp == 0x80)
			continue;

		if (src[i] == '=')
			pad++;
		block[count] = tmp;
		count++;
		if (count == 4) {
			*pos++ = (block[0] << 2) | (block[1] >> 4);
			*pos++ = (block[1] << 4) | (block[2] >> 2);
			*pos++ = (block[2] << 6) | block[3];
			count = 0;
			if (pad) {
				if (pad == 1)
					pos--;
				else if (pad == 2)
					pos -= 2;
				else {
					// Invalid padding.
					return NULL;
				}
				break;
			}
		}
	}

	*out_len = (Int)(pos - out);
	return out;
}
