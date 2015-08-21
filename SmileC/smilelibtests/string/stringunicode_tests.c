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

TEST_SUITE(StringUnicodeTests)

//-------------------------------------------------------------------------------------------------
//  Case-Insensitive Comparison Tests

START_TEST(CompareIShouldMatchEmptyAndIdenticalAsciiStrings)
{
	ASSERT(String_CompareI(String_Empty, String_Empty) == 0);
	ASSERT(String_CompareI(String_FromC("This is a test."), String_FromC("This is a test.")) == 0);
	ASSERT(String_CompareI(String_Create("This\ris\0a\ntest.", 15), String_Create("This\ris\0a\ntest.", 15)) == 0);
}
END_TEST

START_TEST(CompareIShouldLexicallyOrderAsciiStrings)
{
	ASSERT(String_CompareI(String_FromC("Soup"), String_FromC("Nuts")) > 0);
	ASSERT(String_CompareI(String_FromC("Aardvark"), String_FromC("Zoetrope")) < 0);
}
END_TEST

START_TEST(CompareIShouldMatchAsciiStringsThatDifferByCase)
{
	ASSERT(String_CompareI(String_FromC("SOUP"), String_FromC("soup")) == 0);
	ASSERT(String_CompareI(String_FromC("nuts"), String_FromC("NUTS")) == 0);
}
END_TEST

START_TEST(CompareIShouldLexicallyOrderAsciiStringsThatDifferByCase)
{
	ASSERT(String_CompareI(String_FromC("SOUP"), String_FromC("nuts")) > 0);
	ASSERT(String_CompareI(String_FromC("aardvark"), String_FromC("ZOETROPE")) < 0);
}
END_TEST

START_TEST(CompareIShouldLexicallyOrderNonAsciiStringsThatDifferByCase)
{
	ASSERT(String_CompareI(String_FromC("\xC3\xA9tudier"), String_FromC("\xC3\x89TUDIER")) == 0);
	ASSERT(String_CompareI(String_FromC("apres"), String_FromC("APR\xC3\x88S")) < 0);
	ASSERT(String_CompareI(String_FromC("APR\xC3\x88S"), String_FromC("apres")) > 0);
}
END_TEST

START_TEST(CompareIUsesTrueCaseFolding)
{
	ASSERT(String_CompareI(String_FromC("Wasserschlo\xC3\x9F"), String_FromC("WASSERSCHLOSS")) == 0);
	ASSERT(String_CompareI(String_FromC("Wasserschlo\xC3\x9F"), String_FromC("wasserschloss")) == 0);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Case-Insensitive Index-of Tests

/*
SMILE_API Int String_LastIndexOfI(const String str, const String pattern, Int start);
SMILE_API Bool String_ContainsI(const String str, const String pattern);
SMILE_API Bool String_StartsWithI(const String str, const String pattern);
SMILE_API Bool String_EndsWithI(const String str, const String pattern);
*/

START_TEST(IndexOfIShouldFindNothingInEmptyStrings)
{
	ASSERT(String_IndexOfI(String_Empty, String_FromC("This is a test."), 0) == -1);
}
END_TEST

START_TEST(IndexOfIShouldFindEmptyStringsInsideEmptyStrings)
{
	ASSERT(String_IndexOfI(String_Empty, String_Empty, -1) == 0);
	ASSERT(String_IndexOfI(String_Empty, String_Empty, 0) == 0);
	ASSERT(String_IndexOfI(String_Empty, String_Empty, 1) == -1);
}
END_TEST

START_TEST(IndexOfIShouldFindEmptyStringsBeforeAndAfterEveryCharacter)
{
	ASSERT(String_IndexOfI(String_FromC("This is a test."), String_Empty, -1) == 0);
	ASSERT(String_IndexOfI(String_FromC("This is a test."), String_Empty, 0) == 0);
	ASSERT(String_IndexOfI(String_FromC("This is a test."), String_Empty, 10) == 10);
	ASSERT(String_IndexOfI(String_FromC("This is a test."), String_Empty, 15) == 15);
	ASSERT(String_IndexOfI(String_FromC("This is a test."), String_Empty, 16) == -1);
}
END_TEST

START_TEST(IndexOfIFindsContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("TEST");
	String str3 = String_FromC("EmErGeNcY");
	String str4 = String_FromC(" sysTEM.");

	ASSERT(String_IndexOfI(str1, str2, 0) == 10);
	ASSERT(String_IndexOfI(str1, str3, 0) == 22);
	ASSERT(String_IndexOfI(str1, str4, 0) == 44);
}
END_TEST

START_TEST(IndexOfIFindsUnicodeContentWhenItExists)
{
	String str1 = String_FromC("This is a tesst of the emerg\xC3\x89ncy broadcasting syst\xC3\xA9m.");
	String str2 = String_FromC("TE\xC3\x9FT");
	String str3 = String_FromC("EmErG\xC3\xA9NcY");
	String str4 = String_FromC(" sysT\xC3\x89M.");

	ASSERT(String_IndexOfI(str1, str2, 0) == 10);
	ASSERT(String_IndexOfI(str1, str3, 0) == 23);
	ASSERT(String_IndexOfI(str1, str4, 0) == 46);
}
END_TEST

START_TEST(IndexOfIDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("emergnecy");		// Misspelled
	String str3 = String_FromC("broadcastink");		// Also misspelled
	String str4 = String_FromC("xyzzy");			// Not even a word
	String str5 = String_FromC("emerg\xC3\xA9ncy");	// E-with-accent is not E

	ASSERT(String_IndexOfI(str1, str2, 0) == -1);
	ASSERT(String_IndexOfI(str1, str3, 0) == -1);
	ASSERT(String_IndexOfI(str1, str4, 0) == -1);
	ASSERT(String_IndexOfI(str1, str5, 0) == -1);
}
END_TEST

START_TEST(IndexOfIStartsWhereYouTellItToStart)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("TeSt");
	String str3 = String_FromC("EmErGeNcY");

	ASSERT(String_IndexOfI(str1, str2, 5) == 10);
	ASSERT(String_IndexOfI(str1, str2, 10) == 10);
	ASSERT(String_IndexOfI(str1, str2, 20) == -1);

	ASSERT(String_IndexOfI(str1, str3, 0) == 22);
	ASSERT(String_IndexOfI(str1, str3, 21) == 22);
	ASSERT(String_IndexOfI(str1, str3, 22) == 22);
	ASSERT(String_IndexOfI(str1, str3, 23) == -1);
}
END_TEST

START_TEST(IndexOfIClipsStartIndexesToTheString)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("TEST");
	String str3 = String_FromC("EMERGENCY");

	ASSERT(String_IndexOfI(str1, str2, -100) == 10);
	ASSERT(String_IndexOfI(str1, str2, 10) == 10);
	ASSERT(String_IndexOfI(str1, str2, 200) == -1);

	ASSERT(String_IndexOfI(str1, str3, -100) == 22);
	ASSERT(String_IndexOfI(str1, str3, 10) == 22);
	ASSERT(String_IndexOfI(str1, str3, 100) == -1);
}
END_TEST

START_TEST(IndexOfIFindsSubsequentMatches)
{
	String str1 = String_FromC("This is a TEST is a tEsT is a TeSt is a test.");
	String str2 = String_FromC("test");

	ASSERT(String_IndexOfI(str1, str2, 5) == 10);
	ASSERT(String_IndexOfI(str1, str2, 10) == 10);
	ASSERT(String_IndexOfI(str1, str2, 15) == 20);
	ASSERT(String_IndexOfI(str1, str2, 20) == 20);
	ASSERT(String_IndexOfI(str1, str2, 25) == 30);
	ASSERT(String_IndexOfI(str1, str2, 30) == 30);
	ASSERT(String_IndexOfI(str1, str2, 35) == 40);
	ASSERT(String_IndexOfI(str1, str2, 40) == 40);
	ASSERT(String_IndexOfI(str1, str2, 45) == -1);
}
END_TEST

#include "stringunicode_tests.generated.inc"
