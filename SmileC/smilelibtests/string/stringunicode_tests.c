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

//-------------------------------------------------------------------------------------------------
//  Case-Insensitive Last-Index-of Tests

START_TEST(LastIndexOfIShouldFindNothingInEmptyStrings)
{
	ASSERT(String_LastIndexOfI(String_Empty, String_FromC("This is a test."), 0) == -1);
}
END_TEST

START_TEST(LastIndexOfIShouldFindEmptyStringsInsideEmptyStrings)
{
	ASSERT(String_LastIndexOfI(String_Empty, String_Empty, -1) == -1);
	ASSERT(String_LastIndexOfI(String_Empty, String_Empty, 0) == 0);
	ASSERT(String_LastIndexOfI(String_Empty, String_Empty, 1) == 0);
}
END_TEST

START_TEST(LastIndexOfIShouldFindEmptyStringsBeforeAndAfterEveryCharacter)
{
	ASSERT(String_LastIndexOfI(String_FromC("This is a test."), String_Empty, -1) == -1);
	ASSERT(String_LastIndexOfI(String_FromC("This is a test."), String_Empty, 0) == 0);
	ASSERT(String_LastIndexOfI(String_FromC("This is a test."), String_Empty, 10) == 10);
	ASSERT(String_LastIndexOfI(String_FromC("This is a test."), String_Empty, 15) == 15);
	ASSERT(String_LastIndexOfI(String_FromC("This is a test."), String_Empty, 16) == 15);
}
END_TEST

START_TEST(LastIndexOfIFindsContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("TEST");
	String str3 = String_FromC("EmErGeNcY");
	String str4 = String_FromC(" sysTEM.");

	ASSERT(String_LastIndexOfI(str1, str2, 52) == 10);
	ASSERT(String_LastIndexOfI(str1, str3, 52) == 22);
	ASSERT(String_LastIndexOfI(str1, str4, 52) == 44);
}
END_TEST

START_TEST(LastIndexOfIFindsUnicodeContentWhenItExists)
{
	String str1 = String_FromC("This is a tesst of the emerg\xC3\x89ncy broadcasting syst\xC3\xA9m.");
	String str2 = String_FromC("TE\xC3\x9FT");
	String str3 = String_FromC("EmErG\xC3\xA9NcY");
	String str4 = String_FromC(" sysT\xC3\x89M.");

	ASSERT(String_LastIndexOfI(str1, str2, 55) == 10);
	ASSERT(String_LastIndexOfI(str1, str3, 55) == 23);
	ASSERT(String_LastIndexOfI(str1, str4, 55) == 46);
}
END_TEST

START_TEST(LastIndexOfIDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("emergnecy");		// Misspelled
	String str3 = String_FromC("broadcastink");		// Also misspelled
	String str4 = String_FromC("xyzzy");			// Not even a word
	String str5 = String_FromC("emerg\xC3\xA9ncy");	// E-with-accent is not E

	ASSERT(String_LastIndexOfI(str1, str2, 52) == -1);
	ASSERT(String_LastIndexOfI(str1, str3, 52) == -1);
	ASSERT(String_LastIndexOfI(str1, str4, 52) == -1);
	ASSERT(String_LastIndexOfI(str1, str5, 52) == -1);
}
END_TEST

START_TEST(LastIndexOfIStartsWhereYouTellItToStart)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("TeSt");
	String str3 = String_FromC("EmErGeNcY");

	ASSERT(String_LastIndexOfI(str1, str2, 52) == 10);
	ASSERT(String_LastIndexOfI(str1, str2, 14) == 10);
	ASSERT(String_LastIndexOfI(str1, str2, 13) == -1);
	ASSERT(String_LastIndexOfI(str1, str2, 5) == -1);

	ASSERT(String_LastIndexOfI(str1, str3, 52) == 22);
	ASSERT(String_LastIndexOfI(str1, str3, 30) == -1);
	ASSERT(String_LastIndexOfI(str1, str3, 31) == 22);
	ASSERT(String_LastIndexOfI(str1, str3, 32) == 22);
}
END_TEST

START_TEST(LastIndexOfIClipsStartIndexesToTheString)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("TEST");
	String str3 = String_FromC("EMERGENCY");

	ASSERT(String_LastIndexOfI(str1, str2, -100) == -1);
	ASSERT(String_LastIndexOfI(str1, str2, 13) == -1);
	ASSERT(String_LastIndexOfI(str1, str2, 14) == 10);
	ASSERT(String_LastIndexOfI(str1, str2, 15) == 10);
	ASSERT(String_LastIndexOfI(str1, str2, 200) == 10);

	ASSERT(String_LastIndexOfI(str1, str3, -100) == -1);
	ASSERT(String_LastIndexOfI(str1, str3, 30) == -1);
	ASSERT(String_LastIndexOfI(str1, str3, 31) == 22);
	ASSERT(String_LastIndexOfI(str1, str3, 32) == 22);
	ASSERT(String_LastIndexOfI(str1, str3, 100) == 22);
}
END_TEST

START_TEST(LastIndexOfIFindsSubsequentMatches)
{
	String str1 = String_FromC("This is a TEST is a tEsT is a TeSt is a test.");
	String str2 = String_FromC("test");

	ASSERT(String_LastIndexOfI(str1, str2, 5) == -1);
	ASSERT(String_LastIndexOfI(str1, str2, 10) == -1);
	ASSERT(String_LastIndexOfI(str1, str2, 15) == 10);
	ASSERT(String_LastIndexOfI(str1, str2, 20) == 10);
	ASSERT(String_LastIndexOfI(str1, str2, 25) == 20);
	ASSERT(String_LastIndexOfI(str1, str2, 30) == 20);
	ASSERT(String_LastIndexOfI(str1, str2, 35) == 30);
	ASSERT(String_LastIndexOfI(str1, str2, 40) == 30);
	ASSERT(String_LastIndexOfI(str1, str2, 45) == 40);
	ASSERT(String_LastIndexOfI(str1, str2, 50) == 40);
	ASSERT(String_LastIndexOfI(str1, str2, 55) == 40);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Case-Insensitive Contains Tests

START_TEST(ContainsIShouldFindNothingInEmptyStrings)
{
	ASSERT(!String_ContainsI(String_Empty, String_FromC("This is a test.")));
}
END_TEST

START_TEST(ContainsIShouldFindEmptyStringsInsideEmptyStrings)
{
	ASSERT(String_ContainsI(String_Empty, String_Empty));
}
END_TEST

START_TEST(ContainsIShouldFindEmptyStringsInEveryOtherString)
{
	ASSERT(String_ContainsI(String_FromC("This is a test."), String_Empty));
}
END_TEST

START_TEST(ContainsIFindsContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("TEST");
	String str3 = String_FromC("EmErGeNcY");
	String str4 = String_FromC(" sysTEM.");

	ASSERT(String_ContainsI(str1, str2));
	ASSERT(String_ContainsI(str1, str3));
	ASSERT(String_ContainsI(str1, str4));
}
END_TEST

START_TEST(ContainsIFindsUnicodeContentWhenItExists)
{
	String str1 = String_FromC("This is a tesst of the emerg\xC3\x89ncy broadcasting syst\xC3\xA9m.");
	String str2 = String_FromC("TE\xC3\x9FT");
	String str3 = String_FromC("EmErG\xC3\xA9NcY");
	String str4 = String_FromC(" sysT\xC3\x89M.");

	ASSERT(String_ContainsI(str1, str2));
	ASSERT(String_ContainsI(str1, str3));
	ASSERT(String_ContainsI(str1, str4));
}
END_TEST

START_TEST(ContainsIDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("emergnecy");		// Misspelled
	String str3 = String_FromC("broadcastink");		// Also misspelled
	String str4 = String_FromC("xyzzy");			// Not even a word
	String str5 = String_FromC("emerg\xC3\xA9ncy");	// E-with-accent is not E

	ASSERT(!String_ContainsI(str1, str2));
	ASSERT(!String_ContainsI(str1, str3));
	ASSERT(!String_ContainsI(str1, str4));
	ASSERT(!String_ContainsI(str1, str5));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Case-Insensitive Starts-With Tests

START_TEST(StartsWithIMatchesContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("THIS IS");
	String str3 = String_FromC("This is A TEST");
	String str4 = String_FromC("This is a test OF THE");

	ASSERT(String_StartsWithI(str1, str1));
	ASSERT(String_StartsWithI(str1, str2));
	ASSERT(String_StartsWithI(str1, str3));
	ASSERT(String_StartsWithI(str1, str4));
}
END_TEST

START_TEST(StartsWithIDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("Thsi is a test");	// Misspelled
	String str3 = String_FromC("xyzzy");			// Not even a word

	ASSERT(!String_StartsWithI(str1, str2));
	ASSERT(!String_StartsWithI(str1, str3));
}
END_TEST

START_TEST(StartsWithIAlwaysMatchesTheEmptyString)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_StartsWithI(str, String_Empty));
	ASSERT(String_StartsWithI(String_Empty, String_Empty));
}
END_TEST

START_TEST(StartsWithIFindsUnicodeContentWhenItExists)
{
	String str1 = String_FromC("This is a tesst of the emerg\xC3\x89ncy broadcasting syst\xC3\xA9m.");
	String str2 = String_FromC("This is a TE\xC3\x9FT");
	String str3 = String_FromC("EmErG\xC3\xA9NcY");

	ASSERT(String_StartsWithI(str1, str2));
	ASSERT(!String_StartsWithI(str2, str1));
	ASSERT(!String_StartsWithI(str1, str3));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Case-Insensitive Ends-With Tests

START_TEST(EndsWithIMatchesContentWhenItExists)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");
	String str2 = String_FromC("SYSTEM.");
	String str3 = String_FromC("BROADCASTING system.");
	String str4 = String_FromC("EMERGENCY broadcasting system.");

	ASSERT(String_EndsWithI(str1, str1));
	ASSERT(String_EndsWithI(str1, str2));
	ASSERT(String_EndsWithI(str1, str3));
	ASSERT(String_EndsWithI(str1, str4));
}
END_TEST

START_TEST(EndsWithIDoesNotFindContentWhenItDoesNotExist)
{
	String str1 = String_FromC("This is a test of the emergency broadcasting system.");

	String str2 = String_FromC("braodcasting ssytem");	// Misspelled
	String str3 = String_FromC("xyzzy");				// Not even a word

	ASSERT(!String_EndsWithI(str1, str2));
	ASSERT(!String_EndsWithI(str1, str3));
}
END_TEST

START_TEST(EndsWithIAlwaysMatchesTheEmptyString)
{
	String str = String_FromC("This is a test of the emergency broadcasting system.");

	ASSERT(String_EndsWithI(str, String_Empty));
	ASSERT(String_EndsWithI(String_Empty, String_Empty));
}
END_TEST

START_TEST(EndsWithIFindsUnicodeContentWhenItExists)
{
	String str1 = String_FromC("This is a tesst of the emerg\xC3\x89ncy broadcasting syst\xC3\xA9m.");
	String str2 = String_FromC("a TE\xC3\x9FT of THE eMeRg\xC3\xA9nCy broadcasting SYST\xC3\x89M.");
	String str3 = String_FromC("broadcasting SYST\xC3\x89m.");

	ASSERT(String_EndsWithI(str1, str2));
	ASSERT(!String_EndsWithI(str2, str1));
	ASSERT(String_EndsWithI(str1, str3));
}
END_TEST

#include "stringunicode_tests.generated.inc"
