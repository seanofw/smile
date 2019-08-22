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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/numeric/smiletimestamp.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>
#include <smile/numeric/int128.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _timestampChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
};

static Byte _timestampComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	0, 0,
};

static Byte _timestampArithmeticChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_TIMESTAMP,
	0, 0,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(timestamp, "Timestamp");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_TIMESTAMP) {
		return SmileArg_From((SmileObject)SmileTimestamp_Stringify((SmileTimestamp)argv[0].obj));
	}

	return SmileArg_From((SmileObject)timestamp);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileTimestamp obj = (SmileTimestamp)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_TIMESTAMP)
		return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((UInt64)obj->seconds + (UInt64)obj->nanos));

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((UInt64)(PtrInt)obj));
}

//-------------------------------------------------------------------------------------------------
// Parsing

SMILE_EXTERNAL_FUNCTION(Parse)
{
	SmileTimestamp timestamp;
	STATIC_STRING(parseArguments, "Illegal arguments to 'Timestamp.parse' function");

	switch (argc) {

		case 1:
			// The form [parse string].
			if (SMILE_KIND(argv[0].obj) != SMILE_KIND_STRING)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, parseArguments);
			if (!SmileTimestamp_TryParse((String)argv[0].obj, &timestamp))
				return SmileArg_From(NullObject);
			return SmileArg_From((SmileObject)timestamp);

		case 2:
			// The form [obj.parse string].
			if (SMILE_KIND(argv[1].obj) == SMILE_KIND_STRING) {
				// The form [obj.parse string].
				if (!SmileTimestamp_TryParse((String)argv[1].obj, &timestamp))
					return SmileArg_From(NullObject);
				return SmileArg_From((SmileObject)timestamp);
			}
			else {
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, parseArguments);
			}
	}

	return SmileArg_From(NullObject);	// Can't get here, but the compiler doesn't know that.
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(
		SMILE_KIND(argv[1].obj) == SMILE_KIND_TIMESTAMP
		&& ((SmileTimestamp)argv[0].obj)->seconds == ((SmileTimestamp)argv[1].obj)->seconds
		&& ((SmileTimestamp)argv[0].obj)->nanos == ((SmileTimestamp)argv[1].obj)->nanos
	);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(
		SMILE_KIND(argv[1].obj) != SMILE_KIND_TIMESTAMP
		|| ((SmileTimestamp)argv[0].obj)->seconds != ((SmileTimestamp)argv[1].obj)->seconds
		|| ((SmileTimestamp)argv[0].obj)->nanos != ((SmileTimestamp)argv[1].obj)->nanos
	);
}

Inline Int Cmp(SmileTimestamp a, SmileTimestamp b)
{
	if (a->seconds < b->seconds) return -1;
	else if (a->seconds > b->seconds) return +1;
	else if (a->nanos < b->nanos) return -1;
	else if (a->nanos > b->nanos) return +1;
	else return 0;
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	Int cmp = Cmp((SmileTimestamp)argv[0].obj, (SmileTimestamp)argv[1].obj);
	return SmileUnboxedBool_From(cmp < 0);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	Int cmp = Cmp((SmileTimestamp)argv[0].obj, (SmileTimestamp)argv[1].obj);
	return SmileUnboxedBool_From(cmp > 0);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	Int cmp = Cmp((SmileTimestamp)argv[0].obj, (SmileTimestamp)argv[1].obj);
	return SmileUnboxedBool_From(cmp <= 0);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	Int cmp = Cmp((SmileTimestamp)argv[0].obj, (SmileTimestamp)argv[1].obj);
	return SmileUnboxedBool_From(cmp >= 0);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Int cmp = Cmp((SmileTimestamp)argv[0].obj, (SmileTimestamp)argv[1].obj);
	return SmileUnboxedInteger64_From((Int64)cmp);
}

//-------------------------------------------------------------------------------------------------
// Conversion to/from common Unix and Windows forms.
//
// ('string' and 'parse' are responsible for handling standard ISO 8601 form.)

SMILE_EXTERNAL_FUNCTION(ToUnixSeconds)
{
	SmileTimestamp timestamp = (SmileTimestamp)argv[0].obj;
	Int64 unixSeconds = SmileTimestamp_ToUnix(timestamp);
	return SmileUnboxedInteger64_From(unixSeconds);
}

SMILE_EXTERNAL_FUNCTION(ToUnixSecondsReal)
{
	SmileTimestamp timestamp = (SmileTimestamp)argv[0].obj;
	Int64 unixSeconds = SmileTimestamp_ToUnix(timestamp);

	Real64 value = Real64_Add(Real64_FromInt64(unixSeconds),
		Real64_Div(Real64_FromInt64(timestamp->nanos), Real64_FromInt64(1000000000))
	);

	return SmileUnboxedReal64_From(value);
}

SMILE_EXTERNAL_FUNCTION(FromUnixSeconds)
{
	Int64 unixSeconds, nanos;
	Real64 realFracPart64, realIntPart64;
	Real32 realFracPart32, realIntPart32;
	Float64 floatFracPart64, floatIntPart64;
	SmileTimestamp timestamp;
	STATIC_STRING(unsupportedType, "Unsupported type passed as argument 1 to 'from-unix-seconds'.");

	switch (SMILE_KIND(argv[0].obj))
	{
		case SMILE_KIND_UNBOXED_INTEGER64:
			unixSeconds = argv[0].unboxed.i64;
			nanos = 0;
			break;
		case SMILE_KIND_UNBOXED_INTEGER32:
			unixSeconds = argv[0].unboxed.i32;
			nanos = 0;
			break;
		case SMILE_KIND_UNBOXED_INTEGER16:
			unixSeconds = argv[0].unboxed.i16;
			nanos = 0;
			break;
		case SMILE_KIND_UNBOXED_BYTE:
			unixSeconds = argv[0].unboxed.i8;
			nanos = 0;
			break;
		case SMILE_KIND_UNBOXED_REAL64:
			realFracPart64 = Real64_Modf(argv[0].unboxed.r64, &realIntPart64);
			nanos = Real64_ToInt64(Real64_Mul(realFracPart64, Real64_FromInt64(1000000000)));
			unixSeconds = Real64_ToInt64(realIntPart64);
			break;
		case SMILE_KIND_UNBOXED_REAL32:
			realFracPart32 = Real32_Modf(argv[0].unboxed.r32, &realIntPart32);
			nanos = Real32_ToInt64(Real32_Mul(realFracPart32, Real32_FromInt64(1000000000)));
			unixSeconds = Real32_ToInt64(realIntPart32);
			break;
		case SMILE_KIND_UNBOXED_FLOAT64:
			floatFracPart64 = modf(argv[0].unboxed.f64, &floatIntPart64);
			nanos = (Int64)(floatFracPart64 * 1000000000.0);
			unixSeconds = (Int64)floatIntPart64;
			break;
		case SMILE_KIND_UNBOXED_FLOAT32:
			floatFracPart64 = modf(argv[0].unboxed.f32, &floatIntPart64);
			nanos = (Int64)(floatFracPart64 * 1000000000.0);
			unixSeconds = (Int64)floatIntPart64;
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, unsupportedType);
	}

	timestamp = SmileTimestamp_FromUnix(unixSeconds, (UInt32)nanos);
	return SmileArg_From((SmileObject)timestamp);
}

SMILE_EXTERNAL_FUNCTION(ToWindowsTicks)
{
	SmileTimestamp timestamp = (SmileTimestamp)argv[0].obj;
	Int64 windowsTicks = SmileTimestamp_ToWindows(timestamp);
	return SmileUnboxedInteger64_From(windowsTicks);
}

SMILE_EXTERNAL_FUNCTION(FromWindowsTicks)
{
	Int64 windowsTicks;
	SmileTimestamp timestamp;
	STATIC_STRING(unsupportedType, "Unsupported type passed as argument 1 to 'from-windows-ticks'.");

	switch (SMILE_KIND(argv[0].obj))
	{
		case SMILE_KIND_UNBOXED_INTEGER64:
			windowsTicks = argv[0].unboxed.i64;
			break;
		case SMILE_KIND_UNBOXED_INTEGER32:
			windowsTicks = argv[0].unboxed.i32;
			break;
		case SMILE_KIND_UNBOXED_INTEGER16:
			windowsTicks = argv[0].unboxed.i16;
			break;
		case SMILE_KIND_UNBOXED_BYTE:
			windowsTicks = argv[0].unboxed.i8;
			break;
		case SMILE_KIND_UNBOXED_REAL64:
			windowsTicks = Real64_ToInt64(argv[0].unboxed.r64);
			break;
		case SMILE_KIND_UNBOXED_REAL32:
			windowsTicks = Real32_ToInt64(argv[0].unboxed.r32);
			break;
		case SMILE_KIND_UNBOXED_FLOAT64:
			windowsTicks = (Int64)(argv[0].unboxed.f64 + 0.5);
			break;
		case SMILE_KIND_UNBOXED_FLOAT32:
			windowsTicks = (Int64)(argv[0].unboxed.f32 + 0.5);
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, unsupportedType);
	}

	timestamp = SmileTimestamp_FromWindows(windowsTicks);
	return SmileArg_From((SmileObject)timestamp);
}

//-------------------------------------------------------------------------------------------------
// Basic arithmetic (addition and subtraction of simple time units).

typedef struct MultipliersStruct {
	const char *name;
	Bool subtract;
	Int64 secMul, nanoMul;
	Real64 r;
	Float64 f;
} *Multipliers;

static struct MultipliersStruct _addDaysMultiplier = {
	"add-days", False, 60 * 60 * 24, 0, { 0x31C0000000015180ULL }, 1.0 * 60 * 60 * 24,
};
static struct MultipliersStruct _addHoursMultiplier = {
	"add-hours", False, 60 * 60, 0, { 0x31C0000000000E10ULL }, 1.0 * 60 * 60,
};
static struct MultipliersStruct _addMinutesMultiplier = {
	"add-minutes", False, 60, 0, { 0x31C000000000003CULL }, 1.0 * 60,
};
static struct MultipliersStruct _addSecondsMultiplier = {
	"add-seconds", False, 1, 0, { 0x31C0000000000001ULL }, 1.0,
};
static struct MultipliersStruct _addMsecMultiplier = {
	"add-milliseconds", False, 0, 1000000, { 0x3160000000000001ULL }, 0.001,
};
static struct MultipliersStruct _addUsecMultiplier = {
	"add-microseconds", False, 0, 1000, { 0x3100000000000001ULL }, 0.000001,
};
static struct MultipliersStruct _addNsecMultiplier = {
	"add-nanoseconds", False, 0, 1, { 0x30A0000000000001ULL }, 0.000000001,
};

static struct MultipliersStruct _subDaysMultiplier = {
	"sub-days", True, 60 * 60 * 24, 0, { 0x31C0000000015180ULL }, 1.0 * 60 * 60 * 24,
};
static struct MultipliersStruct _subHoursMultiplier = {
	"sub-hours", True, 60 * 60, 0, { 0x31C0000000000E10ULL }, 1.0 * 60 * 60,
};
static struct MultipliersStruct _subMinutesMultiplier = {
	"sub-minutes", True, 60, 0, { 0x31C000000000003CULL }, 1.0 * 60,
};
static struct MultipliersStruct _subSecondsMultiplier = {
	"sub-seconds", True, 1, 0, { 0x31C0000000000001ULL }, 1.0,
};
static struct MultipliersStruct _subMsecMultiplier = {
	"sub-milliseconds", True, 0, 1000000, { 0x3160000000000001ULL }, 0.001,
};
static struct MultipliersStruct _subUsecMultiplier = {
	"sub-microseconds", True, 0, 1000, { 0x3100000000000001ULL }, 0.000001,
};
static struct MultipliersStruct _subNsecMultiplier = {
	"sub-nanoseconds", True, 0, 1, { 0x30A0000000000001ULL }, 0.000000001,
};

SMILE_EXTERNAL_FUNCTION(AddSub)
{
	SmileTimestamp timestamp = (SmileTimestamp)argv[0].obj;
	Multipliers multipliers = (Multipliers)param;
	Int64 ivalue;
	Real64 rvalue, realIntPart64, realFracPart64;
	Float64 fvalue, floatIntPart64, floatFracPart64;
	Int64 nanos, seconds;
	Bool subtract = multipliers->subtract;

	switch (SMILE_KIND(argv[1].obj)) {
		case SMILE_KIND_UNBOXED_INTEGER64:
			ivalue = argv[1].unboxed.i64;
			goto integer_common;
		case SMILE_KIND_UNBOXED_INTEGER32:
			ivalue = argv[1].unboxed.i32;
			goto integer_common;
		case SMILE_KIND_UNBOXED_INTEGER16:
			ivalue = argv[1].unboxed.i16;
			goto integer_common;
		case SMILE_KIND_UNBOXED_BYTE:
			ivalue = argv[1].unboxed.i8;
			goto integer_common;

		integer_common:
			if (ivalue < 0) {
				subtract = !subtract;
				ivalue = -ivalue;
			}
			nanos = multipliers->nanoMul * ivalue;
			seconds = multipliers->secMul * ivalue;
			break;

		case SMILE_KIND_UNBOXED_REAL64:
			rvalue = argv[1].unboxed.r64;
			goto real_common;
		case SMILE_KIND_UNBOXED_REAL32:
			rvalue = Real32_ToReal64(argv[1].unboxed.r32);
			goto real_common;

		real_common:
			if (Real64_IsNeg(rvalue)) {
				subtract = !subtract;
				rvalue = Real64_Neg(rvalue);
			}
			rvalue = Real64_Mul(rvalue, multipliers->r);			// Transform into a count of seconds.
			realFracPart64 = Real64_Modf(Real64_Abs(rvalue), &realIntPart64);	// Split into seconds and subseconds.
			nanos = Real64_ToInt64(Real64_Add(Real64_Mul(realFracPart64, Real64_FromInt64(1000000000LL)), Real64_OneHalf));
			seconds = Real64_ToInt64(realIntPart64);
			break;

		case SMILE_KIND_UNBOXED_FLOAT64:
			fvalue = argv[1].unboxed.f64;
			goto float_common;
		case SMILE_KIND_UNBOXED_FLOAT32:
			fvalue = argv[1].unboxed.f32;
			goto float_common;

		float_common:
			if (fvalue < 0) {
				subtract = !subtract;
			}
			fvalue *= multipliers->f;								// Transform into a count of seconds.
			floatFracPart64 = modf(fabs(fvalue), &floatIntPart64);	// Split into seconds and subseconds.
			nanos = (Int64)(floatFracPart64 * 1000000000.0 + 0.5);
			seconds = (Int64)floatIntPart64;
			break;

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_Format("Unsupported type passed as argument 1 to 'Timestamp.%s'.", multipliers->name));
	}

	// Normalize the nanos to be in [0, 1000000000) by carrying them up into the seconds.
	if (nanos > 1000000000) {
		seconds += nanos / 1000000000;
		nanos %= 1000000000;
	}

	if (subtract) {
		if (timestamp->nanos - nanos < 0) {
			// Too many nanos, so borrow from the seconds.
			nanos = timestamp->nanos - nanos;
			seconds = timestamp->seconds - seconds - 1;
			nanos = (nanos + 1000000000) % 1000000000;
		}
		else {
			// Simple subtraction.
			nanos = timestamp->nanos - nanos;
			seconds = timestamp->seconds - seconds;
		}
	}
	else {
		if (timestamp->nanos + nanos < timestamp->nanos) {
			// Too many nanos, so carry them up into the seconds.
			nanos += timestamp->nanos;
			seconds += timestamp->seconds + 1;
			nanos %= 1000000000;
		}
		else {
			// Simple addition.
			nanos += timestamp->nanos;
			seconds += timestamp->seconds;
		}
	}

	return SmileArg_From((SmileObject)SmileTimestamp_Create(seconds, (UInt32)nanos));
}

SMILE_EXTERNAL_FUNCTION(Diff)
{
	SmileTimestamp t1 = (SmileTimestamp)argv[0].obj;
	SmileTimestamp t2 = (SmileTimestamp)argv[1].obj;
	Int64 seconds, nanos;
	Real64 result;
	static Real64 nanoMultiplier = { 0x30A0000000000001ULL };

	if (SMILE_KIND(t2) != SMILE_KIND_TIMESTAMP)
		return AddSub(argc, argv, param);

	// Calculate the actual exact difference.
	seconds = t1->seconds - t2->seconds;
	nanos = t1->nanos - t2->nanos;

	// If the nanos rolled over, borrow from the seconds.
	if (nanos < 0) {
		nanos += 1000000000;
		seconds--;
	}

	if (nanos == 0) {
		// Short-circuit the math if we only have a count of whole seconds.
		return SmileUnboxedReal64_From(Real64_FromInt64(seconds));
	}

	// Combine seconds and nanos into a single floating-point value.
	result = Real64_Add(Real64_FromInt64(seconds), Real64_Mul(Real64_FromInt64(nanos), nanoMultiplier));
	return SmileUnboxedReal64_From(result);
}

//-------------------------------------------------------------------------------------------------

void SmileTimestamp_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "t", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "t", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "t", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupSynonym("string", "iso-8601");
	SetupFunction("hash", Hash, NULL, "t", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("parse", Parse, NULL, "str", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);

	SetupFunction("unix-seconds", ToUnixSeconds, NULL, "t", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 0, _timestampChecks);
	SetupFunction("unix-seconds-real", ToUnixSecondsReal, NULL, "t", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 0, _timestampChecks);
	SetupFunction("from-unix-seconds", FromUnixSeconds, NULL, "seconds", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("windows-ticks", ToWindowsTicks, NULL, "t", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 0, _timestampChecks);
	SetupFunction("from-windows-ticks", FromWindowsTicks, NULL, "ticks", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 0, NULL);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampChecks);

	SetupFunction("add-seconds", AddSub, &_addSecondsMultiplier, "t sec", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupFunction("sub-seconds", AddSub, &_subSecondsMultiplier, "t sec", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("add-seconds", "+");
	SetupSynonym("add-seconds", "add-sec");
	SetupSynonym("add-seconds", "sub-sec");

	SetupFunction("-", Diff, &_subSecondsMultiplier, "x y", ARG_CHECK_EXACT, 2, 2, 2, _timestampComparisonChecks);

	SetupFunction("add-days", AddSub, &_addDaysMultiplier, "t days", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupFunction("add-hours", AddSub, &_addHoursMultiplier, "t hours", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupFunction("add-minutes", AddSub, &_addMinutesMultiplier, "t minutes", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("add-minutes", "sub-min");
	SetupFunction("add-milliseconds", AddSub, &_addMsecMultiplier, "t msec", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("add-milliseconds", "add-msec");
	SetupFunction("add-microseconds", AddSub, &_addUsecMultiplier, "t usec", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("add-microseconds", "add-usec");
	SetupFunction("add-nanoseconds", AddSub, &_addNsecMultiplier, "t nsec", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("add-nanoseconds", "add-nsec");

	SetupFunction("sub-days", AddSub, &_subDaysMultiplier, "t days", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupFunction("sub-hours", AddSub, &_subHoursMultiplier, "t hours", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupFunction("sub-minutes", AddSub, &_subMinutesMultiplier, "t minutes", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("sub-minutes", "sub-min");
	SetupFunction("sub-milliseconds", AddSub, &_subMsecMultiplier, "t msec", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("sub-milliseconds", "sub-msec");
	SetupFunction("sub-microseconds", AddSub, &_subUsecMultiplier, "t usec", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("sub-microseconds", "sub-usec");
	SetupFunction("sub-nanoseconds", AddSub, &_subNsecMultiplier, "t nsec", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampArithmeticChecks);
	SetupSynonym("sub-nanoseconds", "sub-nsec");

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timestampChecks);
	SetupSynonym("compare", "cmp");
}
