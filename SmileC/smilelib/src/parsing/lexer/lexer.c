//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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
//  Core lexer.

STATIC_STRING(IllegalCharacterMessage, "Unknown or invalid character (character code \"%S\")");

STATIC_STRING(KeywordAnd, "and");
STATIC_STRING(KeywordCatch, "catch");
STATIC_STRING(KeywordDo, "do");
STATIC_STRING(KeywordElse, "else");
STATIC_STRING(KeywordIf, "if");
STATIC_STRING(KeywordIs, "is");
STATIC_STRING(KeywordNew, "new");
STATIC_STRING(KeywordNot, "not");
STATIC_STRING(KeywordOr, "or");
STATIC_STRING(KeywordReturn, "return");
STATIC_STRING(KeywordThen, "then");
STATIC_STRING(KeywordTill, "till");
STATIC_STRING(KeywordTry, "try");
STATIC_STRING(KeywordTypeof, "typeof");
STATIC_STRING(KeywordUnless, "unless");
STATIC_STRING(KeywordUntil, "until");
STATIC_STRING(KeywordVar, "var");
STATIC_STRING(KeywordWhen, "when");
STATIC_STRING(KeywordWhile, "while");

/// <summary>
/// Create a new instance of a lexical analyzer for the given input text.
/// </summary>
/// <param name="input">The input text to begin lexing.</param>
/// <param name="start">The start character within the input text to begin lexing from.</param>
/// <param name="length">The number of input characters to lex.</param>
/// <param name="filename">The name of the file the input text comes from (for error reporting).</param>
/// <param name="firstLine">The one-based number of the first line where the given start character is located.</param>
/// <param name="firstColumn">The one-based number of the first column where the given start character is located.
/// (Note that for these purposes, all characters count as a single column, including tabs.)</param>
/// <returns>The new lexical analyzer for the given input text.</returns>
Lexer Lexer_Create(String input, Int start, Int length,
					String filename, Int firstLine, Int firstColumn)
{
	Lexer lexer = GC_MALLOC_STRUCT(struct LexerStruct);
	Int inputLength = String_Length(input);

	// If the input coordinates make no sense, abort.  We do this test in such a way that it's safe
	// even for very large input values.
	if (start < 0 || length < 0 || start > inputLength || length > inputLength || start + length > inputLength)
		return NULL;

	// Set up the read pointers.
	lexer->input = String_GetBytes(input);
	lexer->src = lexer->input + start;
	lexer->end = lexer->src + length;
	lexer->lineStart = lexer->src - (firstColumn - 1);

	// Set up the location tracking.
	lexer->filename = filename;
	lexer->line = firstLine;

	// Set up the output ring buffer.
	lexer->token = lexer->tokenBuffer;
	lexer->tokenIndex = 0;
	lexer->ungetCount = 0;

	return lexer;
}

//---------------------------------------------------------------------------
//  Core lexer.

/// <summary>
/// Consume one token from the input, and return its type.  This will also update the
/// return data from Lexer_Token() to contain the data for this token, if appropriate.
/// </summary>
/// <param name="lexer">The lexical analyzer to read from.</param>
/// <returns>The kind of the next token in the input (see tokenkind.h).</returns>
Int Lexer_Next(Lexer lexer)
{
	Bool isFirstContentOnLine;
	Bool hasPrecedingWhitespace;
	Byte ch;
	Int code;
	const Byte *src, *start;
	const Byte *end = lexer->end;
	Token token;
	Int tokenKind;
	UInt identifierCharacterKind;

	// Read from the unget stack, if appropriate.
	if (lexer->ungetCount > 0) {
		lexer->token = token = lexer->tokenBuffer + (++lexer->tokenIndex & 15);
		lexer->ungetCount--;
		return token->kind;
	}

	isFirstContentOnLine = False;
	hasPrecedingWhitespace = False;

	// Set up for the next token.
	lexer->token = token = lexer->tokenBuffer + (++lexer->tokenIndex & 15);

	// Loop (using gotos!) until we either get a valid token or run out of input.
retry:
	src = lexer->src;
retryAtSrc:
	if (src >= end)
		return SIMPLE_TOKEN(src, TOKEN_EOI);

	switch (ch = *src++) {

		//--------------------------------------------------------------------------
		//  Whitespace and newlines.

		case '\x00': case '\x01': case '\x02': case '\x03':
		case '\x04': case '\x05': case '\x06': case '\x07':
		case '\x08': case '\x09':              case '\x0B':
		case '\x0C':              case '\x0E': case '\x0F':
		case '\x10': case '\x11': case '\x12': case '\x13':
		case '\x14': case '\x15': case '\x16': case '\x17':
		case '\x18': case '\x19': case '\x1A': case '\x1B':
		case '\x1C': case '\x1D': case '\x1E': case '\x1F':
		case ' ':
			// Simple whitespace characters.  We consume as much whitespace as possible for better
			// performance, since whitespace tends to come in clumps in code.
			while (src < end && (ch = *src) <= '\x20' && ch != '\n' && ch != '\r') src++;
			hasPrecedingWhitespace = True;
			goto retryAtSrc;

		case '\n':
			// Unix-style newlines, and inverted Windows newlines.
			if (src < end && (ch = *src) == '\r')
				src++;
			lexer->line++;
			lexer->lineStart = src;
			isFirstContentOnLine = True;
			hasPrecedingWhitespace = True;
			goto retryAtSrc;

		case '\r':
			// Windows-style newlines, and old Mac newlines.
			if (src < end && (ch = *src) == '\n')
				src++;
			lexer->line++;
			lexer->lineStart = src;
			isFirstContentOnLine = True;
			hasPrecedingWhitespace = True;
			goto retryAtSrc;

		//--------------------------------------------------------------------------
		//  Operators and complex punctuation things (like comments) that start like operators.

		case '/':
			// Punctuation names, but also comments.
			lexer->src = src;
			if ((tokenKind = Lexer_ParseSlash(lexer, isFirstContentOnLine)) != TOKEN_NONE)
				return tokenKind;
			hasPrecedingWhitespace = True;
			goto retry;

		case '-':
			// Subtraction, but also separator lines.
			lexer->src = src - 1;
			if ((tokenKind = Lexer_ParseHyphenOrEquals(lexer, ch, isFirstContentOnLine, hasPrecedingWhitespace)) != TOKEN_NONE)
				return tokenKind;
			hasPrecedingWhitespace = True;
			goto retry;

		case '=':
			// Equate forms, but also separator lines.
			lexer->src = src - 1;
			if ((tokenKind = Lexer_ParseHyphenOrEquals(lexer, ch, isFirstContentOnLine, hasPrecedingWhitespace)) != TOKEN_NONE)
				return tokenKind;
			hasPrecedingWhitespace = True;
			goto retry;

		case '~': case '!': case '?': case '@':
		case '%': case '^': case '&': case '*':
		case '+': case '<': case '>':
			// General punctuation and operator name forms:  [~!?@%^&*=+<>/-]+
			lexer->src = src - 1;
			return Lexer_ParsePunctuation(lexer, isFirstContentOnLine);

		case '.':
			lexer->src = src;
			return Lexer_ParseDot(lexer, isFirstContentOnLine);

		//--------------------------------------------------------------------------
		//  Single-character special tokens.

		case '(': return SIMPLE_TOKEN(src-1, TOKEN_LEFTPARENTHESIS);
		case ')': return SIMPLE_TOKEN(src-1, TOKEN_RIGHTPARENTHESIS);

		case '[': return SIMPLE_TOKEN(src-1, TOKEN_LEFTBRACKET);
		case ']': return SIMPLE_TOKEN(src-1, TOKEN_RIGHTBRACKET);

		case '{': return SIMPLE_TOKEN(src-1, TOKEN_LEFTBRACE);
		case '}': return SIMPLE_TOKEN(src-1, TOKEN_RIGHTBRACE);

		case '|': return SIMPLE_TOKEN(src-1, TOKEN_BAR);
		case ':': return SIMPLE_TOKEN(src-1, TOKEN_COLON);
		case '`': return SIMPLE_TOKEN(src-1, TOKEN_BACKTICK);
		case ',': return SIMPLE_TOKEN(src-1, TOKEN_COMMA);
		case ';': return SIMPLE_TOKEN(src-1, TOKEN_SEMICOLON);

		//--------------------------------------------------------------------------
		//  Numbers.

		case '0':
			// Octal, hexadecimal, and real values.
			lexer->src = src;
			return Lexer_ParseZero(lexer, isFirstContentOnLine);

		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8':
		case '9':
			// Decimal integer, or possibly a real value (if we find a '.').
			lexer->src = src;
			return Lexer_ParseDigit(lexer, isFirstContentOnLine);

		//--------------------------------------------------------------------------
		//  Keywords (also can be identifiers).

		case 'a':
			if (IS_KEYWORD_CHAR(0, 'n') && IS_KEYWORD_CHAR(1, 'd') && IS_KEYWORD_END(2)) {
				src += 2;
				token->text = KeywordAnd;
				return SIMPLE_TOKEN(src - 3, TOKEN_AND);
			}
			goto parseAsName;

		case 'c':
			if (IS_KEYWORD_CHAR(0, 'a') && IS_KEYWORD_CHAR(1, 't') && IS_KEYWORD_CHAR(2, 'c')
				&& IS_KEYWORD_CHAR(3, 'h') && IS_KEYWORD_END(4)) {
				src += 4;
				token->text = KeywordCatch;
				return SIMPLE_TOKEN(src - 5, TOKEN_CATCH);
			}
			goto parseAsName;

		case 'd':
			if (IS_KEYWORD_CHAR(0, 'o') && IS_KEYWORD_END(1)) {
				src += 1;
				token->text = KeywordDo;
				return SIMPLE_TOKEN(src - 2, TOKEN_DO);
			}
			goto parseAsName;

		case 'e':
			if (IS_KEYWORD_CHAR(0, 'l') && IS_KEYWORD_CHAR(1, 's') && IS_KEYWORD_CHAR(2, 'e') && IS_KEYWORD_END(3)) {
				src += 3;
				token->text = KeywordElse;
				return SIMPLE_TOKEN(src - 4, TOKEN_ELSE);
			}
			goto parseAsName;

		case 'i':
			if (IS_KEYWORD_CHAR(0, 'f') && IS_KEYWORD_END(1)) {
				src += 1;
				token->text = KeywordIf;
				return SIMPLE_TOKEN(src - 2, TOKEN_IF);
			}
			else if (IS_KEYWORD_CHAR(0, 's') && IS_KEYWORD_END(1)) {
				src += 1;
				token->text = KeywordIs;
				return SIMPLE_TOKEN(src - 2, TOKEN_IS);
			}
			goto parseAsName;

		case 'n':
			if (IS_KEYWORD_CHAR(0, 'e') && IS_KEYWORD_CHAR(1, 'w') && IS_KEYWORD_END(2)) {
				src += 2;
				token->text = KeywordNew;
				return SIMPLE_TOKEN(src - 3, TOKEN_NEW);
			}
			else if (IS_KEYWORD_CHAR(0, 'o') && IS_KEYWORD_CHAR(1, 't') && IS_KEYWORD_END(2)) {
				src += 2;
				token->text = KeywordNot;
				return SIMPLE_TOKEN(src - 3, TOKEN_NOT);
			}
			goto parseAsName;

		case 'o':
			if (IS_KEYWORD_CHAR(0, 'r') && IS_KEYWORD_END(1)) {
				src += 1;
				token->text = KeywordOr;
				return SIMPLE_TOKEN(src - 2, TOKEN_OR);
			}
			goto parseAsName;

		case 'r':
			if (IS_KEYWORD_CHAR(0, 'e') && IS_KEYWORD_CHAR(1, 't') && IS_KEYWORD_CHAR(2, 'u')
				&& IS_KEYWORD_CHAR(3, 'r') && IS_KEYWORD_CHAR(4, 'n') && IS_KEYWORD_END(5)) {
				src += 5;
				token->text = KeywordReturn;
				return SIMPLE_TOKEN(src - 6, TOKEN_RETURN);
			}
			goto parseAsName;

		case 't':
			if (src >= end)
				goto parseAsName;

			switch (*src) {
				case 'h':
					if (IS_KEYWORD_CHAR(1, 'e') && IS_KEYWORD_CHAR(2, 'n') && IS_KEYWORD_END(3)) {
						src += 3;
						token->text = KeywordThen;
						return SIMPLE_TOKEN(src - 4, TOKEN_THEN);
					}
					goto parseAsName;
				case 'i':
					if (IS_KEYWORD_CHAR(1, 'l') && IS_KEYWORD_CHAR(2, 'l') && IS_KEYWORD_END(3)) {
						src += 3;
						token->text = KeywordTill;
						return SIMPLE_TOKEN(src - 4, TOKEN_TILL);
					}
					goto parseAsName;
				case 'r':
					if (IS_KEYWORD_CHAR(1, 'y') && IS_KEYWORD_END(2)) {
						src += 2;
						token->text = KeywordTry;
						return SIMPLE_TOKEN(src - 3, TOKEN_TRY);
					}
					goto parseAsName;
				case 'y':
					if (IS_KEYWORD_CHAR(1, 'p') && IS_KEYWORD_CHAR(2, 'e') && IS_KEYWORD_CHAR(3, 'o')
						&& IS_KEYWORD_CHAR(4, 'f') && IS_KEYWORD_END(5)) {
						src += 5;
						token->text = KeywordTypeof;
						return SIMPLE_TOKEN(src - 6, TOKEN_TYPEOF);
					}
					goto parseAsName;
				default:
					goto parseAsName;
			}

		case 'u':
			if (IS_KEYWORD_CHAR(0, 'n')) {
				if (IS_KEYWORD_CHAR(1, 't') && IS_KEYWORD_CHAR(2, 'i')
					&& IS_KEYWORD_CHAR(3, 'l') && IS_KEYWORD_END(4)) {
					src += 4;
					token->text = KeywordUntil;
					return SIMPLE_TOKEN(src - 5, TOKEN_UNTIL);
				}
				else if (IS_KEYWORD_CHAR(1, 'l') && IS_KEYWORD_CHAR(2, 'e')
					&& IS_KEYWORD_CHAR(3, 's') && IS_KEYWORD_CHAR(4, 's') && IS_KEYWORD_END(5)) {
					src += 5;
					token->text = KeywordUnless;
					return SIMPLE_TOKEN(src - 6, TOKEN_UNLESS);
				}
			}
			goto parseAsName;

		case 'v':
			if (IS_KEYWORD_CHAR(0, 'a') && IS_KEYWORD_CHAR(1, 'r') && IS_KEYWORD_END(2)) {
				src += 2;
				token->text = KeywordVar;
				return SIMPLE_TOKEN(src - 3, TOKEN_VAR);
			}
			goto parseAsName;

		case 'w':
			if (IS_KEYWORD_CHAR(0, 'h')) {
				if (IS_KEYWORD_CHAR(1, 'i') && IS_KEYWORD_CHAR(2, 'l') && IS_KEYWORD_CHAR(3, 'e') && IS_KEYWORD_END(4)) {
					src += 4;
					token->text = KeywordWhile;
					return SIMPLE_TOKEN(src - 5, TOKEN_WHILE);
				}
				else if (IS_KEYWORD_CHAR(1, 'e') && IS_KEYWORD_CHAR(2, 'n') && IS_KEYWORD_END(3)) {
					src += 3;
					token->text = KeywordWhen;
					return SIMPLE_TOKEN(src - 4, TOKEN_WHEN);
				}
			}
			goto parseAsName;

		//--------------------------------------------------------------------------
		//  Identifiers.

		case 'b': case 'f': case 'g': case 'h': case 'j': case 'k': case 'l': case 'm':
		case 'p': case 'q': case 's': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case '_': case '$':

		parseAsName :
			// General identifier form.
			lexer->src = src-1;
			return Lexer_ParseName(lexer, isFirstContentOnLine);

		//--------------------------------------------------------------------------
		//  Strings and characters.

		case '\"':
			lexer->src = src;
			return Lexer_ParseDynamicString(lexer, isFirstContentOnLine);

		case '\'':
			lexer->src = src;
			return Lexer_ParseRawString(lexer, isFirstContentOnLine);

		case '#':
			lexer->src = src - 1;
			tokenKind = Lexer_ParseLoanword(lexer, isFirstContentOnLine);
			if (tokenKind == TOKEN_NONE) {
				hasPrecedingWhitespace = True;
				goto retry;
			}
			return tokenKind;

		default:
			// Since it isn't a well-known character in the ASCII range, try reading it as a Unicode code point.
			start = --src;
			code = String_ExtractUnicodeCharacterInternal(&src, end);

			// Unicode byte-order mark (zero-width non-breaking space).
			if (code == 0xFEFF)
			{
				hasPrecedingWhitespace = True;
				goto retryAtSrc;
			}

			// Try Unicode identifiers.			
			identifierCharacterKind = SmileIdentifierKind(code);
			if (identifierCharacterKind & IDENTKIND_STARTLETTER) {
				// General identifier form.
				lexer->src = start;
				return Lexer_ParseName(lexer, isFirstContentOnLine);
			}
			else if (identifierCharacterKind & IDENTKIND_PUNCTUATION) {
				// General punctuation and operator name forms:  [~!?@%^&*=+<>/-]+
				lexer->src = start;
				return Lexer_ParsePunctuation(lexer, isFirstContentOnLine);
			}

			// Everything else is an error at this point.
			START_TOKEN(start);
			lexer->token->text = IllegalCharacterMessage;
			return END_TOKEN(TOKEN_ERROR);
	}
}
