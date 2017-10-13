//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2017 Sean Werkema
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

TEST_SUITE(StringIdentTests)

//-------------------------------------------------------------------------------------------------
//  CamelCase tests.

START_TEST(CamelCaseShouldReturnNullGivenNull)
{
	String actualResult = String_CamelCase(NULL, False, False);
	ASSERT(actualResult == NULL);
}
END_TEST

START_TEST(CamelCaseCanConvertInJavaStyleMode)
{
	static const char *stringPairs[] = {
		// Null case.
		"", "",

		// Idempotent changes.
		"foo", "foo",
		"foo2", "foo2",
		"foo234", "foo234",
		"1234", "1234",

		// Actual transforms.
		"foo_bar", "fooBar",
		"Foo_bar", "fooBar",
		"foo_bar_baz_qux", "fooBarBazQux",
		"foo_Bar_baz_Qux", "fooBarBazQux",
		"foo_BAR_baz_Qux", "fooBARBazQux",
		"foo_1234_bar", "foo1234Bar",

		// Weird corner cases.
		"__foo__", "foo",
		"__foo__bar__", "fooBar",
		"__FOO__BAR__", "fooBAR",
		"__1234__", "1234",
		"c_string", "cString",
		"VINCString", "vinCString",
	};

	Int i;

	for (i = 0; i < sizeof(stringPairs) / sizeof(const char *); i += 2) {
		String originalValue = String_FromC(stringPairs[i]);
		const char *expectedResult = stringPairs[i + 1];

		String actualResult = String_CamelCase(originalValue, False, False);

		ASSERT(String_EqualsC(actualResult, expectedResult));
	}
}
END_TEST

START_TEST(CamelCaseCanConvertInDotNetStyleMode)
{
	static const char *stringPairs[] = {
		// Null case.
		"", "",

		// Idempotent changes.
		"foo", "Foo",
		"foo2", "Foo2",
		"foo234", "Foo234",
		"1234", "1234",

		// Actual transforms.
		"foo_bar", "FooBar",
		"Foo_bar", "FooBar",
		"foo_bar_baz_qux", "FooBarBazQux",
		"foo_Bar_baz_Qux", "FooBarBazQux",
		"foo_BAR_baz_Qux", "FooBARBazQux",
		"foo_1234_bar", "Foo1234Bar",

		// Weird corner cases.
		"__foo__", "Foo",
		"__foo__bar__", "FooBar",
		"__FOO__BAR__", "FOOBAR",
		"__1234__", "1234",
		"c_string", "CString",
		"VINCString", "VINCString",
	};

	Int i;

	for (i = 0; i < sizeof(stringPairs) / sizeof(const char *); i += 2) {
		String originalValue = String_FromC(stringPairs[i]);
		const char *expectedResult = stringPairs[i + 1];

		String actualResult = String_CamelCase(originalValue, True, False);

		ASSERT(String_EqualsC(actualResult, expectedResult));
	}
}
END_TEST

START_TEST(CamelCaseCanConvertInJavaStyleModeWithAcronymConversion)
{
	static const char *stringPairs[] = {
		// Null case.
		"", "",

		// Idempotent changes.
		"foo", "foo",
		"foo2", "foo2",
		"foo234", "foo234",
		"1234", "1234",

		// Actual transforms.
		"foo_bar", "fooBar",
		"Foo_bar", "fooBar",
		"foo_bar_baz_qux", "fooBarBazQux",
		"foo_Bar_baz_Qux", "fooBarBazQux",
		"foo_BAR_baz_Qux", "fooBarBazQux",
		"foo_1234_bar", "foo1234Bar",

		// Weird corner cases.
		"__foo__", "foo",
		"__foo__bar__", "fooBar",
		"__FOO__BAR__", "fooBar",
		"__1234__", "1234",
		"c_string", "cString",
		"VINCString", "vinCString",
	};

	Int i;

	for (i = 0; i < sizeof(stringPairs) / sizeof(const char *); i += 2) {
		String originalValue = String_FromC(stringPairs[i]);
		const char *expectedResult = stringPairs[i + 1];

		String actualResult = String_CamelCase(originalValue, False, True);

		ASSERT(String_EqualsC(actualResult, expectedResult));
	}
}
END_TEST

START_TEST(CamelCaseCanConvertInDotNetStyleModeWithAcronymConversion)
{
	static const char *stringPairs[] = {
		// Null case.
		"", "",

		// Idempotent changes.
		"foo", "Foo",
		"foo2", "Foo2",
		"foo234", "Foo234",
		"1234", "1234",

		// Actual transforms.
		"foo_bar", "FooBar",
		"Foo_bar", "FooBar",
		"foo_bar_baz_qux", "FooBarBazQux",
		"foo_Bar_baz_Qux", "FooBarBazQux",
		"foo_BAR_baz_Qux", "FooBarBazQux",
		"foo_1234_bar", "Foo1234Bar",

		// Weird corner cases.
		"__foo__", "Foo",
		"__foo__bar__", "FooBar",
		"__FOO__BAR__", "FooBar",
		"__1234__", "1234",
		"c_string", "CString",
		"VINCString", "VinCString",
	};

	Int i;

	for (i = 0; i < sizeof(stringPairs) / sizeof(const char *); i += 2) {
		String originalValue = String_FromC(stringPairs[i]);
		const char *expectedResult = stringPairs[i + 1];

		String actualResult = String_CamelCase(originalValue, True, True);

		ASSERT(String_EqualsC(actualResult, expectedResult));
	}
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Hyphenize tests.

START_TEST(HyphenizeShouldReturnNullGivenNull)
{
	String actualResult = String_Hyphenize(NULL, '-');
	ASSERT(actualResult == NULL);
}
END_TEST

START_TEST(HyphenizeCanConvertStrings)
{
	static const char *stringPairs[] = {
		// Null case.
		"", "",

		// Idempotent changes.
		"foo", "foo",
		"foo2", "foo2",
		"foo234", "foo234",
		"1234", "1234",

		// Actual transforms.
		"fooBar", "foo-bar",
		"FooBar", "foo-bar",
		"Foo_123", "foo-123",
		"Foo2Bar", "foo2-bar",
		"fooBarBazQux", "foo-bar-baz-qux",

		// Weird corner cases.
		"__foo__", "foo",
		"__foo__bar__", "foo-bar",
		"__FOO__BAR__", "foo-bar",
		"__1234__", "1234",
		"c_string", "c-string",
		"CString", "cstring",
		"VINCString", "vin-cstring",
	};

	Int i;

	for (i = 0; i < sizeof(stringPairs) / sizeof(const char *); i += 2) {
		String originalValue = String_FromC(stringPairs[i]);
		const char *expectedResult = stringPairs[i + 1];

		String actualResult = String_Hyphenize(originalValue, '-');

		ASSERT(String_EqualsC(actualResult, expectedResult));
	}
}
END_TEST


#include "stringident_tests.generated.inc"
