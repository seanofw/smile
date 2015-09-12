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

//-------------------------------------------------------------------------------------------------
//  ToLower Tests

START_TEST(ToLowerDoesNothingToEmptyAndWhitespaceStrings)
{
	ASSERT_STRING(String_ToLower(String_Empty), "", 0);
	ASSERT_STRING(String_ToLower(String_FromC("  \t\r\n  ")), "  \t\r\n  ", 7);
}
END_TEST

START_TEST(ToLowerConvertsAsciiToLowercase)
{
	ASSERT_STRING(String_ToLower(String_FromC("This IS A tEsT.")), "this is a test.", 15);
	ASSERT_STRING(String_ToLower(String_FromC("PACK MY BOX WITH FIVE DOZEN LIQUOR JUGS.")), "pack my box with five dozen liquor jugs.", 40);
}
END_TEST

START_TEST(ToLowerConvertsUnicodeToLowercase)
{
	ASSERT_STRING(String_ToLower(String_FromC("PACK MY BOX WITH FIVE DOZ\xC3\x89N LIQUOR JUGS.")), "pack my box with five doz\xC3\xA9n liquor jugs.", 41);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  ToUpper Tests

START_TEST(ToUpperDoesNothingToEmptyAndWhitespaceStrings)
{
	ASSERT_STRING(String_ToUpper(String_Empty), "", 0);
	ASSERT_STRING(String_ToUpper(String_FromC("  \t\r\n  ")), "  \t\r\n  ", 7);
}
END_TEST

START_TEST(ToUpperConvertsAsciiToUppercase)
{
	ASSERT_STRING(String_ToUpper(String_FromC("This IS A tEsT.")), "THIS IS A TEST.", 15);
	ASSERT_STRING(String_ToUpper(String_FromC("pack my box with five dozen liquor jugs.")), "PACK MY BOX WITH FIVE DOZEN LIQUOR JUGS.", 40);
}
END_TEST

START_TEST(ToUpperConvertsUnicodeToUppercase)
{
	ASSERT_STRING(String_ToUpper(String_FromC("pack my box with five doz\xC3\xA9n liquor jugs.")), "PACK MY BOX WITH FIVE DOZ\xC3\x89N LIQUOR JUGS.", 41);
	ASSERT_STRING(String_ToUpper(String_FromC("pack my box with five dozen liquor jug\xC3\x9F.")), "PACK MY BOX WITH FIVE DOZEN LIQUOR JUGSS.", 41);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  ToTitle Tests

START_TEST(ToTitleDoesNothingToEmptyAndWhitespaceStrings)
{
	ASSERT_STRING(String_ToTitle(String_Empty), "", 0);
	ASSERT_STRING(String_ToTitle(String_FromC("  \t\r\n  ")), "  \t\r\n  ", 7);
}
END_TEST

START_TEST(ToTitleConvertsAsciiToUppercase)
{
	ASSERT_STRING(String_ToTitle(String_FromC("This IS A tEsT.")), "THIS IS A TEST.", 15);
	ASSERT_STRING(String_ToTitle(String_FromC("pack my box with five dozen liquor jugs.")), "PACK MY BOX WITH FIVE DOZEN LIQUOR JUGS.", 40);
}
END_TEST

START_TEST(ToTitleConvertsUnicodeToUppercase)
{
	ASSERT_STRING(String_ToTitle(String_FromC("pack my box with five doz\xC3\xA9n liquor jugs.")), "PACK MY BOX WITH FIVE DOZ\xC3\x89N LIQUOR JUGS.", 41);
}
END_TEST

START_TEST(ToTitleConvertsCertainCompoundUnicodeCodePointsToTitlecase)
{
	ASSERT_STRING(String_ToTitle(String_FromC("pack my box with five dozen liquor jug\xC3\x9F.")), "PACK MY BOX WITH FIVE DOZEN LIQUOR JUGSs.", 41);

	ASSERT_STRING(String_ToTitle(String_FromC("This is a DZ: \xC7\x84.")), "THIS IS A DZ: \xC7\x85.", 17);
	ASSERT_STRING(String_ToTitle(String_FromC("This is a Dz: \xC7\x85.")), "THIS IS A DZ: \xC7\x85.", 17);
	ASSERT_STRING(String_ToTitle(String_FromC("This is a dz: \xC7\x86.")), "THIS IS A DZ: \xC7\x85.", 17);

	ASSERT_STRING(String_ToTitle(String_FromC("This is a LJ: \xC7\x87.")), "THIS IS A LJ: \xC7\x88.", 17);
	ASSERT_STRING(String_ToTitle(String_FromC("This is a Lj: \xC7\x88.")), "THIS IS A LJ: \xC7\x88.", 17);
	ASSERT_STRING(String_ToTitle(String_FromC("This is a lj: \xC7\x89.")), "THIS IS A LJ: \xC7\x88.", 17);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  CaseFold Tests

START_TEST(CaseFoldDoesNothingToEmptyAndWhitespaceStrings)
{
	ASSERT_STRING(String_CaseFold(String_Empty), "", 0);
	ASSERT_STRING(String_CaseFold(String_FromC("  \t\r\n  ")), "  \t\r\n  ", 7);
}
END_TEST

START_TEST(CaseFoldConvertsAsciiToLowercase)
{
	ASSERT_STRING(String_CaseFold(String_FromC("This IS A tEsT.")), "this is a test.", 15);
	ASSERT_STRING(String_CaseFold(String_FromC("PACK MY BOX WITH FIVE DOZEN LIQUOR JUGS.")), "pack my box with five dozen liquor jugs.", 40);
}
END_TEST

START_TEST(CaseFoldConvertsUnicodeToComparableForm)
{
	ASSERT_STRING(String_CaseFold(String_FromC("PACK MY BOX WITH FIVE DOZ\xC3\x89N LIQUOR JUGS.")), "pack my box with five doz\xC3\xA9n liquor jugs.", 41);
	ASSERT_STRING(String_CaseFold(String_FromC("pack my box with five dozen liquor jug\xC3\x9F.")), "pack my box with five dozen liquor jugss.", 41);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unicode Decomposition Tests

START_TEST(DecomposeDoesNothingToEmptyAndWhitespaceAndAsciiStrings)
{
	ASSERT_STRING(String_Decompose(String_Empty), "", 0);
	ASSERT_STRING(String_Decompose(String_FromC("  \t\r\n  ")), "  \t\r\n  ", 7);
	ASSERT_STRING(String_Decompose(String_FromC("This is a test.")), "This is a test.", 15);
	ASSERT_STRING(String_Decompose(String_FromC("Pack my box with five dozen liquor jugs.")), "Pack my box with five dozen liquor jugs.", 40);
}
END_TEST

START_TEST(DecomposeDisassemblesCompoundAccentedCharacters)
{
	ASSERT_STRING(String_Decompose(String_FromC("trouv\xC3\xA9.")), "trouve\xCC\x81.", 9);
	ASSERT_STRING(String_Decompose(String_FromC("\xC3\xA0 bient\xC3\xB4t.")), "a\xCC\x80 biento\xCC\x82t.", 14);
}
END_TEST

START_TEST(DecomposeIgnoresNonAccentedUnicodeCharacters)
{
	ASSERT_STRING(String_Decompose(String_FromC("tr\xC3\x97uv\xC3\xB0.")), "tr\xC3\x97uv\xC3\xB0.", 9);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unicode Composition Tests

START_TEST(ComposeDoesNothingToEmptyAndWhitespaceAndAsciiStrings)
{
	ASSERT_STRING(String_Compose(String_Empty), "", 0);
	ASSERT_STRING(String_Compose(String_FromC("  \t\r\n  ")), "  \t\r\n  ", 7);
	ASSERT_STRING(String_Compose(String_FromC("This is a test.")), "This is a test.", 15);
	ASSERT_STRING(String_Compose(String_FromC("Pack my box with five dozen liquor jugs.")), "Pack my box with five dozen liquor jugs.", 40);
}
END_TEST

START_TEST(ComposeAssemblesCompoundAccentedCharacters)
{
	ASSERT_STRING(String_Compose(String_FromC("trouve\xCC\x81.")), "trouv\xC3\xA9.", 8);
	ASSERT_STRING(String_Compose(String_FromC("a\xCC\x80 biento\xCC\x82t.")), "\xC3\xA0 bient\xC3\xB4t.", 12);
}
END_TEST

START_TEST(ComposeIgnoresNonAccentedUnicodeCharacters)
{
	ASSERT_STRING(String_Compose(String_FromC("tr\xC3\x97uv\xC3\xB0.")), "tr\xC3\x97uv\xC3\xB0.", 9);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unicode Combining-Character Normalization Tests

START_TEST(NormalizeDoesNothingToEmptyAndWhitespaceAndAsciiStrings)
{
	ASSERT_STRING(String_Normalize(String_Empty), "", 0);
	ASSERT_STRING(String_Normalize(String_FromC("  \t\r\n  ")), "  \t\r\n  ", 7);
	ASSERT_STRING(String_Normalize(String_FromC("This is a test.")), "This is a test.", 15);
	ASSERT_STRING(String_Normalize(String_FromC("Pack my box with five dozen liquor jugs.")), "Pack my box with five dozen liquor jugs.", 40);
}
END_TEST

START_TEST(NormalizeIgnoresCompoundCharactersWithoutSeparatedDiacritics)
{
	ASSERT_STRING(String_Normalize(String_FromC("trouv\xC3\xA9.")), "trouv\xC3\xA9.", 8);
	ASSERT_STRING(String_Normalize(String_FromC("\xC3\xA0 bient\xC3\xB4t.")), "\xC3\xA0 bient\xC3\xB4t.", 12);
}
END_TEST

START_TEST(NormalizeIgnoresSingleAccents)
{
	ASSERT_STRING(String_Normalize(String_FromC("trouve\xCC\x81.")), "trouve\xCC\x81.", 9);
	ASSERT_STRING(String_Normalize(String_FromC("a\xCC\x80 biento\xCC\x82t.")), "a\xCC\x80 biento\xCC\x82t.", 14);
}
END_TEST

START_TEST(NormalizeIgnoresNonAccentedUnicodeCharacters)
{
	ASSERT_STRING(String_Normalize(String_FromC("tr\xC3\x97uv\xC3\xB0.")), "tr\xC3\x97uv\xC3\xB0.", 9);
}
END_TEST

START_TEST(NormalizeSortsPairsOfDiacriticsCorrectly)
{
	// 'x' with dot below followed by dot above can be left alone.
	ASSERT_STRING(String_Normalize(String_FromC("x\xCC\xA3\xCC\x87.")), "x\xCC\xA3\xCC\x87.", 6);

	// 'x' with dot above followed by dot below must be transformed.
	ASSERT_STRING(String_Normalize(String_FromC("x\xCC\x87\xCC\xA3.")), "x\xCC\xA3\xCC\x87.", 6);
}
END_TEST

START_TEST(NormalizeSortsManyDiacriticsCorrectly)
{
	// The test below uses the diacritics in this table; the code points (and UTF-8 "magic numbers")
	// are provided here for reference.
	//
	// Tilde overlay           U+0334:  \xCC\xB4:  class 1
	// Palatalized hook below  U+0321:  \xCC\xA1:  class 202
	// Horn                    U+031B:  \xCC\x9B:  class 216
	// Ring below              U+0325:  \xCC\xA5:  class 220
	// Ring above              U+030A:  \xCC\x8A:  class 230
	// Grave                   U+0300:  \xCC\x80:  class 230
	// Acute                   U+0301:  \xCC\x81:  class 230
	// Left angle above        U+031A:  \xCC\x9A:  class 232
	// Double inverted breve   U+0361:  \xCD\xA1:  class 234

	// Try all of the diacritics in forward (sorted) order.  Should be unchanged.
	ASSERT_STRING(String_Normalize(String_FromC("x\xCC\xB4\xCC\xA1\xCC\x9B\xCC\xA5\xCC\x8A\xCC\x80\xCC\x81\xCC\x9A\xCD\xA1.")),
		"x\xCC\xB4\xCC\xA1\xCC\x9B\xCC\xA5\xCC\x8A\xCC\x80\xCC\x81\xCC\x9A\xCD\xA1.", 20);

	// Try all of the diacritics in reverse order.  Should come out reversed, with the class-230 diacritics
	// still in reverse order (i.e., a stable sort was used).
	ASSERT_STRING(String_Normalize(String_FromC("x\xCD\xA1\xCC\x9A\xCC\x8A\xCC\x80\xCC\x81\xCC\xA5\xCC\x9B\xCC\xA1\xCC\xB4.")),
		"x\xCC\xB4\xCC\xA1\xCC\x9B\xCC\xA5\xCC\x8A\xCC\x80\xCC\x81\xCC\x9A\xCD\xA1.", 20);

	// Try all of the diacritics in a random shuffly order.  The class-230 diacritics should stay in order.
	ASSERT_STRING(String_Normalize(String_FromC("x\xCC\xA5\xCC\xA1\xCC\x80\xCC\x9A\xCC\x8A\xCC\xB4\xCD\xA1\xCC\x81\xCC\x9B.")),
		"x\xCC\xB4\xCC\xA1\xCC\x9B\xCC\xA5\xCC\x80\xCC\x8A\xCC\x81\xCC\x9A\xCD\xA1.", 20);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unicode Character-Extraction Tests.

START_TEST(ExtractUnicodeCharacterCorrectlyRecognizesUtf8Sequences)
{
	String str = String_FromC("a\xCC\x80 biento\xCC\x82t. \xC3\xA0 bient\xC3\xB4t.");
	const Int32 expectedResult[] = {
		'a', 0x0300, ' ', 'b', 'i', 'e', 'n', 't', 'o', 0x0302, 't', '.',
		' ', 0x00E0, ' ', 'b', 'i', 'e', 'n', 't', 0x00F4, 't', '.',
	};
	Int i, src;
	Int32 ch;

	for (i = 0, src = 0; i < sizeof(expectedResult) / sizeof(Int32); i++) {
		ch = String_ExtractUnicodeCharacter(str, &src);
		ASSERT(ch == expectedResult[i]);
	}
}
END_TEST

START_TEST(ExtractUnicodeCharacterInternalCorrectlyRecognizesUtf8Sequences)
{
	String str = String_FromC("a\xCC\x80 biento\xCC\x82t. \xC3\xA0 bient\xC3\xB4t.");
	const Int32 expectedResult[] = {
		'a', 0x0300, ' ', 'b', 'i', 'e', 'n', 't', 'o', 0x0302, 't', '.',
		' ', 0x00E0, ' ', 'b', 'i', 'e', 'n', 't', 0x00F4, 't', '.',
	};
	Int i;
	Int32 ch;
	const Byte *src, *end;

	src = String_GetBytes(str);
	end = src + String_Length(str);
	for (i = 0; i < sizeof(expectedResult) / sizeof(Int32); i++) {
		ch = String_ExtractUnicodeCharacterInternal(&src, end);
		ASSERT(ch == expectedResult[i]);
	}
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unicode Code-Page Conversion Tests

START_TEST(ConvertingFromUtf8ToLatin1ConvertsLatin1CodePoints)
{
	String str = String_FromC("\xC3\xA0 bient\xC3\xB4t.");
	ASSERT_STRING(String_ConvertUtf8ToKnownCodePage(str, LEGACY_CODE_PAGE_ISO_8859_1), "\xE0 bient\xF4t.", 10);
}
END_TEST

START_TEST(ConvertingFromUtf8ToLatin1ConvertsNonLatin1CodePointsToQuestionMarks)
{
	String str = String_FromC("a\xCC\x80 biento\xCC\x82t.");
	ASSERT_STRING(String_ConvertUtf8ToKnownCodePage(str, LEGACY_CODE_PAGE_ISO_8859_1), "a? biento?t.", 12);
}
END_TEST

START_TEST(ConvertingFromLatin1ToUtf8ConvertsLatin1CodePointsToCombinedForms)
{
	String str = String_FromC("\xE0 bient\xF4t.");
	ASSERT_STRING(String_ConvertKnownCodePageToUtf8(str, LEGACY_CODE_PAGE_ISO_8859_1), "\xC3\xA0 bient\xC3\xB4t.", 12);
}
END_TEST

START_TEST(ConvertingToWindows1252IsNotTheSameAsLatin1)
{
	String str = String_FromC("\x8A\xE0 \x93\x62ient\xF4t.\x94");
	ASSERT_STRING(String_ConvertKnownCodePageToUtf8(str, LEGACY_CODE_PAGE_ISO_8859_1), "\xC2\x8A\xC3\xA0 \xC2\x93\x62ient\xC3\xB4t.\xC2\x94", 18);

	str = String_FromC("\x8A\xE0 \x93\x62ient\xF4t.\x94");
	ASSERT_STRING(String_ConvertKnownCodePageToUtf8(str, LEGACY_CODE_PAGE_WIN1252), "\xC5\xA0\xC3\xA0 \xE2\x80\x9C\x62ient\xC3\xB4t.\xE2\x80\x9D", 20);
}
END_TEST

START_TEST(ConvertingFromWindows1252IsNotTheSameAsLatin1)
{
	String str = String_FromC("\xC2\x8A\xC3\xA0 \xC2\x93\x62ient\xC3\xB4t.\xC2\x94");
	ASSERT_STRING(String_ConvertUtf8ToKnownCodePage(str, LEGACY_CODE_PAGE_ISO_8859_1), "\x8A\xE0 \x93\x62ient\xF4t.\x94", 13);

	str = String_FromC("\xC5\xA0\xC3\xA0 \xE2\x80\x9C\x62ient\xC3\xB4t.\xE2\x80\x9D");
	ASSERT_STRING(String_ConvertUtf8ToKnownCodePage(str, LEGACY_CODE_PAGE_WIN1252), "\x8A\xE0 \x93\x62ient\xF4t.\x94", 13);
}
END_TEST

START_TEST(ConvertingToAnUnknownCodePageResultsInEmptyString)
{
	String str = String_FromC("\xE0 bient\xF4t.");
	ASSERT_STRING(String_ConvertKnownCodePageToUtf8(str, 12), "", 0);
	ASSERT_STRING(String_ConvertUtf8ToKnownCodePage(str, 12), "", 0);
}
END_TEST

#include "stringunicode_tests.generated.inc"
