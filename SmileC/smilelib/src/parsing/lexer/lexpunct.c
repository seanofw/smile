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
//  Lexer punctuation helpers.

STATIC_STRING(UnterminatedCommentMessage, "Comment that was started on line %d with a \"/*\" has no ending \"*/\"");

/// <summary>
/// Having seen a slash '/' on the input, parse either a comment or a punctuation name.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
/// <param name="isFirstContentOnLine">Whether this identifier is the first non-whitespace non-comment content on its line.</param>
/// <returns>The next token that was found in the input, or TOKEN_NONE if no token was found (i.e., a comment).</returns>
Int Lexer_ParseSlash(Lexer lexer, Bool isFirstContentOnLine)
{
	Byte nextch;
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	Token token = lexer->token;
	Int startLine;
	Byte ch;

	switch (nextch = *src++) {

		// A // single-line comment.
		case '/':
			while (src < end && (ch = *src) != '\n' && ch != '\r')
				src++;
			lexer->src = src;
			return TOKEN_NONE;

		// A multi-line comment.
		case '*':
			startLine = lexer->line;

			for (;;) {
				if (src >= end) {
					START_TOKEN(src - 1);
					lexer->token->text = String_FormatString(UnterminatedCommentMessage, startLine);
					return END_TOKEN(TOKEN_ERROR);
				}

				ch = *src++;
				if (ch == '\n') {
					if (src < end && *src == '\r')
						src++;
					lexer->line++;
				}
				else if (ch == '\r') {
					if (src < end && *src == '\n')
						src++;
					lexer->line++;
				}

				if (ch == '*' && src < end && *src == '/') {
					src++;
					break;
				}
			}

			lexer->src = src;
			return TOKEN_NONE;

		default:
			// Not a comment: General punctuation.
			lexer->src = --src;
			return Lexer_ParsePunctuation(lexer, isFirstContentOnLine);
	}
}

/// <summary>
/// Having seen a hyphen '-' or equal-sign '=', parse either a simple comment or a punctuation name.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
/// <param name="initialChar">The character read so far.</param>
/// <param name="isFirstContentOnLine">Whether this identifier is the first non-whitespace non-comment content on its line.</param>
/// <param name="hasPrecedingWhitespace">Whether there was whitespace immediately before the character that was read.</param>
/// <returns>The next token that was found in the input, or null if no token was found (i.e., a comment).</returns>
Int Lexer_ParseHyphenOrEquals(Lexer lexer, Int initialChar, Bool isFirstContentOnLine, Bool hasPrecedingWhitespace)
{
	Byte ch;
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	Int charCount;
	Int tokenKind;

	// Read hyphens/equals until we run out of them.
	charCount = 1;
	while (src < end && (ch = *src) == initialChar) {
		charCount++;
		src++;
	}

	if (charCount >= 5) {
		// Five or more hyphens/equals is a simple comment, which we flat-out ignore.
		lexer->src = src;
		return TOKEN_NONE;
	}

	// Anything else is a punctuation form, with '=' treated specially.
	tokenKind = Lexer_ParsePunctuation(lexer, isFirstContentOnLine);
	return (tokenKind == TOKEN_EQUAL && !hasPrecedingWhitespace) ? TOKEN_EQUALWITHOUTWHITESPACE : tokenKind;
}

/// <summary>
/// Having seen a dot '.' in the input, parse one of the following four kinds of tokens:
///
///   .     (dot)
///   ..    (range)
///   ...   (ellipsis)
///   .123  (real value)
///
/// We need to figure out which of these four cases we're dealing with, and decode
/// the input accordingly into the correct kind of token.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
/// <param name="isFirstContentOnLine">Whether this token is the first non-whitespace non-comment content on its line.</param>
/// <returns>The next token that was found in the input.</returns>
Int Lexer_ParseDot(Lexer lexer, Bool isFirstContentOnLine)
{
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	Token token = lexer->token;
	Byte ch;

	if (src >= end)
		return SIMPLE_TOKEN(src - 1, TOKEN_DOT);

	if ((ch = *src) == '.') {
		src++;
		// Two dots, so range or ellipsis.
		if (src < end && (ch = *src) == '.') {
			src++;
			// Three dots, so ellipsis.
			return SIMPLE_TOKEN(src - 3, TOKEN_ELLIPSIS);
		}
		else return SIMPLE_TOKEN(src - 2, TOKEN_RANGE);
	}

	if (ch >= '0' && ch <= '9') {
		// We got numeric digits after the dot, so this is actually a real value, not a dot operator.
		lexer->src--;
		return Lexer_ParseReal(lexer, isFirstContentOnLine);
	}

	return SIMPLE_TOKEN(src - 1, TOKEN_DOT);
}
