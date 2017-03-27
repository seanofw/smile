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

#include <smile/parsing/lexer.h>
#include <smile/parsing/tokenkind.h>
#include <smile/parsing/identkind.h>
#include <smile/stringbuilder.h>

#include <smile/internal/staticstring.h>
#include <smile/parsing/internal/lexerinternal.h>

STATIC_STRING(IllegalNameMessage, "Names must not mix characters from different language families");

//---------------------------------------------------------------------------
//  Alphabetic name parsing.

static Bool IsValidRestartCharacter(const char *src, const char *end)
{
	Int code, charsets, identifierCharacterKind;
	char ch;

	switch (ch = *src) {

	case '\\':

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

	case '_': case '$':
		return True;

	default:
		if (ch >= 128) {
			code = String_ExtractUnicodeCharacterInternal(&src, end);
			identifierCharacterKind = SmileIdentifierKind(code);
			charsets |= ((UInt64)1) << ((identifierCharacterKind & IDENTKIND_CHARSET_MASK) >> 4);
			if ((identifierCharacterKind & IDENTKIND_STARTLETTER))
				return True;
		}
		return False;
	}
}

static String ParseNameRaw(Lexer lexer, Bool *hasEscapes)
{
	DECLARE_INLINE_STRINGBUILDER(namebuf, 64);
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start = lexer->src;
	Byte ch;
	Int code;
	UInt64 charsets = 0;
	UInt identifierCharacterKind;

	INIT_INLINE_STRINGBUILDER(namebuf);

	*hasEscapes = False;

readMoreName:
	if (src < end) {
		switch (ch = *src) {

		case '\\':
			if (src > start) {
				StringBuilder_Append(namebuf, start, 0, src - start);
			}
			src++;
			code = Lexer_DecodeEscapeCode(&src, end, True);
			StringBuilder_AppendUnicode(namebuf, code < 0 ? 0xFFFD : (UInt32)code);
			*hasEscapes = True;
			start = src;
			goto readMoreName;

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
			charsets |= ((UInt64)1) << (IDENTKIND_CHARSET_LATIN >> 4);
			src++;
			goto readMoreName;

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9':
		case '_': case '\'': case '\"':
		case '!': case '?': case '$': case '~':
			src++;
			goto readMoreName;

		case '-':
			src++;
			if (src < end && IsValidRestartCharacter(src, end))
				goto readMoreName;
			src--;
			break;

		default:
			if (src > start) {
				StringBuilder_Append(namebuf, start, 0, src - start);
			}
			if (ch >= 128) {
				code = String_ExtractUnicodeCharacterInternal(&src, end);
				identifierCharacterKind = SmileIdentifierKind(code);
				charsets |= ((UInt64)1) << ((identifierCharacterKind & IDENTKIND_CHARSET_MASK) >> 4);
				if ((identifierCharacterKind & (IDENTKIND_MIDDLELETTER | IDENTKIND_STARTLETTER))) {
					StringBuilder_AppendUnicode(namebuf, (UInt32)code);
					start = src;
					goto readMoreName;
				}
			}
			start = src;
			break;
		}
	}

	if (src > start) {
		StringBuilder_Append(namebuf, start, 0, src - start);
	}

	charsets &= ~1;		// Strip the meaningless low bit, if it got set by a Unicode middle character.

	if (charsets && (charsets & (charsets - 1))) {
		// More than one bit is set, as determined by some clever bit twiddling.
		// This is bad, because exactly one bit should always be set, indicating
		// exactly one character set was used in the name.
		lexer->src = src;
		return NULL;
	}

	// Convert whatever's left to the resulting identifier name.
	lexer->src = src;
	return StringBuilder_ToString(namebuf);
}

Int Lexer_ParseName(Lexer lexer, Bool isFirstContentOnLine)
{
	Token token = lexer->token;
	const Byte *src;
	String text;
	Symbol symbol;
	Bool hasEscapes;

	START_TOKEN(lexer->src);

	text = ParseNameRaw(lexer, &hasEscapes);
	if (text == NULL) {
		src = lexer->src;
		token->text = IllegalNameMessage;
		return END_TOKEN(TOKEN_ERROR);
	}

	symbol = lexer->symbolTable != NULL ? SymbolTable_GetSymbol(lexer->symbolTable, text) : 0;
	src = lexer->src;

	token->text = text;
	token->data.symbol = symbol;
	token->hasEscapes = hasEscapes;

	return END_TOKEN(TOKEN_ALPHANAME);
}

//---------------------------------------------------------------------------
//  Punctuative name parsing.

// Common known punctuation forms.
STATIC_STRING(Lexer_NameEqual, "=");
STATIC_STRING(Lexer_NameEq, "==");
STATIC_STRING(Lexer_NameSuperEq, "===");
STATIC_STRING(Lexer_NameNe, "!=");
STATIC_STRING(Lexer_NameSuperNe, "!==");
STATIC_STRING(Lexer_NameLt, "<");
STATIC_STRING(Lexer_NameLe, "<=");
STATIC_STRING(Lexer_NameGt, ">");
STATIC_STRING(Lexer_NameGe, ">=");
STATIC_STRING(Lexer_NamePlus, "+");
STATIC_STRING(Lexer_NameMinus, "-");
STATIC_STRING(Lexer_NameStar, "*");
STATIC_STRING(Lexer_NameSlash, "/");

static String ParsePunctuationRaw(Lexer lexer, Bool *hasEscapes, Int *tokenKind)
{
	DECLARE_INLINE_STRINGBUILDER(escapebuf, 64);
	DECLARE_INLINE_STRINGBUILDER(namebuf, 64);
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start = lexer->src;
	const Byte *nameBytes, *escapeBytes;
	Byte ch;
	Int code;
	UInt identifierCharacterKind;
	Int nameLen;

	INIT_INLINE_STRINGBUILDER(namebuf);
	INIT_INLINE_STRINGBUILDER(escapebuf);

	*hasEscapes = False;

readMorePunctuation:
	if (src < end) {
		switch (ch = *src) {

		case '\\':
			if (src > start) {
				StringBuilder_Append(namebuf, start, 0, src - start);
				StringBuilder_AppendRepeat(escapebuf, 'a', src - start);
			}
			src++;
			code = Lexer_DecodeEscapeCode(&src, end, True);
			StringBuilder_AppendUnicode(namebuf, code < 0 ? 0xFFFD : (UInt32)code);
			StringBuilder_AppendByte(escapebuf, '\\');
			*hasEscapes = True;
			start = src;
			goto readMorePunctuation;

		case '~': case '!': case '?':
		case '@': case '%': case '^': case '&':
		case '*': case '=': case '+': case '<':
		case '>': case '/': case '-':
			src++;
			goto readMorePunctuation;

		default:
			if (src > start) {
				StringBuilder_Append(namebuf, start, 0, src - start);
				StringBuilder_AppendRepeat(escapebuf, 'a', src - start);
			}
			if (ch >= 128) {
				code = String_ExtractUnicodeCharacterInternal(&src, end);
				identifierCharacterKind = SmileIdentifierKind(code);
				if ((identifierCharacterKind & (IDENTKIND_MIDDLELETTER | IDENTKIND_PUNCTUATION))) {
					StringBuilder_AppendUnicode(namebuf, (UInt32)code);
					StringBuilder_AppendByte(escapebuf, 'a');
					start = src;
					goto readMorePunctuation;
				}
			}
			start = src;
			break;
		}
	}

	if (src > start) {
		StringBuilder_Append(namebuf, start, 0, src - start);
		StringBuilder_AppendRepeat(escapebuf, 'a', src - start);
	}

	// Pull out whatever we collected.
	nameBytes = StringBuilder_GetBytes(namebuf);
	escapeBytes = StringBuilder_GetBytes(escapebuf);
	nameLen = StringBuilder_GetLength(namebuf);

	// Recognize certain special trailing-equals forms upfront.
	switch (*nameBytes) {

		case '=':
			if (nameLen == 1) {				// "="
				lexer->src = src;
				*tokenKind = TOKEN_EQUAL;
				return Lexer_NameEqual;
			}
			else if (nameLen == 2 && nameBytes[1] == '=') {			// "=="
				lexer->src = src;
				*tokenKind = TOKEN_PUNCTNAME;
				return Lexer_NameEq;
			}
			else if (nameLen == 3 && nameBytes[1] == '=' && nameBytes[2] == '=') {			// "==="
				lexer->src = src;
				*tokenKind = TOKEN_PUNCTNAME;
				return Lexer_NameSuperEq;
			}
			break;

		case '!':
			if (nameLen == 2 && nameBytes[1] == '=') {			// "!="
				lexer->src = src;
				*tokenKind = TOKEN_PUNCTNAME;
				return Lexer_NameNe;
			}
			else if (nameLen == 3 && nameBytes[1] == '=' && nameBytes[2] == '=') {			// "!=="
				lexer->src = src;
				*tokenKind = TOKEN_PUNCTNAME;
				return Lexer_NameSuperNe;
			}
			break;

		case '<':
			if (nameLen == 2 && nameBytes[1] == '=') {			// "<="
				lexer->src = src;
				*tokenKind = TOKEN_PUNCTNAME;
				return Lexer_NameLe;
			}
			break;

		case '>':
			if (nameLen == 2 && nameBytes[1] == '=') {			// ">="
				lexer->src = src;
				*tokenKind = TOKEN_PUNCTNAME;
				return Lexer_NameGe;
			}
			break;
	}

	// General case.  First, remove any trailing equal signs by ungetting, but we can only unget
	// non-escaped (i.e., legitimate) '=' characters.  We need to remove these because they could
	// be comments, or they could be op-equal forms, like "^=", and we want to be able to detect
	// the "^" as the actual operator.
	while (nameLen > 0 && nameBytes[nameLen - 1] == '=' && escapeBytes[nameLen - 1] != '\\') {
		nameLen--;
		src--;
	}
	StringBuilder_SetLength(namebuf, nameLen);
	StringBuilder_SetLength(escapebuf, nameLen);

	// Convert whatever's left to the resulting identifier name.
	lexer->src = src;

	// Recognize certain special punctuation forms faster (performance optimization).
	if (nameLen == 1) {
		switch (*nameBytes) {
		case '<':
			*tokenKind = TOKEN_PUNCTNAME;
			return Lexer_NameLt;
		case '>':
			*tokenKind = TOKEN_PUNCTNAME;
			return Lexer_NameGt;
		case '+':
			*tokenKind = TOKEN_PUNCTNAME;
			return Lexer_NamePlus;
		case '-':
			*tokenKind = TOKEN_PUNCTNAME;
			return Lexer_NameMinus;
		case '*':
			*tokenKind = TOKEN_PUNCTNAME;
			return Lexer_NameStar;
		case '/':
			*tokenKind = TOKEN_PUNCTNAME;
			return Lexer_NameSlash;
		}
	}

	*tokenKind = TOKEN_PUNCTNAME;
	return StringBuilder_ToString(namebuf);
}

Int Lexer_ParsePunctuation(Lexer lexer, Bool isFirstContentOnLine)
{
	Token token = lexer->token;
	const Byte *src;
	String text;
	Symbol symbol;
	Bool hasEscapes;
	Int tokenKind;

	START_TOKEN(lexer->src);

	text = ParsePunctuationRaw(lexer, &hasEscapes, &tokenKind);
	symbol = lexer->symbolTable != NULL ? SymbolTable_GetSymbol(lexer->symbolTable, text) : 0;
	src = lexer->src;

	token->text = text;
	token->data.symbol = symbol;
	token->hasEscapes = hasEscapes;

	return END_TOKEN(tokenKind);
}
