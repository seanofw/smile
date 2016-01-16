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
//  Single-character punctuation.

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

// TODO: Un-special most of the keywords. Smile was created to avoid keywords and special
// constructs, and here we are with nearly twenty of them.  That's still eleven fewer than
// C's sparse set, but about 95% more than the initial design goal. >_<
static const char *_reservedKeywords[] = {
	"and", "catch", "do", "else",
	"if", "is", "new", "not",
	"or", "return", "then", "till",
	"try", "typeof", "unless", "until",
	"var", "when", "while"
};
static int _reservedKeywordTokens[] = {
	TOKEN_AND, TOKEN_CATCH, TOKEN_DO, TOKEN_ELSE,
	TOKEN_IF, TOKEN_IS, TOKEN_NEW, TOKEN_NOT,
	TOKEN_OR, TOKEN_RETURN, TOKEN_THEN, TOKEN_TILL,
	TOKEN_TRY, TOKEN_TYPEOF, TOKEN_UNLESS, TOKEN_UNTIL,
	TOKEN_VAR, TOKEN_WHEN, TOKEN_WHILE,
};

START_TEST(ShouldRecognizeKeywords)
{
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_reservedKeywords) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format(" \t %s \n ", _reservedKeywords[i]));

		ASSERT(Lexer_Next(lexer) == _reservedKeywordTokens[i]);
		ASSERT_STRING(lexer->token->text, _reservedKeywords[i], strlen(_reservedKeywords[i]));
	}
}
END_TEST

START_TEST(ShouldNotRecognizeEmbeddedKeywords)
{
	String expected;
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_reservedKeywords) / sizeof(const char *); i++) {

		// Try it with a prefix, like "xxxif".
		expected = String_Format("xxx%s", _reservedKeywords[i]);
		lexer = SetupString(String_Format(" \t %S \n ", expected));

		ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
		ASSERT_STRING(lexer->token->text, (const char *)String_GetBytes(expected), (int)String_Length(expected));

		// Try it with a suffix, like "ifxxx".
		expected = String_Format("%sxxx", _reservedKeywords[i]);
		lexer = SetupString(String_Format(" \t %S \n ", expected));

		ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
		ASSERT_STRING(lexer->token->text, (const char *)String_GetBytes(expected), (int)String_Length(expected));

		// Try it outright embedded, like "xxxifxxx".
		expected = String_Format("xxx%sxxx", _reservedKeywords[i]);
		lexer = SetupString(String_Format(" \t %S \n ", expected));

		ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
		ASSERT_STRING(lexer->token->text, (const char *)String_GetBytes(expected), (int)String_Length(expected));

		// Try it with a punctuation suffix, like "if'".
		expected = String_Format("%s'", _reservedKeywords[i]);
		lexer = SetupString(String_Format(" \t %S \n ", expected));

		ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
		ASSERT_STRING(lexer->token->text, (const char *)String_GetBytes(expected), (int)String_Length(expected));
	}
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Raw string forms.

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
	ASSERT(lexer->token->data.i == 'z');

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "y", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == '\n');

	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "z", 1);
	ASSERT(Lexer_Next(lexer) == TOKEN_EQUAL);
	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == '\t');

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 0);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 26);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 32);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 160);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 13);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 7);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 12);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 11);

	ASSERT(Lexer_Next(lexer) == TOKEN_CHAR);
	ASSERT(lexer->token->data.i == 255);
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

//-------------------------------------------------------------------------------------------------
//  Number forms.

#include "lexercore_tests.generated.inc"
