//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

#include <smile/parsing/lexer.h>
#include <smile/parsing/tokenkind.h>
#include <smile/parsing/identkind.h>
#include <smile/stringbuilder.h>

#include <smile/parsing/internal/lexerinternal.h>

//---------------------------------------------------------------------------
//  Number parsing.

STATIC_STRING(IllegalNumericSuffixMessage, "Number has an illegal or unknown suffix \"%S\"");
STATIC_STRING(TrailingGarbageAfterNumberMessage, "Number has illegal trailing text \"%S\"");
STATIC_STRING(IllegalNumericSizeMessage, "Number is too large for its %s type (did you use the wrong numeric suffix?)");
STATIC_STRING(IllegalDecimalIntegerMessage, "Number not a valid decimal integer");
STATIC_STRING(IllegalOctalIntegerMessage, "Number not a valid octal integer");
STATIC_STRING(IllegalHexadecimalIntegerMessage, "Number not a valid hexadecimal integer");

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
		identifierCharacterKind = SmileIdentifierKind(code);
		if (identifierCharacterKind & (IDENTKIND_STARTLETTER | IDENTKIND_MIDDLELETTER)) {
			stringBuilder = StringBuilder_Create();
			StringBuilder_AppendUnicode(stringBuilder, code);
			lexer->src = src;
			lexer->token->text = String_FormatString(TrailingGarbageAfterNumberMessage, StringBuilder_ToString(stringBuilder));
			return False;
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

	return StringBuilder_ToString(stringBuilder);
}

Inline Int ProcessIntegerValue(Lexer lexer, UInt64 value, String text, String suffix)
{
	const Byte *suffixText = String_GetBytes(suffix);
	Int suffixLength = String_Length(suffix);

	if (String_IsNullOrEmpty(suffix)) {
		if (value >= (1ULL << 32)) {
			lexer->token->text = String_FormatString(IllegalNumericSizeMessage, "Integer32");
			return (lexer->token->kind = TOKEN_ERROR);
		}
		else {
			lexer->token->data.i = (Int32)(UInt32)value;
			lexer->token->text = text;
			return (lexer->token->kind = TOKEN_INTEGER32);
		}
	}

	switch (suffixText[0]) {

		case 'l': case 'L':
			if (suffixLength == 1) {
				lexer->token->data.int64 = (Int64)value;
				lexer->token->text = text;
				return (lexer->token->kind = TOKEN_INTEGER64);
			}
			else goto unknown_suffix;

		case 'h': case 'H':
			if (suffixLength == 1) {
				if (value > 65536) {
					lexer->token->text = String_FormatString(IllegalNumericSizeMessage, "Integer16");
					return (lexer->token->kind = TOKEN_ERROR);
				}
				else {
					lexer->token->data.i = (Int32)(UInt32)value;
					lexer->token->text = text;
					return (lexer->token->kind = TOKEN_INTEGER16);
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
					lexer->token->data.i = (Int32)(UInt32)value;
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

	while (src < end && (ch = *src) >= '0' && ch <= '9') {

		digit = ch - '0';

		if (value > 0x1999999999999999ULL) {
			*result = 0;
			lexer->src = src;
			return False;
		}

		value *= 10;

		if (0xFFFFFFFFFFFFFFFFULL - digit < value) {
			*result = 0;
			lexer->src = src;
			return False;
		}

		value += digit;
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

	while (src < end && (ch = *src) >= '0' && ch <= '7') {

		digit = ch - '0';

		if (value > 0x1FFFFFFFFFFFFFFFULL) {
			*result = 0;
			lexer->src = src;
			return False;
		}

		value <<= 3;

		if (0xFFFFFFFFFFFFFFFFULL - digit < value) {
			*result = 0;
			lexer->src = src;
			return False;
		}

		value |= digit;
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
			default:
				src--;
				goto done;
		}

		if (value > 0x0FFFFFFFFFFFFFFFULL) {
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
	const Byte *src = lexer->src;
	Token token = lexer->token;

	UNUSED(lexer);
	UNUSED(isFirstContentOnLine);

	START_TOKEN(src);
	lexer->token->text = String_FromC("Real and Float values are not yet supported.");
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
			END_TOKEN(TOKEN_INTEGER32);
			return ProcessIntegerValue(lexer, 0, ZeroString, LowercaseXString);
		}
		else {
			// This is a hexadecimal integer.
			lexer->src = src;
			if (!ParseHexadecimalInteger(lexer, &value)) {
				lexer->token->text = IllegalHexadecimalIntegerMessage;
				return END_TOKEN(TOKEN_ERROR);
			}
			digitsEnd = src = lexer->src;
			suffix = CollectAlphanumericSuffix(lexer);
			if (!EnsureEndOfNumber(lexer)) {
				lexer->token->text = String_FormatString(IllegalNumericSuffixMessage, String_Format("%c", *lexer->src));
				return END_TOKEN(TOKEN_ERROR);
			}
			END_TOKEN(TOKEN_INTEGER32);
			return ProcessIntegerValue(lexer, value, String_Create(start, digitsEnd - start), suffix);
		}
	}
	else {
		// Octal integer, or possibly a real value (if we find a '.').
		if (!ParseOctalInteger(lexer, &value)) {
			lexer->token->text = IllegalOctalIntegerMessage;
			return END_TOKEN(TOKEN_ERROR);
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
			if (!EnsureEndOfNumber(lexer)) {
				lexer->token->text = String_FormatString(IllegalNumericSuffixMessage, String_Format("%c", *lexer->src));
				return END_TOKEN(TOKEN_ERROR);
			}
			END_TOKEN(TOKEN_INTEGER32);
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
	if (!ParseDecimalInteger(lexer, &value)) {
		lexer->token->text = IllegalDecimalIntegerMessage;
		return END_TOKEN(TOKEN_ERROR);
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
		if (!EnsureEndOfNumber(lexer)) return END_TOKEN(TOKEN_ERROR);
		END_TOKEN(TOKEN_INTEGER32);
		return ProcessIntegerValue(lexer, value, String_Create(start, digitsEnd - start), suffix);
	}
}
