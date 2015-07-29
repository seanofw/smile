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
//  Real-Number Diffing helper.

// Courtesy Bruce Dawson, https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// C port of Bruce's code, with some minor tweaks.  Like Bruce's, this doesn't play nice with zero.
// Don't use this function if you don't know what you're doing.
typedef union
{
	Int64 i;
	Real64 r;
} Real64Mash;

Inline Bool NearlyEqual(Real64 a, Real64 b, int maxUlpsDiff)
{
	Real64Mash am;
	Real64Mash bm;
	Int64 ulpsDiff;

	am.r = a;
	bm.r = b;

	if ((am.i >> 63) != (bm.i >> 63))
		return a == b;

	ulpsDiff = am.i - bm.i;
	if (ulpsDiff < 0) ulpsDiff = -ulpsDiff;

	return ulpsDiff <= maxUlpsDiff;
}

//-------------------------------------------------------------------------------------------------
//  Real-Number-Parsing Tests.

START_TEST(EmptyStringsShouldFailToBeParsedAsReals)
{
	Real64 result;
	ASSERT(String_ParseReal(String_Empty, 10, &result) == False);
	ASSERT(String_ParseReal(String_FromC("  \t\r\n  "), 10, &result) == False);
}
END_TEST

START_TEST(ThingsThatArentRealsShouldFailToBeParsed)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC("gronk"), 10, &result) == False);
	ASSERT(String_ParseReal(String_FromC("true"), 10, &result) == False);
	ASSERT(String_ParseReal(String_FromC(".!?:;"), 10, &result) == False);
	ASSERT(String_ParseReal(String_FromC("11xx"), 10, &result) == False);
	ASSERT(String_ParseReal(String_FromC("0x5a"), 10, &result) == False);
	ASSERT(String_ParseReal(String_FromC("fal"), 10, &result) == False);
}
END_TEST

START_TEST(ShouldParsePositiveDecimalRealsThatLookLikeIntegers)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC("0"), 10, &result) == True && result == 0);
	ASSERT(String_ParseReal(String_FromC("  1  "), 10, &result) == True && result == 1);
	ASSERT(String_ParseReal(String_FromC("3"), 10, &result) == True && result == 3);
	ASSERT(String_ParseReal(String_FromC("  42  "), 10, &result) == True && result == 42);
	ASSERT(String_ParseReal(String_FromC("123'456"), 10, &result) == True && result == 123456);
	ASSERT(String_ParseReal(String_FromC("  3'958'164'207  "), 10, &result) == True && result == (Real64)(Int64)3958164207);

	ASSERT(String_ParseReal(String_FromC("9'223'372'036'854'775'700"), 10, &result) == True && NearlyEqual(result, (Real64)Int64Max, 1));
}
END_TEST

START_TEST(ShouldParseNegativeDecimalRealsThatLookLikeIntegers)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC("-0"), 10, &result) == True && result == 0);
	ASSERT(String_ParseReal(String_FromC("  -1  "), 10, &result) == True && result == -1);
	ASSERT(String_ParseReal(String_FromC("-3"), 10, &result) == True && result == -3);
	ASSERT(String_ParseReal(String_FromC("  -42  "), 10, &result) == True && result == -42);
	ASSERT(String_ParseReal(String_FromC("-123'456"), 10, &result) == True && result == -123456);
	ASSERT(String_ParseReal(String_FromC("  -3'958'164'207  "), 10, &result) == True && result == -(Real64)(Int64)3958164207);

	ASSERT(String_ParseReal(String_FromC("-9'223'372'036'854'775'700"), 10, &result) == True && NearlyEqual(result, -(Real64)Int64Max, 1));
}
END_TEST

START_TEST(ShouldParsePositiveDecimalRealsThatAreOfTheFormIntegerDot)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC("0."), 10, &result) == True && result == 0);
	ASSERT(String_ParseReal(String_FromC("  1.  "), 10, &result) == True && result == 1);
	ASSERT(String_ParseReal(String_FromC("3."), 10, &result) == True && result == 3);
	ASSERT(String_ParseReal(String_FromC("  42.  "), 10, &result) == True && result == 42);
	ASSERT(String_ParseReal(String_FromC("123'456."), 10, &result) == True && result == 123456);
	ASSERT(String_ParseReal(String_FromC("  3'958'164'207.  "), 10, &result) == True && result == (Real64)(Int64)3958164207);
}
END_TEST

START_TEST(ShouldParseNegativeDecimalRealsThatAreOfTheFormIntegerDot)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC("-0."), 10, &result) == True && result == 0);
	ASSERT(String_ParseReal(String_FromC("  -1.  "), 10, &result) == True && result == -1);
	ASSERT(String_ParseReal(String_FromC("-3."), 10, &result) == True && result == -3);
	ASSERT(String_ParseReal(String_FromC("  -42.  "), 10, &result) == True && result == -42);
	ASSERT(String_ParseReal(String_FromC("-123'456."), 10, &result) == True && result == -123456);
	ASSERT(String_ParseReal(String_FromC("  -3'958'164'207.  "), 10, &result) == True && result == -(Real64)(Int64)3958164207);
}
END_TEST

START_TEST(ShouldParsePositiveDecimalRealsThatAreOfTheFormIntegerDotDigits)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC("0.0"), 10, &result) == True && result == 0.0);
	ASSERT(String_ParseReal(String_FromC("0.5"), 10, &result) == True && result == 0.5);
	ASSERT(String_ParseReal(String_FromC("  1.5  "), 10, &result) == True && result == 1.5);
	ASSERT(String_ParseReal(String_FromC("3.14159'26535"), 10, &result) == True && NearlyEqual(result, 3.1415926535, 1));
	ASSERT(String_ParseReal(String_FromC("  42.0  "), 10, &result) == True && result == 42.0);
	ASSERT(String_ParseReal(String_FromC("123'456.789"), 10, &result) == True && NearlyEqual(result, 123456.789, 1));
	ASSERT(String_ParseReal(String_FromC("  3'958'164.270'819  "), 10, &result) == True && NearlyEqual(result, 3958164.270819, 1));
}
END_TEST

START_TEST(ShouldParseNegativeDecimalRealsThatAreOfTheFormIntegerDotDigits)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC("-0.0"), 10, &result) == True && result == 0.0);
	ASSERT(String_ParseReal(String_FromC("-0.5"), 10, &result) == True && result == -0.5);
	ASSERT(String_ParseReal(String_FromC("  -1.5  "), 10, &result) == True && result == -1.5);
	ASSERT(String_ParseReal(String_FromC("-3.14159'26535"), 10, &result) == True && NearlyEqual(result, -3.1415926535, 1));
	ASSERT(String_ParseReal(String_FromC("  -42.0  "), 10, &result) == True && result == -42.0);
	ASSERT(String_ParseReal(String_FromC("-123'456.789"), 10, &result) == True && NearlyEqual(result, -123456.789, 1));
	ASSERT(String_ParseReal(String_FromC("  -3'958'164.270'819  "), 10, &result) == True && NearlyEqual(result, -3958164.270819, 1));
}
END_TEST

START_TEST(ShouldParsePositiveDecimalRealsThatAreOfTheFormDotDigits)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC(".0"), 10, &result) == True && result == 0.0);
	ASSERT(String_ParseReal(String_FromC(".5"), 10, &result) == True && result == 0.5);
	ASSERT(String_ParseReal(String_FromC("  .5  "), 10, &result) == True && result == 0.5);
	ASSERT(String_ParseReal(String_FromC("  .125  "), 10, &result) == True && result == 0.125);
	ASSERT(String_ParseReal(String_FromC(".14159'26535'89793'23"), 10, &result) == True && NearlyEqual(result, 0.14159265358979323, 1));
}
END_TEST

START_TEST(ShouldParseNegativeDecimalRealsThatAreOfTheFormDotDigits)
{
	Real64 result;
	ASSERT(String_ParseReal(String_FromC("-.0"), 10, &result) == True && result == 0.0);
	ASSERT(String_ParseReal(String_FromC("-.5"), 10, &result) == True && result == -0.5);
	ASSERT(String_ParseReal(String_FromC("  -.5  "), 10, &result) == True && result == -0.5);
	ASSERT(String_ParseReal(String_FromC("  -.125  "), 10, &result) == True && result == -0.125);
	ASSERT(String_ParseReal(String_FromC("-.14159'26535'89793'23"), 10, &result) == True && NearlyEqual(result, -0.14159265358979323, 1));
}
END_TEST

#include "stringparse_tests.generated.inc"
