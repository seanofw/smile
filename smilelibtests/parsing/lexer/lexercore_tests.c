//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

#include "../../stdafx.h"

#include <smile/parsing/lexer.h>

TEST_SUITE(LexerCoreTests)

//-------------------------------------------------------------------------------------------------
//  Setup helper.

static Lexer SetupString(String source)
{
	Lexer lexer;

	Smile_ResetEnvironment();

	lexer = Lexer_Create(source, 0, String_Length(source), GetTestScriptName(), 1, 1, False);
	lexer->symbolTable = Smile_SymbolTable;

	return lexer;
}

static Lexer Setup(const char *string)
{
	return SetupString(String_FromC(string));
}

//-------------------------------------------------------------------------------------------------
//  Whitespace and comment tests.

START_TEST(EmptyInputResultsInEoi)
{
	Lexer lexer = Setup("");

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(EoiShouldStayEoiOnceYouReachIt)
{
	Lexer lexer = Setup("");

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(AllWhitespaceInputResultsInEoi)
{
	Lexer lexer = Setup("  \t  \r  \n  \a  \b  \v  \f  \xEF\xBB\xBF  ");	// Unicode BOM at the end.

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(SingleLineCommentsShouldBeSkipped)
{
	Lexer lexer = Setup("// This is a comment.\r\n"
		"// This is a comment on the next line.\r\n"
		"\r\n"
		"// This is another comment.\r\n"
		"\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(SingleLineCommentsShouldBeSkipped2)
{
	Lexer lexer = Setup("// This is a comment.\r\n"
		"// This is a comment on the next line.\r\n"
		".\r\n"
		"// This is another comment.\r\n"
		"foo\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(MultiLineCommentsShouldBeSkipped)
{
	Lexer lexer = Setup("/* This is a comment.\r\n"
		"This is a comment on the next line.\r\n"
		"\r\n"
		"This is another comment. */\r\n"
		"\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(MultiLineCommentsShouldBeSkipped2)
{
	Lexer lexer = Setup("/* This is a comment.\r\n"
		"This is a comment on the next line.*/\r\n"
		".\r\n"
		"/* This is another comment. */\r\n"
		"foo\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(HyphenSeparatorCommentsShouldBeSkipped)
{
	Lexer lexer = Setup("----------\r\n"
		".\r\n"
		"-----\r\n"
		"foo\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(EqualSeparatorCommentsShouldBeSkipped)
{
	Lexer lexer = Setup("==========\r\n"
		".\r\n"
		"=====\r\n"
		"foo\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShortHyphenSequencesShouldBeRecognizedAsIdentifiers)
{
	LexerPosition position;
	Lexer lexer = Setup(".-.--.---.----.-----.");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "-", 1);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "--", 2);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 2);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "---", 3);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "----", 4);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	// Five hyphens is a separator comment.

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShortEqualSequencesShouldBeRecognizedAsSpecials)
{
	LexerPosition position;
	Lexer lexer = Setup(".=.==.===.====.=====.");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "==", 2);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 2);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "===", 3);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "====", 4);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	// Five equal-signs is a separator comment.

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(HyphenSeparatorsShouldBeReturnedInSyntaxHighlighterMode)
{
	LexerPosition position;
	Lexer lexer = Setup("    ----------    \r\n"
		"   -----   \r\n");

	lexer->syntaxHighlighterMode = True;

	ASSERT(Lexer_Next(lexer) == TOKEN_WHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_COMMENT_SEPARATOR_HYPHEN);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 10);

	ASSERT(Lexer_Next(lexer) == TOKEN_WHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_NEWLINE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 2);

	ASSERT(Lexer_Next(lexer) == TOKEN_WHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_COMMENT_SEPARATOR_HYPHEN);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 5);

	ASSERT(Lexer_Next(lexer) == TOKEN_WHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_NEWLINE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 2);

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(EqualSeparatorsShouldBeReturnedInSyntaxHighlighterMode)
{
	LexerPosition position;
	Lexer lexer = Setup("    ==========    \r\n"
		"   =====   \r\n");

	lexer->syntaxHighlighterMode = True;

	ASSERT(Lexer_Next(lexer) == TOKEN_WHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_COMMENT_SEPARATOR_EQUALS);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 10);

	ASSERT(Lexer_Next(lexer) == TOKEN_WHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_NEWLINE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 2);

	ASSERT(Lexer_Next(lexer) == TOKEN_WHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_COMMENT_SEPARATOR_EQUALS);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 5);

	ASSERT(Lexer_Next(lexer) == TOKEN_WHITESPACE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_NEWLINE);
	position = Token_GetPosition(lexer->token);
	ASSERT(position->length == 2);

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

#include "lexercore_tests.generated.inc"
