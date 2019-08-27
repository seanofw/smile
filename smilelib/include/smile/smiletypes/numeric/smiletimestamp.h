#ifndef __SMILE_SMILETYPES_NUMERIC_SMILETIMESTAMP_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILETIMESTAMP_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileTimestampInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	UInt64 seconds;		// Relative to midnight, January 1, year -292277022399, UTC.
	UInt32 nanos;
};

typedef struct DecomposedTimestampStruct {
	Int64 year;
	UInt16 dayOfYear;
	Bool leapYear;
	UInt8 month;
	UInt8 day;
	UInt8 hour;
	UInt8 minute;
	UInt8 second;
	UInt32 nanos;
} *DecomposedTimestamp;

enum {
	DecomposeNanos		= (1 << 0),
	DecomposeSecond		= (1 << 1),
	DecomposeMinute		= (1 << 2),
	DecomposeHour		= (1 << 3),
	DecomposeTime		= DecomposeNanos | DecomposeSecond | DecomposeMinute | DecomposeHour,

	DecomposeLeapYear	= (1 << 4),
	DecomposeDayOfYear	= (1 << 5),
	DecomposeDay		= (1 << 6),
	DecomposeMonth		= (1 << 7),
	DecomposeYear		= (1 << 8),
	DecomposeDate		= DecomposeLeapYear | DecomposeDayOfYear | DecomposeDay | DecomposeMonth | DecomposeYear,

	DecomposeNone		= 0,
	DecomposeAll		= DecomposeTime | DecomposeDate,
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileTimestamp_VTable;

SMILE_API_FUNC SmileTimestamp SmileTimestamp_Create(Int64 seconds, UInt32 nanos);
SMILE_API_FUNC SmileTimestamp SmileTimestamp_CreateFromParts(Int64 year, Int month, Int day, Int hour, Int minute, Int second, Int nanos);
SMILE_API_FUNC SmileTimestamp SmileTimestamp_FromUnix(Int64 secondsSinceEpoch, UInt32 nanos);
SMILE_API_FUNC Int64 SmileTimestamp_ToUnix(SmileTimestamp timestamp);
SMILE_API_FUNC SmileTimestamp SmileTimestamp_FromWindows(Int64 ticks);
SMILE_API_FUNC Int64 SmileTimestamp_ToWindows(SmileTimestamp timestamp);
SMILE_API_FUNC void SmileTimestamp_Decompose(SmileTimestamp self, DecomposedTimestamp result,
	Int decomposeFields);
SMILE_API_FUNC String SmileTimestamp_Stringify(SmileTimestamp self);
SMILE_API_FUNC Bool SmileTimestamp_TryParseDecomposed(String input, DecomposedTimestamp result);

//-------------------------------------------------------------------------------------------------
//  Inline helpers

/// <summary>
/// Create a timestamp from a decomposed-timestamp struct.
/// </summary>
/// <param name="decomposed">The decomposed timestamp, whose year, month, day, hour, minute, second,
/// and nanos fields must be populated (all other fields will be ignored).</param>
/// <returns>A newly-allocated equivalent SmileTimestamp.</return>
Inline SmileTimestamp SmileTimestamp_Compose(DecomposedTimestamp decomposed)
{
	return SmileTimestamp_CreateFromParts(decomposed->year, decomposed->month, decomposed->day,
		decomposed->hour, decomposed->minute, decomposed->second, decomposed->nanos);
}

/// <summary>
/// Attempt to parse an input string as a date/time, and return a timestamp object.
/// </summary>
/// <param name="input">The input string, which must be in ISO 8601 form (or an equivalent
/// degenerate version of it that may have fewer digits for each value).</param>
/// <param name="result">This will be populated with the resulting timestamp (if a
/// valid parse and True is returned) or it will be set to NULL if the input was
/// invalid (and False is returned).</param>
/// <returns>True if the parse was successful, False if the parse failed.</returns>
Inline Bool SmileTimestamp_TryParse(String input, SmileTimestamp *result)
{
	struct DecomposedTimestampStruct decomposed;
	if (!SmileTimestamp_TryParseDecomposed(input, &decomposed)) {
		*result = NULL;
		return False;
	}

	*result = SmileTimestamp_Compose(&decomposed);
	return True;
}

#endif
