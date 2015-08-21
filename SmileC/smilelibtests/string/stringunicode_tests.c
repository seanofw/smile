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

/*
SMILE_API Int String_IndexOfI(const String str, const String pattern, Int start);
SMILE_API Int String_LastIndexOfI(const String str, const String pattern, Int start);
SMILE_API Bool String_ContainsI(const String str, const String pattern);
SMILE_API Bool String_StartsWithI(const String str, const String pattern);
SMILE_API Bool String_EndsWithI(const String str, const String pattern);
*/

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

#include "stringunicode_tests.generated.inc"
