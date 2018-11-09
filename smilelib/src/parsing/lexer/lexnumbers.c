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

#include <stdlib.h>		// For strtod().

// TODO: Ideally shouldn't rely on the (possibly-buggy) native implementation of
// strtod() and instead should use our own infinite-precision conversion routine
// to ensure consistency across platforms.  strtod() is fine on newer versions of
// GCC/Linux, but has at least a one-ULP inaccuracy on just about every other
// compiler/OS (and is notably inconsistent between different versions of VC++).

#include <smile/numeric/real.h>
#include <smile/parsing/lexer.h>
#include <smile/parsing/tokenkind.h>
#include <smile/parsing/identkind.h>
#include <smile/stringbuilder.h>

#include <smile/internal/staticstring.h>
#include <smile/parsing/internal/lexerinternal.h>

//---------------------------------------------------------------------------
//  Number parsing.

STATIC_STRING(IllegalNumericSuffixMessage, "Number has an illegal or unknown suffix \"%S\"");
STATIC_STRING(TrailingGarbageAfterNumberMessage, "Number has illegal trailing text \"%S\"");
STATIC_STRING(IllegalNumericSizeMessage, "Number is too large for its %s type (did you use the wrong numeric suffix?)");
STATIC_STRING(IllegalDecimalIntegerMessage, "Number not a valid decimal integer");
STATIC_STRING(IllegalOctalIntegerMessage, "Number not a valid octal integer");
STATIC_STRING(IllegalHexadecimalIntegerMessage, "Number not a valid hexadecimal integer");
STATIC_STRING(IllegalRealValueMessage, "Number not a valid real or float value");

STATIC_STRING(ZeroString, "0");
STATIC_STRING(LowercaseXString, "x");

static Bool EnsureEndOfNumber(Lexer lexer)
{
	StringBuilder stringBuilder;
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	Byte ch;
	UInt identifierCharacterKind;
	Int32 code;

	// If we have extra garbage at the end of the number, complain.
	if (src < end) {

		// Read the next Unicode code point.
		if ((ch = *src++) < 128)
			code = ch;
		else {
			src--;
			code = String_ExtractUnicodeCharacterInternal(&src, end);
		}

		// Disallow anything that smells like a letterform.
		if (code != '-') {
			identifierCharacterKind = SmileIdentifierKind(code);
			if (identifierCharacterKind & (IDENTKIND_STARTLETTER | IDENTKIND_MIDDLELETTER)) {
				stringBuilder = StringBuilder_Create();
				StringBuilder_AppendUnicode(stringBuilder, code);
				lexer->src = src;
				lexer->token->text = String_FormatString(TrailingGarbageAfterNumberMessage, StringBuilder_ToString(stringBuilder));
				return False;
			}
		}
	}

	return True;
}

Inline Bool IsAlphanumeric(Byte ch)
{
	switch (ch) {
		case 'a': case 'b': case 'c': case 'd':
		case 'e': case 'f': case 'g': case 'h':
		case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p':
		case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D':
		case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9':
			return True;
		default:
			return False;
	}
}

static String CollectAlphanumericSuffix(Lexer lexer)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 16);
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	Byte ch;

	if (src >= end || !IsAlphanumeric(ch = *src))
		return NULL;

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	do {
		src++;
		StringBuilder_AppendByte(stringBuilder, ch);
	} while (src < end && IsAlphanumeric(ch = *src));

	lexer->src = src;
	return StringBuilder_ToString(stringBuilder);
}

Inline Int ProcessIntegerValue(Lexer lexer, UInt64 value, String text, String suffix)
{
	const Byte *suffixText;
	Int suffixLength;

	if (String_IsNullOrEmpty(suffix)) {
		lexer->token->data.int64 = value;
		lexer->token->text = text;
		return (lexer->token->kind = TOKEN_INTEGER64);
	}

	suffixText = String_GetBytes(suffix);
	suffixLength = String_Length(suffix);

	switch (suffixText[0]) {

		case 's': case 'S':
			if (suffixLength == 1) {
				if (value > (1ULL << 16)) {
					lexer->token->text = String_FormatString(IllegalNumericSizeMessage, "Integer16");
					return (lexer->token->kind = TOKEN_ERROR);
				}
				else {
					lexer->token->data.int16 = (Int16)(UInt16)value;
					lexer->token->text = text;
					return (lexer->token->kind = TOKEN_INTEGER16);
				}
			}
			else goto unknown_suffix;

		case 't': case 'T':
			if (suffixLength == 1) {
				if (value >= (1ULL << 32)) {
					lexer->token->text = String_FormatString(IllegalNumericSizeMessage, "Integer32");
					return (lexer->token->kind = TOKEN_ERROR);
				}
				else {
					lexer->token->data.int32 = (Int32)(UInt32)value;
					lexer->token->text = text;
					return (lexer->token->kind = TOKEN_INTEGER32);
				}
			}
			else goto unknown_suffix;

		case 'x': case 'X':
			if (suffixLength == 1) {
				if (value > 256) {
					lexer->token->text = String_FormatString(IllegalNumericSizeMessage, "Byte");
					return (lexer->token->kind = TOKEN_ERROR);
				}
				else {
					lexer->token->data.byte = (Byte)(UInt32)value;
					lexer->token->text = text;
					return (lexer->token->kind = TOKEN_BYTE);
				}
			}
			else goto unknown_suffix;

		default:
		unknown_suffix:
			lexer->token->text = String_FormatString(IllegalNumericSuffixMessage, suffix);
			return (lexer->token->kind = TOKEN_ERROR);
	}
}

static Bool ParseDecimalInteger(Lexer lexer, UInt64 *result)
{
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start = src;
	UInt64 value = 0;
	UInt digit;
	Byte ch;

	while (src < end) {

		if ((ch = *src) >= '0' && ch <= '9') {
			digit = ch - '0';

			if (value > 0x1999999999999999ULL) {
				while (src < end) {
					if (((ch = *src) >= '0' && ch <= '9')
						|| ch == '_' || ch == '\'' || ch == '\"') {
						src++;
					}
					else break;
				}
				*result = 0;
				lexer->src = src;
				return False;
			}

			value *= 10;

			if (0xFFFFFFFFFFFFFFFFULL - digit < value) {
				while (src < end) {
					if (((ch = *src) >= '0' && ch <= '9')
						|| ch == '_' || ch == '\'' || ch == '\"') {
						src++;
					}
					else break;
				}
				*result = 0;
				lexer->src = src;
				return False;
			}

			value += digit;
			src++;
		}
		else if (ch == '_' || ch == '\'' || ch == '\"') {
			src++;
		}
		else break;
	}

	*result = value;
	lexer->src = src;
	return src > start;
}

static Bool ParseOctalInteger(Lexer lexer, UInt64 *result)
{
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start = src;
	UInt64 value = 0;
	UInt digit;
	Byte ch;

	while (src < end) {

		if ((ch = *src) >= '0' && ch <= '7') {
			digit = ch - '0';

			if (value > 0x1FFFFFFFFFFFFFFFULL) {
				while (src < end) {
					if (((ch = *src) >= '0' && ch <= '9')
						|| ch == '_' || ch == '\'' || ch == '\"') {
						src++;
					}
					else break;
				}
				*result = 0;
				lexer->src = src;
				return False;
			}

			value <<= 3;

			if (0xFFFFFFFFFFFFFFFFULL - digit < value) {
				while (src < end) {
					if (((ch = *src) >= '0' && ch <= '9')
						|| ch == '_' || ch == '\'' || ch == '\"') {
						src++;
					}
					else break;
				}
				*result = 0;
				lexer->src = src;
				return False;
			}

			value |= digit;
			src++;
		}
		else if (ch == '_' || ch == '\'' || ch == '\"') {
			src++;
		}
		else if (ch == '8' || ch == '9') {
			while (src < end) {
				if (((ch = *src) >= '0' && ch <= '9')
					|| ch == '_' || ch == '\'' || ch == '\"') {
					src++;
				}
				else break;
			}
			*result = 0;
			lexer->src = src;
			return False;
		}
		else break;
	}

	*result = value;
	lexer->src = src;
	return src > start;
}

static Bool ParseHexadecimalInteger(Lexer lexer, UInt64 *result)
{
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start = src;
	UInt64 value = 0;
	UInt digit;

	while (src < end) {

		switch (*src++) {
			case '0': digit = 0; break;
			case '1': digit = 1; break;
			case '2': digit = 2; break;
			case '3': digit = 3; break;
			case '4': digit = 4; break;
			case '5': digit = 5; break;
			case '6': digit = 6; break;
			case '7': digit = 7; break;
			case '8': digit = 8; break;
			case '9': digit = 9; break;
			case 'a': case 'A': digit = 10; break;
			case 'b': case 'B': digit = 11; break;
			case 'c': case 'C': digit = 12; break;
			case 'd': case 'D': digit = 13; break;
			case 'e': case 'E': digit = 14; break;
			case 'f': case 'F': digit = 15; break;

			case '_': case '\'': case '\"': continue;

			default:
				src--;
				goto done;
		}

		if (value > 0x0FFFFFFFFFFFFFFFULL) {
			Byte ch;
			while (src < end) {
				if (((ch = *src) >= '0' && ch <= '9')
					|| ch == '_' || ch == '\'' || ch == '\"'
					|| (ch >= 'a' && ch <= 'f')
					|| (ch >= 'A' && ch <= 'F')) {
					src++;
				}
				else break;
			}
			*result = 0;
			lexer->src = src;
			return False;
		}

		value = (value << 4) | digit;
	}

done:
	*result = value;
	lexer->src = src;
	return src > start;
}

Int Lexer_ParseReal(Lexer lexer, Bool isFirstContentOnLine)
{
	DECLARE_INLINE_STRINGBUILDER(digitBuilder, 256);	// 256 is plenty for most numbers, but it can grow if necessary.
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start;
	Byte ch;
	Token token = lexer->token;
	const Byte *digits;
	String digitString, suffix;
	const Byte *suffixText;
	Float64 float64;

	UNUSED(isFirstContentOnLine);

	INIT_INLINE_STRINGBUILDER(digitBuilder);

	START_TOKEN(src);

	// Collect integer digits.
	start = src;
	while (src < end && (ch = *src) >= '0' && ch <= '9') {
		src++;
		if (src + 1 < end
			&& ((ch = *src) == '\'' || ch == '\"' || ch == '_')
			&& src[1] >= '0' && src[1] <= '9') {
			if (src > start) {
				StringBuilder_Append(digitBuilder, start, 0, src - start);
			}
			src++;
			start = src;
		}
	}

	// Copy into the digitBuilder whatever integers are left.
	if (src > start) {
		StringBuilder_Append(digitBuilder, start, 0, src - start);
	}

	// Collect the decimal point.
	if (src < end && *src == '.') {
		src++;
		StringBuilder_AppendByte(digitBuilder, '.');

		// Collect fractional digits.
		start = src;
		while (src < end && (ch = *src) >= '0' && ch <= '9') {
			src++;
			if (src + 1 < end
				&& ((ch = *src) == '\'' || ch == '\"' || ch == '_')
				&& src[1] >= '0' && src[1] <= '9') {
				if (src > start) {
					StringBuilder_Append(digitBuilder, start, 0, src - start);
				}
				src++;
				start = src;
			}
		}
		if (src > start) {
			StringBuilder_Append(digitBuilder, start, 0, src - start);
		}
	}

	lexer->src = src;

	// Make the result C-friendly.
	StringBuilder_AppendByte(digitBuilder, '\0');

	// Extract out the raw text of the number.
	digitString = StringBuilder_ToString(digitBuilder);
	digits = String_GetBytes(digitString);

	// Get any trailing type identifiers.
	suffix = CollectAlphanumericSuffix(lexer);
	src = lexer->src;	// END_TOKEN needs the correct 'src' value.

	// And make sure the result is clean.
	if (!EnsureEndOfNumber(lexer)) {
		token->text = IllegalRealValueMessage;
		return END_TOKEN(TOKEN_ERROR);
	}

	suffixText = suffix != NULL ? String_GetBytes(suffix) : (const Byte *)"";
	if (suffixText[0] == '\0') {
		// Real64.
		if (!Real64_TryParse(digitString, &token->data.real64)) {
			token->text = IllegalRealValueMessage;
			return END_TOKEN(TOKEN_ERROR);
		}
		token->text = digitString;
		return END_TOKEN(TOKEN_REAL64);
	}
	else if (suffixText[0] == 'F' || suffixText[0] == 'f') {
		if (suffixText[1] == '\0') {
			// Float64.
			float64 = strtod((char *)digits, NULL);
			token->data.float64 = float64;
			token->text = digitString;
			return END_TOKEN(TOKEN_FLOAT64);
		}
		else goto badSuffix;
	}
	else if (suffixText[0] == 'L' || suffixText[0] == 'l') {
		// 128-bit something-or-other.
		if (suffixText[1] == '\0') {
			// Real128.
			if (!Real128_TryParse(digitString, &token->data.real128)) {
				token->text = IllegalRealValueMessage;
				return END_TOKEN(TOKEN_ERROR);
			}
			token->text = String_Concat(digitString, suffix);
			return END_TOKEN(TOKEN_REAL128);
		}
		else if ((suffixText[1] == 'F' || suffixText[1] == 'f') && suffixText[2] == '\0') {
			// Float128 (not yet supported).
			goto badSuffix;
		}
		else goto badSuffix;
	}
	else if (suffixText[0] == 'T' || suffixText[0] == 't') {
		// 32-bit something-or-other.
		if (suffixText[1] == '\0') {
			// Real32.
			if (!Real32_TryParse(digitString, &token->data.real32)) {
				token->text = IllegalRealValueMessage;
				return END_TOKEN(TOKEN_ERROR);
			}
			token->text = String_Concat(digitString, suffix);
			return END_TOKEN(TOKEN_REAL32);
		}
		else if ((suffixText[1] == 'F' || suffixText[1] == 'f') && suffixText[2] == '\0') {
			// Float32.
			float64 = strtod((char *)digits, NULL);
			token->data.float32 = (Float32)float64;
			token->text = digitString;
			return END_TOKEN(TOKEN_FLOAT32);
		}
		else goto badSuffix;
	}
	else goto badSuffix;

badSuffix:
	token->text = String_FormatString(IllegalNumericSuffixMessage, suffix);
	return END_TOKEN(TOKEN_ERROR);
}

Int Lexer_ParseZero(Lexer lexer, Bool isFirstContentOnLine)
{
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start, *digitsEnd;
	Token token = lexer->token;
	Byte ch;
	UInt64 value;
	String suffix;

	START_TOKEN(src - 1);
	start = src - 1;

	if (src < end && ((ch = *src) == 'x' || ch == 'X')) {
		// Hexadecimal integer, or possibly a zero byte.
		src++;

		if (src >= end || !(((ch = *src) >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) {
			// Not an error; this is decimal zero, as a byte.
			lexer->src = src;
			if (!EnsureEndOfNumber(lexer)) {
				lexer->token->text = String_FormatString(IllegalNumericSuffixMessage, String_Format("x%c", ch));
				return END_TOKEN(TOKEN_ERROR);
			}
			END_TOKEN(TOKEN_INTEGER64);
			return ProcessIntegerValue(lexer, 0, ZeroString, LowercaseXString);
		}
		else {
			// This is a hexadecimal integer.
			lexer->src = src;
			if (!ParseHexadecimalInteger(lexer, &value)) {
				lexer->token->text = IllegalHexadecimalIntegerMessage;
				return END_TOKEN(TOKEN_ERROR);
			}
			digitsEnd = lexer->src;
			suffix = CollectAlphanumericSuffix(lexer);
			src = lexer->src;	// END_TOKEN needs the correct 'src' value.
			if (!EnsureEndOfNumber(lexer)) {
				lexer->token->text = String_FormatString(IllegalNumericSuffixMessage, String_Format("%c", *lexer->src));
				return END_TOKEN(TOKEN_ERROR);
			}
			END_TOKEN(TOKEN_INTEGER64);
			return ProcessIntegerValue(lexer, value, String_Create(start, digitsEnd - start), suffix);
		}
	}
	else {
		// Octal integer, or possibly a real value (if we find a '.').
		lexer->src = start;
		if (!ParseOctalInteger(lexer, &value)) {
			src = lexer->src;
			if (!(src < lexer->end && *src == '.' && (src + 1 >= lexer->end || src[1] != '.'))) {
				lexer->token->text = IllegalOctalIntegerMessage;
				return END_TOKEN(TOKEN_ERROR);
			}
		}
		digitsEnd = src = lexer->src;
		if (src < lexer->end && *src == '.' && (src+1 >= lexer->end || src[1] != '.')) {
			// Found a '.' (and it's not part of a ".."), so rewind back and re-parse this as a real or float value.
			lexer->src = start;
			return Lexer_ParseReal(lexer, isFirstContentOnLine);
		}
		else {
			// Collected a whole octal value, so finish it.
			suffix = CollectAlphanumericSuffix(lexer);
			src = lexer->src;	// END_TOKEN needs the correct 'src' value.
			if (!EnsureEndOfNumber(lexer)) {
				lexer->token->text = String_FormatString(IllegalNumericSuffixMessage, String_Format("%c", *lexer->src));
				return END_TOKEN(TOKEN_ERROR);
			}
			END_TOKEN(TOKEN_INTEGER64);
			return ProcessIntegerValue(lexer, value, String_Create(start, digitsEnd - start), suffix);
		}
	}
}

Int Lexer_ParseDigit(Lexer lexer, Bool isFirstContentOnLine)
{
	const Byte *src = lexer->src;
	const Byte *start, *digitsEnd;
	Token token = lexer->token;
	UInt64 value;
	String suffix;

	START_TOKEN(src - 1);
	start = src - 1;

	// Decimal integer, or possibly a real value (if we find a '.').
	lexer->src = start;
	if (!ParseDecimalInteger(lexer, &value)) {
		src = lexer->src;
		if (!(src < lexer->end && *src == '.' && (src + 1 >= lexer->end || src[1] != '.'))) {
			lexer->token->text = IllegalDecimalIntegerMessage;
			return END_TOKEN(TOKEN_ERROR);
		}
	}
	digitsEnd = src = lexer->src;
	if (src < lexer->end && *src == '.' && (src + 1 == lexer->end || src[1] != '.')) {
		// Found a '.' (and it's not part of a ".."), so rewind back and re-parse this as a real or float value.
		lexer->src = start;
		return Lexer_ParseReal(lexer, isFirstContentOnLine);
	}
	else {
		// Collected a whole octal value, so finish it.
		suffix = CollectAlphanumericSuffix(lexer);
		src = lexer->src;	// END_TOKEN needs the correct 'src' value.
		if (!EnsureEndOfNumber(lexer)) return END_TOKEN(TOKEN_ERROR);
		END_TOKEN(TOKEN_INTEGER64);
		return ProcessIntegerValue(lexer, value, String_Create(start, digitsEnd - start), suffix);
	}
}
