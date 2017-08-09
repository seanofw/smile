//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _integer16Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
};

static Byte _integer16ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
	0, 0,
};

static Byte _parseChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
};

typedef struct MathInfoStruct {
	Bool isLoud;
} *MathInfo;

static struct MathInfoStruct _loudMath[] = { True };
static struct MathInfoStruct _quietMath[] = { False };

STATIC_STRING(_divideByZero, "Divide by zero error");
STATIC_STRING(_negativeLog, "Logarithm of negative or zero value");
STATIC_STRING(_negativeSqrt, "Square root of negative number");

STATIC_STRING(_invalidTypeError, "All arguments to 'Integer16.%s' must be of type 'Integer16'");

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Integer64'");
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
STATIC_STRING(_parseArguments, "Illegal arguments to 'parse' function");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_INTEGER16)
		return SmileUnboxedBool_From(!!argv[0].unboxed.i16);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_INTEGER16)
		return SmileUnboxedInteger64_From(argv[0].unboxed.i16);

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	Int64 numericBase;
	STATIC_STRING(integer16, "Integer16");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_INTEGER16) {
		if (argc == 2) {
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_INTEGER64)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)argv[1].unboxed.i64;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;

		return SmileArg_From((SmileObject)String_CreateFromInteger((Int64)argv[0].unboxed.i16, (Int)numericBase, False));
	}

	return SmileArg_From((SmileObject)integer16);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_INTEGER16)
		return SmileUnboxedInteger64_From(argv[0].unboxed.i16);

	return SmileUnboxedInteger64_From((PtrInt)argv[0].obj ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From(argv[0].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return SmileUnboxedInteger32_From(argv[0].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	return SmileUnboxedByte_From((Byte)argv[0].unboxed.i16);
}

//-------------------------------------------------------------------------------------------------
// Parsing

SMILE_EXTERNAL_FUNCTION(Parse)
{
	Int64 numericBase;
	Int64 value;

	switch (argc) {

		case 1:
			// The form [parse string].
			if (SMILE_KIND(argv[0].obj) != SMILE_KIND_STRING)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _parseArguments);
			if (!String_ParseInteger((String)argv[0].obj, 10, &value))
				return SmileArg_From(NullObject);
			return SmileUnboxedInteger16_From((Int16)value);

		case 2:
			// Either the form [parse string base] or [obj.parse string].
			if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING && SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_INTEGER64) {
				// The form [parse string base].
				numericBase = (Int)argv[1].unboxed.i64;
				if (numericBase < 2 || numericBase > 36)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
				if (!String_ParseInteger((String)argv[0].obj, (Int)numericBase, &value))
					return SmileArg_From(NullObject);
				return SmileUnboxedInteger16_From((Int16)value);
			}
			else if (SMILE_KIND(argv[1].obj) == SMILE_KIND_STRING) {
				// The form [obj.parse string].
				if (!String_ParseInteger((String)argv[1].obj, 10, &value))
					return SmileArg_From(NullObject);
				return SmileUnboxedInteger16_From((Int16)value);
			}
			else {
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _parseArguments);
			}

		case 3:
			// The form [obj.parse string base].
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_STRING || SMILE_KIND(argv[2].obj) != SMILE_KIND_UNBOXED_INTEGER64)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _parseArguments);
			numericBase = (Int)argv[2].unboxed.i64;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
			if (!String_ParseInteger((String)argv[1].obj, (Int)numericBase, &value))
				return SmileArg_From(NullObject);
			return SmileUnboxedInteger16_From((Int16)value);
	}

	return SmileArg_From(NullObject);	// Can't get here, but the compiler doesn't know that.
}

//-------------------------------------------------------------------------------------------------
// Arithmetic operators

SMILE_EXTERNAL_FUNCTION(Plus)
{
	Int16 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = argv[0].unboxed.i16;
			x += argv[1].unboxed.i16;
			return SmileUnboxedInteger16_From(x);
		
		case 3:
			x = argv[0].unboxed.i16;
			x += argv[1].unboxed.i16;
			x += argv[2].unboxed.i16;
			return SmileUnboxedInteger16_From(x);
		
		case 4:
			x = argv[0].unboxed.i16;
			x += argv[1].unboxed.i16;
			x += argv[2].unboxed.i16;
			x += argv[3].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		default:
			x = argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				x += argv[i].unboxed.i16;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	Int16 x;
	Int i;

	switch (argc) {
		case 1:
			x = -argv[0].unboxed.i16;
			return SmileUnboxedInteger16_From(x);
		
		case 2:
			x = argv[0].unboxed.i16;
			x -= argv[1].unboxed.i16;
			return SmileUnboxedInteger16_From(x);
		
		case 3:
			x = argv[0].unboxed.i16;
			x -= argv[1].unboxed.i16;
			x -= argv[2].unboxed.i16;
			return SmileUnboxedInteger16_From(x);
		
		case 4:
			x = argv[0].unboxed.i16;
			x -= argv[1].unboxed.i16;
			x -= argv[2].unboxed.i16;
			x -= argv[3].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		default:
			x = argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				x -= argv[i].unboxed.i16;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Star)
{
	Int16 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = argv[0].unboxed.i16;
			x *= argv[1].unboxed.i16;
			return SmileUnboxedInteger16_From(x);
		
		case 3:
			x = argv[0].unboxed.i16;
			x *= argv[1].unboxed.i16;
			x *= argv[2].unboxed.i16;
			return SmileUnboxedInteger16_From(x);
		
		case 4:
			x = argv[0].unboxed.i16;
			x *= argv[1].unboxed.i16;
			x *= argv[2].unboxed.i16;
			x *= argv[3].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		default:
			x = argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				x *= argv[i].unboxed.i16;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UStar)
{
	UInt16 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt16)argv[0].unboxed.i16;
			x *= (UInt16)argv[1].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = (UInt16)argv[0].unboxed.i16;
			x *= (UInt16)argv[1].unboxed.i16;
			x *= (UInt16)argv[2].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = (UInt16)argv[0].unboxed.i16;
			x *= (UInt16)argv[1].unboxed.i16;
			x *= (UInt16)argv[2].unboxed.i16;
			x *= (UInt16)argv[3].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		default:
			x = (UInt16)argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				x *= (UInt16)argv[i].unboxed.i16;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

/// <summary>
/// Deal with division-by-zero.
/// </summary>
/// <param name="param">A pointer to a MathInfo struct that describes how to handle divide-by-zero.</param>
/// <returns>0 if this is a quiet divide-by-zero, or a thrown exception if this is supposed to be an error.</returns>
Inline SmileArg DivideByZero(void *param)
{
	MathInfo mathInfo = (MathInfo)param;

	if (mathInfo->isLoud)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return SmileUnboxedInteger16_From(0);
}

/// <summary>
/// Perform division classic-C-style, which rounds towards zero no matter what the sign is.
/// </summary>
Inline Int16 CDiv(Int16 dividend, Int16 divisor)
{
	if (dividend < 0) {
		if (divisor < 0)
			return (Int16)((UInt16)-dividend / (UInt16)-divisor);
		else
			return -(Int16)((UInt16)-dividend / (UInt16)divisor);
	}
	else if (divisor < 0)
		return -(Int16)((UInt16)dividend / (UInt16)-divisor);
	else
		return (Int16)((UInt16)dividend / (UInt16)divisor);
}

/// <summary>
/// Perform division like mathematicians expect, which rounds toward negative infinity (always).
/// </summary>
Inline Int16 MathematiciansDiv(Int16 dividend, Int16 divisor)
{
	if (dividend < 0) {
		if (divisor < 0) {
			return (Int16)((UInt16)-dividend / (UInt16)-divisor);
		}
		else {
			UInt16 positiveQuotient = (UInt16)-dividend / (UInt16)divisor;
			UInt16 positiveRemainder = (UInt16)-dividend % (UInt16)divisor;
			return positiveRemainder == 0 ? -(Int16)positiveQuotient : -(Int16)positiveQuotient - 1;
		}
	}
	else if (divisor < 0) {
		UInt16 positiveQuotient = (UInt16)dividend / (UInt16)-divisor;
		UInt16 positiveRemainder = (UInt16)dividend % (UInt16)-divisor;
		return positiveRemainder == 0 ? -(Int16)positiveQuotient : -(Int16)positiveQuotient - 1;
	}
	else {
		return dividend / divisor;
	}
}

SMILE_EXTERNAL_FUNCTION(Slash)
{
	Int16 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[2].unboxed.i16) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[2].unboxed.i16) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[3].unboxed.i16) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedInteger16_From(x);

		default:
			x = argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i16) == 0)
					return DivideByZero(param);
				x = MathematiciansDiv(x, y);
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(USlash)
{
	UInt16 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (UInt16)argv[2].unboxed.i16) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (UInt16)argv[2].unboxed.i16) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (UInt16)argv[3].unboxed.i16) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedInteger16_From(x);

		default:
			x = (UInt16)argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				if ((y = (UInt16)argv[i].unboxed.i16) == 0)
					return DivideByZero(param);
				x /= y;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Div)
{
	Int16 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[2].unboxed.i16) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[2].unboxed.i16) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[3].unboxed.i16) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedInteger16_From(x);

		default:
			x = argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i16) == 0)
					return DivideByZero(param);
				x = CDiv(x, y);
			}
			return SmileUnboxedInteger16_From(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Int16 MathematiciansModulus(Int16 x, Int16 y)
{
	Int16 rem;

	if (x < 0) {
		if (y < 0)
			return -(-x % -y);
		else {
			rem = -x % y;
			return rem != 0 ? y - rem : 0;
		}
	}
	else if (y < 0) {
		rem = x % -y;
		return rem != 0 ? y + rem : 0;
	}
	else
		return x % y;
}

/// <summary>
/// Perform remainder, in which the result has the same sign as the dividend (x).
/// </summary>
Inline Int16 MathematiciansRemainder(Int16 x, Int16 y)
{
	Int16 rem;

	if (x < 0) {
		if (y < 0) {
			rem = -x % -y;
			return rem != 0 ? rem + y : 0;
		}
		else
			return -(-x % y);
	}
	else if (y < 0)
		return x % -y;
	else {
		rem = x % y;
		return rem != 0 ? rem - y : 0;
	}
}

SMILE_EXTERNAL_FUNCTION(Mod)
{
	Int16 x = argv[0].unboxed.i16;
	Int16 y = argv[1].unboxed.i16;

	if (y == 0)
		return DivideByZero(param);
	
	return SmileUnboxedInteger16_From(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(UMod)
{
	UInt16 x = (UInt16)argv[0].unboxed.i16;
	UInt16 y = (UInt16)argv[1].unboxed.i16;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedInteger16_From(x % y);
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	Int16 x = argv[0].unboxed.i16;
	Int16 y = argv[1].unboxed.i16;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedInteger16_From(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	Int16 value = argv[0].unboxed.i16;

	return value == 0 ? SmileUnboxedInteger16_From(0)
		: value > 0 ? SmileUnboxedInteger16_From(1)
		: SmileUnboxedInteger16_From(-1);
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
	Int16 value = argv[0].unboxed.i16;

	return value < 0 ? SmileUnboxedInteger16_From(-value) : argv[0];
}

SMILE_EXTERNAL_FUNCTION(Clip)
{
	Int16 value = argv[0].unboxed.i16;
	Int16 min = argv[1].unboxed.i16;
	Int16 max = argv[2].unboxed.i16;

	if (value > max) {
		value = max;
		return value < min ? argv[1] : argv[2];
	}
	else if (value < min) {
		return argv[1];
	}
	else return argv[0];
}

SMILE_EXTERNAL_FUNCTION(UClip)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;
	UInt16 min = (UInt16)argv[1].unboxed.i16;
	UInt16 max = (UInt16)argv[2].unboxed.i16;

	if (value > max) {
		value = max;
		return value < min ? argv[1] : argv[2];
	}
	else if (value < min) {
		return argv[1];
	}
	else return argv[0];
}

SMILE_EXTERNAL_FUNCTION(Min)
{
	Int16 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) < x) x = y;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) < x) x = y;
			if ((y = argv[2].unboxed.i16) < x) x = y;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) < x) x = y;
			if ((y = argv[2].unboxed.i16) < x) x = y;
			if ((y = argv[3].unboxed.i16) < x) x = y;
			return SmileUnboxedInteger16_From(x);

		default:
			x = argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i16) < x) x = y;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UMin)
{
	UInt16 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) < x) x = y;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) < x) x = y;
			if ((y = (UInt16)argv[2].unboxed.i16) < x) x = y;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) < x) x = y;
			if ((y = (UInt16)argv[2].unboxed.i16) < x) x = y;
			if ((y = (UInt16)argv[3].unboxed.i16) < x) x = y;
			return SmileUnboxedInteger16_From(x);

		default:
			x = (UInt16)argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				if ((y = (UInt16)argv[i].unboxed.i16) < x) x = y;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Max)
{
	Int16 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) > x) x = y;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) > x) x = y;
			if ((y = argv[2].unboxed.i16) > x) x = y;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = argv[0].unboxed.i16;
			if ((y = argv[1].unboxed.i16) > x) x = y;
			if ((y = argv[2].unboxed.i16) > x) x = y;
			if ((y = argv[3].unboxed.i16) > x) x = y;
			return SmileUnboxedInteger16_From(x);

		default:
			x = argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i16) > x) x = y;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UMax)
{
	UInt16 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) > x) x = y;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) > x) x = y;
			if ((y = (UInt16)argv[2].unboxed.i16) > x) x = y;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = (UInt16)argv[0].unboxed.i16;
			if ((y = (UInt16)argv[1].unboxed.i16) > x) x = y;
			if ((y = (UInt16)argv[2].unboxed.i16) > x) x = y;
			if ((y = (UInt16)argv[3].unboxed.i16) > x) x = y;
			return SmileUnboxedInteger16_From(x);

		default:
			x = (UInt16)argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				if ((y = (UInt16)argv[i].unboxed.i16) > x) x = y;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

Inline Int16 IntPower(Int16 value, Int16 exponent)
{
	if (exponent < 0) return 0;

	Int16 result = 1;

	while (exponent > 0) {
		if ((exponent & 1) != 0) {
			result *= value;
		}
		exponent >>= 1;
		value *= value;
	}

	return result;
}

SMILE_EXTERNAL_FUNCTION(Power)
{
	Int16 x;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i16;
			x = IntPower(x, argv[1].unboxed.i16);
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = argv[0].unboxed.i16;
			x = IntPower(x, argv[1].unboxed.i16);
			x = IntPower(x, argv[2].unboxed.i16);
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = argv[0].unboxed.i16;
			x = IntPower(x, argv[1].unboxed.i16);
			x = IntPower(x, argv[2].unboxed.i16);
			x = IntPower(x, argv[3].unboxed.i16);
			return SmileUnboxedInteger16_From(x);

		default:
			x = argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				x = IntPower(x, argv[i].unboxed.i16);
			}
			return SmileUnboxedInteger16_From(x);
	}
}

Inline UInt16 IntSqrt(UInt16 value)
{
	UInt16 root, bit, trial;

	root = 0;
	bit = (1U << 14);

	do {
		trial = root + bit;
		if (value >= trial) {
			value -= trial;
			root = trial + bit;
		}
		root >>= 1;
		bit >>= 2;
	} while (bit != 0);

	return root;
}

SMILE_EXTERNAL_FUNCTION(Sqrt)
{
	Int16 value = argv[0].unboxed.i16;

	if (value < 0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);
		return SmileUnboxedInteger16_From(0);
	}

	return SmileUnboxedInteger16_From(IntSqrt(value));
}

SMILE_EXTERNAL_FUNCTION(Pow2Q)
{
	Int16 value = argv[0].unboxed.i16;

	return SmileUnboxedBool_From(value > 0 && (value & (value - 1)) == 0);
}

SMILE_EXTERNAL_FUNCTION(NextPow2)
{
	Int16 value = (Int16)argv[0].unboxed.i16;
	UInt16 uvalue = (UInt16)value;

	if (value < 0) return SmileUnboxedInteger16_From(0);
	if (value == 0) return SmileUnboxedInteger16_From(1);

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue |= uvalue >> 8;
	uvalue++;

	return SmileUnboxedInteger16_From(uvalue);
}

SMILE_EXTERNAL_FUNCTION(IntLg)
{
	Int16 value = (Int16)argv[0].unboxed.i16;
	UInt16 uvalue = (UInt16)value;
	UInt16 log;

	if (value <= 0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeLog);
		return SmileUnboxedInteger16_From(0);
	}

	log = 0;
	if ((uvalue & 0xFF00) != 0) uvalue >>= 8, log += 8;
	if ((uvalue & 0x00F0) != 0) uvalue >>= 4, log += 4;
	if ((uvalue & 0x000C) != 0) uvalue >>= 2, log += 2;
	if ((uvalue & 0x0002) != 0) uvalue >>= 1, log += 1;

	return SmileUnboxedInteger16_From((Int16)log);
}

//-------------------------------------------------------------------------------------------------
// Bitwise operators

SMILE_EXTERNAL_FUNCTION(BitAnd)
{
	UInt16 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt16)argv[0].unboxed.i16;
			x &= (UInt16)argv[1].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = (UInt16)argv[0].unboxed.i16;
			x &= (UInt16)argv[1].unboxed.i16;
			x &= (UInt16)argv[2].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = (UInt16)argv[0].unboxed.i16;
			x &= (UInt16)argv[1].unboxed.i16;
			x &= (UInt16)argv[2].unboxed.i16;
			x &= (UInt16)argv[3].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		default:
			x = (UInt16)argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				x &= (UInt16)argv[i].unboxed.i16;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitOr)
{
	UInt16 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt16)argv[0].unboxed.i16;
			x |= (UInt16)argv[1].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = (UInt16)argv[0].unboxed.i16;
			x |= (UInt16)argv[1].unboxed.i16;
			x |= (UInt16)argv[2].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = (UInt16)argv[0].unboxed.i16;
			x |= (UInt16)argv[1].unboxed.i16;
			x |= (UInt16)argv[2].unboxed.i16;
			x |= (UInt16)argv[3].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		default:
			x = (UInt16)argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				x |= (UInt16)argv[i].unboxed.i16;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitXor)
{
	UInt16 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt16)argv[0].unboxed.i16;
			x ^= (UInt16)argv[1].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		case 3:
			x = (UInt16)argv[0].unboxed.i16;
			x ^= (UInt16)argv[1].unboxed.i16;
			x ^= (UInt16)argv[2].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		case 4:
			x = (UInt16)argv[0].unboxed.i16;
			x ^= (UInt16)argv[1].unboxed.i16;
			x ^= (UInt16)argv[2].unboxed.i16;
			x ^= (UInt16)argv[3].unboxed.i16;
			return SmileUnboxedInteger16_From(x);

		default:
			x = (UInt16)argv[0].unboxed.i16;
			for (i = 1; i < argc; i++) {
				x ^= (UInt16)argv[i].unboxed.i16;
			}
			return SmileUnboxedInteger16_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitNot)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	return SmileUnboxedInteger16_From(~value);
}

//-------------------------------------------------------------------------------------------------
// Shift/rotate operators

SMILE_EXTERNAL_FUNCTION(LogicalShiftLeft)
{
	UInt16 x = (UInt16)argv[0].unboxed.i16;
	UInt16 y = (UInt16)argv[1].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)(x << y));
}

SMILE_EXTERNAL_FUNCTION(LogicalShiftRight)
{
	UInt16 x = (UInt16)argv[0].unboxed.i16;
	UInt16 y = (UInt16)argv[1].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)(x >> y));
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftLeft)
{
	Int16 x = argv[0].unboxed.i16;
	Int16 y = argv[1].unboxed.i16;

	return SmileUnboxedInteger16_From(x << y);
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftRight)
{
	Int16 x = argv[0].unboxed.i16;
	Int16 y = argv[1].unboxed.i16;

	return SmileUnboxedInteger16_From(x >> y);
}

SMILE_EXTERNAL_FUNCTION(RotateLeft)
{
	UInt16 x = (UInt16)argv[0].unboxed.i16;
	UInt16 y = (UInt16)argv[1].unboxed.i16;

	return SmileUnboxedInteger16_From(Smile_RotateLeft16(x, y));
}

SMILE_EXTERNAL_FUNCTION(RotateRight)
{
	UInt16 x = (UInt16)argv[0].unboxed.i16;
	UInt16 y = (UInt16)argv[1].unboxed.i16;

	return SmileUnboxedInteger16_From(Smile_RotateRight16(x, y));
}

//-------------------------------------------------------------------------------------------------
// Bit twiddling

Inline UInt32 CountBitsSet(UInt32 value)
{
	value = value - ((value >> 1) & 0x55555555);
	value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
	return ((value + (value >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

Inline UInt16 ComputeReverseBits(UInt16 value)
{
	value = ((value >> 1) & 0x5555) | ((value & 0x5555) << 1);
	value = ((value >> 2) & 0x3333) | ((value & 0x3333) << 2);
	value = ((value >> 4) & 0x0F0F) | ((value & 0x0F0F) << 4);
	value = (value >> 8) | (value << 8);
	return value;
}

Inline UInt16 ComputeCountOfRightZeros(UInt16 value)
{
	UInt16 c = 16;
	value &= ~value + 1;
	if (value != 0) c--;
	if ((value & 0x00FF) != 0) c -= 8;
	if ((value & 0x0F0F) != 0) c -= 4;
	if ((value & 0x3333) != 0) c -= 2;
	if ((value & 0x5555) != 0) c -= 1;
	return c;
}

SMILE_EXTERNAL_FUNCTION(CountOnes)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)CountBitsSet(value));
}

SMILE_EXTERNAL_FUNCTION(CountZeros)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)CountBitsSet(~value));
}

SMILE_EXTERNAL_FUNCTION(Parity)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	value ^= value >> 8;
	value ^= value >> 4;
	value &= 0xF;
	value = (0x6996 >> value) & 1;

	return SmileUnboxedInteger16_From((Int16)value);
}

SMILE_EXTERNAL_FUNCTION(ReverseBits)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)ComputeReverseBits(value));
}

SMILE_EXTERNAL_FUNCTION(ReverseBytes)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	value = (UInt16)( ((value >>  8) & 0x00FFU)
						| ((value <<  8) & 0xFF00U) );

	return SmileUnboxedInteger16_From((Int16)value);
}

SMILE_EXTERNAL_FUNCTION(CountRightZeros)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)ComputeCountOfRightZeros(value));
}

SMILE_EXTERNAL_FUNCTION(CountRightOnes)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)ComputeCountOfRightZeros(~value));
}

SMILE_EXTERNAL_FUNCTION(CountLeftZeros)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)ComputeCountOfRightZeros(ComputeReverseBits(value)));
}

SMILE_EXTERNAL_FUNCTION(CountLeftOnes)
{
	UInt16 value = (UInt16)argv[0].unboxed.i16;

	return SmileUnboxedInteger16_From((Int16)ComputeCountOfRightZeros(~ComputeReverseBits(value)));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_INTEGER16
		&& argv[0].unboxed.i16 == argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_INTEGER16
		|| argv[0].unboxed.i16 != argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i16 < argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i16 > argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i16 <= argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i16 >= argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(ULt)
{
	return SmileUnboxedBool_From((UInt16)argv[0].unboxed.i16 < (UInt16)argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(UGt)
{
	return SmileUnboxedBool_From((UInt16)argv[0].unboxed.i16 > (UInt16)argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(ULe)
{
	return SmileUnboxedBool_From((UInt16)argv[0].unboxed.i16 <= (UInt16)argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(UGe)
{
	return SmileUnboxedBool_From((UInt16)argv[0].unboxed.i16 >= (UInt16)argv[1].unboxed.i16);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Int16 x = argv[0].unboxed.i16;
	Int16 y = argv[1].unboxed.i16;

	if (x == y)
		return SmileUnboxedInteger16_From(0);
	else if (x < y)
		return SmileUnboxedInteger16_From(-1);
	else
		return SmileUnboxedInteger16_From(+1);
}

SMILE_EXTERNAL_FUNCTION(UCompare)
{
	UInt16 x = (UInt16)argv[0].unboxed.i16;
	UInt16 y = (UInt16)argv[1].unboxed.i16;

	if (x == y)
		return SmileUnboxedInteger16_From(0);
	else if (x < y)
		return SmileUnboxedInteger16_From(-1);
	else
		return SmileUnboxedInteger16_From(+1);
}

//-------------------------------------------------------------------------------------------------

void SmileInteger16_Setup(SmileUserObject base)
{
	SmileUnboxedInteger16_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("int64", ToInt64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("int32", ToInt32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("int16", ToInt16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("byte", ToByte, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);

	SetupFunction("parse", Parse, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 1, 1, _parseChecks);

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupSynonym("+", "+~");
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupSynonym("-", "-~");
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("*~", UStar, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);

	SetupFunction("/", Slash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("/!", Slash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("/~", USlash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("/!~", USlash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("div", Div, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("div!", Div, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("div~", USlash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("div!~", USlash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("mod", Mod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("mod!", Mod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("mod~", UMod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("mod!~", UMod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("rem", Rem, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("rem!", Rem, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("rem~", UMod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("rem!~", UMod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer16Checks);
	SetupFunction("clip~", UClip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer16Checks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("min~", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("max~", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("^", Power, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("sqrt", Sqrt, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("sqrt!", Sqrt, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("pow2?", Pow2Q, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("next-pow2", NextPow2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("int-lg", IntLg, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("int-lg!", IntLg, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);

	SetupFunction("band", BitAnd, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("bor", BitOr, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("bxor", BitXor, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("~", BitNot, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);

	SetupFunction("<<<", LogicalShiftLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction(">>>", LogicalShiftRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("<<", ArithmeticShiftLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction(">>", ArithmeticShiftRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("<<+", RotateLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("+>>", RotateRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);

	SetupFunction("count-ones", CountOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("count-zeros", CountZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("parity", Parity, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("reverse-bits", ReverseBits, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("reverse-bytes", ReverseBytes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("count-right-zeros", CountRightZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("count-right-ones", CountRightOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("count-left-zeros", CountLeftZeros , NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("count-left-ones", CountLeftOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16ComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16ComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("<~", ULt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction(">~", UGt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("<=~", ULe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction(">=~", UGe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupSynonym("compare", "cmp");
	SetupFunction("compare~", UCompare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupSynonym("compare~", "cmp~");
}
