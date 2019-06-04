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

#include "../stdafx.h"

TEST_SUITE(StringFormatTests)

//-------------------------------------------------------------------------------------------------
//  Formatting Tests.

START_TEST(EmptyFormatStringsShouldProduceEmptyOutput)
{
	ASSERT_STRING(String_FormatString(String_Empty), NULL, 0);
	ASSERT_STRING(String_FormatString(NULL), NULL, 0);
}
END_TEST

START_TEST(FormatStringsWithoutFormatCharactersInThemResultInTheSameOutput)
{
	const char *expectedResult = "This is a test. Pack my box with five dozen liquor jugs.";
	ASSERT_STRING(String_Format("This is a test. Pack my box with five dozen liquor jugs."), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatTreatsUnusualCharactersLikeAllOtherCharacters)
{
	String formatString = String_Create("This is a test.\r\n Pack my box\0 with five dozen\xFF liquor jugs.", 60);
	ASSERT_STRING(String_FormatString(formatString), String_ToC(formatString), String_Length(formatString));
}
END_TEST

START_TEST(FormatCanInsertCStyleStrings)
{
	const char *expectedResult = "This is a test. Pack my box with five dozen liquor jugs.";
	ASSERT_STRING(String_Format("This is a test. Pack my %s with five dozen %s jugs.", "box", "liquor"), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanInsertNullCStyleStrings)
{
	const char *expectedResult = "This is a test. Pack my  with five dozen liquor jugs.";
	ASSERT_STRING(String_Format("This is a test. Pack my %s with five dozen %s jugs.", NULL, "liquor"), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanInsertStringObjects)
{
	const char *expectedResult = "This is a test. Pack my box with five dozen liquor jugs.";
	ASSERT_STRING(String_Format("This is a test. Pack my %S with five dozen %S jugs.", String_FromC("box"), String_FromC("liquor")), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanInsertStringObjectsWithWeirdCharactersInThem)
{
	String expectedResult = String_Create("This is a test. Pack my bo\0x with five dozen li\rq\0u\nor jugs.", 60);
	ASSERT_STRING(String_Format("This is a test. Pack my %S with five dozen %S jugs.", String_Create("bo\0x", 4), String_Create("li\rq\0u\nor", 9)), String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(FormatCanInsertNullStringObjects)
{
	const char *expectedResult = "This is a test. Pack my  with five dozen liquor jugs.";
	ASSERT_STRING(String_Format("This is a test. Pack my %S with five dozen %S jugs.", NULL, String_FromC("liquor")), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanInsertCharacters)
{
	const char *expectedResult = "This is a test insert of 'x'.";
	ASSERT_STRING(String_Format("This is a test insert of '%c'.", 'x'), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanInsertSpecialCharacters)
{
	String expectedResult = String_Create("This is a test insert of '\0'.", 29);
	ASSERT_STRING(String_Format("This is a test insert of '%c'.", '\0'), String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(FormatReplacesDoublePercentWithSinglePercent)
{
	const char *expectedResult = "This is a % character.";
	ASSERT_STRING(String_Format("This is a %% character."), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanInsertLowercasePointers)
{
	const char *expectedResult = sizeof(void *) > 4 ? "This is a 0x00000000abad1dea." : "This is a 0xabad1dea.";
	ASSERT_STRING(String_Format("This is a %p.", (void *)(PtrInt)0xABAD1DEA), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanInsertUppercasePointers)
{
	const char *expectedResult = sizeof(void *) > 4 ? "This is a 0x00000000ABAD1DEA." : "This is a 0xABAD1DEA.";
	ASSERT_STRING(String_Format("This is a %P.", (void *)(PtrInt)0xABAD1DEA), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatSkipsUnknownInserts)
{
	const char *expectedResult = "This is a %q insert.";
	ASSERT_STRING(String_Format("This is a %q insert."), expectedResult, StrLen(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  %u Formatting Tests.

START_TEST(FormatInsertsSignedIntsForU)
{
	const char *expectedResult = "This is a 12345 insert.";
	ASSERT_STRING(String_Format("This is a %u insert.", (UInt)12345), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0 insert.";
	ASSERT_STRING(String_Format("This is a %u insert.", (UInt)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 1 insert.";
	ASSERT_STRING(String_Format("This is a %u insert.", (UInt)1), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanSpacePadU)
{
	const char *expectedResult = "This is a 12345 insert.";
	ASSERT_STRING(String_Format("This is a %4u insert.", (UInt)12345), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a    0 insert.";
	ASSERT_STRING(String_Format("This is a %4u insert.", (UInt)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a    1 insert.";
	ASSERT_STRING(String_Format("This is a %4u insert.", (UInt)1), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanZeroPadU)
{
	const char *expectedResult = "This is a 12345 insert.";
	ASSERT_STRING(String_Format("This is a %04u insert.", (UInt)12345), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0000 insert.";
	ASSERT_STRING(String_Format("This is a %04u insert.", (UInt)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0001 insert.";
	ASSERT_STRING(String_Format("This is a %04u insert.", (UInt)1), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatInsertsUnsigned32BitIntsForHU)
{
	const char *expectedResult = "This is a 12345 insert.";
	ASSERT_STRING(String_Format("This is a %hu insert.", (UInt32)12345), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0 insert.";
	ASSERT_STRING(String_Format("This is a %hu insert.", (UInt32)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 1 insert.";
	ASSERT_STRING(String_Format("This is a %hu insert.", (UInt32)1), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 4294967295 insert.";
	ASSERT_STRING(String_Format("This is a %hu insert.", UInt32Max), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatInsertsUnsigned64BitIntsForHU)
{
	const char *expectedResult = "This is a 12345 insert.";
	ASSERT_STRING(String_Format("This is a %lu insert.", (UInt64)12345), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0 insert.";
	ASSERT_STRING(String_Format("This is a %lu insert.", (UInt64)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 1 insert.";
	ASSERT_STRING(String_Format("This is a %lu insert.", (UInt64)1), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 18446744073709551615 insert.";
	ASSERT_STRING(String_Format("This is a %lu insert.", UInt64Max), expectedResult, StrLen(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  %x Formatting Tests.

START_TEST(FormatInsertsSignedIntsForLowercaseX)
{
	const char *expectedResult = "This is a abad1dea insert.";
	ASSERT_STRING(String_Format("This is a %x insert.", (UInt)0xABAD1DEA), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0 insert.";
	ASSERT_STRING(String_Format("This is a %x insert.", (UInt)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 1 insert.";
	ASSERT_STRING(String_Format("This is a %x insert.", (UInt)1), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatInsertsSignedIntsForUppercaseX)
{
	const char *expectedResult = "This is a ABAD1DEA insert.";
	ASSERT_STRING(String_Format("This is a %X insert.", (UInt)0xABAD1DEA), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0 insert.";
	ASSERT_STRING(String_Format("This is a %X insert.", (UInt)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 1 insert.";
	ASSERT_STRING(String_Format("This is a %X insert.", (UInt)1), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanSpacePadX)
{
	const char *expectedResult = "This is a abad1dea insert.";
	ASSERT_STRING(String_Format("This is a %4x insert.", (UInt)0xABAD1DEA), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a    0 insert.";
	ASSERT_STRING(String_Format("This is a %4x insert.", (UInt)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a    1 insert.";
	ASSERT_STRING(String_Format("This is a %4x insert.", (UInt)1), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatCanZeroPadX)
{
	const char *expectedResult = "This is a abad1dea insert.";
	ASSERT_STRING(String_Format("This is a %04x insert.", (UInt)0xABAD1DEA), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0000 insert.";
	ASSERT_STRING(String_Format("This is a %04x insert.", (UInt)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0001 insert.";
	ASSERT_STRING(String_Format("This is a %04x insert.", (UInt)1), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatInsertsUnsigned32BitIntsForHX)
{
	const char *expectedResult = "This is a abad1dea insert.";
	ASSERT_STRING(String_Format("This is a %hx insert.", (UInt32)0xABAD1DEA), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0 insert.";
	ASSERT_STRING(String_Format("This is a %hx insert.", (UInt32)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 1 insert.";
	ASSERT_STRING(String_Format("This is a %hx insert.", (UInt32)1), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a ffffffff insert.";
	ASSERT_STRING(String_Format("This is a %hx insert.", UInt32Max), expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(FormatInsertsUnsigned64BitIntsForHX)
{
	const char *expectedResult = "This is a abad1dea insert.";
	ASSERT_STRING(String_Format("This is a %lx insert.", (UInt64)0xABAD1DEA), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 0 insert.";
	ASSERT_STRING(String_Format("This is a %lx insert.", (UInt64)0), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a 1 insert.";
	ASSERT_STRING(String_Format("This is a %lx insert.", (UInt64)1), expectedResult, StrLen(expectedResult));

	expectedResult = "This is a ffffffffffffffff insert.";
	ASSERT_STRING(String_Format("This is a %lx insert.", UInt64Max), expectedResult, StrLen(expectedResult));
}
END_TEST

#include "stringformat_tests.generated.inc"

