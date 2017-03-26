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

TEST_SUITE(LexerStringTests)

//-------------------------------------------------------------------------------------------------
//  Setup helper.

static Lexer SetupString(String source)
{
	Lexer lexer;

	Smile_ResetEnvironment();

	lexer = Lexer_Create(source, 0, String_Length(source), GetTestScriptName(), 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	return lexer;
}

static Lexer Setup(const char *string)
{
	return SetupString(String_FromC(string));
}

//-------------------------------------------------------------------------------------------------
//  String forms.

START_TEST(ShouldRecognizeCharacters)
{
	Lexer lexer = Setup(
		"x = 'z'\n"
		"y = '\\n'\n"
		"z = '\\t'\n"
		"'\\0' '\\x1A' '\\32' '\\xA0' '\\r' '\\a' '\\f' '\\v' '\\255'\n"
	);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 'z');

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == '\n');

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "z", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == '\t');

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 0);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 26);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 32);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 160);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 13);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 7);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 12);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 11);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.byte == 255);
}
END_TEST

START_TEST(ShouldRecognizeSingleLineRawStrings)
{
	Lexer lexer = Setup(
		"x = ''This is a test.''\n"
		"y = ''This is a test.\\n''\n"
	);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_RAWSTRING);
	ASSERT_STRING(lexer->token->text, "This is a test.", 15);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_RAWSTRING);
	ASSERT_STRING(lexer->token->text, "This is a test.\\n", 17);
}
END_TEST

START_TEST(ShouldRecognizeMultiLineRawStrings)
{
	Lexer lexer = Setup(
		"x = '''This is a test.''\n"
		"y = ''This is a test.\\n'''\n"
		"\n"
		"x = ''''This is a test.'''\n"
		"y = '''This is a test.\\n''''\n"
	);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_RAWSTRING);
	ASSERT_STRING(lexer->token->text, "This is a test.''\ny = ''This is a test.\\n", 41);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_RAWSTRING);
	ASSERT_STRING(lexer->token->text, "This is a test.'''\ny = '''This is a test.\\n", 43);
}
END_TEST

START_TEST(ShouldRecognizeTheEmptyString)
{
	Lexer lexer = Setup(
		"x = \"\"\n"
		"y = \"Foo\"\n"
		);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_DYNSTRING);
	ASSERT_STRING(lexer->token->text, "", 0);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_DYNSTRING);
	ASSERT_STRING(lexer->token->text, "Foo", 3);
}
END_TEST

START_TEST(ShouldRecognizeSingleLineDynamicStrings)
{
	Lexer lexer = Setup(
		"x = \"This is a test.\"\n"
		"y = \"This is a test.\\n\"\n"
		);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_DYNSTRING);
	ASSERT_STRING(lexer->token->text, "This is a test.", 15);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_DYNSTRING);
	ASSERT_STRING(lexer->token->text, "This is a test.\\n", 17);
}
END_TEST

START_TEST(ShouldRecognizeMultiLineDynamicStrings)
{
	Lexer lexer = Setup(
		"x = \"\"\"This is a test.\"\"\n"
		"y = \"\"This is a test.\\n\"\"\"\n"
		"\n"
		"x = \"\"\"\"This is a test.\"\"\"\n"
		"y = \"\"\"This is a test.\\n\"\"\"\"\n"
		);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_DYNSTRING);
	ASSERT_STRING(lexer->token->text, "This is a test.\"\"\ny = \"\"This is a test.\\n", 41);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_DYNSTRING);
	ASSERT_STRING(lexer->token->text, "This is a test.\"\"\"\ny = \"\"\"This is a test.\\n", 43);
}
END_TEST

#include "lexerstring_tests.generated.inc"
