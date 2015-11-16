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
//  Loanwords (regex, XML, JSON, etc.)

STATIC_STRING(UnterminatedRegexMessage, "Regular expression that was started on line %d has no ending /");
STATIC_STRING(UnknownLoanwordMessage, "Loanword \"%S\" is unknown.");
STATIC_STRING(UnsupportedLoanwordMessage, "Loanword \"%S\" is unsupported in this version of the Smile interpreter.");
STATIC_STRING(IllegalLoanwordMessage, "Loanword \"%S\" must be followed by whitespace.");

Int Lexer_ParseLoanword(Lexer lexer, Bool isFirstContentOnLine)
{
	const Byte *src = lexer->src;
	const Byte *end = lexer->end;
	const Byte *nameText;
	Token token = lexer->token;
	Int startLine;
	Int nameLength;
	Byte ch;

	START_TOKEN(src - 1);

	startLine = lexer->line;

	// There must be content of some kind after the '#'.
	if (src >= end) {
		lexer->token->text = String_FormatString(UnknownLoanwordMessage, String_FromC("#"));
		return END_TOKEN(TOKEN_ERROR);
	}

	switch (ch = *src++) {

		case '#':
			// The special "##" operator for constructing Lisp-like immediate cons-cells (dotted pairs).
			lexer->src = src;
			return END_TOKEN(TOKEN_DOUBLEHASH);

		case '!':
			// A Unix-style hashbang.  We treat it as a comment to the end of the line if we see it.
			while (src < end && (ch = *src) != '\n' && ch != '\r') {
				src++;
			}
			lexer->src = src;
			return TOKEN_NONE;

		case '/':
			// A regex.  Consume everything up to and including the next slash, using \ as an escape,
			// and then consume any trailing alphanumeric characters as flags.
			lexer->src = src;
			lexer->token->text = String_FormatString(UnsupportedLoanwordMessage, String_FromC("#/"));
			return END_TOKEN(TOKEN_ERROR);

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
		case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
		case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case '_': case '$':
			// A named loanword.

			// Get the name first.
			if (Lexer_ParseName(lexer, isFirstContentOnLine) == TOKEN_ERROR)
				return TOKEN_ERROR;

			// Make sure whitespace follows the name.
			src = lexer->src;
			if (src < end) {
				if (*src >= ' ') {
					lexer->src = src;
					lexer->token->text = String_FormatString(IllegalLoanwordMessage, String_Concat(String_FromC("#"), lexer->token->text));
					return END_TOKEN(TOKEN_ERROR);
				}
				src++;
			}

			nameText = String_GetBytes(lexer->token->text);
			nameLength = String_Length(lexer->token->text);

			switch (nameText[0]) {
				case 'j':
					if (nameLength == 4 && !MemCmp((const char *)nameText, "json", 4))
					{
						// Start of a JSON object or JSON array.  Consume characters up to the next matching '}' or ']',
						// following nesting rules for '}' and ']', skipping content inside "quotes", and allowing \ to
						// escape characters inside "quotes".
						lexer->src = src;
						lexer->token->text = String_FormatString(UnsupportedLoanwordMessage, String_Concat(String_FromC("#"), lexer->token->text));
						return END_TOKEN(TOKEN_ERROR);
					}
					goto unknown_loanword;

				case 'h':
					if (nameLength == 4 && !MemCmp((const char *)nameText, "html", 4))
					{
						// Start of an <html> insert.  This will scan the given opening tag and then read characters
						// until it finds the matching closing tag, and return that data as an Insert_Html token.
						// This understands the odd HTML rules about self-closing tags, and knows that certain tags,
						// like <script> and <style> and <xmp>, need to be processed verbatim.  This will return
						// the resulting consumed data as an Insert_Html token.
						lexer->src = src;
						lexer->token->text = String_FormatString(UnsupportedLoanwordMessage, String_Concat(String_FromC("#"), lexer->token->text));
						return END_TOKEN(TOKEN_ERROR);
					}
					goto unknown_loanword;

				case 'x':
					if (nameLength == 3 && !MemCmp((const char *)nameText, "xml", 3))
					{
						// Start of an <xml> insert.  This will scan the given opening tag and then read characters
						// until it finds the matching closing tag, and return that data as an Insert_Xml token.
						lexer->src = src;
						lexer->token->text = String_FormatString(UnsupportedLoanwordMessage, String_Concat(String_FromC("#"), lexer->token->text));
						return END_TOKEN(TOKEN_ERROR);
					}
					goto unknown_loanword;

				case 'i':
					if (nameLength == 7 && !MemCmp((const char *)nameText, "include", 7)) {
						lexer->src = src;
						return END_TOKEN(TOKEN_LOANWORD_INCLUDE);
					}
					goto unknown_loanword;

				case 's':
					if (nameLength == 6 && !MemCmp((const char *)nameText, "syntax", 6)) {
						lexer->src = src;
						return END_TOKEN(TOKEN_LOANWORD_SYNTAX);
					}
					goto unknown_loanword;

				case 'b':
					if (nameLength == 3 && !MemCmp((const char *)nameText, "brk", 3)) {
						lexer->src = src;
						return END_TOKEN(TOKEN_LOANWORD_BRK);
					}
					goto unknown_loanword;

				default:
				unknown_loanword:
					lexer->src = src;
					lexer->token->text = String_FormatString(UnknownLoanwordMessage, String_Concat(String_FromC("#"), lexer->token->text));
					return END_TOKEN(TOKEN_ERROR);
			}

		default:
			lexer->src = --src;
			lexer->token->text = String_FormatString(UnknownLoanwordMessage, String_Concat(String_FromC("#"), String_CreateRepeat(ch, 1)));
			return END_TOKEN(TOKEN_ERROR);
	}
}
