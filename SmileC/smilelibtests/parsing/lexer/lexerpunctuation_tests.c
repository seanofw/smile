//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

#include "../../stdafx.h"

#include <smile/parsing/lexer.h>

TEST_SUITE(LexerPunctuationTests)

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
//  Punctuation forms.

START_TEST(ShouldRecognizePrimitiveSingleCharacterTokens)
{
	Lexer lexer = Setup(", ; { } [ ] ( ) : ` |");

	ASSERT(Lexer_Next(lexer) == TOKEN_COMMA);
	ASSERT(Lexer_Next(lexer) == TOKEN_SEMICOLON);
	ASSERT(Lexer_Next(lexer) == TOKEN_LEFTBRACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_RIGHTBRACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_LEFTBRACKET);
	ASSERT(Lexer_Next(lexer) == TOKEN_RIGHTBRACKET);
	ASSERT(Lexer_Next(lexer) == TOKEN_LEFTPARENTHESIS);
	ASSERT(Lexer_Next(lexer) == TOKEN_RIGHTPARENTHESIS);
	ASSERT(Lexer_Next(lexer) == TOKEN_COLON);
	ASSERT(Lexer_Next(lexer) == TOKEN_BACKTICK);
	ASSERT(Lexer_Next(lexer) == TOKEN_BAR);
}
END_TEST

START_TEST(ShouldRecognizeSpecialSingleCharacterTokens)
{
	Lexer lexer = Setup("+ - / * < > .\n"
		"+= = =");

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.plus);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.minus);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.slash);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.star);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.lt);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.gt);
	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.plus);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
}
END_TEST

START_TEST(ShouldRecognizeSingleCharacterPunctIdents)
{
	Lexer lexer = Setup("! % ^ & ~ ?= %= ^= &= ~=\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "!", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "%", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "^", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "&", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "~", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "?", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "%", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "^", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "&", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "~", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Multi-character punctuation forms.

START_TEST(ShouldRecognizeSpecialCompoundPunctuation)
{
	Lexer lexer = Setup("<= >= != == === !== .. ... ##\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.le);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.ge);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.ne);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.eq);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.supereq_);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.superne_);
	ASSERT(Lexer_Next(lexer) == TOKEN_DOTDOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_DOTDOTDOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_DOUBLEHASH);
}
END_TEST

START_TEST(ShouldRecognizeGeneralPunctuativeForms)
{
	Lexer lexer = Setup(
		"&& ^^ ** ++ -- +>> <<+ << >> <+> <-> <--> *~* /-/\n"
		"&&= ^^= **= ++= --= +>>= <<+= <<= >>= <+>= <->= <-->= *~*= /-/=\n"
		);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "&&", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "^^", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "**", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "++", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "--", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "+>>", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<<+", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<<", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, ">>", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<+>", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<->", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<-->", 4);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "*~*", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "/-/", 3);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "&&", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "^^", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "**", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "++", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "--", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "+>>", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<<+", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<<", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, ">>", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<+>", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<->", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "<-->", 4);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "*~*", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "/-/", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
}
END_TEST

START_TEST(ShouldHandleWeirdPunctuationCornerCases)
{
	Lexer lexer = Setup(
		"+==+ += +== =+= =+\n"
		"&==& &= &== =&= =&\n"
		);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "+==+", 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.plus);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.plus);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.eq);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "=+", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "=+", 2);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "&==&", 4);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "&", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "&", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT(lexer->token->data.symbol == Smile_KnownSymbols.eq);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "=&", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "=&", 2);
}
END_TEST

#include "lexerpunctuation_tests.generated.inc"
