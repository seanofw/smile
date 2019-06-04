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

#include <smile/internal/staticstring.h>
#include <smile/parsing/internal/lexerinternal.h>
#include <smile/regex.h>
#include <smile/smiletypes/smilelist.h>

STATIC_STRING(UnterminatedRegexMessage, "Regular expression that was started on line %d has no ending /");

/// <summary>
/// Consume characters until a trailing '/', allowing '\' to escape subsequent characters.
/// </summary>
static String Lexer_ParseRegexPattern(Lexer lexer)
{
	const Byte *start, *src, *end;
	Byte ch;

	src = start = lexer->src;
	end = lexer->end;

	// Consume the pattern.
	while (src < end && (ch = *src) != '/') {
		if (ch == '\\') {
			if (src < end - 1)
				src += 2;
			else
				src = end;
		}
		else if (ch == '\r') {
			src++;
			if (src < end && *src == '\n')
				src++;
			lexer->line++;
		}
		else if (ch == '\n') {
			src++;
			if (src < end && *src == '\r')
				src++;
			lexer->line++;
		}
		else src++;
	}
	lexer->src = src;

	// Make the pattern string and return it.
	return String_Create(start, src - start);
}

/// <summary>
/// Consume any naming characters as option flags.  Even though most of these
/// aren't valid option flags, they get consumed to keep the semantics of names sane.
/// They'll be validated (afterward) by the Regex_Create() function after they're collected.
/// </summary>
static String Lexer_ParseRegexOptions(Lexer lexer)
{
	const Byte *start, *src, *end;
	Byte ch;

	src = start = lexer->src;
	end = lexer->end;

readMoreOptions:
	if (src < end) {
		switch (ch = *src) {
		case '\\':
			src++;
			if (src < end) src++;
			goto readMoreOptions;

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
			src++;
			goto readMoreOptions;

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9':
		case '_': case '\'': case '\"':
		case '!': case '?': case '$': case '~':
			src++;
			goto readMoreOptions;

		case '-':
			src++;
			if (src < end && Lexer_IsValidRestartCharacter(src, end))
				goto readMoreOptions;
			src--;
			break;

		default:
			if (ch >= 128) {
				String_ExtractUnicodeCharacterInternal(&src, end);
				goto readMoreOptions;
			}
			break;
		}
	}
	lexer->src = src;

	// Make the pattern string and return it.
	return String_Create(start, src - start);
}

/// <summary>
/// Consume characters until a trailing '/', allowing '\' to escape subsequent characters;
/// and then after the '/', consume any alphanumeric characters as options.
/// </summary>
Int Lexer_ParseRegex(Lexer lexer, Bool isFirstContentOnLine)
{
	Token token = lexer->token;
	String pattern, flags;
	Int startLine = lexer->line;
	const Byte *src;
	Regex regex;
	String errorMessage;

	// Collect the pattern (but don't resolve backslash-escapes; that's for the regex engine to do).
	pattern = Lexer_ParseRegexPattern(lexer);

	// If we didn't reach a terminating '/', then fail.
	if ((src = lexer->src) == lexer->end) {
		lexer->token->text = String_FormatString(UnterminatedRegexMessage, startLine);
		return END_TOKEN(TOKEN_ERROR);
	}

	// Skip the '/'.
	lexer->src = ++src;

	// Collect the flags (unvalidated).
	flags = Lexer_ParseRegexOptions(lexer);
	src = lexer->src;

	// Make the token, except for its data.
	token->isFirstContentOnLine = isFirstContentOnLine;
	END_TOKEN(TOKEN_LOANWORD_REGEX);

	// Now create the regex, and validate its syntax.
	regex = Regex_Create(pattern, flags, &errorMessage);
	if (errorMessage != NULL) {
		lexer->token->text = errorMessage;
		return END_TOKEN(TOKEN_ERROR);
	}

	// Save the regex as the thing we're actually returning.
	token->data.ptr = regex;

	// And we're done.
	return token->kind;
}

/// <summary>
/// Consume all whitespace characters in the range of 0-32, stopping at the first instance of '\r' or '\n'.
/// If a '\r' is reached, and a '\n' follows it, consume the '\n'; likewise, if a '\n' is reached and a '\r'
/// follows it, consume the '\r'.
/// </summary>
/// <param name="lexer">The lexer to advance past the whitespace on this line.</param>
/// <returns>True if more content exists in the input; false if we reached EOI.</returns>
Bool Lexer_ConsumeWhitespaceOnThisLine(Lexer lexer)
{
	const Byte *src;
	const Byte *end;
	Byte ch;
	
	end = lexer->end;
	src = lexer->src;
retryAtSrc:
	if (src >= end) {
		lexer->src = src;
		return False;
	}

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
			goto retryAtSrc;

		case '\n':
			// Unix-style newlines, and inverted Windows newlines.
			if (src < end && (ch = *src) == '\r')
				src++;
			lexer->line++;
			lexer->lineStart = src;
			goto retryAtSrc;

		case '\r':
			// Windows-style newlines, and old Mac newlines.
			if (src < end && (ch = *src) == '\n')
				src++;
			lexer->line++;
			lexer->lineStart = src;
			goto retryAtSrc;
	}

	lexer->src = src - 1;
	return True;
}

RegexMatch Lexer_ConsumeRegex(Lexer lexer, Regex regex)
{
	Int matchLength;
	const Byte *src;
	const Byte *end, *matchEnd;
	Byte ch;

	src = lexer->src;
	end = lexer->end;

	// Match the regex.  The regex engine does all the hard work here; we just call it.
	RegexMatch match = Regex_MatchHere(regex, lexer->stringInput, src - lexer->input);
	if (match == NULL)
		return NULL;

	// Advance the input.  We have to do this character-by-character, because
	// we need to make sure that we're correctly tracking line numbers and column offsets.
	matchLength = match->indexedCaptures[0].length;
	matchEnd = src + matchLength;
	if (matchEnd > end) matchEnd = end;		// Should never happen, but don't trust your inputs.

	// Spin fast over the input, collecting line numbers as we find them.
	while (src < matchEnd) {
		if ((ch = *src++) == '\n') {
			// Unix-style newlines, and inverted Windows newlines.
			if (src < end && (ch = *src) == '\r')
				src++;
			lexer->line++;
			lexer->lineStart = src;
		}
		else if (ch == '\r') {
			// Windows-style newlines, and old Mac newlines.
			if (src < end && (ch = *src) == '\n')
				src++;
			lexer->line++;
			lexer->lineStart = src;
		}
	}

	lexer->src = src;
	return match;
}
