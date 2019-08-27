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
#include <smile/smiletypes/numeric/smiletimestamp.h>

TEST_SUITE(TimestampTests)

//-------------------------------------------------------------------------------------------------
//  Composition Tests.

START_TEST(CanComposeNormalDates)
{
	SmileTimestamp timestamp;

	// 1970-01-01 00:00:00  -->  62135596800.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62135596800LL);
	ASSERT(timestamp->nanos == 0);

	// 1975-01-01 00:00:00  -->  62293363200.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1975, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62293363200LL);
	ASSERT(timestamp->nanos == 0);

	// 1981-03-01 00:00:00  -->  62487849600.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1981, .month = 3, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62487849600LL);
	ASSERT(timestamp->nanos == 0);

	// 1981-05-01 00:00:00  -->  62493120000.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1981, .month = 5, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62493120000LL);
	ASSERT(timestamp->nanos == 0);

	// 1990-12-31 00:00:00  -->  62798198400.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1990, .month = 12, .day = 31, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62798198400LL);
	ASSERT(timestamp->nanos == 0);
}
END_TEST

START_TEST(CanComposeDatesInvolvingLeapYears)
{
	SmileTimestamp timestamp;

	// Leap year, but doesn't matter.
	// 1972-01-01 00:00:00  -->  62198668800.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1972, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62198668800LL);
	ASSERT(timestamp->nanos == 0);

	// Leap year, but doesn't matter.
	// 1976-01-01 00:00:00  -->  62324899200.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1976, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62324899200LL);
	ASSERT(timestamp->nanos == 0);

	// NOT a leap year (by 100-year rule), and that fact matters.
	// 1900-03-01 00:00:00  -->  59931705600.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1900, .month = 3, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 59931705600LL);
	ASSERT(timestamp->nanos == 0);

	// Leap year, and that fact matters.
	// 1984-05-01 00:00:00  -->  62587814400.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1984, .month = 5, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62587814400LL);
	ASSERT(timestamp->nanos == 0);

	// Leap year, and that fact matters.
	// 1988-12-31 00:00:00  -->  62735126400.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1988, .month = 12, .day = 31, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62735126400LL);
	ASSERT(timestamp->nanos == 0);

	// IS a leap year (by 400-year rule), and that fact matters.
	// 2000-12-31 00:00:00  -->  63113817600.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 2000, .month = 12, .day = 31, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 63113817600LL);
	ASSERT(timestamp->nanos == 0);
}
END_TEST

START_TEST(CanComposeTimes)
{
	SmileTimestamp timestamp;

	// 1970-01-01 00:00:00  -->  62135596800.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0, .nanos = 0, });
	ASSERT(timestamp->seconds == 62135596800LL);
	ASSERT(timestamp->nanos == 0);

	// 1970-01-01 12:34:56  -->  62135642096.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1970, .month = 1, .day = 1, .hour = 12, .minute = 34, .second = 56, .nanos = 0, });
	ASSERT(timestamp->seconds == 62135642096LL);
	ASSERT(timestamp->nanos == 0);

	// 1970-01-01 23:59:59  -->  62135683199.0
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1970, .month = 1, .day = 1, .hour = 23, .minute = 59, .second = 59, .nanos = 0, });
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 0);

	// 1970-01-01 23:59:59.123456789  -->  62135683199.123456789
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1970, .month = 1, .day = 1, .hour = 23, .minute = 59, .second = 59, .nanos = 123456789, });
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 123456789);

	// 1970-01-01 23:59:59.999999999  -->  62135683199.999999999
	timestamp = SmileTimestamp_Compose(&(struct DecomposedTimestampStruct)
		{ .year = 1970, .month = 1, .day = 1, .hour = 23, .minute = 59, .second = 59, .nanos = 999999999, });
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 999999999);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  ISO 8601 Parsing Tests.

START_TEST(CanParseWellFormedIso8601DatesAndTimes)
{
	SmileTimestamp timestamp;

	// 1970-01-01 00:00:00  -->  62135596800.0
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T00:00:00Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135596800LL);
	ASSERT(timestamp->nanos == 0);

	// 1990-12-31 00:00:00  -->  62798198400.0
	ASSERT(SmileTimestamp_TryParse(String_FromC("1990-12-31T00:00:00Z"), &timestamp));
	ASSERT(timestamp->seconds == 62798198400LL);
	ASSERT(timestamp->nanos == 0);

	// 1970-01-01 12:34:56  -->  62135642096.0
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T12:34:56Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135642096LL);
	ASSERT(timestamp->nanos == 0);

	// 1970-01-01 23:59:59  -->  62135683199.0
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T23:59:59Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 0);

	// 1970-01-01 23:59:59 + whitespace  -->  62135683199.0
	ASSERT(SmileTimestamp_TryParse(String_FromC("   \t   1970-01-01T23:59:59Z   \r\n   "), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 0);

	// 1970-01-01 23:59:59.0  -->  62135683199.0
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T23:59:59.0Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 0);

	// 1970-01-01 23:59:59.1  -->  62135683199.1
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T23:59:59.1Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 100000000);

	// 1970-01-01 23:59:59.12345  -->  62135683199.12345
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T23:59:59.12345Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 123450000);

	// 1970-01-01 23:59:59.123456789  -->  62135683199.123456789
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T23:59:59.123456789Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 123456789);

	// 1970-01-01 23:59:59.0123456789  -->  62135683199.012345678
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T23:59:59.0123456789Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 12345678);

	// 1970-01-01 23:59:59.1234567891234  -->  62135683199.123456789
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T23:59:59.1234567891234Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 123456789);

	// 1970-01-01 23:59:59.999999999  -->  62135683199.999999999
	ASSERT(SmileTimestamp_TryParse(String_FromC("1970-01-01T23:59:59.999999999Z"), &timestamp));
	ASSERT(timestamp->seconds == 62135683199LL);
	ASSERT(timestamp->nanos == 999999999);
}
END_TEST

START_TEST(CannotParseMalformedIso8601DatesAndTimes)
{
	SmileTimestamp timestamp;

	ASSERT(!SmileTimestamp_TryParse(String_FromC(""), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("    "), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("1990"), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("19900101"), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("1990-01-01"), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("1990-01-01 12:34:56"), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("1990-01-01 12:34:56Z"), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("1990:01:01T12:34:56Z"), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("1990-01-01T12-34-56Z"), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("1990-01-01T99:99:99Z"), &timestamp));
	ASSERT(!SmileTimestamp_TryParse(String_FromC("1990-99-99T12:34:56Z"), &timestamp));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Decomposition Tests.

START_TEST(CanDecomposeNormalDates)
{
	SmileTimestamp timestamp;
	struct DecomposedTimestampStruct decomposed;

	// 62135596800.0  -->  1970-01-01 00:00:00
	timestamp = SmileTimestamp_Create(62135596800LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1970 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// 62293363200.0  -->  1975-01-01 00:00:00
	timestamp = SmileTimestamp_Create(62293363200LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1975 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// 62487849600.0  -->  1981-03-01 00:00:00
	timestamp = SmileTimestamp_Create(62487849600LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1981 && decomposed.month == 3 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// 62493120000.0  -->  1981-05-01 00:00:00
	timestamp = SmileTimestamp_Create(62493120000LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1981 && decomposed.month == 5 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// 62798198400.0  -->  1990-12-31 00:00:00
	timestamp = SmileTimestamp_Create(62798198400LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1990 && decomposed.month == 12 && decomposed.day == 31);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);
}
END_TEST

START_TEST(CanDecomposeDatesInvolvingLeapYears)
{
	SmileTimestamp timestamp;
	struct DecomposedTimestampStruct decomposed;

	// Leap year, but doesn't matter.
	// 62198668800.0  -->  1972-01-01 00:00:00
	timestamp = SmileTimestamp_Create(62198668800LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1972 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// Leap year, but doesn't matter.
	// 62324899200.0  -->  1976-01-01 00:00:00
	timestamp = SmileTimestamp_Create(62324899200LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1976 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// NOT a leap year (by 100-year rule), and that fact matters.
	// 59931705600.0  -->  1900-03-01 00:00:00
	timestamp = SmileTimestamp_Create(59931705600LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1900 && decomposed.month == 3 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// Leap year, and that fact matters.
	// 62587814400.0  -->  1984-05-01 00:00:00
	timestamp = SmileTimestamp_Create(62587814400LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1984 && decomposed.month == 5 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// Leap year, and that fact matters.
	// 62735126400.0  -->  1988-12-31 00:00:00
	timestamp = SmileTimestamp_Create(62735126400LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1988 && decomposed.month == 12 && decomposed.day == 31);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// IS a leap year (by 400-year rule), and that fact matters.
	// 63113817600.0  -->  2000-12-31 00:00:00
	timestamp = SmileTimestamp_Create(63113817600LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 2000 && decomposed.month == 12 && decomposed.day == 31);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);
}
END_TEST

START_TEST(CanDecomposeTimes)
{
	SmileTimestamp timestamp;
	struct DecomposedTimestampStruct decomposed;

	// 62135596800.0  -->  1970-01-01 00:00:00
	timestamp = SmileTimestamp_Create(62135596800LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1970 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 0 && decomposed.minute == 0 && decomposed.second == 0);
	ASSERT(decomposed.nanos == 0);

	// 62135642096.0  -->  1970-01-01 12:34:56
	timestamp = SmileTimestamp_Create(62135642096LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1970 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 12 && decomposed.minute == 34 && decomposed.second == 56);
	ASSERT(decomposed.nanos == 0);

	// 62135683199.0  -->  1970-01-01 23:59:59
	timestamp = SmileTimestamp_Create(62135683199LL, 0);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1970 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 23 && decomposed.minute == 59 && decomposed.second == 59);
	ASSERT(decomposed.nanos == 0);

	// 62135683199.123456789  -->  1970-01-01 23:59:59.123456789
	timestamp = SmileTimestamp_Create(62135683199LL, 123456789);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1970 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 23 && decomposed.minute == 59 && decomposed.second == 59);
	ASSERT(decomposed.nanos == 123456789);

	// 62135683199.999999999  -->  1970-01-01 23:59:59.999999999
	timestamp = SmileTimestamp_Create(62135683199LL, 999999999);
	SmileTimestamp_Decompose(timestamp, &decomposed, DecomposeAll);
	ASSERT(decomposed.year == 1970 && decomposed.month == 1 && decomposed.day == 1);
	ASSERT(decomposed.hour == 23 && decomposed.minute == 59 && decomposed.second == 59);
	ASSERT(decomposed.nanos == 999999999);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  ISO 8601 Stringification Tests.

START_TEST(CanStringifyIso8601DatesAndTimes)
{
	SmileTimestamp timestamp;
	String string;

	// 62135596800.0  -->  1970-01-01 00:00:00
	timestamp = SmileTimestamp_Create(62135596800LL, 0);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1970-01-01T00:00:00Z", 20);

	// 62798198400.0  -->  1990-12-31 00:00:00
	timestamp = SmileTimestamp_Create(62798198400LL, 0);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1990-12-31T00:00:00Z", 20);

	// 62135642096.0  -->  1970-01-01 12:34:56
	timestamp = SmileTimestamp_Create(62135642096LL, 0);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1970-01-01T12:34:56Z", 20);

	// 62135683199.0  -->  1970-01-01 23:59:59
	timestamp = SmileTimestamp_Create(62135683199LL, 0);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1970-01-01T23:59:59Z", 20);

	// 62135683199.1  -->  1970-01-01 23:59:59.1
	timestamp = SmileTimestamp_Create(62135683199LL, 100000000);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1970-01-01T23:59:59.1Z", 22);

	// 62135683199.12345  -->  1970-01-01 23:59:59.12345
	timestamp = SmileTimestamp_Create(62135683199LL, 123450000);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1970-01-01T23:59:59.12345Z", 26);

	// 62135683199.123456789  -->  1970-01-01 23:59:59.123456789
	timestamp = SmileTimestamp_Create(62135683199LL, 123456789);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1970-01-01T23:59:59.123456789Z", 30);

	// 62135683199.012345678  --> 1970-01-01 23:59:59.012345678
	timestamp = SmileTimestamp_Create(62135683199LL, 12345678);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1970-01-01T23:59:59.012345678Z", 30);

	// 62135683199.999999999  -->  1970-01-01 23:59:59.999999999
	timestamp = SmileTimestamp_Create(62135683199LL, 999999999);
	string = SmileTimestamp_Stringify(timestamp);
	ASSERT_STRING(string, "1970-01-01T23:59:59.999999999Z", 30);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unix Conversion Tests.

START_TEST(CanConvertUnixEpochValuesToTimestamps)
{
	SmileTimestamp timestamp;

	// Unix 0  -->  1970-01-01 00:00:00  -->  62135596800.0
	timestamp = SmileTimestamp_FromUnix(0LL, 0);
	ASSERT(timestamp->seconds == 62135596800LL);
	ASSERT(timestamp->nanos == 0);

	// Unix 1  -->  1970-01-01 00:00:01  -->  62135596801.0
	timestamp = SmileTimestamp_FromUnix(1LL, 0);
	ASSERT(timestamp->seconds == 62135596801LL);
	ASSERT(timestamp->nanos == 0);

	// Unix -1  -->  1969-12-31 12:59:59  -->  62135596799.0
	timestamp = SmileTimestamp_FromUnix(-1LL, 0);
	ASSERT(timestamp->seconds == 62135596799LL);
	ASSERT(timestamp->nanos == 0);

	// Unix 0x7FFFFFFF  -->  2038-01-19 03:14:07  -->  64283080447.0
	timestamp = SmileTimestamp_FromUnix(0x7FFFFFFFLL, 0);
	ASSERT(timestamp->seconds == 64283080447LL);
	ASSERT(timestamp->nanos == 0);

	// Unix 1566206356  -->  2019-08-19 09:19:16  -->  63701803156.0
	timestamp = SmileTimestamp_FromUnix(1566206356, 0);
	ASSERT(timestamp->seconds == 63701803156LL);
	ASSERT(timestamp->nanos == 0);
}
END_TEST

START_TEST(CanConvertTimestampValuesToUnixEpoch)
{
	Int64 unixSeconds;

	// Unix 0  -->  1970-01-01 00:00:00  -->  62135596800.0
	unixSeconds = SmileTimestamp_ToUnix(SmileTimestamp_Create(62135596800LL, 0));
	ASSERT(unixSeconds == 0);

	// Unix 1  -->  1975-01-01 00:00:01  -->  62293363201.0
	unixSeconds = SmileTimestamp_ToUnix(SmileTimestamp_Create(62135596801LL, 0));
	ASSERT(unixSeconds == 1);

	// Unix -1  -->  1969-12-31 12:59:59  -->  62135596799.0
	unixSeconds = SmileTimestamp_ToUnix(SmileTimestamp_Create(62135596799LL, 0));
	ASSERT(unixSeconds == -1);

	// Unix 0x7FFFFFFF  -->  2038-01-19 03:14:07  -->  64283080447.0
	unixSeconds = SmileTimestamp_ToUnix(SmileTimestamp_Create(64283080447LL, 0));
	ASSERT(unixSeconds == 0x7FFFFFFF);

	// Unix 1566206356  -->  2019-08-19 09:19:16  -->  63701803156.0
	unixSeconds = SmileTimestamp_ToUnix(SmileTimestamp_Create(63701803156LL, 0));
	ASSERT(unixSeconds == 1566206356);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Windows Conversion Tests.

START_TEST(CanConvertWindowsFiletimesToTimestamps)
{
	SmileTimestamp timestamp;

	// Windows 0  -->  1601-01-01 00:00:00  -->  50491123200.0
	timestamp = SmileTimestamp_FromWindows(0LL);
	ASSERT(timestamp->seconds == 50491123200LL);
	ASSERT(timestamp->nanos == 0);

	// Windows 1  -->  1601-01-01 00:00:00.0000001  -->  50491123200.000000100
	timestamp = SmileTimestamp_FromWindows(1LL);
	ASSERT(timestamp->seconds == 50491123200LL);
	ASSERT(timestamp->nanos == 100);

	// Windows 116444736000000000  -->  1970-01-01 00:00:00  -->  62135596800.0
	timestamp = SmileTimestamp_FromWindows(116444736000000000LL);
	ASSERT(timestamp->seconds == 62135596800LL);
	ASSERT(timestamp->nanos == 0);

	// Windows 116444736000000001  -->  1970-01-01 00:00:00.00000001  -->  62135596800.000000100
	timestamp = SmileTimestamp_FromWindows(116444736000000001LL);
	ASSERT(timestamp->seconds == 62135596800LL);
	ASSERT(timestamp->nanos == 100);

	// Windows 116444736009999999  -->  1970-01-01 00:00:00.9999999  -->  62135596800.999999900
	timestamp = SmileTimestamp_FromWindows(116444736009999999LL);
	ASSERT(timestamp->seconds == 62135596800LL);
	ASSERT(timestamp->nanos == 999999900);

	// Windows 116444736010000000  -->  1970-01-01 00:00:01  -->  62135596801.0
	timestamp = SmileTimestamp_FromWindows(116444736010000000LL);
	ASSERT(timestamp->seconds == 62135596801LL);
	ASSERT(timestamp->nanos == 0);

	// Windows 132106799560000000  -->  2019-08-19 09:19:16  -->  63701803156.0
	timestamp = SmileTimestamp_FromWindows(132106799560000000LL);
	ASSERT(timestamp->seconds == 63701803156LL);
	ASSERT(timestamp->nanos == 0);
}
END_TEST

START_TEST(CanConvertNegativeWindowsFiletimesToTimestamps)
{
	SmileTimestamp timestamp;

	// Windows -1  -->  1600-12-31 12:59:59.9999999  -->  50491123199.999999900
	timestamp = SmileTimestamp_FromWindows(-1LL);
	ASSERT(timestamp->seconds == 50491123199LL);
	ASSERT(timestamp->nanos == 999999900);

	// Windows -9999998  -->  1600-12-31 12:59:59.0000002  -->  50491123199.000000200
	timestamp = SmileTimestamp_FromWindows(-9999998LL);
	ASSERT(timestamp->seconds == 50491123199LL);
	ASSERT(timestamp->nanos == 200);

	// Windows -9999999  -->  1600-12-31 12:59:59.0000001  -->  50491123199.000000100
	timestamp = SmileTimestamp_FromWindows(-9999999LL);
	ASSERT(timestamp->seconds == 50491123199LL);
	ASSERT(timestamp->nanos == 100);

	// Windows -10000000  -->  1600-12-31 12:59:59  -->  50491123199.0
	timestamp = SmileTimestamp_FromWindows(-10000000LL);
	ASSERT(timestamp->seconds == 50491123199LL);
	ASSERT(timestamp->nanos == 0);

	// Windows -10000001  -->  1600-12-31 12:59:58.9999999  -->  50491123198.999999900
	timestamp = SmileTimestamp_FromWindows(-10000001LL);
	ASSERT(timestamp->seconds == 50491123198LL);
	ASSERT(timestamp->nanos == 999999900);

	// Windows -504911232000000000  -->  0000-01-01 00:00:00  -->  0.0
	timestamp = SmileTimestamp_FromWindows(-504911232000000000LL);
	ASSERT(timestamp->seconds == 0);
	ASSERT(timestamp->nanos == 0);
}
END_TEST

START_TEST(CanConvertTimestampValuesToWindowsFiletimes)
{
	Int64 ticks;

	// 50491123200.000000000  -->  1601-01-01 00:00:00  -->  Windows 0
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123200LL, 0));
	ASSERT(ticks == 0);

	// 50491123200.000000049  -->  1601-01-01 00:00:00.0000000  -->  Windows 0
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123200LL, 49));
	ASSERT(ticks == 0);

	// 50491123200.000000050  -->  1601-01-01 00:00:00.0000001  -->  Windows 1
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123200LL, 50));
	ASSERT(ticks == 1);

	// 50491123200.000000100  -->  1601-01-01 00:00:00.0000001  -->  Windows 1
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123200LL, 100));
	ASSERT(ticks == 1);

	// 62135596800.000000000  -->  1970-01-01 00:00:00  -->  Windows 116444736000000000
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(62135596800LL, 0));
	ASSERT(ticks == 116444736000000000LL);

	// 62135596800.000000100  -->  1970-01-01 00:00:00.00000001  -->  Windows 116444736000000001
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(62135596800LL, 100));
	ASSERT(ticks == 116444736000000001LL);

	// 62135596800.999999900  -->  1970-01-01 00:00:00.9999999  -->  Windows 116444736009999999
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(62135596800LL, 999999900));
	ASSERT(ticks == 116444736009999999LL);

	// 62135596801.000000000  -->  1970-01-01 00:00:01  -->  Windows 116444736010000000
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(62135596801LL, 0));
	ASSERT(ticks == 116444736010000000LL);

	// 63701803156.000000000  -->  2019-08-19 09:19:16  -->  Windows 132106799560000000
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(63701803156LL, 0));
	ASSERT(ticks == 132106799560000000LL);

	// 63701803156.000000049  -->  2019-08-19 09:19:16  -->  Windows 132106799560000000
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(63701803156LL, 49));
	ASSERT(ticks == 132106799560000000LL);

	// 63701803156.000000050  -->  2019-08-19 09:19:16  -->  Windows 132106799560000001
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(63701803156LL, 50));
	ASSERT(ticks == 132106799560000001LL);
}
END_TEST

START_TEST(CanConvertTimestampsToNegativeWindowsFiletimes)
{
	Int64 ticks;

	// 50491123199.999999900  -->  1600-12-31 12:59:59.9999999  --> Windows -1
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123199LL, 999999900));
	ASSERT(ticks == -1LL);

	// 50491123199.000000200  -->  1600-12-31 12:59:59.0000002  -->  Windows -9999998
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123199LL, 200));
	ASSERT(ticks == -9999998LL);

	// 50491123199.000000100  -->  1600-12-31 12:59:59.0000001  -->  Windows -9999999
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123199LL, 100));
	ASSERT(ticks == -9999999LL);

	// 50491123199.0  -->  1600-12-31 12:59:59  -->  Windows -10000000
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123199LL, 0));
	ASSERT(ticks == -10000000LL);

	// 50491123198.999999900  -->  1600-12-31 12:59:58.9999999  -->  Windows -10000001
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(50491123198LL, 999999900));
	ASSERT(ticks == -10000001LL);

	// 0.0  -->  0000-01-01 00:00:00  -->  Windows -504911232000000000
	ticks = SmileTimestamp_ToWindows(SmileTimestamp_Create(0, 0));
	ASSERT(ticks == -504911232000000000LL);
}
END_TEST

#include "timestamp_tests.generated.inc"
