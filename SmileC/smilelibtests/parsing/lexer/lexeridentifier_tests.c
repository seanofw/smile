//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

#include "../../stdafx.h"

#include <smile/parsing/lexer.h>

TEST_SUITE(LexerIdentifierTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Setup helper.

static Lexer SetupString(String source)
{
	Lexer lexer;

	Smile_ResetEnvironment();

	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	return lexer;
}

static Lexer Setup(const char *string)
{
	return SetupString(String_FromC(string));
}

//-------------------------------------------------------------------------------------------------
//  Alphabetic identifiers and keywords.

START_TEST(ShouldRecognizeSingleCharacterAlphaIdents)
{
	Lexer lexer = Setup("a b c d e a= b= c= d= e=\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "a", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "b", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "c", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "d", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "e", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "a", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "b", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "c", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "d", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "e", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
}
END_TEST

START_TEST(ShouldRecognizeSimpleAlphaIdentifiers)
{
	Lexer lexer = Setup(
		"foo bar baz Foo Bar Baz FooBar FOOBAR FOO_BAR __BARFOO _barfoo $foo foo$bar\n"
		);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "foo", 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "bar", 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "baz", 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "Foo", 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "Bar", 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "Baz", 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "FooBar", 6);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "FOOBAR", 6);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "FOO_BAR", 7);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "__BARFOO", 8);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "_barfoo", 7);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "$foo", 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "foo$bar", 7);
}
END_TEST

START_TEST(ShouldRecognizeAlphaIdentsWithEmbeddedPunct)
{
	Lexer lexer = Setup(
		"x' = func x\n"
		"y' = f**ck\n"
		"z'' = pl-rk? z\n"
		"w = x-y\n"
		"w = x+z\n"
		"w = x-1\n"
		"w = x+1\n"
		"This-is-a-really-long-name-you-shouldn't-use-but-could!"
		" x"
	);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x'", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "func", 4);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y'", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "f", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "**", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "ck", 2);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "z''", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "pl-rk?", 6);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "z", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "w", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x-y", 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "w", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "+", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "z", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "w", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "-", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "w", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "+", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "This-is-a-really-long-name-you-shouldn't-use-but-could!", 55);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
}
END_TEST

START_TEST(ShouldRecognizeAlphaOpEqualsForms)
{
	Lexer lexer = Setup(
		"x sin= y\n"
		"x cos'= y\n"
		"x tanm1''= y\n"
		"x cotp1'''== y\n"
		"x sec-1 === y\n"
		);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "sin", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "cos'", 4);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "tanm1''", 7);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "cotp1'''", 8);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "==", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "sec", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "-", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "===", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);
}
END_TEST

#include "lexeridentifier_tests.generated.inc"
