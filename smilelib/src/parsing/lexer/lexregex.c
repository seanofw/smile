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
	SmileList creationCall;
	LexerPosition position;
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

	// Now, create — and then discard! — the regex.  This may seem odd, but it fulfills two purposes:
	// First, it validates the correctness of the regex:  It parses it fully at lexical-analysis time,
	// so we can be certain it's correct before letting the program run.  Second, even though we've
	// discarded it, the compiled copy of the regex is still sitting in the global regex cache, so when
	// the program runs, it won't need to compile the regex again; it'll just locate the correct
	// compiled regex object by matching strings to a known cache entry.  (The compiler may be able to
	// even optimize this further, by recognizing patterns of the form [Regex.of pattern options], and
	// simply transforming those into a load of a known handle.  But that work doesn't belong here,
	// since #/.../ is only a shorthand for writing [Regex.of pattern options].)
	regex = Regex_Create(pattern, flags, &errorMessage);
	if (errorMessage != NULL) {
		lexer->token->text = errorMessage;
		return END_TOKEN(TOKEN_ERROR);
	}

	// The constructed lists will have the whole regex position as their position.  This isn't perfect,
	// but it's close enough that any error-reporting will be more-or-less in the right place.
	position = Token_GetPosition(lexer->token);

	// We got it, so return it as a pre-built list of the form [Regex.of pattern-string options-string].
	creationCall =
		SmileList_ConsWithSource(
			(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._dotSymbol,
				(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects.RegexSymbol,
					(SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects.ofSymbol,
						NullObject,
						position),
					position),
				position),
			(SmileObject)SmileList_ConsWithSource((SmileObject)pattern,
				(SmileObject)SmileList_ConsWithSource((SmileObject)flags,
					NullObject,
					position),
				position),
			position);

	// Attach the list to the token, so it really can be returned.
	token->data.ptr = creationCall;

	// And we're done.
	return token->kind;
}
