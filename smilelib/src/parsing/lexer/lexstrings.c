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
#include <smile/stringbuilder.h>

#include <smile/internal/staticstring.h>
#include <smile/parsing/internal/lexerinternal.h>

//---------------------------------------------------------------------------
//  String parsing.

STATIC_STRING(UnterminatedRawStringMessage, "Raw string that was started on line %d with a ' has no ending '");
STATIC_STRING(UnterminatedDynamicStringMessage, "String that was started on line %d with a \" has no ending \"");
STATIC_STRING(UnterminatedCharMessage, "Character that was started on line %d has no ending '");

/// <summary>
/// Having seen a single quote '\"', parse it into a dynamic string.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
/// <returns>The next token that was found in the input, or TOKEN_ERROR if a broken token was found (i.e., unterminated string).</returns>
Int Lexer_ParseDynamicString(Lexer lexer)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 1024);
	Int initialQuotes, quoteCount;
	Int startLine;
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start;
	Token token = lexer->token;

	START_TOKEN(src - 1);
	INIT_INLINE_STRINGBUILDER(stringBuilder);

	startLine = lexer->line;

	// Count up how many quotes this string starts with.
	initialQuotes = 1;
	while (src < end && *src == '\"') {
		src++;
		initialQuotes++;
	}

	// If it's exactly two, this is the empty string.
	if (initialQuotes == 2) {
		lexer->src = src;
		lexer->token->text = String_Empty;
		return END_TOKEN(TOKEN_DYNSTRING);
	}

	// This variable will keep track of how many closing quotes we've seen in sequence.
	quoteCount = 0;

	// This keeps track of the non-appended part of the input so far.
	start = src;

	// This is a dynamic string (possibly multiline), so collect content until we reach a closing
	// set of quotes that matches the opening set of quotes.
retry:

	if (src >= end) {
		// Ran out of content, so this is an unterminated string.
		lexer->src = src;
		lexer->token->text = String_FormatString(UnterminatedDynamicStringMessage, startLine);
		return END_TOKEN(TOKEN_ERROR);
	}

	switch (*src++) {

		// Handle newlines of the first kind.
		case '\n':
			if (initialQuotes == 1) {
				// Single-line dynamic string, so disallow line breaks.
				lexer->src = src;
				lexer->token->text = String_FormatString(UnterminatedDynamicStringMessage, startLine);
				return END_TOKEN(TOKEN_ERROR);
			}
			if (src < end && *src == '\r') {
				src++;
			}
			lexer->line++;
			goto retry;

		// Handle newlines of the second kind.
		case '\r':
			if (initialQuotes == 1) {
				// Single-line dynamic string, so disallow line breaks.
				lexer->src = src;
				lexer->token->text = String_FormatString(UnterminatedDynamicStringMessage, startLine);
				return END_TOKEN(TOKEN_ERROR);
			}
			if (src < end && *src == '\n') {
				src++;
			}
			lexer->line++;
			goto retry;

		// If we see quotes, count them up so we know when we've reached the end of the string.
		case '\"':
			if (++quoteCount == initialQuotes)
				break;
			goto retry;

		// If we see a backslash, we need to consume the character that follows it so that the parser
		// has enough content to be able to disassemble the string later on.
		case '\\':
			quoteCount = 0;
			if (src < end) src++;
			goto retry;

		// Everything else just gets collected to be appended to the output.
		default:
			quoteCount = 0;
			goto retry;
	}

	// Collect up whatever's left.
	if ((src - start) - initialQuotes > 0) {
		StringBuilder_Append(stringBuilder, start, 0, (src - start) - initialQuotes);
	}

	lexer->src = src;
	lexer->token->text = StringBuilder_ToString(stringBuilder);

	return END_TOKEN(initialQuotes > 1 ? TOKEN_LONGDYNSTRING : TOKEN_DYNSTRING);
}

/// <summary>
/// Having seen a single apostrophe '\'', parse its contents into a single 8-bit Char
/// or into a single 32-bit Uni.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
/// <returns>The next token that was found in the input, or TOKEN_ERROR if a broken token was found (i.e., unterminated string).</returns>
Int Lexer_ParseChar(Lexer lexer)
{
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	Token token = lexer->token;
	Byte ch;
	Int value;
	Int startLine;
	Bool isUni = False;

	START_TOKEN(src - 1);

	startLine = lexer->line;

	if (src >= end) {
		// Ran out of content, so this is an unterminated char.
		lexer->src = src;
		lexer->token->text = String_FormatString(UnterminatedCharMessage, startLine);
		return END_TOKEN(TOKEN_ERROR);
	}

	ch = *src++;
	if (ch == '\\') {
		isUni = (src < end && ((ch = *src) == 'u' || ch == 'U'));
		value = Lexer_DecodeEscapeCode(&src, end, False);
	}
	else if (ch <= '\x1F') {
		// Linebreaks and control codes are disallowed, so this is treated as an unterminated char.
		lexer->src = src;
		lexer->token->text = String_FormatString(UnterminatedCharMessage, startLine);
		return END_TOKEN(TOKEN_ERROR);
	}
	else if (ch >= 0x80) {
		src--;
		value = String_ExtractUnicodeCharacterInternal(&src, end);
		isUni = True;
	}
	else {
		value = ch;
	}

	if (src >= end || *src != '\'' || value < 0 || value > 0x110000) {
		// Too much content for a char, so treat this as an unterminated char.
		lexer->src = src;
		lexer->token->text = String_FormatString(UnterminatedCharMessage, startLine);
		return END_TOKEN(TOKEN_ERROR);
	}
	src++;

	if (isUni) {
		lexer->token->data.uni = (UInt32)value;
		return END_TOKEN(TOKEN_UNI);
	}
	else {
		lexer->token->data.ch = (Byte)(UInt32)value;
		return END_TOKEN(TOKEN_CHAR);
	}
}

/// <summary>
/// Having seen a single apostrophe '\'', parse it into a raw string or a character.
/// </summary>
/// <param name="lexer">The lexical analyzer.</param>
/// <returns>The next token that was found in the input, or TOKEN_ERROR if a broken token was found (i.e., unterminated string).</returns>
Int Lexer_ParseRawString(Lexer lexer)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 1024);
	Int initialApostrophes, quoteCount;
	Int startLine;
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *start;
	Token token = lexer->token;

	START_TOKEN(src - 1);
	INIT_INLINE_STRINGBUILDER(stringBuilder);

	startLine = lexer->line;

	// Count up how many apostrophes this string starts with.
	initialApostrophes = 1;
	while (src < end && *src == '\'') {
		src++;
		initialApostrophes++;
	}

	// If it's just one, parse it as a single char.
	if (initialApostrophes == 1) {
		return Lexer_ParseChar(lexer);
	}

	// This variable will keep track of how many closing apostrophes we've seen in sequence.
	quoteCount = 0;

	// This keeps track of the non-appended part of the input so far.
	start = src;

	// This is a raw string (possibly multiline), so collect content until we reach a closing
	// set of apostrophes that matches the opening set of apostrophes.
retry:

	if (src >= end) {
		// Ran out of content, so this is an unterminated string.
		lexer->src = src;
		lexer->token->text = String_FormatString(UnterminatedRawStringMessage, startLine);
		return END_TOKEN(TOKEN_ERROR);
	}

	switch (*src++) {

		// Handle newlines of the first kind.
		case '\n':
			if (initialApostrophes == 2) {
				// Single-line raw string, so disallow line breaks.
				lexer->src = src;
				lexer->token->text = String_FormatString(UnterminatedRawStringMessage, startLine);
				return END_TOKEN(TOKEN_ERROR);
			}
			if (src < end && *src == '\r') {
				src++;
			}
			lexer->line++;
			goto retry;

		// Handle newlines of the second kind.
		case '\r':
			if (initialApostrophes == 2) {
				// Single-line raw string, so disallow line breaks.
				lexer->src = src;
				lexer->token->text = String_FormatString(UnterminatedRawStringMessage, startLine);
				return END_TOKEN(TOKEN_ERROR);
			}
			if (src < end && *src == '\n') {
				src++;
			}
			lexer->line++;
			goto retry;

		// If we see apostrophes, count them up so we know when we've reached the end of the string.
		case '\'':
			if (++quoteCount == initialApostrophes)
				break;
			goto retry;

		// Everything else just gets collected to be appended to the output.
		default:
			quoteCount = 0;
			goto retry;
	}

	// Collect up whatever's left.
	if ((src - start) - initialApostrophes > 0) {
		StringBuilder_Append(stringBuilder, start, 0, (src - start) - initialApostrophes);
	}

	lexer->src = src;
	lexer->token->text = StringBuilder_ToString(stringBuilder);

	return END_TOKEN(initialApostrophes == 2 ? TOKEN_RAWSTRING : TOKEN_LONGRAWSTRING);
}

