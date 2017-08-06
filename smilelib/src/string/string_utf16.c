//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2017 Sean Werkema
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

/// <summary>
/// Round the given value up to the next power of two, if the value is itself not a power of two.
/// </summary>
static Int NextPow2(Int value)
{
	UInt uvalue = (UInt)value;

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue |= uvalue >> 8;
	uvalue |= uvalue >> 16;
#	if SizeofInt > 4
		uvalue |= uvalue >> 32;
#	endif
	uvalue++;

	return (Int)uvalue;
}

/// <summary>
/// Convert the given buffer of UTF-16 words, in host-order (i.e., UTF-16BE
/// if this computer is big-endian, or UTF-16LE if this computer is little-endian)
/// to a standard UTF-8 String object.
/// </summary>
/// <param name="text">The buffer of UTF-16 words to convert to a UTF-8 string.</param>
/// <param name="length">The length of the buffer, in words.</param>
/// <returns>A String that contains UTF-8 code points that match the provided UTF-16 code points.</returns>
String String_FromUtf16(const UInt16 *text, Int length)
{
	const UInt16 *end;
	UInt16 ch;
	Int32 value;
	String result;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	if (length <= 0) return String_Empty;

	end = text + length;

	while (text < end) {
		ch = *text++;
		if (ch < 0x80) {
			// Easy case: ASCII and friends.
			StringBuilder_AppendByte(stringBuilder, (Byte)ch);
		}
		else if (ch < 0xD800 || ch >= 0xE000) {
			// Values in the Basic Multilingual Plane can be decoded using the normal Unicode decoder.
			StringBuilder_AppendUnicode(stringBuilder, ch);
		}
		else if (ch < 0xDC00) {
			// Got a high surrogate.  Is there a low surrogate after it?
			value = (ch & 0x3FF) << 10;
			if (text < end && (ch = *text) < 0xE000 && ch >= 0xDC00) {
				// Got the low surrogate, so now that we know the whole value, we can append it.
				value |= (ch & 0x3FF);
				StringBuilder_AppendUnicode(stringBuilder, value);
			}
			else {
				// No low surrogate, so this is an error.
				StringBuilder_AppendUnicode(stringBuilder, 0xFFFD);
			}
		}
		else {
			// A low surrogate without a high surrogate is an error.
			StringBuilder_AppendUnicode(stringBuilder, 0xFFFD);
		}
	}

	result = StringBuilder_ToString(stringBuilder);
	return result;
}

/// <summary>
/// Convert the given string to a buffer of UTF-16 words, in host order (i.e., UTF-16BE
/// if this computer is big-endian, or UTF-16LE if this computer is little-endian).
/// </summary>
/// <param name="str">The string to convert from UTF-8 to UTF-16.</param>
/// <param name="lengthOutput">If this is non-NULL, it will be set to the resulting number of UTF-16 words.</param>
/// <returns>A buffer containing the converted string.  This will be a nul-terminated ('\0'-terminated) buffer,
/// so it is compatible with various preexisting C functions that expect a 16-bit wchar_t*.</returns>
UInt16 *String_ToUtf16(const String str, Int *lengthOutput)
{
	UInt16 *result, *dest;
	const Byte *src, *end;
	Byte ch;
	Int length;
	Int resultLength, resultMax;
	UInt16 highpart, lowpart;

	// Empty is as empty does.
	if (String_IsNullOrEmpty(str)) {
		result = (UInt16 *)GC_MALLOC_ATOMIC(1 * sizeof(UInt16));
		if (result == NULL)
			Smile_Abort_OutOfMemory();
		*result = 0;
		if (lengthOutput != NULL)
			*lengthOutput = 0;
		return result;
	}

	// Get the actual start and end of the string.
	length = String_Length(str);
	src = (Byte *)String_GetBytes(str);
	end = src + length;

	// Allocate enough space for the most likely result:  One char in equals one wide char out.
	resultMax = NextPow2(length);
	if (resultMax < 16) resultMax = 16;
	result = (UInt16 *)GC_MALLOC_ATOMIC(resultMax * sizeof(UInt16));
	if (result == NULL)
		Smile_Abort_OutOfMemory();
	resultLength = 0;
	dest = result;

#define GROW_IF_NEEDED \
	if (resultLength >= resultMax) { \
		Int newMax = resultMax * 2; \
		UInt16 *newResult = (UInt16 *)GC_MALLOC_ATOMIC(newMax * sizeof(UInt16)); \
		if (newResult == NULL) \
			Smile_Abort_OutOfMemory(); \
		MemCpy(newResult, result, resultLength * sizeof(UInt16)); \
		result = newResult; \
		resultMax = newMax; \
	}

	// Read the source as Unicode, decoding each code point from UTF-8, and write UTF-16 code
	// points to the output.
	while (src < end) {
		ch = *src;
		if (ch < 0x80) {
			// Easy case: ASCII in, ASCII out.
			GROW_IF_NEEDED
			*dest++ = (UInt16)ch;
			resultLength++;
			src++;
		}
		else {
			// Harder case: A UTF-8 value above ASCII.  So decode it from the input...
			Int32 value = String_ExtractUnicodeCharacterInternal(&src, end);

			// If the code point is in [U+0000, U+D7FF] or [U+E000, U+FFFF], we can store
			// it as a single 16-bit word in the output.  Characters in the range of
			// [U+D800, U+DFFF] are disallowed, and result in U+FFFD error codes coming out.
			// Everything higher than U+10000 (inclusive) is stored as two 16-bit words.
			if (value < 0xD800) {
				// Easy case:  The low part of the Basic Multilingual Plane (BMP).
				GROW_IF_NEEDED
				*dest++ = (UInt16)value;
				resultLength++;
			}
			else if (value < 0xE000) {
				// Disallowed character results in 0xFFFD (replacement/unknown character error).
				GROW_IF_NEEDED
				*dest++ = (UInt16)0xFFFD;
				resultLength++;
			}
			else if (value < 0x10000) {
				// The upper part of the BMP.
				GROW_IF_NEEDED;
				*dest++ = (UInt16)value;
				resultLength++;
			}
			else if (value < 0x110000) {
				// A high value that needs to have two 16-bit words as output.
				value -= 0x10000;
				highpart = (value >> 10) & 0x3FF;
				lowpart = value & 0x3FF;
				GROW_IF_NEEDED
				*dest++ = highpart | 0xD800;
				GROW_IF_NEEDED
				*dest++ = lowpart | 0xDC00;
				resultLength++;
			}
			else {
				// An invalid non-Unicode value that shouldn't exist.
				GROW_IF_NEEDED
				*dest++ = (UInt16)0xFFFD;
				resultLength++;
			}
		}
	}

	// Append a nul '\0' word, just to be compatible with APIs that expect it.
	GROW_IF_NEEDED
	*dest++ = 0;

	// Return the length, if they care.
	if (lengthOutput != NULL)
		*lengthOutput = resultLength;

	return result;
}
