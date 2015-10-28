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
STATIC_STRING(IllegalNumericSizeMessage, "Number is too large for its %S type (did you forget a numeric suffix?)");

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
		lexer->token->data.i = (Int32)(UInt32)value;
		lexer->token->text = text;
		return (lexer->token->kind = TOKEN_INTEGER32);
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
				lexer->token->data.i = (Int32)(UInt32)value;
				lexer->token->text = text;
				return (lexer->token->kind = TOKEN_INTEGER16);
			}
			else goto unknown_suffix;

		case 'x': case 'X':
			if (suffixLength == 1) {
				lexer->token->data.i = (Int32)(UInt32)value;
				lexer->token->text = text;
				return (lexer->token->kind = TOKEN_BYTE);
			}
			else goto unknown_suffix;

		default:
		unknown_suffix:
			lexer->token->text = String_FormatString(IllegalNumericSuffixMessage, suffix);
			return (lexer->token->kind = TOKEN_ERROR);
	}
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

	if (src < end && ((ch = *src) == 'x' || ch == 'X'))
	{
		// Hexadecimal integer.
		src++;

		if (src >= end || !(((ch = *src) >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))
		{
			// Not an error; this is decimal zero, as a byte.
			lexer->src = src;
			if (!EnsureEndOfNumber(lexer)) return END_TOKEN(TOKEN_ERROR);
			END_TOKEN(TOKEN_INTEGER32);
			return ProcessIntegerValue(lexer, 0, ZeroString, LowercaseXString);
		}

		lexer->src = src;
		value = /*HexadecimalIntegerParser.ParseIntegerValue(this)*/ 0;
		digitsEnd = src = lexer->src;
		suffix = CollectAlphanumericSuffix(lexer);
		if (!EnsureEndOfNumber(lexer)) return END_TOKEN(TOKEN_ERROR);
		END_TOKEN(TOKEN_INTEGER32);
		return ProcessIntegerValue(lexer, value, String_Create(start, digitsEnd - start), suffix);
	}

	// Octal integer, or possibly a real value (if we find a '.').
	//return ParseNumber('0', OctalIntegerParser, value => ZeroString + (Utf8String)Convert.ToString((long)value, 8));
	return TOKEN_NONE;
}

Int Lexer_ParseDigit(Lexer lexer, Bool isFirstContentOnLine)
{
	UNUSED(lexer);
	UNUSED(isFirstContentOnLine);
	return TOKEN_NONE;
}
