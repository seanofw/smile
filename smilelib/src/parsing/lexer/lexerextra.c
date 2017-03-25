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

#include <smile/types.h>
#include <smile/string.h>

/// <summary>
/// Compute the hex character value of the given character.
/// </summary>
/// <param name="ch">The character to look up.</param>
/// <returns>0-9 for 0-9, 10-15 for a-f and A-F, and -1 for everything else.</returns>
Inline Int HexValue(Byte ch)
{
	switch (ch) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
		default: return -1;
	}
}

/// <summary>
/// After having read a \ character in the input, decode the escape code following it.
/// We recognize the following special C-style forms:
/// 
///    \a				- 7
///    \b				- 8
///    \t				- 9
///    \n				- 10
///    \v				- 11
///    \f				- 12
///    \r				- 13
///    \e				- 27
///    \x** or \X**		- Hex-encoded value, exactly one byte (one or two hex chars).
///    \u** or \U**		- Unicode value, multiple hex bytes, variable length (mod 2^32).  If followed by a ';' the ';' will also be consumed.
///    \###				- Decimal-encoded value (and NOT octal-encoded), exactly one byte (at most three digits).
///    \\ or \' or \" or \...	- Special escape codes
///
/// Among the "special" escape codes, you may potentially escape any character, if 'allowUnknowns'
/// is enabled; however, under no circumstances may you escape control codes in the range of 0-31,
/// and a backslash before a high-valued character escapes the full UTF-8 code point (potentially
/// up to four bytes).
/// </summary>
/// <param name="input">A pointer to a pointer to the current source bytes.</param>
/// <param name="end">A pointer to the end of the input (one past the last valid character).</param>
/// <param name="allowUnknowns">Whether to allow unknown escape characters as legitimate input that
/// yield themselves (True), or whether to reject unknown escape codes as garbage (False).</param>
/// <returns>Returns a decoded character.</returns>
SMILE_API_FUNC Int Lexer_DecodeEscapeCode(const Byte **input, const Byte *end, Bool allowUnknowns)
{
	const Byte *src = *input;
	Byte ch;
	Int b1, b2;
	UInt value;

	// If there's no input, give up.
	if (src >= end)
		return -1;

	switch (ch = *src++) {

		// Handle the common named escapes:  a, b, t, n, v, f, r, and e.
		case 'a':
			*input = src;
			return '\x07';
		case 'b':
			*input = src;
			return '\x08';
		case 't':
			*input = src;
			return '\x09';
		case 'n':
			*input = src;
			return '\x0A';
		case 'v':
			*input = src;
			return '\x0B';
		case 'f':
			*input = src;
			return '\x0C';
		case 'r':
			*input = src;
			return '\x0D';
		case 'e':
			*input = src;
			return '\x1E';

		// Handle the special escapes:  ', ", and \, among others.
		case '\'': case '\"': case '\\':
			*input = src;
			return ch;

		// Handle hex forms, which start with \x or \X followed by one or two hexadecimal characters.
		case 'x': case 'X':
		{
			// At least one hex digit is required.
			if (src >= end || (b1 = HexValue(*src)) == -1) {
				*input = src;
				return -1;
			}
			src++;

			// One more digit is optional.
			if (src < end && (b2 = HexValue(*src)) >= 0) {
				src++;
				value = (UInt)((b1 << 4) | b2);
			}
			else {
				value = (UInt)b1;
			}

			*input = src;
			return value;
		}

		// Handle Unicode forms, which start with \u or \U followed by one or more hexadecimal
		// characters.  If the hex characters are followed by ';', it will also be consumed, allowing
		// the character to be safely separated from any succeeding characters.  Any value larger
		// than 2^32 will be treated as mod 2^32.  Any resulting value larger than 0x110000 (the largest
		// representable Unicode character) will be returned as -1.
		case 'u': case 'U':
		{
			// At least one hex digit is required.
			if (src >= end || (b1 = HexValue(*src)) == -1) {
				*input = src;
				return -1;
			}
			src++;

			// Consume hex digits until we run out of them.
			value = b1;
			while (src < end && (b2 = HexValue(*src)) >= 0) {
				src++;
				value <<= 4;
				value |= b2;
			}

			// If there is a trailing ';', consume that too.
			if (src < end && *src == ';') {
				src++;
			}

			*input = src;

			// Obey mod-2^32 semantics, as per the formal defintion.
			value &= 0xFFFFFFFF;

			// Disallow anything above 0x110000, as per the formal definition.
			return value < 0x110000 ? (Int)(UInt)value : (Int)-1;
		}

		// Handle decimal forms, which start with any decimal digit followed by at most two more decimal digits.
		// Values greater than 255 will be discarded (returned as -1).
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		{
			UInt value;

			// Consume the first digit.
			value = ch - '0';

			// Consume an optional second digit.
			if (src < end && (ch = *src) >= '0' && ch <= '9') {
				src++;
				value *= 10;
				value += (ch - '0');
			}

			// Consume an optional third digit.
			if (src < end && (ch = *src) >= '0' && ch <= '9') {
				src++;
				value *= 10;
				value += (ch - '0');
			}

			*input = src;
			return value <= 255 ? (Int)value : (Int)-1;
		}

		// Everything else is (maybe) illegal, depending on the caller's opinion.
		default:
			if (!allowUnknowns || ch < 32) return -1;

			if (ch < 128) {
				// Escaped character in the ASCII range.
				*input = src;
				return ch;
			}
			else {
				// Escaped character in the Unicode higher-than-ASCII range.
				value = String_ExtractUnicodeCharacterInternal(&src, end);
				*input = src;
				return ch;
			}
	}
}
