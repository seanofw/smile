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

#include <smile/parsing/lexer.h>
#include <smile/parsing/tokenkind.h>
#include <smile/parsing/identkind.h>

#include <smile/internal/staticstring.h>
#include <smile/parsing/internal/lexerinternal.h>

//---------------------------------------------------------------------------
//  Core lexer.

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
/// <param name="syntaxHighlighterMode">Whether to return ALL lexemes as tokens (True), or whether to return
/// only the semantically-significant lexemes, discarding semantically-insiginificant things like
/// whitespace and comments (False).  You usually want False for this argument unless you're using the
/// Lexer inside an IDE's text editor or developing some other kind of similar tooling for code-analysis.</param>
/// <returns>The new lexical analyzer for the given input text.</returns>
Lexer Lexer_Create(String input, Int start, Int length, String filename, Int firstLine, Int firstColumn, Bool syntaxHighlighterMode)
{
	Lexer lexer = GC_MALLOC_STRUCT(struct LexerStruct);
	Int inputLength = String_Length(input);

	// If the input coordinates make no sense, abort.  We do this test in such a way that it's safe
	// even for very large input values.
	if (start < 0 || length < 0 || start > inputLength || length > inputLength || start + length > inputLength)
		return NULL;

	// Set up the read pointers.
	lexer->stringInput = input;
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

	// Set up the extra internal state flags.
	lexer->_hasPrecedingWhitespace = True;				// Behave like there's infinite whitespace before the first character.
	lexer->_isFirstContentOnLine = (firstColumn <= 1);	// We're starting a line if we're in column 1.

	// Whether to run in syntax-highlighter mode (where we return ALL content, even whitespace).
	lexer->syntaxHighlighterMode = syntaxHighlighterMode;

	return lexer;
}

/// <summary>
/// Get the position of the next token that will be read.  Note that since it hasn't
/// been read yet, the length of this position is always zero.  This does correctly handle
/// ungets, or tokens being put back on the input.
/// </summary>
/// <param name="lexer">The lexer to get the position of.</param>
/// <returns>The next lexer position, excluding its length.</returns>
LexerPosition Lexer_GetPosition(Lexer lexer)
{
	LexerPosition position;
	Token token;

	if (lexer->ungetCount > 0) {
		token = lexer->tokenBuffer + ((lexer->tokenIndex + 1) & 15);
		position = Token_GetPosition(token);
	}
	else {
		position = GC_MALLOC_STRUCT(struct LexerPositionStruct);
		if (position == NULL)
			Smile_Abort_OutOfMemory();
		position->filename = lexer->filename;
		position->line = (Int32)lexer->line;
		position->lineStart = (Int32)(lexer->lineStart - lexer->input);
		position->column = (Int32)(lexer->src - lexer->lineStart + 1);
	}

	position->length = 0;
	return position;
}

/// <summary>
/// Make a safe clone of a position on the heap.
/// </summary>
/// <param name="position">The position to clone.</param>
/// <returns>A copy of the provided position, located on the GC heap.</returns>
LexerPosition LexerPosition_Clone(LexerPosition position)
{
	LexerPosition newPosition = GC_MALLOC_STRUCT(struct LexerPositionStruct);
	if (newPosition == NULL)
		Smile_Abort_OutOfMemory();
	MemCpy(newPosition, position, sizeof(struct LexerPositionStruct));
	return newPosition;
}

/// <summary>
/// Compare two lexer positions to see if they are identical/equivalent.
/// </summary>
/// <param name="a">The first position to compare.</param>
/// <param name="b">The first position to compare.</param>
/// <returns>True if they are identical positions, False if they are not identical.</returns>
Bool LexerPosition_Equals(LexerPosition a, LexerPosition b)
{
	if (a == NULL) return (b == NULL);
	else if (b == NULL) return False;

	return (a == b
		|| (a->line == b->line
			&& a->column == b->column
			&& a->lineStart == b->lineStart
			&& a->length == b->length
			&& String_Equals(a->filename, b->filename)));
}

/// <summary>
/// Make a safe clone of a token on the heap.
/// </summary>
/// <param name="token">The token to clone.</param>
/// <returns>A copy of the provided token, located on the GC heap.</returns>
Token Token_Clone(Token token)
{
	Token newToken = GC_MALLOC_STRUCT(struct TokenStruct);
	if (newToken == NULL)
		Smile_Abort_OutOfMemory();
	MemCpy(newToken, token, sizeof(struct TokenStruct));
	return newToken;
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
	Byte ch;
	Int code;
	const Byte *src, *start;
	const Byte *end = lexer->end;
	Token token;
	Int tokenKind;

	// Read from the unget stack, if appropriate.
	if (lexer->ungetCount > 0) {
		lexer->token = token = lexer->tokenBuffer + (++lexer->tokenIndex & 15);
		lexer->ungetCount--;
		return token->kind;
	}

	// Set up for the next token.
	lexer->token = token = lexer->tokenBuffer + (++lexer->tokenIndex & 15);
	MemSet(token, 0, sizeof(struct TokenStruct));

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
			start = src - 1;
			while (src < end && (ch = *src) <= '\x20' && ch != '\n' && ch != '\r') src++;
			if (lexer->syntaxHighlighterMode) {
				tokenKind = SIMPLE_TOKEN(start, TOKEN_WHITESPACE);
				lexer->_hasPrecedingWhitespace = True;
				return tokenKind;
			}
			else {
				lexer->_hasPrecedingWhitespace = True;
				goto retryAtSrc;
			}

		case '\n':
			// Unix-style newlines, and inverted Windows newlines.
			start = src - 1;
			if (src < end && (ch = *src) == '\r')
				src++;
			if (lexer->syntaxHighlighterMode) {
				tokenKind = SIMPLE_TOKEN(start, TOKEN_NEWLINE);
				lexer->line++;
				lexer->lineStart = src;
				lexer->_isFirstContentOnLine = True;
				lexer->_hasPrecedingWhitespace = True;
				return tokenKind;
			}
			else {
				lexer->line++;
				lexer->lineStart = src;
				lexer->_isFirstContentOnLine = True;
				lexer->_hasPrecedingWhitespace = True;
			}
			goto retryAtSrc;

		case '\r':
			// Windows-style newlines, and old Mac newlines.
			start = src - 1;
			if (src < end && (ch = *src) == '\n')
				src++;
			if (lexer->syntaxHighlighterMode) {
				tokenKind = SIMPLE_TOKEN(start, TOKEN_NEWLINE);
				lexer->line++;
				lexer->lineStart = src;
				lexer->_isFirstContentOnLine = True;
				lexer->_hasPrecedingWhitespace = True;
				return tokenKind;
			}
			else {
				lexer->line++;
				lexer->lineStart = src;
				lexer->_isFirstContentOnLine = True;
				lexer->_hasPrecedingWhitespace = True;
			}
			goto retryAtSrc;

		//--------------------------------------------------------------------------
		//  Operators and complex punctuation things (like comments) that start like operators.

		case '/':
			// Punctuation names, but also comments.
			lexer->src = src;
			if ((tokenKind = Lexer_ParseSlash(lexer)) != TOKEN_NONE)
				return tokenKind;
			goto retry;

		case '-':
			// Subtraction, but also separator lines.
			lexer->src = src - 1;
			if ((tokenKind = Lexer_ParseHyphenOrEquals(lexer, ch)) != TOKEN_NONE)
				return tokenKind;
			goto retry;

		case '=':
			// Equate forms, but also separator lines.
			lexer->src = src - 1;
			if ((tokenKind = Lexer_ParseHyphenOrEquals(lexer, ch)) != TOKEN_NONE)
				return tokenKind;
			goto retry;

		case '~': case '!': case '?': case '@':
		case '%': case '^': case '&': case '*':
		case '+': case '<': case '>':
			// General punctuation and operator name forms:  [~!?@%^&*=+<>/-]+
			lexer->src = src - 1;
			return Lexer_ParsePunctuation(lexer);

		case '.':
			lexer->src = src;
			return Lexer_ParseDot(lexer);

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
			return Lexer_ParseZero(lexer);

		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8':
		case '9':
			// Decimal integer, or possibly a real value (if we find a '.').
			lexer->src = src;
			return Lexer_ParseDigit(lexer);

		//--------------------------------------------------------------------------
		//  Identifiers.

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
		case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
		case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case '_': case '$':
		case '\\':
			// Alpha/letter identifier form.
			lexer->src = src-1;
			return Lexer_ParseName(lexer, True);

		//--------------------------------------------------------------------------
		//  Strings and characters.

		case '\"':
			lexer->src = src;
			return Lexer_ParseDynamicString(lexer);

		case '\'':
			lexer->src = src;
			return Lexer_ParseRawString(lexer);

		case '#':
			lexer->src = src - 1;
			tokenKind = Lexer_ParseLoanword(lexer);
			if (tokenKind == TOKEN_NONE) {
				lexer->_hasPrecedingWhitespace = True;
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
				if (lexer->syntaxHighlighterMode) {
					tokenKind = SIMPLE_TOKEN(start, TOKEN_WHITESPACE_BOM);
					lexer->_hasPrecedingWhitespace = True;
					return tokenKind;
				}
				else {
					lexer->_hasPrecedingWhitespace = True;
					goto retryAtSrc;
				}
			}

			// Try Unicode identifiers.	
			{
				UInt identifierCharacterKind = SmileIdentifierKind(code);
				if (identifierCharacterKind & IDENTKIND_STARTLETTER) {
					// General identifier form.
					lexer->src = start;
					return Lexer_ParseName(lexer, True);
				}
				else if (identifierCharacterKind & IDENTKIND_PUNCTUATION) {
					// General punctuation and operator name forms:  [~!?@%^&*=+<>/-]+
					lexer->src = start;
					return Lexer_ParsePunctuation(lexer);
				}
			}

			// Everything else is an error at this point.
			START_TOKEN(start);
			lexer->token->text = String_Format("Unknown or invalid character (character code \"%C\")", code);
			return END_TOKEN(TOKEN_ERROR);
	}
}

/// <summary>
/// An easy interface for external (non-Smile) callers to invoke Lexer_Next().
/// </summary>
/// <param name="lexer">The lexer to read from.</param>
/// <returns>The next token in the input stream.</returns>
Token Lexer_NextToken(Lexer lexer)
{
	Lexer_Next(lexer);
	return lexer->token;
}

/// <summary>
/// An easy interface for external (non-Smile) callers to invoke Lexer_Peek().
/// </summary>
/// <param name="lexer">The lexer to read from.</param>
/// <returns>The next token in the input stream (but it won't be consumed.</returns>
Token Lexer_PeekToken(Lexer lexer)
{
	Token result;

	Lexer_Next(lexer);
	result = lexer->token;

	Lexer_Unget(lexer);

	return result;
}

/// <summary>
/// An easy interface for external (non-Smile) callers to invoke Lexer_Unget().
/// </summary>
/// <param name="lexer">The lexer to push the current token back onto.</param>
void Lexer_UngetToken(Lexer lexer)
{
	Lexer_Unget(lexer);
}
