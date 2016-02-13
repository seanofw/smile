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

TEST_SUITE(LexerUnicodeTests)

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
//  Unicode Latin identifiers.

START_TEST(ShouldRecognizeUnicodeFromTheLatinSupplementSet)
{
	// Try "Façade" as a name (alpha unicode character in a middle position).
	Lexer lexer = Setup("  \t  Fa\xC3\xA7\x61\x64\x65  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "Fa\xC3\xA7\x61\x64\x65", 7);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);

	// Try "être" as a name (alpha unicode character in an initial position).
	lexer = Setup("  \t  \xC3\xAAtre  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xC3\xAAtre", 5);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);

	// Try "trouvé" as a name (alpha unicode character in a final position).
	lexer = Setup("  \t  trouv\xC3\xA9  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "trouv\xC3\xA9", 7);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShouldRecognizeUnicodeFromTheLatinExtendedSets)
{
	// Try "ȓɂœķĦƕɏḜṻỿⱾ" as a name (alpha unicode characters from the Extended-A, Extended-B, and Extended Additional sets).
	Lexer lexer = Setup("  \t  \xC8\x93\xC9\x82\xC5\x93\xC4\xB7\xC4\xA6\xC6\x95\xC9\x8F\xE1\xB8\x9C\xE1\xB9\xBB\xE1\xBB\xBF  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xC8\x93\xC9\x82\xC5\x93\xC4\xB7\xC4\xA6\xC6\x95\xC9\x8F\xE1\xB8\x9C\xE1\xB9\xBB\xE1\xBB\xBF", 23);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unicode punctuation forms.

START_TEST(ShouldRecognizeUnicodeSuffixes)
{
	// Try "Façade°" as a name (Latin Unicode suffix character).
	Lexer lexer = Setup("  \t  Fa\xC3\xA7\x61\x64\x65\xC2\xB0  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "Fa\xC3\xA7\x61\x64\x65\xC2\xB0", 9);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);

	// Try "Façade†" as a name (Unicode Punctuation suffix character).
	lexer = Setup("  \t  Fa\xC3\xA7\x61\x64\x65\xE2\x80\xA0  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "Fa\xC3\xA7\x61\x64\x65\xE2\x80\xA0", 10);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);

	// Try "¡español!" and "¿español?" as a name (Unicode punctuation-like character as an alphabetic character).
	lexer = Setup("  \t  \xC2\xA1\x65spa\xC3\xB1ol!  \xC2\xBF\x65spa\xC3\xB1ol?  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xC2\xA1\x65spa\xC3\xB1ol!", 11);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xC2\xBF\x65spa\xC3\xB1ol?", 11);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShouldRecognizeUnicodePunctuationForms)
{
	// Try "∀" and "∃" and "∀∃∑∫" as operators (Unicode math symbols).
	Lexer lexer = Setup("  \t  \xE2\x88\x80  \xE2\x88\x83 \t \xE2\x88\x80\xE2\x88\x83\xE2\x88\x91\xE2\x88\xAB  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "\xE2\x88\x80", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "\xE2\x88\x83", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "\xE2\x88\x80\xE2\x88\x83\xE2\x88\x91\xE2\x88\xAB", 12);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);

	// Try "→" and "↠" and "→↠⇒⇛" as operators (Unicode arrows).
	lexer = Setup("  \t  \xE2\x86\x92  \xE2\x86\xA0 \t \xE2\x86\x92\xE2\x86\xA0\xE2\x87\x92\xE2\x87\x9B  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "\xE2\x86\x92", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "\xE2\x86\xA0", 3);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "\xE2\x86\x92\xE2\x86\xA0\xE2\x87\x92\xE2\x87\x9B", 12);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);

	// Try "!>→" and "→>!" as operators (mix Unicode and ASCII punctuation).
	lexer = Setup("  \t  !>\xE2\x86\x92  \xE2\x86\x92>!  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "!>\xE2\x86\x92", 5);
	ASSERT(Lexer_Next(lexer) == TOKEN_PUNCTNAME);
	ASSERT_STRING(lexer->token->text, "\xE2\x86\x92>!", 5);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unicode Greek forms.

START_TEST(ShouldRecognizeGreekNames)
{
	// Try "έτος" and "μικρός" as names (Greek "year" and "small").
	Lexer lexer = Setup("  \t \xCE\xAD\xCF\x84\xCE\xBF\xCF\x82  \xCE\xBC\xCE\xB9\xCE\xBA\xCF\x81\xCF\x8C\xCF\x82  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xCE\xAD\xCF\x84\xCE\xBF\xCF\x82", 8);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xCE\xBC\xCE\xB9\xCE\xBA\xCF\x81\xCF\x8C\xCF\x82", 12);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShouldRecognizeGreekNamesWithPunctuationSuffixes)
{
	// Try "έτος'" and "μικρός'" as names (Greek with trailing prime).
	Lexer lexer = Setup("  \t \xCE\xAD\xCF\x84\xCE\xBF\xCF\x82'  \xCE\xBC\xCE\xB9\xCE\xBA\xCF\x81\xCF\x8C\xCF\x82'  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xCE\xAD\xCF\x84\xCE\xBF\xCF\x82'", 9);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xCE\xBC\xCE\xB9\xCE\xBA\xCF\x81\xCF\x8C\xCF\x82'", 13);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShouldDisallowMixingGreekAndLatinInAName)
{
	// Try "έτοςabc" and "abcέτος" as names.
	Lexer lexer = Setup("  \t \xCE\xAD\xCF\x84\xCE\xBF\xCF\x82\x61\x62\x63  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	lexer = Setup("  \t abc\xCE\xAD\xCF\x84\xCE\xBF\xCF\x82  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unicode Cyrillic forms.

//-------------------------------------------------------------------------------------------------
//  Unicode Armenian forms.

//-------------------------------------------------------------------------------------------------
//  Unicode Hebrew forms.

START_TEST(ShouldRecognizeHebrewNames)
{
	// Try "אבא" and "סדין" as names (Hebrew "aba," or "father," and "sadin," or "sheet").
	Lexer lexer = Setup("  \t  \xD7\x90\xD7\x91\xD7\x90  \xD7\xA1\xD7\x93\xD7\x99\xD7\x9F  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xD7\x90\xD7\x91\xD7\x90", 6);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xD7\xA1\xD7\x93\xD7\x99\xD7\x9F", 8);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShouldRecognizeHebrewNamesWithPunctuationSuffixes)
{
	// Try "אבא'" and "סדין'" as names (Hebrew words with trailing ' characters).
	Lexer lexer = Setup("  \t  \xD7\x90\xD7\x91\xD7\x90'  \xD7\xA1\xD7\x93\xD7\x99\xD7\x9F'  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xD7\x90\xD7\x91\xD7\x90'", 7);
	ASSERT(Lexer_Next(lexer) == TOKEN_ALPHANAME);
	ASSERT_STRING(lexer->token->text, "\xD7\xA1\xD7\x93\xD7\x99\xD7\x9F'", 9);
	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

START_TEST(ShouldDisallowMixingHebrewAndLatinInAName)
{
	// Try "אבאaba" as a name.
	Lexer lexer = Setup("  \t  \xD7\x90\xD7\x91\xD7\x90\x61\x62\x61  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);

	// Try "abaאבא" as a name.
	lexer = Setup("  \t  aba\xD7\x90\xD7\x91\xD7\x90  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_ERROR);
}
END_TEST

#include "lexerunicode_tests.generated.inc"
