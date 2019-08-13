//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#include <smile/numeric/real64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/numeric/smiletimestamp.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>

//-------------------------------------------------------------------------------------------------
// Definitions, and static data.

#define SECONDS_PER_MINUTE (60)
#define SECONDS_PER_HOUR (60 * SECONDS_PER_MINUTE)
#define SECONDS_PER_DAY (24 * SECONDS_PER_HOUR)
#define SECONDS_PER_WEEK (7 * SECONDS_PER_DAY)
#define DAYS_PER_400_YEARS (365 * 400 + 97)
#define DAYS_PER_100_YEARS (365 * 100 + 24)
#define DAYS_PER_4_YEARS (365 * 4 + 1)

#define SECONDS_PER_YEAR (31556952LL)	// == 60 * 60 * 24 * 365.2425

#define ABSOLUTE_ZERO_YEAR (-292277022399LL)
#define INTERNAL_YEAR (1LL)
#define ABSOLUTE_TO_INTERNAL ((ABSOLUTE_ZERO_YEAR - INTERNAL_YEAR) * SECONDS_PER_YEAR)
#define INTERNAL_TO_ABSOLUTE (-ABSOLUTE_TO_INTERNAL)
#define INTERNAL_TO_UNIX ((Int64)(1969*365 + 1969/4 - 1969/100 + 1969/400) * SECONDS_PER_DAY)
#define SECONDS_FROM_WINDOWS_TO_UNIX_EPOCH (11644473600LL)
#define SECONDS_FROM_UNIX_TO_WINDOWS_EPOCH (-SECONDS_FROM_WINDOWS_TO_UNIX_EPOCH)
#define UNIX_TO_INTERNAL (-INTERNAL_TO_UNIX)

#define WINDOWS_TICKS_PER_SECOND (10000000)
#define WINDOWS_TICKS_PER_NANOSECOND (100)

SMILE_EASY_OBJECT_VTABLE(SmileTimestamp);

//-------------------------------------------------------------------------------------------------
// Core creation function.

/// <summary>
/// Create a timestamp object, which consists of a count of seconds since the
/// start of the epoch, and a count of nanoseconds since the start of the given
/// second.  This function should ONLY be used internally, since the count of
/// seconds will vary if the absolute year is changed.
/// </summary>
/// <param name="seconds">The number of seconds since the start of the epoch.
/// Currently, the epoch starts at midnight on January 1, year -292277022399 UTC,
/// using the Gregorian calendar, but this may change in future releases.</param>
/// <param name="nanos">The number of nanoseconds since the start of the given second.</param>
/// <returns>A new SmileTimestamp object that represents the given seconds/nanos tuple.</returns>
SmileTimestamp SmileTimestamp_Create(Int64 seconds, UInt32 nanos)
{
	SmileTimestamp timestamp = GC_MALLOC_STRUCT(struct SmileTimestampInt);
	if (timestamp == NULL) Smile_Abort_OutOfMemory();
	timestamp->base = (SmileObject)Smile_KnownBases.Timestamp;
	timestamp->kind = SMILE_KIND_TIMESTAMP;
	timestamp->vtable = SmileTimestamp_VTable;
	timestamp->seconds = seconds;
	timestamp->nanos = nanos;
	return timestamp;
}

//-------------------------------------------------------------------------------------------------
// Conversion to/from Unix and Windows equivalents.

/// <summary>
/// Create a SmileTimestamp object from a count of seconds since the start of the
/// Unix epoch on midnight, January 1, 1970 UTC.
/// </summary>
/// <param name="seconds">The number of seconds since the start of the Unix epoch.</param>
/// <param name="nanos">The number of nanoseconds since the start of the given second.</param>
/// <returns>A new SmileTimestamp object that represents the given time.</returns>
SmileTimestamp SmileTimestamp_FromUnix(Int64 secondsSinceEpoch, UInt32 nanos)
{
	return SmileTimestamp_Create(secondsSinceEpoch + UNIX_TO_INTERNAL, nanos);
}

/// <summary>
/// Convert a SmileTimestamp to a count of seconds since the start of the Unix epoch
/// on midnight, January 1, 1970 UTC (discarding the nanosecond-level resolution).
/// </summary>
/// <returns>The equivalent count of seconds since the start of the Unix epoch.</returns>
Int64 SmileTimestamp_ToUnix(SmileTimestamp timestamp)
{
	return timestamp->seconds + INTERNAL_TO_UNIX;
}

/// <summary>
/// Create a SmileTimestamp object from a count of 100-nanosecond ticks since the start
/// of the Windows epoch on midnight, January 1, 1601 UTC.
/// </summary>
/// <param name="ticks">The number of ticks since the start of the Windows epoch.</param>
/// <returns>A new SmileTimestamp object that represents the given time.</returns>
SmileTimestamp SmileTimestamp_FromWindows(Int64 ticks)
{
	Int64 seconds = ticks / WINDOWS_TICKS_PER_SECOND;
	UInt32 nanos = (ticks % WINDOWS_TICKS_PER_SECOND) * WINDOWS_TICKS_PER_NANOSECOND;
	return SmileTimestamp_Create(seconds + SECONDS_FROM_WINDOWS_TO_UNIX_EPOCH + UNIX_TO_INTERNAL, nanos);
}

/// <summary>
/// Convert a SmileTimestamp to a count of 100-nanosecond ticks since the start of the
/// Windows epoch on midnight, January 1, 1601 UTC.
/// </summary>
/// <returns>The equivalent count of 100-nanosecond ticks since the start of the Windows epoch.</returns>
Int64 SmileTimestamp_ToWindows(SmileTimestamp timestamp)
{
	Int64 seconds = timestamp->seconds + INTERNAL_TO_UNIX + SECONDS_FROM_UNIX_TO_WINDOWS_EPOCH;
	Int64 ticks = timestamp->nanos / WINDOWS_TICKS_PER_NANOSECOND;
	ticks += seconds * WINDOWS_TICKS_PER_NANOSECOND;
	return ticks;
}

//-------------------------------------------------------------------------------------------------
// Timestamp properties.

static SmileObject SmileTimestamp_GetProperty(SmileTimestamp self, Symbol propertyName)
{
	struct DecomposedTimestampStruct decomposed;

	if (propertyName == Smile_KnownSymbols.seconds)
		return (SmileObject)SmileInteger64_Create(self->seconds);
	else if (propertyName == Smile_KnownSymbols.nanos)
		return (SmileObject)SmileInteger64_Create(self->nanos);
	else if (propertyName == Smile_KnownSymbols.value) {
		Real64 value = Real64_Add(Real64_FromInt64(self->seconds),
			Real64_Div(Real64_FromInt64(self->nanos), Real64_FromInt64(1000000000))
		);
		return (SmileObject)SmileReal64_Create(value);
	}
	else if (propertyName == Smile_KnownSymbols.second) {
		SmileTimestamp_Decompose(self, &decomposed, DecomposeSecond);
		return (SmileObject)SmileInteger64_Create(decomposed.second);
	}
	else if (propertyName == Smile_KnownSymbols.minute) {
		SmileTimestamp_Decompose(self, &decomposed, DecomposeMinute);
		return (SmileObject)SmileInteger64_Create(decomposed.minute);
	}
	else if (propertyName == Smile_KnownSymbols.hour) {
		SmileTimestamp_Decompose(self, &decomposed, DecomposeHour);
		return (SmileObject)SmileInteger64_Create(decomposed.hour);
	}
	else if (propertyName == Smile_KnownSymbols.day) {
		SmileTimestamp_Decompose(self, &decomposed, DecomposeDay);
		return (SmileObject)SmileInteger64_Create(decomposed.day);
	}
	else if (propertyName == Smile_KnownSymbols.month) {
		SmileTimestamp_Decompose(self, &decomposed, DecomposeMonth);
		return (SmileObject)SmileInteger64_Create(decomposed.month);
	}
	else if (propertyName == Smile_KnownSymbols.year) {
		SmileTimestamp_Decompose(self, &decomposed, DecomposeYear);
		return (SmileObject)SmileInteger64_Create(decomposed.year);
	}
	else if (propertyName == Smile_KnownSymbols.day_of_year) {
		SmileTimestamp_Decompose(self, &decomposed, DecomposeDayOfYear);
		return (SmileObject)SmileInteger64_Create(decomposed.dayOfYear);
	}
	else if (propertyName == Smile_KnownSymbols.leap_year_) {
		SmileTimestamp_Decompose(self, &decomposed, DecomposeLeapYear);
		return (SmileObject)SmileInteger64_Create(decomposed.leapYear);
	}
	else return NullObject;
}

static void SmileTimestamp_SetProperty(SmileTimestamp self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);

	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a Timestamp, which is read-only.",
			SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

static Bool SmileTimestamp_HasProperty(SmileTimestamp self, Symbol propertyName)
{
	UNUSED(self);

	return (propertyName == Smile_KnownSymbols.year
		|| propertyName == Smile_KnownSymbols.month
		|| propertyName == Smile_KnownSymbols.day
		|| propertyName == Smile_KnownSymbols.day_of_year
		|| propertyName == Smile_KnownSymbols.leap_year_
		|| propertyName == Smile_KnownSymbols.hour
		|| propertyName == Smile_KnownSymbols.minute
		|| propertyName == Smile_KnownSymbols.second
		|| propertyName == Smile_KnownSymbols.seconds
		|| propertyName == Smile_KnownSymbols.nanos
		|| propertyName == Smile_KnownSymbols.value);
}

static SmileList SmileTimestamp_GetPropertyNames(SmileTimestamp self)
{
	UNUSED(self);

	return SmileList_CreateList(
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.year),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.month),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.day),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.day_of_year),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.leap_year_),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.hour),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.minute),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.second),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.seconds),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.nanos),
		(SmileObject)SmileSymbol_Create(Smile_KnownSymbols.value),
		NULL
	);
}

//-------------------------------------------------------------------------------------------------
// Decomposition and stringification.

/// <summary>
/// _daysBefore[m] is a count of the number of days in a non-leap year
/// before month m (zero-based) begins.  There's also a final entry for
/// month 12, which counts the number of days before January of the next year (365).
/// </summary>
static Int16 _daysBefore[] = {
	0,
	31,
	31 + 28,
	31 + 28 + 31,
	31 + 28 + 31 + 30,
	31 + 28 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30,
	31 + 28 + 31 + 30 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
};


/// <summary>
/// Determine if the given year is a leap year or not, applying the 4-year rule, the
/// 100-year rule, and the 400-year rule.
/// </summary>
/// <param name="year">The year to test, like '2001'.</param>
/// <returns>True if the given year is a leap year (like '2008' is) and False if the
/// given year is not a leap year (like '2009' isn't).</returns>
Inline Bool IsLeapYear(Int year)
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

/// <summary>
/// Decompose a timestamp into more traditional year/month/day/hour/minute/second
/// pieces.  This will only populate those fields of the result identified by
/// the 'decomposeFields' flags, and it will attempt to short-circuit expensive
/// date computations that aren't asked for.
/// </summary>
/// <param name="self">The timestamp to decompose.</param>
/// <param name="result">A pointer to a DecomposedTimestamp that will be populated
/// with the resulting year, month, day, hour, minute, second, etc. derived from
/// the given timestamp (in UTC).</param>
/// <param name="decomposeFields">Which fields to populate in the decomposed result,
/// as determined by the given Decompose* bit flags.  This function will attempt to
/// avoid unnecessary computation for fields that are not asked for, so you should
/// always ask for as few fields as possible if you want this to run efficiently.</param>
void SmileTimestamp_Decompose(SmileTimestamp self, DecomposedTimestamp result, Int decomposeFields)
{
	UInt64 time;
	UInt64 year;
	UInt64 n;
	UInt32 day, month;
	UInt32 begin, end;
	Int64 trueYear;
	Bool leapYear;

	time = self->seconds + INTERNAL_TO_ABSOLUTE;

	// Extract out the time-of-day.  This is the easy part.
	if (decomposeFields & DecomposeNanos)
		result->nanos = self->nanos;

	// Short-circuit the time part if the caller doesn't want it.
	if (decomposeFields & (DecomposeSecond | DecomposeMinute | DecomposeHour)) {
		if (decomposeFields & DecomposeSecond)
			result->second = time % 60;
		time /= 60;
		if (decomposeFields & DecomposeMinute)
			result->minute = time % 60;
		time /= 60;
		if (decomposeFields & DecomposeHour)
			result->hour = time % 24;
		time /= 24;
	}
	else time /= 60 * 60 * 24;

	// Short-circuit the date part if the caller doesn't want it.
	if (!(decomposeFields & DecomposeDate))
		return;

	// Now deal with the fact that years are uneven and months are uneven and calendars are weird.
	//
	// We follow the 400/100/4-year rule for leap years, but not the proposed 4000-year rule.
	//
	// This algorithm was liberally stolen from Golang's absDate() function (thanks, Go team!),
	// as theirs is a bit nicer than the code I designed the last time I needed to do this.
	// You can find their implementation here:  https://golang.org/src/time/time.go

	n = time / DAYS_PER_400_YEARS;
	year = 400 * n;
	time -= DAYS_PER_400_YEARS * n;

	n = time / DAYS_PER_100_YEARS;
	n -= n >> 2;
	year += 100 * n;
	time -= DAYS_PER_100_YEARS * n;

	n = time / DAYS_PER_4_YEARS;
	year += 4 * n;
	time -= DAYS_PER_4_YEARS * n;

	n = time / 365;
	n -= n >> 2;
	year += n;
	time -= 365 * n;

	// Record the year.
	trueYear = (Int64)year + ABSOLUTE_ZERO_YEAR;
	if (decomposeFields & DecomposeYear)
		result->year = trueYear;

	// Whatever's leftover is the day of the year.
	day = (UInt32)time;
	if (decomposeFields & DecomposeDayOfYear)
		result->dayOfYear = (UInt16)day;

	// Short-circuit the month and day if they're not needed.
	if (!(decomposeFields & (DecomposeMonth | DecomposeDay)))
		return;

	// Handle leap years by adjusting the date, with a special rule for February.
	leapYear = IsLeapYear(trueYear);
	if (decomposeFields & DecomposeLeapYear)
		result->leapYear = leapYear;
	if (leapYear) {
		if (day > 31 + 29 - 1) day--;
		else if (day == 31 + 29 - 1) {
			if (decomposeFields & DecomposeMonth)
				result->month = 2;
			if (decomposeFields & DecomposeDay)
				result->day = 29;		// Leap day in February.
			return;
		}
	}

	// Come up with a rough estimate of the correct month by assuming that
	// all months are 31 days, and then correct that estimate if it's off.
	month = day / 31;
	end = _daysBefore[month + 1];
	if (day >= end)
		month++, begin = end;
	else
		begin = _daysBefore[month];

	// Finally, assign the correct month/day.
	if (decomposeFields & DecomposeMonth)
		result->month = (UInt8)(month + 1);		// Months are one-based.
	if (decomposeFields & DecomposeDay)
		result->day = (UInt8)(day - begin + 1);
}

/// <summary>
/// Convert a timestamp to its equivalent ISO 8601 form.  This uses a slightly
/// extended form, allowing a '-' to precede the year for BCE years, and it will
/// always include at least one digit's worth of sub-second precision after the
/// decimal point.  The resulting string will always end in 'Z' to indicate that
/// the ISO 8601 form is in the UTC timezone.
/// </summary>
/// <param name="self">The timestamp to convert to a string.</param>
/// <returns>The stringified timestamp, in ISO 8601 form.</returns>
String SmileTimestamp_Stringify(SmileTimestamp self)
{
	struct DecomposedTimestampStruct decomposed;
	String nanos;
	const Byte *nanosText, *nanosEnd, *nanosNewEnd;

	SmileTimestamp_Decompose(self, &decomposed, DecomposeAll);

	// Format the nanos as a padded unsigned integer.
	nanos = String_Format("%09u", decomposed.nanos);

	// Trim trailing zeros from the nanos, but always leave at least one digit
	// (even if that digit is just a zero).
	nanosText = String_GetBytes(nanos);
	nanosEnd = nanosText + String_Length(nanos);
	for (nanosNewEnd = nanosEnd; nanosNewEnd > nanosText + 1 && nanosNewEnd[-1] == '0'; nanosNewEnd--) ;
	if (nanosNewEnd != nanosEnd) {
		nanos = String_Create(nanosText, nanosNewEnd - nanosText);
	}

	// Format the timestamp in ISO 8601 form, in UTC (since timestamp objects are always UTC).
	return String_Format("%04ld-%02hd-%02hdT%02hd:%02hd:%02hd.%SZ",
		decomposed.year, (Int32)decomposed.month, (Int32)decomposed.day,
		(Int32)decomposed.hour, (Int32)decomposed.minute, (Int32)decomposed.second,
		nanos);
}

//-------------------------------------------------------------------------------------------------
// Parsing, and creation from parts or from decomposed timestamps.

/// <summary>
/// Given a value in 'lo' that must be in the range of [0, base),
/// this normalizes that value, adding or subtracting any overflow
/// into 'hi'.
/// </summary>
/// <param name="hi">The high component to adjust if 'lo' overflows or underflows.</param>
/// <param name="lo">The low component to validate and reduce to the range of [0, base).</param>
/// <param name="base">One more than the highest value allowed in 'lo'.</param>
Inline void Normalize(Int *hi, Int *lo, Int base)
{
	if (*lo < 0) {
		Int n = (-*lo - 1) / base + 1;
		*hi -= (Int64)n;
		*lo += n * base;
	}
	if (*lo >= base) {
		Int n = *lo / base;
		*hi += (Int64)n;
		*lo -= n * base;
	}
}

/// <summary>
/// Attempt to parse the given input as an ASCII decimal number in the range of [0, limit].
/// </summary>
/// <param name="src">A reference to a pointer to the start of the given input.  This will be adjusted
/// to reflect how many characters are consumed.</param>
/// <param name="end">A pointer to the end of the given input.  Parsing will stop here.</param>
/// <param name="result">This value will be populated with the parsed number on a successful
/// parse, or with 0 on a failed parse.</param>
/// <param name="limit">The maximum allowed value that may be returned.</param>
/// <returns>True if the parse was successful (and *src and *result will be updated), or
/// False if the parse failed (and *result will be set to 0).</returns>
static Bool TryParseNumber(const Byte **src, const Byte *end, UInt64 *result, UInt64 limit)
{
	UInt64 value = 0;
	const Byte *ptr;

	// Parse digits until their combination exceeds 'limit', the maximum allowed value.
	for (ptr = *src; ptr < end; ptr++) {
		if (*ptr >= '0' && *ptr <= '9') {
			value *= 10;
			value += *ptr - '0';
			if (value > limit) {
				*result = 0;
				return False;
			}
		}
		else break;
	}

	// If we got no digits, that's a fail.
	if (ptr == *src) {
		*result = 0;
		return False;
	}

	// Got a value in range, so return it.
	*src = ptr;
	*result = value;
	return True;
}

/// <summary>
/// Attempt to parse the given input as an ASCII decimal fraction of at most 9 digits.
/// </summary>
/// <param name="src">A reference to a pointer to the start of the given input.  This will be adjusted
/// to reflect how many characters are consumed.</param>
/// <param name="end">A pointer to the end of the given input.  Parsing will stop here.</param>
/// <param name="result">This value will be populated with the parsed fraction.  It will
/// always contain exactly nine digits of precision, no matter how many digits are provided
/// in the input.</param>
static void ParseNanos(const Byte **src, const Byte *end, UInt64 *result)
{
	UInt64 value = 0;
	Int digits = 0;
	const Byte *ptr;

	// Parse fractional digits.
	for (ptr = *src; ptr < end; ptr++) {
		if (*ptr >= '0' && *ptr <= '9') {
			value *= 10;
			value += *ptr - '0';
			if (++digits >= 9) {
				// Too many digits, so it's still valid, but we need to
				// discard any remaining.  (A better algorithm would use the
				// next digit to round the nanos to the nearest, but since
				// that might involve rounding up every part of the *entire*
				// timestamp to the next year, it's not implemented here, and
				// we simply truncate instead.)
				while (ptr < end && *ptr >= '0' && *ptr <= '9')
					ptr++;
				break;
			}
		}
		else break;
	}

	// If there were too few digits, we need to round them until we have an
	// accurate count of nanoseconds.
	while (digits < 9) {
		value *= 10;
		digits++;
	}

	// Return the result.
	*result = value;
	*src = ptr;
}

/// <summary>
/// Compose a timestamp from individual time components, in UTC.
/// </summary>
/// <param name="year">The year, which must be greater than or equal to -292277022399.</param>
/// <param name="month">The month of the year, in the range of [1, 12].  Months outside this
/// range will be "rolled over" into the next or previous year.</param>
/// <param name="day">The day of month, in the range of [1, DaysPerThisMonth].  Days outside this
/// range will be "rolled over" into the next or previous month.</param>
/// <param name="hour">The hour of the day, in the range of [0, 23].  Hours outside this
/// range will be "rolled over" into the next or previous day.</param>
/// <param name="minute">The minute of the day, in the range of [0, 59].  Minutes outside this
/// range will be "rolled over" into the next or previous hour.</param>
/// <param name="second">The second of the minute, in the range of [0, 59].  Seconds outside this
/// range will be "rolled over" into the next or previous minute.</param>
/// <param name="nanos">The number of nanosecond since the start of this second, in the range of
/// [0, 999999999].  Nanos outside this range will be "rolled over" into the next or previous
/// second.</param>
SmileTimestamp SmileTimestamp_CreateFromParts(Int64 year, Int month, Int day,
	Int hour, Int minute, Int second, Int nanos)
{
	UInt64 y;
	UInt64 n;
	UInt64 d;
	UInt64 abs;
	Int64 secs;
	SmileTimestamp timestamp;

	// Normalize the month, overflowing into the year.
	month--;
	Normalize(&year, &month, 12);
	month++;

	// Normalize nanos through hours, overflowing into day.
	Normalize(&second, &nanos, 1000000000);
	Normalize(&minute, &second, 60);
	Normalize(&hour, &minute, 60);
	Normalize(&day, &hour, 24);

	y = (UInt64)((Int64)year - ABSOLUTE_ZERO_YEAR);

	// Compute days since the absolute epoch.
	//
	// We follow the 400/100/4-year rule for leap years, but not the proposed 4000-year rule.
	//
	// This algorithm was liberally stolen from Golang's absDate() function (thanks, Go team!),
	// as theirs is a bit nicer than the code I designed the last time I needed to do this.
	// You can find their implementation here:  https://golang.org/src/time/time.go

	n = y / 400;
	y -= 400 * n;
	d = DAYS_PER_400_YEARS * n;

	n = y / 100;
	y -= 100 * n;
	d += DAYS_PER_100_YEARS * n;

	n = y / 4;
	y -= 4 * n;
	d += DAYS_PER_4_YEARS * n;

	n = y;
	d += 365 * n;

	// Add in days before this month.
	d += _daysBefore[month - 1];
	if (IsLeapYear(year) && month >= 3) d++; // February 29

	// Add in days before today.
	d += day - 1;

	abs = d * SECONDS_PER_DAY;
	abs += (hour * SECONDS_PER_HOUR + minute * SECONDS_PER_MINUTE + second);
	secs = ((Int64)abs + ABSOLUTE_TO_INTERNAL);

	timestamp = SmileTimestamp_Create(secs, (UInt32)nanos);
	return timestamp;
}

/// <summary>
/// Attempt to parse the given input as a ISO 8601 time string in the UTC timezone
/// (i.e., an ISO 8601 time that ends in 'Z').
/// </summary>
/// <param name="input">The input string to parse, which must contain a valid
/// ISO 8601 time string in UTC.  The time string may be preceded or followed by
/// whitespace characters, but no other non-ISO 8601 characters are permitted.</param>
/// <param name="result">This structure will be populated with the resulting
/// date/time values, if the parse is valid.</param>
/// <returns>True for a valid parse, False for an invalid input string.</returns>
Bool SmileTimestamp_TryParseDecomposed(String input, DecomposedTimestamp result)
{
	Bool negativeYear;
	UInt64 year, month, day, hour, minute, second, nanos;
	const Byte *src = String_GetBytes(input);
	const Byte *end = src + String_Length(input);

	// Skip initial whitespace to be friendly.
	while (src < end && *src <= ' ') src++;

	if (src >= end)
		return False;

	// Optional sign.
	if (*src == '-')
		src++, negativeYear = True;
	else if (*src == '+')
		src++, negativeYear = False;
	else
		negativeYear = False;

	// Parse the date, which must be of the form Y-M or Y-M-D.
	if (!TryParseNumber(&src, end, &year, 999999999))
		return False;
	if (src >= end || *src++ != '-')
		return False;
	if (!TryParseNumber(&src, end, &month, 12))
		return False;
	if (src < end && *src == '-') {
		src++;
		if (!TryParseNumber(&src, end, &day, 31))
			return False;
	}
	else day = 1;

	// Parse the time, if this is followed by 'T'.  The time may have various
	// fragmentary forms:  Hour is required after 'T', but any omitted parts are
	// taken as zero.
	if (src < end && *src == 'T') {
		src++;
		if (!TryParseNumber(&src, end, &hour, 23))
			return False;
		if (src < end && *src == ':') {
			src++;
			if (!TryParseNumber(&src, end, &minute, 59))
				return False;
			if (src < end && *src == ':') {
				src++;
				if (!TryParseNumber(&src, end, &second, 59))
					return False;
				if (src < end && *src == '.') {
					src++;
					ParseNanos(&src, end, &nanos);
				}
				else nanos = 0;
			}
			else second = nanos = 0;
		}
		else minute = second = nanos = 0;
	}
	else hour = minute = second = nanos = 0;

	// Must be in the UTC timezone, as indicated by a trailing 'Z'.
	if (!(src < end && *src++ == 'Z'))
		return False;

	// Skip trailing whitespace to be friendly.
	while (src < end && *src <= ' ') src++;

	// Must be no other leftover characters.
	if (src != end)
		return False;

	if (year > -ABSOLUTE_ZERO_YEAR)
		return False;

	// Populate the result.
	result->year = negativeYear ? -(Int64)year : (Int64)year;
	result->month = (UInt8)month;
	result->day = (UInt8)day;
	result->hour = (UInt8)hour;
	result->minute = (UInt8)minute;
	result->second = (UInt8)second;
	result->nanos = (UInt32)nanos;

	// Calculate the derived fields.  This doesn't take *too* long, so we always
	// do it, even though it's likely not needed by many callers.
	result->leapYear = IsLeapYear(result->year);
	result->dayOfYear = (UInt16)(_daysBefore[result->month - 1] + day - 1);
	if (result->leapYear && result->month > 2)
		result->dayOfYear++;

	return True;
}

//-------------------------------------------------------------------------------------------------
// Hashing, equality, stringification, and other things required by the vtable.

static UInt32 SmileTimestamp_Hash(SmileTimestamp self)
{
	UInt64 seconds = self->seconds;
	UInt32 nanos = self->nanos;
	return Smile_ApplyHashOracle(seconds + nanos);
}

static String SmileTimestamp_ToString(SmileTimestamp timestamp, SmileUnboxedData unboxedData)
{
	UNUSED(unboxedData);

	return SmileTimestamp_Stringify(timestamp);
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileTimestamp)
SMILE_EASY_OBJECT_NO_CALL(SmileTimestamp, "A Timestamp object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileTimestamp)
SMILE_EASY_OBJECT_NO_UNBOX(SmileTimestamp)

SMILE_EASY_OBJECT_COMPARE(SmileTimestamp, SMILE_KIND_TIMESTAMP, a->seconds == b->seconds && a->nanos == b->nanos)
SMILE_EASY_OBJECT_DEEP_COMPARE(SmileTimestamp, SMILE_KIND_TIMESTAMP, a->seconds == b->seconds && a->nanos == b->nanos)
SMILE_EASY_OBJECT_TOBOOL(SmileTimestamp, True)
