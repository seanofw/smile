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

TEST_SUITE(LexerLoanwordTests)

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
//  Double-hash loanwords.

START_TEST(ShouldRecognizeDoubleHash)
{
	Lexer lexer = Setup("  \t  ##  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_DOUBLEHASH);
}
END_TEST

START_TEST(ShouldNotRecognizeSingleOrTripleHash)
{
	Lexer lexer = Setup("  \t  #  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	lexer = Setup("  \t  ###  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_DOUBLEHASH);
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Hashbang loanwords.

START_TEST(ShouldTreatHashBangsAsLineComments)
{
	Lexer lexer = Setup("  \t  #!/usr/bin/smile foo \r\n"
						"bar \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "bar", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Include loanwords.

START_TEST(ShouldRecognizeInclude)
{
	Lexer lexer = Setup("  \t  #include \"foo.sm\"  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_LOANWORD_INCLUDE);
	ASSERT(Lexer_Next(lexer) == TOKEN_DYNSTRING);
	ASSERT_STRING(lexer->token->text, "foo.sm", 6);
}
END_TEST

START_TEST(ShouldNotRecognizeNonIncludes)
{
	Lexer lexer = Setup("  \t  #includex  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	lexer = Setup("  \t  #Include  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	lexer = Setup("  \t  #INCLUDE  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Syntax loanwords.

START_TEST(ShouldRecognizeSyntax)
{
	Lexer lexer = Setup("  \t  #syntax STMT [] []  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_LOANWORD_SYNTAX);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "STMT", 4);
	ASSERT(Lexer_Next(lexer) == TOKEN_LEFTBRACKET);
	ASSERT(Lexer_Next(lexer) == TOKEN_RIGHTBRACKET);
	ASSERT(Lexer_Next(lexer) == TOKEN_LEFTBRACKET);
	ASSERT(Lexer_Next(lexer) == TOKEN_RIGHTBRACKET);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShouldNotRecognizeNonSyntaxes)
{
	Lexer lexer = Setup("  \t  #syntaxx  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	lexer = Setup("  \t  #Syntax  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	lexer = Setup("  \t  #SYNTAX  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Break loanwords.

START_TEST(ShouldRecognizeBreak)
{
	Lexer lexer = Setup("  \t  #brk [print]  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_LOANWORD_BRK);
	ASSERT(Lexer_Next(lexer) == TOKEN_LEFTBRACKET);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "print", 5);
	ASSERT(Lexer_Next(lexer) == TOKEN_RIGHTBRACKET);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShouldNotRecognizeNonBreaks)
{
	Lexer lexer = Setup("  \t  #brkk  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	lexer = Setup("  \t  #Brk  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	lexer = Setup("  \t  #BRK  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);
}
END_TEST

#include "lexerloanword_tests.generated.inc"
