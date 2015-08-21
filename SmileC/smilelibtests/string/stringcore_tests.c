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

TEST_SUITE(StringCoreTests)

//-------------------------------------------------------------------------------------------------
//  Creation Tests.
//
//  Note:  There is no explicit test for String_GetBytes() or String_Length() because
//  those functions are invoked by AssertString(), so they're implicitly tested by nearly
//  every test function below.

START_TEST(EmptyStringShouldBePredefined)
{
	AssertString(String_Empty, NULL, 0);
}
END_TEST

START_TEST(IsNullOrEmptyRecognizesNullAndEmptyStrings)
{
	String str = String_FromC("This is a test.");
	ASSERT(!String_IsNullOrEmpty(str));
	ASSERT(String_IsNullOrEmpty(NULL));
	ASSERT(String_IsNullOrEmpty(String_Empty));
}
END_TEST

START_TEST(CanCreateEmptyString)
{
	String str = String_Create(NULL, 0);
	AssertString(str, NULL, 0);
}
END_TEST

START_TEST(CanCreateFromText)
{
	String str = String_Create("This is a test." + 4, 6);
	AssertString(str, " is a ", 6);
}
END_TEST

START_TEST(CreateCopiesItsInput)
{
	char test[20];
	String str;

	strcpy(test, "This is a test.");
	str = String_Create(test, strlen(test));
	strcpy(test, "Hello, World.");

	AssertString(str, "This is a test.", 15);
}
END_TEST

START_TEST(CreateCanContainWeirdCharacters)
{
	String str = String_Create(" \t\r\n\0\xFF\x7F\xA0\xCC " + 1, 8);
	AssertString(str, "\t\r\n\0\xFF\x7F\xA0\xCC", 8);
}
END_TEST

START_TEST(CreateRepeatCreatesManyOfTheSameLetter)
{
	String str = String_CreateRepeat('*', 64);
	AssertString(str, "****************************************************************", 64);
}
END_TEST

START_TEST(CreateRepeatCreatesAnEmptyStringForZero)
{
	String str = String_CreateRepeat('*', 0);
	AssertString(str, NULL, 0);
}
END_TEST

START_TEST(CreateFromCCanConvertCStrings)
{
	String str = String_FromC("This is a test.");
	AssertString(str, "This is a test.", 15);
}
END_TEST

START_TEST(CreateFromCConvertsEmptyStrings)
{
	String str = String_FromC("");
	AssertString(str, NULL, 0);
}
END_TEST

START_TEST(CreateFromCCopiesItsInput)
{
	char test[20];
	String str;

	strcpy(test, "This is a test.");
	str = String_FromC(test);
	strcpy(test, "Hello, World.");

	AssertString(str, "This is a test.", 15);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Joining tests

START_TEST(ConcatManyCanHandleEmptyInput)
{
	String joined = String_ConcatMany(NULL, 0);
	AssertString(joined, NULL, 0);
}
END_TEST

START_TEST(ConcatManyCanHandleOneString)
{
	String str = String_FromC("This is a test.");
	String joined = String_ConcatMany(&str, 1);
	AssertString(joined, "This is a test.", 15);
}
END_TEST

START_TEST(ConcatManyCanHandleTwoStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("Hello, World.");
	String joined;
	String strs[2];

	strs[0] = str1;
	strs[1] = str2;
	joined = String_ConcatMany(strs, 2);

	AssertString(joined, "This is a test.Hello, World.", 28);
}
END_TEST

START_TEST(ConcatManyCanHandleManyStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("Hello, World.");
	String str3 = String_FromC("Narf poit zort fjord.");
	String str4 = String_FromC("Red green blue.");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_ConcatMany(strs, 4);

	AssertString(joined, "This is a test.Hello, World.Narf poit zort fjord.Red green blue.", 64);
}
END_TEST

START_TEST(ConcatManyCanHandleEmptyStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("");
	String str3 = String_FromC("Narf poit zort fjord.");
	String str4 = String_FromC("");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_ConcatMany(strs, 4);

	AssertString(joined, "This is a test.Narf poit zort fjord.", 36);
}
END_TEST

START_TEST(ConcatManyCanHandleBizarreDegenerateCases)
{
	String str1 = String_FromC("");
	String str2 = String_FromC("");
	String str3 = String_FromC("");
	String str4 = String_FromC("");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_ConcatMany(strs, 4);

	AssertString(joined, "", 0);
}
END_TEST

START_TEST(JoinCanHandleEmptyInput)
{
	String glue = String_FromC(",");
	String joined = String_Join(glue, NULL, 0);
	AssertString(joined, NULL, 0);
}
END_TEST

START_TEST(JoinCanHandleOneString)
{
	String glue = String_FromC(",");
	String str = String_FromC("This is a test.");
	String joined = String_Join(glue, &str, 1);
	AssertString(joined, "This is a test.", 15);
}
END_TEST

START_TEST(JoinCanHandleTwoStrings)
{
	String glue = String_FromC(",");
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("Hello, World.");
	String joined;
	String strs[2];

	strs[0] = str1;
	strs[1] = str2;
	joined = String_Join(glue, strs, 2);

	AssertString(joined, "This is a test.,Hello, World.", 29);
}
END_TEST

START_TEST(JoinCanHandleTwoStringsWithEmptyGlue)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("Hello, World.");
	String joined;
	String strs[2];

	strs[0] = str1;
	strs[1] = str2;
	joined = String_Join(String_Empty, strs, 2);

	AssertString(joined, "This is a test.Hello, World.", 28);
}
END_TEST

START_TEST(JoinCanHandleManyStrings)
{
	String glue = String_FromC(",");
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("Hello, World.");
	String str3 = String_FromC("Narf poit zort fjord.");
	String str4 = String_FromC("Red green blue.");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_Join(glue, strs, 4);

	AssertString(joined, "This is a test.,Hello, World.,Narf poit zort fjord.,Red green blue.", 67);
}
END_TEST

START_TEST(JoinCanHandleManyStringsWithEmptyGlue)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("Hello, World.");
	String str3 = String_FromC("Narf poit zort fjord.");
	String str4 = String_FromC("Red green blue.");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_Join(String_Empty, strs, 4);

	AssertString(joined, "This is a test.Hello, World.Narf poit zort fjord.Red green blue.", 64);
}
END_TEST

START_TEST(JoinCanHandleEmptyStrings)
{
	String glue = String_FromC(",");
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("");
	String str3 = String_FromC("Narf poit zort fjord.");
	String str4 = String_FromC("");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_Join(glue, strs, 4);

	AssertString(joined, "This is a test.,,Narf poit zort fjord.,", 39);
}
END_TEST

START_TEST(JoinCanHandleEmptyStringsWithEmptyGlue)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("");
	String str3 = String_FromC("Narf poit zort fjord.");
	String str4 = String_FromC("");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_Join(String_Empty, strs, 4);

	AssertString(joined, "This is a test.Narf poit zort fjord.", 36);
}
END_TEST

START_TEST(JoinCanHandleBizarreDegenerateCases)
{
	String glue = String_FromC(",");
	String str1 = String_FromC("");
	String str2 = String_FromC("");
	String str3 = String_FromC("");
	String str4 = String_FromC("");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_Join(glue, strs, 4);

	AssertString(joined, ",,,", 3);
}
END_TEST

START_TEST(JoinCanHandleBizarreDegenerateCasesWithEmptyGlue)
{
	String str1 = String_FromC("");
	String str2 = String_FromC("");
	String str3 = String_FromC("");
	String str4 = String_FromC("");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_Join(String_Empty, strs, 4);

	AssertString(joined, "", 0);
}
END_TEST

START_TEST(SlashAppendCanHandleEmptyInput)
{
	String joined = String_SlashAppend(NULL, 0);
	AssertString(joined, NULL, 0);
}
END_TEST

START_TEST(SlashAppendCanHandleOnePlainString)
{
	String str = String_FromC("my-path");
	String joined = String_SlashAppend(&str, 1);
	AssertString(joined, "my-path", 7);
}
END_TEST

START_TEST(SlashAppendDoesNotAffectSlashesOnASingleString)
{
	String str = String_FromC("//my-path//");
	String joined = String_SlashAppend(&str, 1);
	AssertString(joined, "//my-path//", 11);

	str = String_FromC("\\\\my-path\\\\");
	joined = String_SlashAppend(&str, 1);
	AssertString(joined, "\\\\my-path\\\\", 11);
}
END_TEST

START_TEST(SlashAppendCanHandleTwoPlainStrings)
{
	String str1 = String_FromC("my-path");
	String str2 = String_FromC("your-path");
	String joined;
	String strs[2];

	strs[0] = str1;
	strs[1] = str2;
	joined = String_SlashAppend(strs, 2);

	AssertString(joined, "my-path/your-path", 17);
}
END_TEST

START_TEST(SlashAppendDoesNotDamageSlashesAtStartsOrEndsOfStrings)
{
	String str1 = String_FromC("//my-path");
	String str2 = String_FromC("your-path//");
	String joined;
	String strs[2];

	strs[0] = str1;
	strs[1] = str2;
	joined = String_SlashAppend(strs, 2);

	AssertString(joined, "//my-path/your-path//", 21);

	str1 = String_FromC("\\\\my-path");
	str2 = String_FromC("your-path\\\\");

	strs[0] = str1;
	strs[1] = str2;
	joined = String_SlashAppend(strs, 2);

	AssertString(joined, "\\\\my-path/your-path\\\\", 21);
}
END_TEST

START_TEST(SlashAppendCanHandleManyPlainStrings)
{
	String str1 = String_FromC("my-path");
	String str2 = String_FromC("your-path");
	String str3 = String_FromC("our-path");
	String str4 = String_FromC("their-path");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_SlashAppend(strs, 4);

	AssertString(joined, "my-path/your-path/our-path/their-path", 37);
}
END_TEST

START_TEST(SlashAppendCleansUpMedialSlashes)
{
	String str1 = String_FromC("//my-path/");
	String str2 = String_FromC("\\\\your-path//");
	String str3 = String_FromC("/our-path\\");
	String str4 = String_FromC("their-path\\\\");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_SlashAppend(strs, 4);

	AssertString(joined, "//my-path/your-path/our-path/their-path\\\\", 41);
}
END_TEST

START_TEST(SlashAppendCanHandleEmptyPlainStrings)
{
	String str1 = String_FromC("my-path");
	String str2 = String_FromC("/");
	String str3 = String_FromC("your-path");
	String str4 = String_FromC("");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_SlashAppend(strs, 4);

	AssertString(joined, "my-path/your-path", 17);
}
END_TEST

START_TEST(SlashAppendCanHandleBizarreDegenerateCases)
{
	String str1 = String_FromC("");
	String str2 = String_FromC("");
	String str3 = String_FromC("");
	String str4 = String_FromC("");
	String joined;
	String strs[4];

	strs[0] = str1;
	strs[1] = str2;
	strs[2] = str3;
	strs[3] = str4;
	joined = String_SlashAppend(strs, 4);

	AssertString(joined, "", 0);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Comparison tests

START_TEST(EqualsDetectsEqualStringsByReference)
{
	String str1 = String_FromC("This is a test.");
	ASSERT(String_Equals(str1, str1));
	ASSERT(String_Equals(String_Empty, String_Empty));
}
END_TEST

START_TEST(EqualsDetectsEqualStringsByContent)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("This is a test.");
	String str3 = String_FromC("Hello, World.");
	ASSERT(String_Equals(str1, str2));
	ASSERT(!String_Equals(str1, str3));
}
END_TEST

START_TEST(EqualsHandlesUnusualCharacters)
{
	String str1 = String_Create("This is\0\r\n\xFF\xAA a test.", 20);
	String str2 = String_Create("This is\0\r\n\xFF\xAA a test.", 20);
	ASSERT(String_Equals(str1, str2));
}
END_TEST

START_TEST(EqualsComparesTheFullString)
{
	String str1 = String_Create("This is\0\r\n\xFF\xAA an aardvark.", 24);
	String str2 = String_Create("This is\0\r\n\xFF\xAA an aardvark.", 24);
	String str3 = String_Create("This is\0\r\n\xFF\xAA an oglethorpe.", 26);
	ASSERT(String_Equals(str1, str2));
	ASSERT(!String_Equals(str1, str3));
}
END_TEST

START_TEST(HashCodesForIdenticalStringsAreIdentical)
{
	Int hash1 = String_GetHashCode(String_FromC("This is an aardvark."));
	Int hash2 = String_GetHashCode(String_FromC("This is an aardvark."));
	ASSERT(hash1 == hash2);

	hash1 = String_GetHashCode(String_Empty);
	hash2 = String_GetHashCode(String_Empty);
	ASSERT(hash1 == hash2);
}
END_TEST

START_TEST(HashCodesForDifferentStringsAreDifferent)
{
	Int hash1 = String_GetHashCode(String_FromC("This is an aardvark."));
	Int hash2 = String_GetHashCode(String_FromC("This is an oglethorpe."));
	ASSERT(hash1 != hash2);

	hash1 = String_GetHashCode(String_FromC("This is an aardvark."));
	hash2 = String_GetHashCode(String_Empty);
	ASSERT(hash1 != hash2);
}
END_TEST

START_TEST(CompareDetectsEqualStrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_FromC("This is a test.");
	String str3 = String_FromC("Hello, World.");
	ASSERT(String_Compare(str1, str2) == 0);
	ASSERT(String_Compare(str1, str3) != 0);
}
END_TEST

START_TEST(CompareOrdersStringsByTheirBytes)
{
	String str1 = String_FromC("This is a test: A.");
	String str2 = String_FromC("This is a test: B.");
	String str3 = String_FromC("This is a test: a.");
	String str4 = String_FromC("This is a test: b.");
	ASSERT(String_Compare(str1, str2) < 0);
	ASSERT(String_Compare(str1, str3) < 0);
	ASSERT(String_Compare(str4, str3) > 0);
	ASSERT(String_Compare(str3, str2) > 0);
}
END_TEST

START_TEST(CompareOrdersStringsByTheirLengthWhenEqual)
{
	String str1 = String_FromC("This is a test");
	String str2 = String_FromC("This is a test of the");
	String str3 = String_FromC("This is a test of the emergency");
	String str4 = String_FromC("This is a test of the emergency broadcasting system.");
	ASSERT(String_Compare(str1, str2) < 0);
	ASSERT(String_Compare(str1, str3) < 0);
	ASSERT(String_Compare(str4, str3) > 0);
	ASSERT(String_Compare(str3, str2) > 0);
}
END_TEST

START_TEST(CompareRangeDetectsEqualStrings)
{
	String str1 = String_FromC("ABCThis is a test.ABC");
	String str2 = String_FromC("ABCDThis is a test.ABCD");
	String str3 = String_FromC("ABCDHello, World.ABCD");
	ASSERT(String_CompareRange(str1, 3, 15, str2, 4, 15) == 0);
	ASSERT(String_CompareRange(str1, 3, 15, str3, 4, 13) != 0);
}
END_TEST

START_TEST(CompareRangeOrdersStringsByTheirBytes)
{
	String str1 = String_FromC("ABCThis is a test: A.");
	String str2 = String_FromC("ABCDThis is a test: B.");
	String str3 = String_FromC("ABCDEThis is a test: a.");
	String str4 = String_FromC("ABCDEFThis is a test: b.");
	ASSERT(String_CompareRange(str1, 3, 17, str2, 4, 17) < 0);
	ASSERT(String_CompareRange(str1, 3, 17, str3, 5, 17) < 0);
	ASSERT(String_CompareRange(str4, 6, 17, str3, 5, 17) > 0);
	ASSERT(String_CompareRange(str3, 5, 17, str2, 4, 17) > 0);
}
END_TEST

START_TEST(CompareRangeOrdersStringsByTheirLengthWhenEqual)
{
	String str1 = String_FromC("ABCThis is a test");
	String str2 = String_FromC("ABCDThis is a test of the");
	String str3 = String_FromC("ABCDEThis is a test of the emergency");
	String str4 = String_FromC("ABCDEFThis is a test of the emergency broadcasting system.");
	ASSERT(String_CompareRange(str1, 3, 14, str2, 4, 21) < 0);
	ASSERT(String_CompareRange(str1, 3, 14, str3, 5, 31) < 0);
	ASSERT(String_CompareRange(str4, 6, 42, str3, 5, 31) > 0);
	ASSERT(String_CompareRange(str3, 5, 31, str2, 4, 21) > 0);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Substring tests

START_TEST(SubstringAtExtractsSubstrings)
{
	String str1 = String_FromC("This is a test.");
	String str2 = String_SubstringAt(str1, 8);
	AssertString(str2, "a test.", 7);
}
END_TEST

START_TEST(SubstringAtExtractsNothingForEmptyStrings)
{
	String str1 = String_SubstringAt(String_Empty, 0);
	AssertString(str1, NULL, 0);
}
END_TEST

START_TEST(SubstringAtExtractsNothingAtOrAfterTheEndOfTheString)
{
	String str1, str2, str3;

	str1 = String_FromC("This is a test.");
	str2 = String_SubstringAt(str1, 15);
	AssertString(str2, NULL, 0);

	str3 = String_SubstringAt(str1, 20);
	AssertString(str3, NULL, 0);
}
END_TEST

START_TEST(SubstringAtClipsToTheStartOfTheString)
{
	String str1, str2, str3;

	str1 = String_FromC("This is a test.");
	str2 = String_SubstringAt(str1, 0);
	AssertString(str2, "This is a test.", 15);

	str3 = String_SubstringAt(str1, -10);
	AssertString(str3, "This is a test.", 15);
}
END_TEST

START_TEST(SubstringExtractsSubstrings)
{
	String str1, str2, str3;

	str1 = String_FromC("This is a test of the emergency broadcasting system.");
	str2 = String_Substring(str1, 8, 6);
	AssertString(str2, "a test", 6);

	str3 = String_Substring(str1, 22, 9);
	AssertString(str3, "emergency", 9);
}
END_TEST

START_TEST(SubstringClipsTheLengthParameter)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_Substring(str1, 22, 100);
	AssertString(str2, "emergency broadcasting system.", 30);
}
END_TEST

START_TEST(SubstringClipsTheStartParameter)
{
	String str1, str2, str3;

	str1 = String_FromC("This is a test of the emergency broadcasting system.");

	str2 = String_Substring(str1, -20, 100);
	AssertString(str2, "This is a test of the emergency broadcasting system.", 52);

	str3 = String_Substring(str1, 0, 100);
	AssertString(str3, "This is a test of the emergency broadcasting system.", 52);
}
END_TEST

START_TEST(SubstringCorrectsTheLengthParameterIfStartIsClipped)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_Substring(str1, -5, 20);
	AssertString(str2, "This is a test ", 15);
}
END_TEST

START_TEST(SubstringReturnsNothingForAZeroOrNegativeLength)
{
	String str1, str2, str3;

	str1 = String_FromC("This is a test of the emergency broadcasting system.");

	str2 = String_Substring(str1, 22, 0);
	AssertString(str2, NULL, 0);

	str3 = String_Substring(str1, 22, -10);
	AssertString(str3, NULL, 0);
}
END_TEST

START_TEST(SubstringExtractsNothingAtOrAfterTheEndOfTheString)
{
	String str1, str2, str3;

	str1 = String_FromC("This is a test.");
	str2 = String_Substring(str1, 15, 10);
	AssertString(str2, NULL, 0);

	str3 = String_Substring(str1, 20, 10);
	AssertString(str3, NULL, 0);
}
END_TEST

START_TEST(AtExtractsIndividualCharacters)
{
	String str = String_FromC("This is a test.");
	ASSERT(String_At(str, 0) == 'T');
	ASSERT(String_At(str, 3) == 's');
	ASSERT(String_At(str, 14) == '.');
}
END_TEST

START_TEST(AtReturnsNulAtExactlyTheStringLength)
{
	String str = String_FromC("This is a test.");
	ASSERT(String_Length(str) == 15);
	ASSERT(String_At(str, 15) == '\0');
}
END_TEST

START_TEST(ConcatJoinsStrings)
{
	String str1, str2, str3;

	str1 = String_FromC("This is a test");
	str2 = String_FromC(" of the emergency ");
	str3 = String_FromC("broadcasting system.");

	AssertString(String_Concat(str1, str2), "This is a test of the emergency ", 32);
	AssertString(String_Concat(str2, str3), " of the emergency broadcasting system.", 38);
	AssertString(String_Concat(String_Concat(str1, str2), str3), "This is a test of the emergency broadcasting system.", 52);
}
END_TEST

START_TEST(ConcatHandlesEmptyStrings)
{
	String str = String_FromC("This is a test.");

	AssertString(String_Concat(str, String_Empty), "This is a test.", 15);
	AssertString(String_Concat(String_Empty, str), "This is a test.", 15);
	AssertString(String_Concat(String_Empty, String_Empty), NULL, 0);
}
END_TEST

START_TEST(ConcatByteTacksOnBytes)
{
	String str = String_FromC("This is a test");
	AssertString(String_ConcatByte(str, '.'), "This is a test.", 15);
	AssertString(String_ConcatByte(str, '?'), "This is a test?", 15);
	AssertString(String_ConcatByte(str, '\0'), "This is a test\0", 15);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Content-searching tests

START_TEST(IndexOfFindsContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("test");
	String str3 = String_FromC("emergency");
	String str4 = String_FromC(" system.");

	ASSERT(String_IndexOf(str1, str2, 0) == 10);
	ASSERT(String_IndexOf(str1, str3, 0) == 22);
	ASSERT(String_IndexOf(str1, str4, 0) == 44);
}
END_TEST

START_TEST(IndexOfDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("emergnecy");		// Misspelled
	String str3 = String_FromC("broadcastink");		// Also misspelled
	String str4 = String_FromC("xyzzy");			// Not even a word

	ASSERT(String_IndexOf(str1, str2, 0) == -1);
	ASSERT(String_IndexOf(str1, str3, 0) == -1);
	ASSERT(String_IndexOf(str1, str4, 0) == -1);
}
END_TEST

START_TEST(IndexOfStartsWhereYouTellItToStart)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("test");
	String str3 = String_FromC("emergency");

	ASSERT(String_IndexOf(str1, str2, 5) == 10);
	ASSERT(String_IndexOf(str1, str2, 10) == 10);
	ASSERT(String_IndexOf(str1, str2, 20) == -1);

	ASSERT(String_IndexOf(str1, str3, 0) == 22);
	ASSERT(String_IndexOf(str1, str3, 21) == 22);
	ASSERT(String_IndexOf(str1, str3, 22) == 22);
	ASSERT(String_IndexOf(str1, str3, 23) == -1);
}
END_TEST

START_TEST(IndexOfClipsStartIndexesToTheString)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("test");
	String str3 = String_FromC("emergency");

	ASSERT(String_IndexOf(str1, str2, -100) == 10);
	ASSERT(String_IndexOf(str1, str2, 10) == 10);
	ASSERT(String_IndexOf(str1, str2, 200) == -1);

	ASSERT(String_IndexOf(str1, str3, -100) == 22);
	ASSERT(String_IndexOf(str1, str3, 10) == 22);
	ASSERT(String_IndexOf(str1, str3, 100) == -1);
}
END_TEST

START_TEST(IndexOfFindsSubsequentMatches)
{
	String str1 = String_FromC("This is a test is a test is a test is a test.");
	String str2 = String_FromC("test");

	ASSERT(String_IndexOf(str1, str2,  5) == 10);
	ASSERT(String_IndexOf(str1, str2, 10) == 10);
	ASSERT(String_IndexOf(str1, str2, 15) == 20);
	ASSERT(String_IndexOf(str1, str2, 20) == 20);
	ASSERT(String_IndexOf(str1, str2, 25) == 30);
	ASSERT(String_IndexOf(str1, str2, 30) == 30);
	ASSERT(String_IndexOf(str1, str2, 35) == 40);
	ASSERT(String_IndexOf(str1, str2, 40) == 40);
	ASSERT(String_IndexOf(str1, str2, 45) == -1);
}
END_TEST

START_TEST(IndexOfFindsTheEmptyStringEverywhere)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_IndexOf(str, String_Empty, -10) == 0);		// Start should clip to the start of the string.

	ASSERT(String_IndexOf(str, String_Empty, 0) == 0);
	ASSERT(String_IndexOf(str, String_Empty, 10) == 10);
	ASSERT(String_IndexOf(str, String_Empty, 22) == 22);
	ASSERT(String_IndexOf(str, String_Empty, 52) == 52);

	ASSERT(String_IndexOf(str, String_Empty, 53) == -1);		// Regular strings can't be found past the end either.
}
END_TEST

START_TEST(LastIndexOfFindsContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("test");
	String str3 = String_FromC("emergency");
	String str4 = String_FromC(" system.");

	ASSERT(String_LastIndexOf(str1, str2, 52) == 10);
	ASSERT(String_LastIndexOf(str1, str3, 52) == 22);
	ASSERT(String_LastIndexOf(str1, str4, 52) == 44);
}
END_TEST

START_TEST(LastIndexOfDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("emergnecy");		// Misspelled
	String str3 = String_FromC("broadcastink");		// Also misspelled
	String str4 = String_FromC("xyzzy");			// Not even a word

	ASSERT(String_LastIndexOf(str1, str2, 52) == -1);
	ASSERT(String_LastIndexOf(str1, str3, 52) == -1);
	ASSERT(String_LastIndexOf(str1, str4, 52) == -1);
}
END_TEST

START_TEST(LastIndexOfStartsWhereYouTellItToStart)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("test");
	String str3 = String_FromC("emergency");

	ASSERT(String_LastIndexOf(str1, str2, 52) == 10);
	ASSERT(String_LastIndexOf(str1, str2, 14) == 10);
	ASSERT(String_LastIndexOf(str1, str2, 13) == -1);
	ASSERT(String_LastIndexOf(str1, str2, 5) == -1);

	ASSERT(String_LastIndexOf(str1, str3, 52) == 22);
	ASSERT(String_LastIndexOf(str1, str3, 30) == -1);
	ASSERT(String_LastIndexOf(str1, str3, 31) == 22);
	ASSERT(String_LastIndexOf(str1, str3, 32) == 22);
}
END_TEST

START_TEST(LastIndexOfClipsStartIndexesToTheString)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("test");
	String str3 = String_FromC("emergency");

	ASSERT(String_LastIndexOf(str1, str2, -100) == -1);
	ASSERT(String_LastIndexOf(str1, str2, 13) == -1);
	ASSERT(String_LastIndexOf(str1, str2, 14) == 10);
	ASSERT(String_LastIndexOf(str1, str2, 15) == 10);
	ASSERT(String_LastIndexOf(str1, str2, 200) == 10);

	ASSERT(String_LastIndexOf(str1, str3, -100) == -1);
	ASSERT(String_LastIndexOf(str1, str3, 30) == -1);
	ASSERT(String_LastIndexOf(str1, str3, 31) == 22);
	ASSERT(String_LastIndexOf(str1, str3, 32) == 22);
	ASSERT(String_LastIndexOf(str1, str3, 100) == 22);
}
END_TEST

START_TEST(LastIndexOfFindsSubsequentMatches)
{
	String str1 = String_FromC("This is a test is a test is a test is a test.");
	String str2 = String_FromC("test");

	ASSERT(String_LastIndexOf(str1, str2, 5) == -1);
	ASSERT(String_LastIndexOf(str1, str2, 10) == -1);
	ASSERT(String_LastIndexOf(str1, str2, 15) == 10);
	ASSERT(String_LastIndexOf(str1, str2, 20) == 10);
	ASSERT(String_LastIndexOf(str1, str2, 25) == 20);
	ASSERT(String_LastIndexOf(str1, str2, 30) == 20);
	ASSERT(String_LastIndexOf(str1, str2, 35) == 30);
	ASSERT(String_LastIndexOf(str1, str2, 40) == 30);
	ASSERT(String_LastIndexOf(str1, str2, 45) == 40);
	ASSERT(String_LastIndexOf(str1, str2, 50) == 40);
	ASSERT(String_LastIndexOf(str1, str2, 55) == 40);
}
END_TEST

START_TEST(LastIndexOfFindsTheEmptyStringEverywhere)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_LastIndexOf(str, String_Empty, -10) == -1);		// Regular strings can't be found past the start either.

	ASSERT(String_LastIndexOf(str, String_Empty, 0) == 0);
	ASSERT(String_LastIndexOf(str, String_Empty, 10) == 10);
	ASSERT(String_LastIndexOf(str, String_Empty, 22) == 22);
	ASSERT(String_LastIndexOf(str, String_Empty, 52) == 52);

	ASSERT(String_LastIndexOf(str, String_Empty, 53) == 52);
}
END_TEST

START_TEST(ContainsFindsContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("test");
	String str3 = String_FromC("emergency");
	String str4 = String_FromC(" system.");

	ASSERT(String_Contains(str1, str2));
	ASSERT(String_Contains(str1, str3));
	ASSERT(String_Contains(str1, str4));
}
END_TEST

START_TEST(ContainsDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("emergnecy");		// Misspelled
	String str3 = String_FromC("broadcastink");		// Also misspelled
	String str4 = String_FromC("xyzzy");			// Not even a word

	ASSERT(!String_Contains(str1, str2));
	ASSERT(!String_Contains(str1, str3));
	ASSERT(!String_Contains(str1, str4));
}
END_TEST

START_TEST(ContainsFindsTheEmptyStringEverywhere)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_Contains(str, String_Empty));
	ASSERT(String_Contains(String_Empty, String_Empty));
}
END_TEST

START_TEST(StartsWithMatchesContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("This is");
	String str3 = String_FromC("This is a test");
	String str4 = String_FromC("This is a test of the");

	ASSERT(String_StartsWith(str1, str1));
	ASSERT(String_StartsWith(str1, str2));
	ASSERT(String_StartsWith(str1, str3));
	ASSERT(String_StartsWith(str1, str4));
}
END_TEST

START_TEST(StartsWithDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("Thsi is a test");	// Misspelled
	String str3 = String_FromC("this is a test");	// Lowercase
	String str4 = String_FromC("xyzzy");			// Not even a word

	ASSERT(!String_StartsWith(str1, str2));
	ASSERT(!String_StartsWith(str1, str3));
	ASSERT(!String_StartsWith(str1, str4));
}
END_TEST

START_TEST(StartsWithAlwaysMatchesTheEmptyString)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_StartsWith(str, String_Empty));
	ASSERT(String_StartsWith(String_Empty, String_Empty));
}
END_TEST

START_TEST(IndexOfCharFindsCharactersWhenTheyExist)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_IndexOfChar(str, 'T', 0) == 0);
	ASSERT(String_IndexOfChar(str, 'i', 0) == 2);
	ASSERT(String_IndexOfChar(str, 'y', 0) == 30);
}
END_TEST

START_TEST(IndexOfCharDoesNotFindCharactersWhenTheyDoNotExist)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_IndexOfChar(str, '?', 0) == -1);
	ASSERT(String_IndexOfChar(str, 'x', 0) == -1);
	ASSERT(String_IndexOfChar(str, '\xFF', 0) == -1);
	ASSERT(String_IndexOfChar(str, '\x00', 0) == -1);
}
END_TEST

START_TEST(IndexOfCharStartsWhereYouTellIt)
{
	String str = String_FromC( "This is a test of the emergency broadcasting system.");

	ASSERT(String_IndexOfChar(str, 'i', 0) == 2);
	ASSERT(String_IndexOfChar(str, 'i', 4) == 5);
	ASSERT(String_IndexOfChar(str, 'i', 10) == 41);

	ASSERT(String_IndexOfChar(str, 'y', 0) == 30);
	ASSERT(String_IndexOfChar(str, 'y', 29) == 30);
	ASSERT(String_IndexOfChar(str, 'y', 30) == 30);
	ASSERT(String_IndexOfChar(str, 'y', 31) == 46);
	ASSERT(String_IndexOfChar(str, 'y', 47) == -1);
}
END_TEST

START_TEST(LastIndexOfCharFindsCharactersWhenTheyExist)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_LastIndexOfChar(str, 'T', 52) == 0);
	ASSERT(String_LastIndexOfChar(str, 'i', 52) == 41);
	ASSERT(String_LastIndexOfChar(str, 'y', 52) == 46);
}
END_TEST

START_TEST(LastIndexOfCharDoesNotFindCharactersWhenTheyDoNotExist)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_LastIndexOfChar(str, '?', 52) == -1);
	ASSERT(String_LastIndexOfChar(str, 'x', 52) == -1);
	ASSERT(String_LastIndexOfChar(str, '\xFF', 52) == -1);
	ASSERT(String_LastIndexOfChar(str, '\x00', 52) == -1);
}
END_TEST

START_TEST(LastIndexOfCharStartsWhereYouTellIt)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_LastIndexOfChar(str, 'i', 1) == -1);
	ASSERT(String_LastIndexOfChar(str, 'i', 4) == 2);
	ASSERT(String_LastIndexOfChar(str, 'i', 52) == 41);

	ASSERT(String_LastIndexOfChar(str, 'y', 1) == -1);
	ASSERT(String_LastIndexOfChar(str, 'y', 29) == -1);
	ASSERT(String_LastIndexOfChar(str, 'y', 30) == 30);
	ASSERT(String_LastIndexOfChar(str, 'y', 31) == 30);
	ASSERT(String_LastIndexOfChar(str, 'y', 52) == 46);
}
END_TEST

START_TEST(IndexOfAnyCharFindsCharactersWhenTheyExist)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"tes", 3, 0) == 3);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"te", 2, 0) == 10);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 0) == 23);
}
END_TEST

START_TEST(IndexOfAnyCharDoesNotFindCharactersWhenTheyDoNotExist)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"?x\xFF\0", 4, 0) == -1);
}
END_TEST

START_TEST(IndexOfAnyCharStartsWhereYouTellIt)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, -10) == 23);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 0) == 23);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 24) == 30);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 31) == 32);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 33) == 46);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 47) == 50);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 51) == -1);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 52) == -1);
	ASSERT(String_IndexOfAnyChar(str, (const Byte *)"bmy", 3, 100) == -1);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  String-replace tests

//SMILE_API String String_Replace(const String str, const String pattern, const String replacement);
//SMILE_API String String_ReplaceChar(const String str, Byte pattern, Byte replacement);

START_TEST(ReplaceDoesNothingToAnEmptyString)
{
	String str1 = String_FromC("foo");
	String str2 = String_FromC("bar");

	AssertString(String_Replace(String_Empty, str1, str2), NULL, 0);
	AssertString(String_Replace(NULL, str1, str2), NULL, 0);
}
END_TEST

START_TEST(ReplaceDoesNothingWithAnEmptyPattern)
{
	String str1 = String_FromC("foo");
	String str2 = String_FromC("bar");

	AssertString(String_Replace(str1, String_Empty, str2), "foo", 3);
	AssertString(String_Replace(str1, NULL, str2), "foo", 3);
}
END_TEST

START_TEST(ReplaceSubstitutesContentWhereItMatches)
{
	String str1 = String_FromC("This is a test for testing tests.");
	String str2 = String_FromC("test");
	String str3 = String_FromC("foo");

	AssertString(String_Replace(str1, str2, str3), "This is a foo for fooing foos.", 30);
}
END_TEST

START_TEST(ReplaceRemovesContentWhenTheReplacementIsNullOrEmpty)
{
	String str1 = String_FromC("This is a test for testing tests.");
	String str2 = String_FromC("test");

	AssertString(String_Replace(str1, str2, String_Empty), "This is a  for ing s.", 21);
	AssertString(String_Replace(str1, str2, NULL), "This is a  for ing s.", 21);
}
END_TEST

START_TEST(ReplaceSubstitutesContentWhereItDoesNotOverlap)
{
	String str1 = String_FromC("This is a testest for testestesting testestests.");
	String str2 = String_FromC("test");
	String str3 = String_FromC("foo");

	AssertString(String_Replace(str1, str2, str3), "This is a fooest for fooesfooing fooesfoos.", 43);
}
END_TEST

START_TEST(ReplaceSubstitutesContentEvenWithControlCodesAndHighASCIIValuesInIt)
{
	String str1 = String_Create("This is a te\0\xFF\xAA\x1Ast for te\0\xFF\xAA\x1Asting te\0\xFF\xAA\x1Asts.", 45);
	String str2 = String_Create("te\0\xFF\xAA\x1Ast", 8);
	String str3 = String_Create("f\0o\xFFo", 5);

	AssertString(String_Replace(str1, str2, str3), "This is a f\0o\xFFo for f\0o\xFFoing f\0o\xFFos.", 36);
}
END_TEST

START_TEST(ReplaceCharSubstitutesCharsWhereTheyMatch)
{
	String str1 = String_FromC("This is a test for testing tests.");
	String str2 = String_ReplaceChar(str1, 't', 'x');
	String str3 = String_ReplaceChar(str2, 'e', 'y');
	String str4 = String_ReplaceChar(str3, 's', 'o');

	AssertString(str2, "This is a xesx for xesxing xesxs.", 33);
	AssertString(str3, "This is a xysx for xysxing xysxs.", 33);
	AssertString(str4, "Thio io a xyox for xyoxing xyoxo.", 33);
}
END_TEST

#include "stringcore_tests.generated.inc"
