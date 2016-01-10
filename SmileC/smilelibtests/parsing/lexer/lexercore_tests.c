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

TEST_SUITE(LexerCoreTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Setup helper.

static Lexer Setup(const char *string)
{
	String source;
	Lexer lexer;

	Smile_ResetEnvironment();

	source = String_FromC(string);

	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	return lexer;
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
	Lexer lexer = Setup("  \t  \r  \n  \a  \b  \v  \f  ");

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
		"+\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_PLUS);
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
		"+\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_PLUS);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(HyphenSeparatorCommentsShouldBeSkipped)
{
	Lexer lexer = Setup("----------\r\n"
		".\r\n"
		"-----\r\n"
		"+\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_PLUS);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(EqualSeparatorCommentsShouldBeSkipped)
{
	Lexer lexer = Setup("==========\r\n"
		".\r\n"
		"=====\r\n"
		"+\r\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_PLUS);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Identifiers and punctuation.

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

	ASSERT(Lexer_Next(lexer) == TOKEN_PLUS);
	ASSERT(Lexer_Next(lexer) == TOKEN_MINUS);
	ASSERT(Lexer_Next(lexer) == TOKEN_SLASH);
	ASSERT(Lexer_Next(lexer) == TOKEN_STAR);
	ASSERT(Lexer_Next(lexer) == TOKEN_LT);
	ASSERT(Lexer_Next(lexer) == TOKEN_GT);
	ASSERT(Lexer_Next(lexer) == TOKEN_DOT);
	ASSERT(Lexer_Next(lexer) == TOKEN_PLUS);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
}
END_TEST

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

START_TEST(ShouldRecognizeSpecialCompoundPunctuation)
{
	Lexer lexer = Setup("<= >= != == === !== .. ... ##\n");

	ASSERT(Lexer_Next(lexer) == TOKEN_LE);
	ASSERT(Lexer_Next(lexer) == TOKEN_GE);
	ASSERT(Lexer_Next(lexer) == TOKEN_NE);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQ);
	ASSERT(Lexer_Next(lexer) == TOKEN_SUPEREQ);
	ASSERT(Lexer_Next(lexer) == TOKEN_SUPERNE);
	ASSERT(Lexer_Next(lexer) == TOKEN_RANGE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ELLIPSIS);
	ASSERT(Lexer_Next(lexer) == TOKEN_DOUBLEHASH);
}
END_TEST

START_TEST(ShouldRecognizeGeneralPunctuativeForms)
{
	Lexer lexer = Setup(
		"&& ^^ ** ++ -- +>> <<+ << >> <+> <-> <--> *~*\n"
		"&&= ^^= **= ++= --= +>>= <<+= <<= >>= <+>= <->= <-->= *~*=\n"
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

	ASSERT(Lexer_Next(lexer) == TOKEN_PLUS);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);

	ASSERT(Lexer_Next(lexer) == TOKEN_PLUS);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQ);

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
	ASSERT(Lexer_Next(lexer) == TOKEN_EQ);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "=&", 2);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);

	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "=&", 2);
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
		"y' = f**ck y\n"
		"z'' = pl-rk? z\n"
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
	ASSERT_STRING(lexer->token->text, "f**ck", 5);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "z''", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "pl-rk?", 6);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "z", 1);

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
		"x tan-1''= y\n"
		"x cot+1'''== y\n"
		"x sec*1\"=== y\n"
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
	ASSERT_STRING(lexer->token->text, "tan-1''", 7);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUALWITHOUTWHITESPACE);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "cot+1'''", 8);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQ);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "x", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "sec*1\"", 6);
	ASSERT(Lexer_Next(lexer) == TOKEN_SUPEREQ);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);
}
END_TEST

#include "lexercore_tests.generated.inc"
