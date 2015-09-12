//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

#include "../stdafx.h"

TEST_SUITE(StringExtraTests)

//-------------------------------------------------------------------------------------------------
//  Split tests.

START_TEST(SplitHandlesEmptyStrings)
{
	String *pieces;
	Int numPieces;

	numPieces = String_SplitWithOptions(String_FromC(""), String_FromC(";"), 0, 0, &pieces);

	ASSERT(numPieces == 1);
	ASSERT_STRING(pieces[0], NULL, 0);
}
END_TEST

START_TEST(SplitExplodesTheCharactersIfThePatternIsEmpty)
{
	String *pieces;
	Int numPieces;

	numPieces = String_SplitWithOptions(String_FromC("This is a test."), String_FromC(""), 0, 0, &pieces);

	ASSERT(numPieces == 15);
	ASSERT_STRING(pieces[0], "T", 1);
	ASSERT_STRING(pieces[1], "h", 1);
	ASSERT_STRING(pieces[2], "i", 1);
	ASSERT_STRING(pieces[3], "s", 1);
	ASSERT_STRING(pieces[4], " ", 1);
	ASSERT_STRING(pieces[5], "i", 1);
	ASSERT_STRING(pieces[6], "s", 1);
	ASSERT_STRING(pieces[7], " ", 1);
	ASSERT_STRING(pieces[8], "a", 1);
	ASSERT_STRING(pieces[9], " ", 1);
	ASSERT_STRING(pieces[10], "t", 1);
	ASSERT_STRING(pieces[11], "e", 1);
	ASSERT_STRING(pieces[12], "s", 1);
	ASSERT_STRING(pieces[13], "t", 1);
	ASSERT_STRING(pieces[14], ".", 1);
}
END_TEST

START_TEST(SplitCanSplitUpStrings)
{
	String *pieces;
	Int numPieces;

	numPieces = String_SplitWithOptions(String_FromC("This is a test."), String_FromC(" "), 0, 0, &pieces);

	ASSERT(numPieces == 4);
	ASSERT_STRING(pieces[0], "This", 4);
	ASSERT_STRING(pieces[1], "is", 2);
	ASSERT_STRING(pieces[2], "a", 1);
	ASSERT_STRING(pieces[3], "test.", 5);
}
END_TEST

START_TEST(SplitCanSplitUpStringsUsingALargeSplitterString)
{
	String *pieces;
	Int numPieces;

	numPieces = String_SplitWithOptions(String_FromC("testers testtested testing testworthy."), String_FromC("test"), 0, 0, &pieces);

	ASSERT(numPieces == 6);
	ASSERT_STRING(pieces[0], NULL, 0);
	ASSERT_STRING(pieces[1], "ers ", 4);
	ASSERT_STRING(pieces[2], NULL, 0);
	ASSERT_STRING(pieces[3], "ed ", 3);
	ASSERT_STRING(pieces[4], "ing ", 4);
	ASSERT_STRING(pieces[5], "worthy.", 7);
}
END_TEST

START_TEST(SplitDoesntCutOffWhenTheLimitIsLarge)
{
	String *pieces;
	Int numPieces;

	numPieces = String_SplitWithOptions(String_FromC("testers testtested testing testworthy."), String_FromC("test"), 10, 0, &pieces);

	ASSERT(numPieces == 6);
	ASSERT_STRING(pieces[0], NULL, 0);
	ASSERT_STRING(pieces[1], "ers ", 4);
	ASSERT_STRING(pieces[2], NULL, 0);
	ASSERT_STRING(pieces[3], "ed ", 3);
	ASSERT_STRING(pieces[4], "ing ", 4);
	ASSERT_STRING(pieces[5], "worthy.", 7);
}
END_TEST

START_TEST(SplitCanDiscardEmptyStrings)
{
	String *pieces;
	Int numPieces;

	numPieces = String_SplitWithOptions(String_FromC("testers testtested testing testworthy."), String_FromC("test"), 0, StringSplitOptions_RemoveEmptyEntries, &pieces);

	ASSERT(numPieces == 4);
	ASSERT_STRING(pieces[0], "ers ", 4);
	ASSERT_STRING(pieces[1], "ed ", 3);
	ASSERT_STRING(pieces[2], "ing ", 4);
	ASSERT_STRING(pieces[3], "worthy.", 7);
}
END_TEST

START_TEST(SplitCutsOffAtTheLimit)
{
	String *pieces;
	Int numPieces;

	numPieces = String_SplitWithOptions(String_FromC("testers testtested testing testworthy."), String_FromC("test"), 4, 0, &pieces);

	ASSERT(numPieces == 4);
	ASSERT_STRING(pieces[0], NULL, 0);
	ASSERT_STRING(pieces[1], "ers ", 4);
	ASSERT_STRING(pieces[2], NULL, 0);
	ASSERT_STRING(pieces[3], "ed ", 3);
}
END_TEST

START_TEST(SplitCutsOffAtTheLimitWhenDiscardingEmptyStrings)
{
	String *pieces;
	Int numPieces;

	numPieces = String_SplitWithOptions(String_FromC("testers testtested testing testworthy."), String_FromC("test"), 3, StringSplitOptions_RemoveEmptyEntries, &pieces);

	ASSERT(numPieces == 3);
	ASSERT_STRING(pieces[0], "ers ", 4);
	ASSERT_STRING(pieces[1], "ed ", 3);
	ASSERT_STRING(pieces[2], "ing ", 4);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Extra-function tests.

START_TEST(RawReverseReordersText)
{
	ASSERT_STRING(String_RawReverse(String_Empty), NULL, 0);
	ASSERT_STRING(String_RawReverse(String_FromC("This is a test.")), ".tset a si sihT", 15);
	ASSERT_STRING(String_RawReverse(String_Create("\xEF\xBB\xBFThis\xC2\xA0is a\0test.", 19)), ".tset\0a si\xA0\xC2sihT\xBF\xBB\xEF", 19);
}
END_TEST

START_TEST(ReverseReordersTextButPreservesCharacters)
{
	ASSERT_STRING(String_Reverse(String_Empty), NULL, 0);
	ASSERT_STRING(String_Reverse(String_FromC("This is a test.")), ".tset a si sihT", 15);
	ASSERT_STRING(String_Reverse(String_Create("\xEF\xBB\xBFThis\xC2\xA0is a\0test.", 19)), ".tset\0a si\xC2\xA0sihT\xEF\xBB\xBF", 19);
}
END_TEST

START_TEST(RepeatClonesStrings)
{
	ASSERT_STRING(String_Repeat(String_Empty, 10), NULL, 0);
	ASSERT_STRING(String_Repeat(String_FromC("test:"), 0), NULL, 0);
	ASSERT_STRING(String_Repeat(String_FromC("test:"), 1), "test:", 5);
	ASSERT_STRING(String_Repeat(String_FromC("test:"), 2), "test:test:", 10);
	ASSERT_STRING(String_Repeat(String_FromC("test:"), 10), "test:test:test:test:test:test:test:test:test:test:", 50);
}
END_TEST

START_TEST(PadStartPadsTheStartOfShortStrings)
{
	String str = String_FromC("This is a test.");

	ASSERT_STRING(String_PadStart(String_Empty, 0, '.'), NULL, 0);
	ASSERT_STRING(String_PadStart(String_Empty, 10, '.'), "..........", 10);

	ASSERT_STRING(String_PadStart(str, 0, ':'), "This is a test.", 15);
	ASSERT_STRING(String_PadStart(str, 10, ':'), "This is a test.", 15);
	ASSERT_STRING(String_PadStart(str, 15, ':'), "This is a test.", 15);
	ASSERT_STRING(String_PadStart(str, 16, ':'), ":This is a test.", 16);
	ASSERT_STRING(String_PadStart(str, 20, ':'), ":::::This is a test.", 20);
}
END_TEST

START_TEST(PadEndPadsTheEndOfShortStrings)
{
	String str = String_FromC("This is a test.");

	ASSERT_STRING(String_PadEnd(String_Empty, 0, '.'), NULL, 0);
	ASSERT_STRING(String_PadEnd(String_Empty, 10, '.'), "..........", 10);

	ASSERT_STRING(String_PadEnd(str, 0, ':'), "This is a test.", 15);
	ASSERT_STRING(String_PadEnd(str, 10, ':'), "This is a test.", 15);
	ASSERT_STRING(String_PadEnd(str, 15, ':'), "This is a test.", 15);
	ASSERT_STRING(String_PadEnd(str, 16, ':'), "This is a test.:", 16);
	ASSERT_STRING(String_PadEnd(str, 20, ':'), "This is a test.:::::", 20);
}
END_TEST

START_TEST(TrimEndRemovesWhitespaceFromTheEndOfStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_Create(" \x1A\t\r\n\b\1\0This \x1A\t\r\n\b\1\0is \x1A\t\r\n\b\1\0a \x1A\t\r\n\b\1\0test. \x1A\t\r\n\b\1\0", 52);
	String str3 = String_Create(" \x1A\t\r\n\b\1\0This \x1A\t\r\n\b\1\0is \x1A\t\r\n\b\1\0a \x1A\t\r\n\b\1\0test.", 44);

	ASSERT_STRING(String_TrimEnd(str1), String_GetBytes(str1), String_Length(str1));
	ASSERT_STRING(String_TrimEnd(str2), String_GetBytes(str3), String_Length(str3));
}
END_TEST

START_TEST(TrimStartRemovesWhitespaceFromTheStartOfStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_Create(" \x1A\t\r\n\b\1\0This \x1A\t\r\n\b\1\0is \x1A\t\r\n\b\1\0a \x1A\t\r\n\b\1\0test. \x1A\t\r\n\b\1\0", 52);
	String str3 = String_Create("This \x1A\t\r\n\b\1\0is \x1A\t\r\n\b\1\0a \x1A\t\r\n\b\1\0test. \x1A\t\r\n\b\1\0", 44);

	ASSERT_STRING(String_TrimStart(str1), String_GetBytes(str1), String_Length(str1));
	ASSERT_STRING(String_TrimStart(str2), String_GetBytes(str3), String_Length(str3));
}
END_TEST

START_TEST(TrimRemovesWhitespaceFromBothEndsOfStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_Create(" \x1A\t\r\n\b\1\0This \x1A\t\r\n\b\1\0is \x1A\t\r\n\b\1\0a \x1A\t\r\n\b\1\0test. \x1A\t\r\n\b\1\0", 52);
	String str3 = String_Create("This \x1A\t\r\n\b\1\0is \x1A\t\r\n\b\1\0a \x1A\t\r\n\b\1\0test.", 36);

	ASSERT_STRING(String_Trim(str1), String_GetBytes(str1), String_Length(str1));
	ASSERT_STRING(String_Trim(str2), String_GetBytes(str3), String_Length(str3));
}
END_TEST

START_TEST(CompactWhitespaceResultsInTrimmingAndSingleSpaceCharactersInStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_Create(" \x1A\t \r\n \b\1\0This \x1A\t \r\n \b\1\0is \x1A\t \r\n \b\1\0a \x1A\t \r\n \b\1\0test. \x1A\t \r\n \b\1\0", 62);

	ASSERT_STRING(String_CompactWhitespace(str1), String_GetBytes(str1), String_Length(str1));
	ASSERT_STRING(String_CompactWhitespace(str2), String_GetBytes(str1), String_Length(str1));
}
END_TEST

START_TEST(AddCSlashesMakesStringsSafe)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_Create(" \x1A\t \r\n \b\x01\x00 This is a test. \x1A\t \r\n \b\x01\x00 ", 37);
	String str3 = String_Create(" \\x1A\\t \\r\\n \\b\\x01\\x00 This is a test. \\x1A\\t \\r\\n \\b\\x01\\x00 ", 63);

	ASSERT_STRING(String_AddCSlashes(str1), String_GetBytes(str1), String_Length(str1));
	ASSERT_STRING(String_AddCSlashes(str2), String_GetBytes(str3), String_Length(str3));
}
END_TEST

START_TEST(StripCSlashesDecodesStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_Create(" \x1A\t \r\n \b\x01\x00 This is a test. \x1A\t \r\n \b\x0F\x00 ", 37);
	String str3 = String_Create(" \\x1A\\t \\r\\n \\b\\x01\\x00 This is a test. \\x1A\\t \\r\\n \\b\\15\\0 ", 60);

	ASSERT_STRING(String_StripCSlashes(str1), String_GetBytes(str1), String_Length(str1));
	ASSERT_STRING(String_StripCSlashes(str3), String_GetBytes(str2), String_Length(str2));
}
END_TEST

START_TEST(AddCSlashesEncodesSpecialCharactersSpecially)
{
	static const Byte specialChars[] = { '\a', '\b', '\t', '\n', '\v', '\f', '\r', '\'', '\"', '\\' };
	static const Byte *encodings[] = { "\\a", "\\b", "\\t", "\\n", "\\v", "\\f", "\\r", "\\'", "\\\"", "\\\\" };
	Int i;
	String str, encodedResult;

	for (i = 0; i < sizeof(specialChars) / sizeof(const Byte); i++) {
		str = String_Create(&specialChars[i], 1);
		encodedResult = String_AddCSlashes(str);

		ASSERT_STRING(encodedResult, encodings[i], StrLen(encodings[i]));
	}
}
END_TEST

START_TEST(StripCSlashesDecodesSpecialCharacters)
{
	static const Byte specialChars[] = { '\a', '\b', '\t', '\n', '\v', '\f', '\r', '\'', '\"', '\\' };
	static const Byte *encodings[] = { "\\a", "\\b", "\\t", "\\n", "\\v", "\\f", "\\r", "\\'", "\\\"", "\\\\" };
	Int i;
	String str, decodedResult;

	for (i = 0; i < sizeof(specialChars) / sizeof(const Byte); i++) {
		str = String_Create(encodings[i], 2);
		decodedResult = String_StripCSlashes(str);

		ASSERT_STRING(decodedResult, &specialChars[i], 1);
	}
}
END_TEST

START_TEST(AddCSlashesEncodesNonAsciiCharactersAsHexCodes)
{
	static const Byte specialChars[] = {
		'\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06',
		                                                '\x0E', '\x0F',

		'\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
		'\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F',

		                                                        '\x7F',

		'\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87',
		'\x88', '\x89', '\x8A', '\x8B', '\x8C', '\x8D', '\x8E', '\x8F',
		'\x90', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96', '\x97',
		'\x98', '\x99', '\x9A', '\x9B', '\x9C', '\x9D', '\x9E', '\x9F',

		'\xA0', '\xA1', '\xA2', '\xA3', '\xA4', '\xA5', '\xA6', '\xA7',
		'\xA8', '\xA9', '\xAA', '\xAB', '\xAC', '\xAD', '\xAE', '\xAF',
		'\xB0', '\xB1', '\xB2', '\xB3', '\xB4', '\xB5', '\xB6', '\xB7',
		'\xB8', '\xB9', '\xBA', '\xBB', '\xBC', '\xBD', '\xBE', '\xBF',

		'\xC0', '\xC1', '\xC2', '\xC3', '\xC4', '\xC5', '\xC6', '\xC7',
		'\xC8', '\xC9', '\xCA', '\xCB', '\xCC', '\xCD', '\xCE', '\xCF',
		'\xD0', '\xD1', '\xD2', '\xD3', '\xD4', '\xD5', '\xD6', '\xD7',
		'\xD8', '\xD9', '\xDA', '\xDB', '\xDC', '\xDD', '\xDE', '\xDF',

		'\xE0', '\xE1', '\xE2', '\xE3', '\xE4', '\xE5', '\xE6', '\xE7',
		'\xE8', '\xE9', '\xEA', '\xEB', '\xEC', '\xED', '\xEE', '\xEF',
		'\xF0', '\xF1', '\xF2', '\xF3', '\xF4', '\xF5', '\xF6', '\xF7',
		'\xF8', '\xF9', '\xFA', '\xFB', '\xFC', '\xFD', '\xFE', '\xFF',
	};

	Int i;
	String str, encodedResult;
	Byte expectedResult[8];

	for (i = 0; i < sizeof(specialChars) / sizeof(const Byte); i++) {
		str = String_Create(&specialChars[i], 1);
		encodedResult = String_AddCSlashes(str);
		sprintf((char *)expectedResult, "\\x%02X", specialChars[i]);

		ASSERT_STRING(encodedResult, expectedResult, 4);
	}
}
END_TEST

START_TEST(StripCSlashesDecodesAllHexCodes)
{
	String str1;
	Int i;
	Byte buffer1[10], buffer2[10];

	for (i = 0; i < 16; i++) {
		sprintf((char *)buffer1, "\\x%X", (int)i);
		buffer2[0] = (Byte)i;
		str1 = String_Create(buffer1, 3);
		ASSERT_STRING(String_StripCSlashes(str1), buffer2, 1);
	}

	for (i = 0; i < 16; i++) {
		sprintf((char *)buffer1, "\\x%x", (int)i);
		buffer2[0] = (Byte)i;
		str1 = String_Create(buffer1, 3);
		ASSERT_STRING(String_StripCSlashes(str1), buffer2, 1);
	}

	for (i = 0; i < 256; i++) {
		sprintf((char *)buffer1, "\\x%02X", (int)i);
		buffer2[0] = (Byte)i;
		str1 = String_Create(buffer1, 4);
		ASSERT_STRING(String_StripCSlashes(str1), buffer2, 1);
	}

	for (i = 0; i < 256; i++) {
		sprintf((char *)buffer1, "\\x%02x", (int)i);
		buffer2[0] = (Byte)i;
		str1 = String_Create(buffer1, 4);
		ASSERT_STRING(String_StripCSlashes(str1), buffer2, 1);
	}
}
END_TEST

START_TEST(StripCSlashesDecodesAllDecimalCodes)
{
	String str1;
	Int i;
	Byte buffer1[10], buffer2[10];

	for (i = 0; i < 256; i++) {
		sprintf((char *)buffer1, "\\%u", (int)i);
		buffer2[0] = (Byte)i;
		str1 = String_Create(buffer1, StrLen((const char *)buffer1));
		ASSERT_STRING(String_StripCSlashes(str1), buffer2, 1);
	}

	for (i = 0; i < 256; i++) {
		sprintf((char *)buffer1, "\\%02u", (int)i);
		buffer2[0] = (Byte)i;
		str1 = String_Create(buffer1, StrLen((const char *)buffer1));
		ASSERT_STRING(String_StripCSlashes(str1), buffer2, 1);
	}

	for (i = 0; i < 256; i++) {
		sprintf((char *)buffer1, "\\%03u", (int)i);
		buffer2[0] = (Byte)i;
		str1 = String_Create(buffer1, StrLen((const char *)buffer1));
		ASSERT_STRING(String_StripCSlashes(str1), buffer2, 1);
	}
}
END_TEST

START_TEST(Rot13EncodesTextWithEpicL33tness)
{
	String str = String_FromC("PACK MY BOX WITH FIVE DOZEN LIQUOR JUGS.  pack my box with five dozen liquor jugs.");
	const char *expectedResult = "CNPX ZL OBK JVGU SVIR QBMRA YVDHBE WHTF.  cnpx zl obk jvgu svir qbmra yvdhbe whtf.";

	ASSERT_STRING(String_Rot13(str), expectedResult, StrLen(expectedResult));
	ASSERT_STRING(String_Rot13(String_FromC(expectedResult)), String_GetBytes(str), String_Length(str));
}
END_TEST

START_TEST(RegexEscapeAddsBackslashesToRegexPunctuationMarks)
{
	const Byte escapablePunctuation[] = {
		'\\', '*', '+', '?', '|', '{', '}', '[', ']', '(', ')', '^', '$', '.', '#',
	};
	char buffer[64];
	String str;
	Int i;

	for (i = 0; i < sizeof(escapablePunctuation) / sizeof(Byte); i++) {
		sprintf(buffer, "<This is a %c test.>", escapablePunctuation[i]);
		str = String_FromC(buffer);
		sprintf(buffer, "<This is a \\%c test\\.>", escapablePunctuation[i]);
		ASSERT_STRING(String_RegexEscape(str), buffer, StrLen(buffer));
	}
}
END_TEST

START_TEST(RegexEscapeAddsBackslashesToControlCodes)
{
	char specialchars[] = {
		'a', 'b', 't', 'n', 'v', 'f', 'r',
	};

	char buffer[64];
	String str;
	int i;

	for (i = 0; i < 32; i++) {
		sprintf(buffer, "<This is a %c test.>", (char)i);
		str = String_Create(buffer, 19);

		if (i < 7 || i > 13) {
			sprintf(buffer, "<This is a \\x%02X test\\.>", i);
		}
		else {
			sprintf(buffer, "<This is a \\%c test\\.>", specialchars[i - 7]);
		}

		ASSERT_STRING(String_RegexEscape(str), buffer, StrLen(buffer));
	}
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Wildcard-match tests.

typedef struct {
	Bool expectedResult;
	const char *pattern;
	const char *text;
} WildcardTestPattern;

static WildcardTestPattern WildcardTestPatterns[] = {

	// Simple suffix patterns.
	{ True, "*.txt", ".txt" },
	{ True, "*.txt", "test.txt" },
	{ True, "*.txt", "flarb.txt" },
	{ False, "*.txt", ".tx" },
	{ False, "*.txt", "txt" },
	{ False, "*.txt", "image.png" },
	{ False, "*.txt", "flarb.txt.foo" },
	{ False, "*.txt", ".txt.test" },

	// Simple prefix patterns.
	{ True, "read*", "read" },
	{ True, "read*", "read.me" },
	{ True, "read*", "readme.txt" },
	{ False, "read*", "rea" },
	{ False, "read*", "ead.me" },
	{ False, "read*", "image.png" },
	{ False, "read*", "dontread" },
	{ False, "read*", "dontread.me" },

	// Simple bounded patterns.
	{ True, "read*.txt", "read.txt" },
	{ True, "read*.txt", "read.me.txt" },
	{ True, "read*.txt", "readme.txt" },
	{ False, "read*.txt", "read" },
	{ False, "read*.txt", ".txt" },
	{ False, "read*.txt", "image.png" },
	{ False, "read*.txt", "dontread.txt" },
	{ False, "read*.txt", "read.txt.foo" },

	// Patterns with floating internal content.
	{ True, "read*me*.txt", "readme.txt" },
	{ True, "read*me*.txt", "read-me-v2.txt" },
	{ True, "read*me*.txt", "readmeme.txt" },
	{ True, "read*me*.txt", "readreadme.txt.txt" },
	{ False, "read*me*.txt", "read.txt" },
	{ False, "read*me*.txt", "readme" },
	{ False, "read*me*.txt", "me.txt" },
	{ False, "read*me*.txt", "readmetxt" },
	{ False, "read*me*.txt", "read.metxt" },
	{ False, "read*me*.txt", "read-me.txt." },
	{ False, "read*me*.txt", ".read-me.txt" },

	// Patterns with required unknown trailing characters.
	{ True, "read.???", "read.txt" },
	{ True, "read.???", "read.png" },
	{ True, "read.???", "read..sm" },
	{ False, "read.???", "read.sm" },
	{ False, "read.???", "read.text" },
	{ False, "read.???", "ream.txt" },
	{ False, "read.???", "soup.txt" },

	// Patterns with required unknown prefix characters.
	{ True, "????.txt", "read.txt" },
	{ True, "????.txt", "soup.txt" },
	{ True, "????.txt", "nuts.txt" },
	{ True, "????.txt", ".txt.txt" },
	{ False, "????.txt", "nut.txt" },
	{ False, "????.txt", "read.tx" },
	{ False, "????.txt", "r.txt.tx" },
	{ False, "????.txt", "gronk.txt" },
	{ False, "????.txt", "txt.read" },

	// Patterns with middle unknown characters.
	{ True, "read?.txt", "read1.txt" },
	{ True, "read?.txt", "read2.txt" },
	{ True, "read?.txt", "reads.txt" },
	{ True, "read?.txt", "read..txt" },
	{ False, "read?.txt", "readme.txt" },
	{ False, "read?.txt", "read1.tx" },
	{ False, "read?.txt", "read.txt" },

	// Patterns with "?*" wildcards.
	{ True, "read?*.txt", "read1.txt" },
	{ True, "read?*.txt", "read2.txt" },
	{ True, "read?*.txt", "reads.txt" },
	{ True, "read?*.txt", "read..txt" },
	{ True, "read?*.txt", "readme.txt" },
	{ False, "read?*.txt", "read1.tx" },
	{ False, "read?*.txt", "read.txt" },

	// Pathological cases.
	{ True, "*me*me*me*", "mememe" },
	{ True, "*me*me*me*", "emme mime meem" },
	{ False, "*me*me*me*", "emme mim meem" },
	{ False, "*me*me*me*", "emm mime meem" },
	{ False, "*me*me*me*", "emme mime emmim" },
	{ True, "*mem*mem*mem*", "memmemmem" },
	{ True, "*mem*mem*mem*", "  mem  mem  mem  " },
	{ True, "*mem*mem*mem*", "  mem  memmem  " },
	{ False, "*mem*mem*mem*", "  mem  memem  " },
};

static WildcardTestPattern WildcardPathTestPatterns[] = {
	{ True, "*.txt", ".txt" },
	{ True, "*.txt", "test.txt" },
	{ False, "*.txt", "foo/test.txt" },
	{ True, "*/*.txt", "foo/test.txt" },
	{ False, "*/*.txt", "footest.txt" },
	{ True, "foo/*.txt", "foo/test.txt" },
	{ True, "foo*/*.txt", "foobar/test.txt" },
	{ False, "foo*/*.txt", "foo//test.txt" },
	{ False, "foo*/*.txt", "foobar//test.txt" },
	{ True, "foo*/*.txt", "foobar/test.txt" },
};

static WildcardTestPattern WildcardEscapeTestPatterns[] = {
	{ True, "*.txt", ".txt" },
	{ False, "\\*.txt", ".txt" },
	{ True, "\\*.txt", "*.txt" },
	{ True, "*.txt", "test.txt" },
	{ False, "*\\*.txt", "test.txt" },
	{ True, "*\\*.txt", "test*.txt" },
	{ True, "\\**.txt", "*test.txt" },
	{ False, "\\**.txt", "test.txt" },
};

START_TEST(WildcardMatchWorksInNormalMode)
{
	WildcardTestPattern *wildcardTestPattern;
	const int numPatterns = sizeof(WildcardTestPatterns) / sizeof(WildcardTestPattern);
	int i;
	Bool result;
	String pattern;

	for (i = 0; i < numPatterns; i++) {
		wildcardTestPattern = &WildcardTestPatterns[i];
		pattern = String_FromC(wildcardTestPattern->pattern);
		result = String_WildcardMatch(pattern, String_FromC(wildcardTestPattern->text), 0);
		ASSERT(result == wildcardTestPattern->expectedResult);
	}
}
END_TEST

START_TEST(WildcardMatchWorksCaseInsensitive)
{
	WildcardTestPattern *wildcardTestPattern;
	const int numPatterns = sizeof(WildcardTestPatterns) / sizeof(WildcardTestPattern);
	int i;
	Bool result;
	String pattern;

	for (i = 0; i < numPatterns; i++) {
		wildcardTestPattern = &WildcardTestPatterns[i];
		pattern = String_ToUpper(String_FromC(wildcardTestPattern->pattern));
		result = String_WildcardMatch(pattern, String_FromC(wildcardTestPattern->text), StringWildcardOptions_CaseInsensitive);
		ASSERT(result == wildcardTestPattern->expectedResult);
	}
}
END_TEST

START_TEST(WildcardMatchWorksInFilenameMode)
{
	WildcardTestPattern *wildcardTestPattern;
	const int numPatterns = sizeof(WildcardPathTestPatterns) / sizeof(WildcardTestPattern);
	int i;
	Bool result;
	String pattern;

	for (i = 0; i < numPatterns; i++) {
		wildcardTestPattern = &WildcardPathTestPatterns[i];
		pattern = String_FromC(wildcardTestPattern->pattern);
		result = String_WildcardMatch(pattern, String_FromC(wildcardTestPattern->text), StringWildcardOptions_FilenameMode);
		ASSERT(result == wildcardTestPattern->expectedResult);
	}
}
END_TEST

START_TEST(WildcardMatchWorksInEscapeMode)
{
	WildcardTestPattern *wildcardTestPattern;
	const int numPatterns = sizeof(WildcardEscapeTestPatterns) / sizeof(WildcardTestPattern);
	int i;
	Bool result;
	String pattern;

	for (i = 0; i < numPatterns; i++) {
		wildcardTestPattern = &WildcardEscapeTestPatterns[i];
		pattern = String_FromC(wildcardTestPattern->pattern);
		result = String_WildcardMatch(pattern, String_FromC(wildcardTestPattern->text), StringWildcardOptions_BackslashEscapes);

		ASSERT(result == wildcardTestPattern->expectedResult);
	}
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  English-join tests.

START_TEST(JoinEnglishNamesWorksForEmptyInput)
{
	ASSERT_STRING(String_JoinEnglishNames(NULL, 0, String_FromC("and")), NULL, 0);
}
END_TEST

START_TEST(JoinEnglishNamesWorksForOneValue)
{
	String buffer[5];
	buffer[0] = String_FromC("Alice");
	ASSERT_STRING(String_JoinEnglishNames(buffer, 1, String_FromC("and")), "Alice", 5);
}
END_TEST

START_TEST(JoinEnglishNamesWorksForTwoValues)
{
	String buffer[5];
	buffer[0] = String_FromC("Alice");
	buffer[1] = String_FromC("Bill");
	ASSERT_STRING(String_JoinEnglishNames(buffer, 2, String_FromC("and")), "Alice and Bill", 14);
}
END_TEST

START_TEST(JoinEnglishNamesWorksForThreeValues)
{
	String buffer[5];
	buffer[0] = String_FromC("Alice");
	buffer[1] = String_FromC("Bill");
	buffer[2] = String_FromC("Charles");
	ASSERT_STRING(String_JoinEnglishNames(buffer, 3, String_FromC("and")), "Alice, Bill, and Charles", 24);
}
END_TEST

START_TEST(JoinEnglishNamesWorksForFourValues)
{
	String buffer[5];
	buffer[0] = String_FromC("Alice");
	buffer[1] = String_FromC("Bill");
	buffer[2] = String_FromC("Charles");
	buffer[3] = String_FromC("Diane");
	ASSERT_STRING(String_JoinEnglishNames(buffer, 4, String_FromC("and")), "Alice, Bill, Charles, and Diane", 31);
}
END_TEST

#include "stringextra_tests.generated.inc"
