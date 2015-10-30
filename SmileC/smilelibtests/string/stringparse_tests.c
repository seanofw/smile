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

TEST_SUITE(StringParseTests)

//-------------------------------------------------------------------------------------------------
//  Boolean-Parsing Tests.

START_TEST(EmptyStringsShouldFailToBeParsedAsBooleans)
{
	Bool result;
	ASSERT(String_ParseBool(String_Empty, &result) == False);
	ASSERT(String_ParseBool(String_FromC("  \t\r\n  "), &result) == False);
}
END_TEST

START_TEST(ThingsThatArentBooleanShouldFailToBeParsed)
{
	Bool result;
	ASSERT(String_ParseBool(String_FromC("gronk"), &result) == False);
	ASSERT(String_ParseBool(String_FromC("93"), &result) == False);
	ASSERT(String_ParseBool(String_FromC(".!?:;"), &result) == False);
	ASSERT(String_ParseBool(String_FromC("tru"), &result) == False);
	ASSERT(String_ParseBool(String_FromC("truerly"), &result) == False);
	ASSERT(String_ParseBool(String_FromC("fal"), &result) == False);
	ASSERT(String_ParseBool(String_FromC("falsehood"), &result) == False);
}
END_TEST

START_TEST(KnownBooleanTruthValuesShouldParseAsTrue)
{
	Bool result;
	ASSERT(String_ParseBool(String_FromC("1"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC("t"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC("T"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC("true"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC("TRUE"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC("True"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC("tRuE"), &result) == True && result == True);
}
END_TEST

START_TEST(WhitespaceShouldNotAffectBooleanTruthValueParsing)
{
	Bool result;
	ASSERT(String_ParseBool(String_FromC(" \t1\r\n"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC(" \tt\r\n"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC(" \ttrue\r\n"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC(" \tTRUE\r\n"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC(" \tTrue\r\n"), &result) == True && result == True);
	ASSERT(String_ParseBool(String_FromC(" \ttRuE\r\n"), &result) == True && result == True);
}
END_TEST

START_TEST(KnownBooleanFalsehoodValuesShouldParseAsFalse)
{
	Bool result;
	ASSERT(String_ParseBool(String_FromC("0"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC("f"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC("F"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC("false"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC("FALSE"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC("False"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC("fAlSe"), &result) == True && result == False);
}
END_TEST

START_TEST(WhitespaceShouldNotAffectBooleanFalsehoodValueParsing)
{
	Bool result;
	ASSERT(String_ParseBool(String_FromC(" \t0\r\n"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC(" \tf\r\n"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC(" \tfalse\r\n"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC(" \tFALSE\r\n"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC(" \tFalse\r\n"), &result) == True && result == False);
	ASSERT(String_ParseBool(String_FromC(" \tfAlSe\r\n"), &result) == True && result == False);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Integer-Parsing Tests.

START_TEST(EmptyStringsShouldFailToBeParsedAsIntegers)
{
	Int64 result;
	ASSERT(String_ParseInteger(String_Empty, 10, &result) == False);
	ASSERT(String_ParseInteger(String_FromC("  \t\r\n  "), 10, &result) == False);
}
END_TEST

START_TEST(ThingsThatArentIntegersShouldFailToBeParsed)
{
	Int64 result;
	ASSERT(String_ParseInteger(String_FromC("gronk"), 10, &result) == False);
	ASSERT(String_ParseInteger(String_FromC("true"), 10, &result) == False);
	ASSERT(String_ParseInteger(String_FromC(".!?:;"), 10, &result) == False);
	ASSERT(String_ParseInteger(String_FromC("11xx"), 10, &result) == False);
	ASSERT(String_ParseInteger(String_FromC("0x5a"), 10, &result) == False);
	ASSERT(String_ParseInteger(String_FromC("fal"), 10, &result) == False);
}
END_TEST

START_TEST(ShouldParsePositiveDecimalIntegers)
{
	Int64 result;
	ASSERT(String_ParseInteger(String_FromC("0"), 10, &result) == True && result == 0);
	ASSERT(String_ParseInteger(String_FromC("  1  "), 10, &result) == True && result == 1);
	ASSERT(String_ParseInteger(String_FromC("3"), 10, &result) == True && result == 3);
	ASSERT(String_ParseInteger(String_FromC("  42  "), 10, &result) == True && result == 42);
	ASSERT(String_ParseInteger(String_FromC("123456"), 10, &result) == True && result == 123456);
	ASSERT(String_ParseInteger(String_FromC("  3958164207  "), 10, &result) == True && result == (Int64)3958164207);
	ASSERT(String_ParseInteger(String_FromC("9223372036854775807"), 10, &result) == True && result == Int64Max);
}
END_TEST

START_TEST(ShouldParsePositiveHexIntegers)
{
	Int64 result;
	ASSERT(String_ParseInteger(String_FromC("0"), 16, &result) == True && result == 0);
	ASSERT(String_ParseInteger(String_FromC("  1  "), 16, &result) == True && result == 1);
	ASSERT(String_ParseInteger(String_FromC("3"), 16, &result) == True && result == 3);
	ASSERT(String_ParseInteger(String_FromC("  1f  "), 16, &result) == True && result == 0x1F);
	ASSERT(String_ParseInteger(String_FromC("42"), 16, &result) == True && result == 0x42);
	ASSERT(String_ParseInteger(String_FromC("  a83  "), 16, &result) == True && result == 0xA83);
	ASSERT(String_ParseInteger(String_FromC("123456"), 16, &result) == True && result == 0x123456);
	ASSERT(String_ParseInteger(String_FromC("  1e48da9b  "), 16, &result) == True && result == 0x1e48da9b);
	ASSERT(String_ParseInteger(String_FromC("7fffffffffffffff"), 16, &result) == True && result == Int64Max);
}
END_TEST

START_TEST(ShouldParseNegativeDecimalIntegers)
{
	Int64 result;
	ASSERT(String_ParseInteger(String_FromC("  -1  "), 10, &result) == True && result == -1);
	ASSERT(String_ParseInteger(String_FromC("-3"), 10, &result) == True && result == -3);
	ASSERT(String_ParseInteger(String_FromC("  -42  "), 10, &result) == True && result == -42);
	ASSERT(String_ParseInteger(String_FromC("-123456"), 10, &result) == True && result == -123456);
	ASSERT(String_ParseInteger(String_FromC("  -3958164207  "), 10, &result) == True && result == -(Int64)3958164207);
	ASSERT(String_ParseInteger(String_FromC("-9223372036854775808"), 10, &result) == True && result == Int64Min);
}
END_TEST

START_TEST(ShouldParseNegativeHexIntegers)
{
	Int64 result;
	ASSERT(String_ParseInteger(String_FromC("  -1  "), 16, &result) == True && result == -1);
	ASSERT(String_ParseInteger(String_FromC("-3"), 16, &result) == True && result == -3);
	ASSERT(String_ParseInteger(String_FromC("  -1f  "), 16, &result) == True && result == -0x1F);
	ASSERT(String_ParseInteger(String_FromC("-42"), 16, &result) == True && result == -0x42);
	ASSERT(String_ParseInteger(String_FromC("  -a83  "), 16, &result) == True && result == -0xA83);
	ASSERT(String_ParseInteger(String_FromC("-123456"), 16, &result) == True && result == -0x123456);
	ASSERT(String_ParseInteger(String_FromC("  -1e48da9b  "), 16, &result) == True && result == -0x1e48da9b);
	ASSERT(String_ParseInteger(String_FromC("-8000000000000000"), 16, &result) == True && result == Int64Min);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Float-Number Diffing helper.

// Courtesy Bruce Dawson, https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// C port of Bruce's code, with some minor tweaks.  Like Bruce's, this doesn't play nice with zero.
// Don't use this function if you don't know what you're doing.
typedef union
{
	Int64 i;
	Float64 r;
} Float64Mash;

Inline UInt64 BinaryDiff(Float64 a, UInt64 b)
{
	Float64Mash am;
	Float64Mash bm;
	Int64 ulpsDiff;

	am.r = a;
	bm.i = b;

	if ((am.i >> 63) != (bm.i >> 63))
		return 0;

	ulpsDiff = am.i - bm.i;
	if (ulpsDiff < 0) ulpsDiff = -ulpsDiff;

	return ulpsDiff;
}

//-------------------------------------------------------------------------------------------------
//  Float-Number-Parsing Tests.

START_TEST(EmptyStringsShouldFailToBeParsedAsFloats)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_Empty, 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC("  \t\r\n  "), 10, &result) == False);
}
END_TEST

START_TEST(ThingsThatArentFloatsShouldFailToBeParsed)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("gronk"), 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC("true"), 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC(".!?:;"), 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC("11xx"), 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC("0x5a"), 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC("fal"), 10, &result) == False);
}
END_TEST

START_TEST(ThingsThatAreSimilarToFloatsButNotFloatsShouldFailToBeParsed)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("-123.7f"), 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC("- 123.7"), 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC("-123. 7"), 10, &result) == False);
	ASSERT(String_ParseFloat(String_FromC("-123 .7"), 10, &result) == False);
}
END_TEST

START_TEST(ShouldParsePositiveDecimalFloatsThatLookLikeIntegers)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("0"), 10, &result) == True && result == 0);
	ASSERT(String_ParseFloat(String_FromC("  1  "), 10, &result) == True && result == 1);
	ASSERT(String_ParseFloat(String_FromC("3"), 10, &result) == True && result == 3);
	ASSERT(String_ParseFloat(String_FromC("  42  "), 10, &result) == True && result == 42);
	ASSERT(String_ParseFloat(String_FromC("123'456"), 10, &result) == True && result == 123456);
	ASSERT(String_ParseFloat(String_FromC("  3'958'164'207  "), 10, &result) == True && result == (Float64)(Int64)3958164207);

	ASSERT(String_ParseFloat(String_FromC("9'223'372'036'854'775'700"), 10, &result) == True && result == (Float64)Int64Max);
}
END_TEST

START_TEST(ShouldParseNegativeDecimalFloatsThatLookLikeIntegers)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("-0"), 10, &result) == True && result == 0);
	ASSERT(String_ParseFloat(String_FromC("  -1  "), 10, &result) == True && result == -1);
	ASSERT(String_ParseFloat(String_FromC("-3"), 10, &result) == True && result == -3);
	ASSERT(String_ParseFloat(String_FromC("  -42  "), 10, &result) == True && result == -42);
	ASSERT(String_ParseFloat(String_FromC("-123'456"), 10, &result) == True && result == -123456);
	ASSERT(String_ParseFloat(String_FromC("  -3'958'164'207  "), 10, &result) == True && result == -(Float64)(Int64)3958164207);

	ASSERT(String_ParseFloat(String_FromC("-9'223'372'036'854'775'700"), 10, &result) == True && result == -(Float64)Int64Max);
}
END_TEST

START_TEST(ShouldParsePositiveDecimalFloatsThatAreOfTheFormIntegerDot)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("0."), 10, &result) == True && result == 0);
	ASSERT(String_ParseFloat(String_FromC("  1.  "), 10, &result) == True && result == 1);
	ASSERT(String_ParseFloat(String_FromC("3."), 10, &result) == True && result == 3);
	ASSERT(String_ParseFloat(String_FromC("  42.  "), 10, &result) == True && result == 42);
	ASSERT(String_ParseFloat(String_FromC("123'456."), 10, &result) == True && result == 123456);
	ASSERT(String_ParseFloat(String_FromC("  3'958'164'207.  "), 10, &result) == True && result == (Float64)(Int64)3958164207);
}
END_TEST

START_TEST(ShouldParseNegativeDecimalFloatsThatAreOfTheFormIntegerDot)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("-0."), 10, &result) == True && result == 0);
	ASSERT(String_ParseFloat(String_FromC("  -1.  "), 10, &result) == True && result == -1);
	ASSERT(String_ParseFloat(String_FromC("-3."), 10, &result) == True && result == -3);
	ASSERT(String_ParseFloat(String_FromC("  -42.  "), 10, &result) == True && result == -42);
	ASSERT(String_ParseFloat(String_FromC("-123'456."), 10, &result) == True && result == -123456);
	ASSERT(String_ParseFloat(String_FromC("  -3'958'164'207.  "), 10, &result) == True && result == -(Float64)(Int64)3958164207);
}
END_TEST

START_TEST(ShouldParsePositiveDecimalFloatsThatAreOfTheFormIntegerDotDigits)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("0.0"), 10, &result) == True && result == 0.0);
	ASSERT(String_ParseFloat(String_FromC("0.5"), 10, &result) == True && result == 0.5);
	ASSERT(String_ParseFloat(String_FromC("  1.5  "), 10, &result) == True && result == 1.5);
	ASSERT(String_ParseFloat(String_FromC("3.14159'26535"), 10, &result) == True && result == 3.1415926535);
	ASSERT(String_ParseFloat(String_FromC("  42.0  "), 10, &result) == True && result == 42.0);
	ASSERT(String_ParseFloat(String_FromC("123'456.789"), 10, &result) == True && result == 123456.789);
	ASSERT(String_ParseFloat(String_FromC("  3'958'164.270'819  "), 10, &result) == True && result == 3958164.270819);
}
END_TEST

START_TEST(ShouldParseNegativeDecimalFloatsThatAreOfTheFormIntegerDotDigits)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("-0.0"), 10, &result) == True && result == 0.0);
	ASSERT(String_ParseFloat(String_FromC("-0.5"), 10, &result) == True && result == -0.5);
	ASSERT(String_ParseFloat(String_FromC("  -1.5  "), 10, &result) == True && result == -1.5);
	ASSERT(String_ParseFloat(String_FromC("-3.14159'26535"), 10, &result) == True && result == -3.1415926535);
	ASSERT(String_ParseFloat(String_FromC("  -42.0  "), 10, &result) == True && result == -42.0);
	ASSERT(String_ParseFloat(String_FromC("-123'456.789"), 10, &result) == True && result == -123456.789);
	ASSERT(String_ParseFloat(String_FromC("  -3'958'164.270'819  "), 10, &result) == True && result == -3958164.270819);
}
END_TEST

START_TEST(ShouldParsePositiveDecimalFloatsThatAreOfTheFormDotDigits)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC(".0"), 10, &result) == True && result == 0.0);
	ASSERT(String_ParseFloat(String_FromC(".5"), 10, &result) == True && result == 0.5);
	ASSERT(String_ParseFloat(String_FromC("  .5  "), 10, &result) == True && result == 0.5);
	ASSERT(String_ParseFloat(String_FromC("  .125  "), 10, &result) == True && result == 0.125);
	ASSERT(String_ParseFloat(String_FromC(".14159'26535'89793'23"), 10, &result) == True && result == 0.14159265358979323);
}
END_TEST

START_TEST(ShouldParseNegativeDecimalFloatsThatAreOfTheFormDotDigits)
{
	Float64 result;
	ASSERT(String_ParseFloat(String_FromC("-.0"), 10, &result) == True && result == 0.0);
	ASSERT(String_ParseFloat(String_FromC("-.5"), 10, &result) == True && result == -0.5);
	ASSERT(String_ParseFloat(String_FromC("  -.5  "), 10, &result) == True && result == -0.5);
	ASSERT(String_ParseFloat(String_FromC("  -.125  "), 10, &result) == True && result == -0.125);
	ASSERT(String_ParseFloat(String_FromC("-.14159'26535'89793'23"), 10, &result) == True && result == -0.14159265358979323);
}
END_TEST

START_TEST(ShouldParsePositiveExponents)
{
	Float64 result;
	Bool parsed;

	parsed = String_ParseFloat(String_FromC("1.0e5"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x40f86a0000000000ULL) == 0);

	parsed = String_ParseFloat(String_FromC("1.0e+5"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x40f86a0000000000ULL) == 0);

	parsed = String_ParseFloat(String_FromC("1.2345e17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x437b6951ef585a00ULL) == 0);

	parsed = String_ParseFloat(String_FromC("1.2345e+17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x437b6951ef585a00ULL) == 0);

	parsed = String_ParseFloat(String_FromC(".12345e17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x4345eddb25e04800ULL) == 0);

	parsed = String_ParseFloat(String_FromC("0.12345e17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x4345eddb25e04800ULL) == 0);

	parsed = String_ParseFloat(String_FromC("-1.0e5"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xc0f86a0000000000ULL) == 0);

	parsed = String_ParseFloat(String_FromC("-1.0e+5"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xc0f86a0000000000ULL) == 0);

	parsed = String_ParseFloat(String_FromC("-1.2345e17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xc37b6951ef585a00ULL) == 0);

	parsed = String_ParseFloat(String_FromC("-1.2345e+17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xc37b6951ef585a00ULL) == 0);

	parsed = String_ParseFloat(String_FromC("-.12345e17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xc345eddb25e04800ULL) == 0);

	parsed = String_ParseFloat(String_FromC("-0.12345e17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xc345eddb25e04800ULL) == 0);
}
END_TEST

START_TEST(ShouldParseNegativeExponents)
{
	Float64 result;
	Bool parsed;

	parsed = String_ParseFloat(String_FromC("1.0e-5"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3ee4f8b588e368f1ULL) == 0);

	parsed = String_ParseFloat(String_FromC("-1.0e-5"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xbee4f8b588e368f1ULL) == 0);

	parsed = String_ParseFloat(String_FromC("1.2345e-17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3c6c7733a7c7d2fcULL) <= 1);

	parsed = String_ParseFloat(String_FromC("-1.2345e-17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xbc6c7733a7c7d2fcULL) <= 1);

	parsed = String_ParseFloat(String_FromC(".12345e-17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3c36c5c2ec9fdbfdULL) == 0);

	parsed = String_ParseFloat(String_FromC("-.12345e-17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xbc36c5c2ec9fdbfdULL) == 0);

	parsed = String_ParseFloat(String_FromC("0.12345e-17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3c36c5c2ec9fdbfdULL) == 0);

	parsed = String_ParseFloat(String_FromC("-0.12345e-17"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0xbc36c5c2ec9fdbfdULL) == 0);
}
END_TEST

// TODO: Mandate all strings parse perfectly to the nearest IEEE 754 value.  This is HARD.
//       See David M. Gay's dtoa.c library for a possible future enhancement.

// All of these numbers come from articles on http://www.exploringbinary.com .
// Big shout-out to Rick Regan for discovering all these useful test cases
// (even if we don't pass them all within 0 ULPs yet).
START_TEST(CertainProblematicNumbersParseExactly)
{
	Float64 result;
	Bool parsed;

	parsed = String_ParseFloat(String_FromC("6'929'495'644'600'919.5"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x43389e56ee5e7a58ULL) == 0);

	parsed = String_ParseFloat(String_FromC("3.74557'44005'95258'3e15"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x432a9d28ff412a75ULL) == 0);

	parsed = String_ParseFloat(String_FromC("9'214'843'084'008'499"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x43405e6cec57761aULL) == 0);

	parsed = String_ParseFloat(String_FromC("1'777'820'000'000'000'000'001"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x4458180d5bad2e3eULL) == 0);

	parsed = String_ParseFloat(String_FromC("0.39329'22657'273"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3fd92bb352c4623aULL) == 0);

	parsed = String_ParseFloat(String_FromC("0.50000'00000'00000'16653'34536'93773'48106'35447'50213'62304'6875"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3fe0000000000002ULL) == 0);

	parsed = String_ParseFloat(String_FromC("0.50000'00000'00000'16656'05587'48085'61867'43949'36533'64479'54177'85644'53125"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3fe0000000000002ULL) == 0);

	parsed = String_ParseFloat(String_FromC("0.50000'00000'00000'16654'70062'20929'54986'89698'43373'63392'11463'92822'26562'5"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3fe0000000000002ULL) == 0);
}
END_TEST

// All of these numbers come from articles on http://www.exploringbinary.com .
// Big shout-out to Rick Regan for discovering all these useful test cases
// (even if we don't pass them all within 0 ULPs yet).
START_TEST(VeryProblematicNumbersParseWithinOneOrTwoUlps)
{
	Float64 result;
	Bool parsed;

	parsed = String_ParseFloat(String_FromC(".00000'00000'00000'00000'14159'26535'89793'23"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x3b9abf0026c70bbdULL) <= 1);

	parsed = String_ParseFloat(String_FromC("6.92949'56446'00919'5e15"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x43389e56ee5e7a58ULL) <= 1);

	parsed = String_ParseFloat(String_FromC("30'078'505'129'381'147'446'200"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x44997a3c7271b021ULL) <= 1);

	parsed = String_ParseFloat(String_FromC("2.22507'38585'07201'2e-308"), 10, &result);
	ASSERT(parsed == True && BinaryDiff(result, 0x0010000000000000ULL) <= 2);
}
END_TEST

#include "stringparse_tests.generated.inc"
