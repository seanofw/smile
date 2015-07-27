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
	ASSERT(String_ParseReal(String_FromC("123456"), 10, &result) == True && result == 123456);
	ASSERT(String_ParseReal(String_FromC("  3958164207  "), 10, &result) == True && result == (Real64)(Int64)3958164207);

	// Courtesy of the weirdness of floating-point rounding, this should also pass.
	ASSERT(String_ParseReal(String_FromC("9223372036854775700"), 10, &result) == True && result == (Real64)Int64Max);
}
END_TEST

#include "stringparse_tests.generated.inc"
