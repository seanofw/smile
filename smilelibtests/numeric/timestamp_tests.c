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

#include "timestamp_tests.generated.inc"
