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

static Byte _byteChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
};

static Byte _byteComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
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

STATIC_STRING(_invalidTypeError, "All arguments to 'Byte.%s' must be of type 'Byte'");

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Integer64'");
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
STATIC_STRING(_parseArguments, "Illegal arguments to 'parse' function");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BYTE)
		return SmileUnboxedBool_From(!!argv[0].unboxed.i8);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BYTE)
		return SmileUnboxedInteger64_From(argv[0].unboxed.i8);

	return SmileUnboxedInteger64_From(0);
}

STATIC_STRING(_Byte, "Byte");

SMILE_EXTERNAL_FUNCTION(ToString)
{
	Int64 numericBase;

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BYTE) {
		if (argc == 2) {
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_INTEGER64)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)argv[1].unboxed.i64;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;

		return SmileArg_From((SmileObject)String_CreateFromInteger((Int64)argv[0].unboxed.i8, (Int)numericBase, False));
	}

	return SmileArg_From((SmileObject)_Byte);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BYTE)
		return SmileUnboxedInteger64_From(argv[0].unboxed.i8);

	return SmileUnboxedInteger64_From((PtrInt)argv[0].obj ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From(argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return SmileUnboxedInteger32_From(argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return SmileUnboxedInteger16_From(argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	return argv[0];
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
			return SmileUnboxedByte_From((Byte)value);

		case 2:
			// Either the form [parse string base] or [obj.parse string].
			if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING && SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_INTEGER64) {
				// The form [parse string base].
				numericBase = (Int)argv[1].unboxed.i64;
				if (numericBase < 2 || numericBase > 36)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
				if (!String_ParseInteger((String)argv[0].obj, (Int)numericBase, &value))
					return SmileArg_From(NullObject);
				return SmileUnboxedByte_From((Byte)value);
			}
			else if (SMILE_KIND(argv[1].obj) == SMILE_KIND_STRING) {
				// The form [obj.parse string].
				if (!String_ParseInteger((String)argv[1].obj, 10, &value))
					return SmileArg_From(NullObject);
				return SmileUnboxedByte_From((Byte)value);
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
			return SmileUnboxedByte_From((Byte)value);
	}

	return SmileArg_From(NullObject);	// Can't get here, but the compiler doesn't know that.
}

//-------------------------------------------------------------------------------------------------
// Arithmetic operators

SMILE_EXTERNAL_FUNCTION(Plus)
{
	Byte x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = argv[0].unboxed.i8;
			x += argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);
		
		case 3:
			x = argv[0].unboxed.i8;
			x += argv[1].unboxed.i8;
			x += argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			x += argv[1].unboxed.i8;
			x += argv[2].unboxed.i8;
			x += argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x += argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	Byte x;
	Int i;

	switch (argc) {
		case 1:
			x = -argv[0].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 2:
			x = argv[0].unboxed.i8;
			x -= argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 3:
			x = argv[0].unboxed.i8;
			x -= argv[1].unboxed.i8;
			x -= argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			x -= argv[1].unboxed.i8;
			x -= argv[2].unboxed.i8;
			x -= argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x -= argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Star)
{
	SByte x;
	Int i;

	switch (argc) {
		case 2:
			x = (SByte)argv[0].unboxed.i8;
			x *= (SByte)argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (SByte)argv[0].unboxed.i8;
			x *= (SByte)argv[1].unboxed.i8;
			x *= (SByte)argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (SByte)argv[0].unboxed.i8;
			x *= (SByte)argv[1].unboxed.i8;
			x *= (SByte)argv[2].unboxed.i8;
			x *= (SByte)argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = (SByte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x *= (SByte)argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UStar)
{
	Byte x;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i8;
			x *= argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 3:
			x = argv[0].unboxed.i8;
			x *= argv[1].unboxed.i8;
			x *= argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			x *= argv[1].unboxed.i8;
			x *= argv[2].unboxed.i8;
			x *= argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x *= argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
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

	return SmileUnboxedByte_From(0);
}

/// <summary>
/// Perform division classic-C-style, which rounds towards zero no matter what the sign is.
/// </summary>
Inline SByte CDiv(SByte dividend, SByte divisor)
{
	if (dividend < 0) {
		if (divisor < 0)
			return (SByte)((Byte)-dividend / (Byte)-divisor);
		else
			return -(SByte)((Byte)-dividend / (Byte)divisor);
	}
	else if (divisor < 0)
		return -(SByte)((Byte)dividend / (Byte)-divisor);
	else
		return (SByte)((Byte)dividend / (Byte)divisor);
}

/// <summary>
/// Perform division like mathematicians expect, which rounds toward negative infinity (always).
/// </summary>
Inline SByte MathematiciansDiv(SByte dividend, SByte divisor)
{
	if (dividend < 0) {
		if (divisor < 0) {
			return (SByte)((Byte)-dividend / (Byte)-divisor);
		}
		else {
			Byte positiveQuotient = (Byte)-dividend / (Byte)divisor;
			Byte positiveRemainder = (Byte)-dividend % (Byte)divisor;
			return positiveRemainder == 0 ? -(SByte)positiveQuotient : -(SByte)positiveQuotient - 1;
		}
	}
	else if (divisor < 0) {
		Byte positiveQuotient = (Byte)dividend / (Byte)-divisor;
		Byte positiveRemainder = (Byte)dividend % (Byte)-divisor;
		return positiveRemainder == 0 ? -(SByte)positiveQuotient : -(SByte)positiveQuotient - 1;
	}
	else {
		return dividend / divisor;
	}
}

SMILE_EXTERNAL_FUNCTION(Slash)
{
	SByte x, y;
	Int i;

	switch (argc) {
		case 2:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedByte_From(x);

		case 3:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = (SByte)argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedByte_From(x);

		case 4:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = (SByte)argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = (SByte)argv[3].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedByte_From(x);

		default:
			x = (SByte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = (SByte)argv[i].unboxed.i8) == 0)
					return DivideByZero(param);
				x = MathematiciansDiv(x, y);
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(USlash)
{
	Byte x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedByte_From(x);

		case 3:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[3].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i8) == 0)
					return DivideByZero(param);
				x /= y;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Div)
{
	SByte x, y;
	Int i;

	switch (argc) {
		case 2:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedByte_From(x);

		case 3:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = (SByte)argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedByte_From(x);

		case 4:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = (SByte)argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = (SByte)argv[3].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedByte_From(x);

		default:
			x = (SByte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = (SByte)argv[i].unboxed.i8) == 0)
					return DivideByZero(param);
				x = CDiv(x, y);
			}
			return SmileUnboxedByte_From(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline SByte MathematiciansModulus(SByte x, SByte y)
{
	SByte rem;

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
Inline SByte MathematiciansRemainder(SByte x, SByte y)
{
	SByte rem;

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
	SByte x = (SByte)argv[0].unboxed.i8;
	SByte y = (SByte)argv[1].unboxed.i8;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedByte_From(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(UMod)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedByte_From(x % y);
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	SByte x = (SByte)argv[0].unboxed.i8;
	SByte y = (SByte)argv[1].unboxed.i8;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedByte_From(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	SByte value = (SByte)argv[0].unboxed.i8;

	return value == 0 ? SmileUnboxedByte_From(0)
		: value > 0 ? SmileUnboxedByte_From(1)
		: SmileUnboxedByte_From((Byte)(SByte)-1);
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
	SByte value = (SByte)argv[0].unboxed.i8;

	return value < 0 ? SmileUnboxedByte_From(-value) : argv[0];
}

SMILE_EXTERNAL_FUNCTION(Clip)
{
	SByte value = (SByte)argv[0].unboxed.i8;
	SByte min = (SByte)argv[1].unboxed.i8;
	SByte max = (SByte)argv[2].unboxed.i8;

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
	Byte value = argv[0].unboxed.i8;
	Byte min = argv[1].unboxed.i8;
	Byte max = argv[2].unboxed.i8;

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
	SByte x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) < x) x = y;
			if ((y = (SByte)argv[2].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) < x) x = y;
			if ((y = (SByte)argv[2].unboxed.i8) < x) x = y;
			if ((y = (SByte)argv[3].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		default:
			x = (SByte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = (SByte)argv[i].unboxed.i8) < x) x = y;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UMin)
{
	Byte x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		case 3:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) < x) x = y;
			if ((y = argv[2].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) < x) x = y;
			if ((y = argv[2].unboxed.i8) < x) x = y;
			if ((y = argv[3].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i8) < x) x = y;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Max)
{
	SByte x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) > x) x = y;
			if ((y = (SByte)argv[2].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (SByte)argv[0].unboxed.i8;
			if ((y = (SByte)argv[1].unboxed.i8) > x) x = y;
			if ((y = (SByte)argv[2].unboxed.i8) > x) x = y;
			if ((y = (SByte)argv[3].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		default:
			x = (SByte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = (SByte)argv[i].unboxed.i8) > x) x = y;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UMax)
{
	Byte x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		case 3:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) > x) x = y;
			if ((y = argv[2].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) > x) x = y;
			if ((y = argv[2].unboxed.i8) > x) x = y;
			if ((y = argv[3].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i8) > x) x = y;
			}
			return SmileUnboxedByte_From(x);
	}
}

Inline SByte IntPower(SByte value, SByte exponent)
{
	if (exponent < 0) return 0;

	SByte result = 1;

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
	SByte x;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i8;
			x = IntPower(x, argv[1].unboxed.i8);
			return SmileUnboxedByte_From(x);

		case 3:
			x = argv[0].unboxed.i8;
			x = IntPower(x, argv[1].unboxed.i8);
			x = IntPower(x, argv[2].unboxed.i8);
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			x = IntPower(x, argv[1].unboxed.i8);
			x = IntPower(x, argv[2].unboxed.i8);
			x = IntPower(x, argv[3].unboxed.i8);
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x = IntPower(x, argv[i].unboxed.i8);
			}
			return SmileUnboxedByte_From(x);
	}
}

Inline Byte IntSqrt(Byte value)
{
	Byte root, bit, trial;

	root = 0;
	bit = (1U << 7);

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
	SByte value = argv[0].unboxed.i8;

	if (value < 0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);
		return SmileUnboxedByte_From(0);
	}

	return SmileUnboxedByte_From(IntSqrt(value));
}

SMILE_EXTERNAL_FUNCTION(Pow2Q)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedBool_From(value > 0 && (value & (value - 1)) == 0);
}

SMILE_EXTERNAL_FUNCTION(NextPow2)
{
	SByte value = (SByte)argv[0].unboxed.i8;
	Byte uvalue = (Byte)value;

	if (value < 0) return SmileUnboxedByte_From(0);
	if (value == 0) return SmileUnboxedByte_From(1);

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue++;

	return SmileUnboxedByte_From(uvalue);
}

SMILE_EXTERNAL_FUNCTION(IntLg)
{
	SByte value = (SByte)argv[0].unboxed.i8;
	Byte uvalue = (Byte)value;
	Byte log;

	if (value <= 0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeLog);
		return SmileUnboxedByte_From(0);
	}

	log = 0;
	if ((uvalue & 0x000000F0) != 0) uvalue >>= 4, log += 4;
	if ((uvalue & 0x0000000C) != 0) uvalue >>= 2, log += 2;
	if ((uvalue & 0x00000002) != 0) uvalue >>= 1, log += 1;

	return SmileUnboxedByte_From((Byte)log);
}

//-------------------------------------------------------------------------------------------------
// Bitwise operators

SMILE_EXTERNAL_FUNCTION(BitAnd)
{
	Byte x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = argv[0].unboxed.i8;
			x &= argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);
		
		case 3:
			x = argv[0].unboxed.i8;
			x &= argv[1].unboxed.i8;
			x &= argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			x &= argv[1].unboxed.i8;
			x &= argv[2].unboxed.i8;
			x &= argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x &= argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitOr)
{
	Byte x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = argv[0].unboxed.i8;
			x |= argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);
		
		case 3:
			x = argv[0].unboxed.i8;
			x |= argv[1].unboxed.i8;
			x |= argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			x |= argv[1].unboxed.i8;
			x |= argv[2].unboxed.i8;
			x |= argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x |= argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitXor)
{
	Byte x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = argv[0].unboxed.i8;
			x ^= argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);
		
		case 3:
			x = argv[0].unboxed.i8;
			x ^= argv[1].unboxed.i8;
			x ^= argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			x ^= argv[1].unboxed.i8;
			x ^= argv[2].unboxed.i8;
			x ^= argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x ^= argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitNot)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedByte_From(~value);
}

//-------------------------------------------------------------------------------------------------
// Shift/rotate operators

SMILE_EXTERNAL_FUNCTION(LogicalShiftLeft)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	return SmileUnboxedByte_From(x << y);
}

SMILE_EXTERNAL_FUNCTION(LogicalShiftRight)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	return SmileUnboxedByte_From(x >> y);
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftLeft)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	return SmileUnboxedByte_From(x << y);
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftRight)
{
	SByte x = (SByte)argv[0].unboxed.i8;
	SByte y = (SByte)argv[1].unboxed.i8;

	return SmileUnboxedByte_From((Byte)(x >> y));
}

SMILE_EXTERNAL_FUNCTION(RotateLeft)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	return SmileUnboxedByte_From(Smile_RotateLeft8(x, y));
}

SMILE_EXTERNAL_FUNCTION(RotateRight)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	return SmileUnboxedByte_From(Smile_RotateRight8(x, y));
}

//-------------------------------------------------------------------------------------------------
// Bit twiddling

Inline UInt32 CountBitsSet(UInt32 value)
{
	value = value - ((value >> 1) & 0x55555555);
	value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
	return ((value + (value >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

Inline Byte ComputeReverseBits(Byte value)
{
	value = ((value >> 1) & 0x55) | ((value & 0x55) << 1);
	value = ((value >> 2) & 0x33) | ((value & 0x33) << 2);
	value = (value >> 4) | (value << 4);
	return value;
}

Inline Byte ComputeCountOfRightZeros(Byte value)
{
	Byte c = 8;
	value &= ~value + 1;
	if (value != 0) c--;
	if ((value & 0x0F) != 0) c -= 4;
	if ((value & 0x33) != 0) c -= 2;
	if ((value & 0x55) != 0) c -= 1;
	return c;
}

SMILE_EXTERNAL_FUNCTION(CountOnes)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)CountBitsSet(value));
}

SMILE_EXTERNAL_FUNCTION(CountZeros)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)CountBitsSet(~value));
}

SMILE_EXTERNAL_FUNCTION(Parity)
{
	Byte value = argv[0].unboxed.i8;

	value ^= value >> 4;
	value &= 0xF;
	value = (0x6996 >> value) & 1;

	return SmileUnboxedByte_From(value);
}

SMILE_EXTERNAL_FUNCTION(ReverseBits)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedByte_From(ComputeReverseBits(value));
}

SMILE_EXTERNAL_FUNCTION(ReverseBytes)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(CountRightZeros)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedByte_From(ComputeCountOfRightZeros(value));
}

SMILE_EXTERNAL_FUNCTION(CountRightOnes)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedByte_From(ComputeCountOfRightZeros(~value));
}

SMILE_EXTERNAL_FUNCTION(CountLeftZeros)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedByte_From(ComputeCountOfRightZeros(ComputeReverseBits(value)));
}

SMILE_EXTERNAL_FUNCTION(CountLeftOnes)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedByte_From(ComputeCountOfRightZeros(~ComputeReverseBits(value)));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_BYTE
		&& argv[0].unboxed.i8 == argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_BYTE
		|| argv[0].unboxed.i8 != argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From((SByte)argv[0].unboxed.i8 < (SByte)argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From((SByte)argv[0].unboxed.i8 >(SByte)argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From((SByte)argv[0].unboxed.i8 <= (SByte)argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From((SByte)argv[0].unboxed.i8 >= (SByte)argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ULt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i8 < argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(UGt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i8 > argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ULe)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i8 <= argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(UGe)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i8 >= argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	SByte x = (SByte)argv[0].unboxed.i8;
	SByte y = (SByte)argv[1].unboxed.i8;

	if (x == y)
		return SmileUnboxedByte_From(0);
	else if (x < y)
		return SmileUnboxedByte_From((Byte)(SByte)-1);
	else
		return SmileUnboxedByte_From(+1);
}

SMILE_EXTERNAL_FUNCTION(UCompare)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	if (x == y)
		return SmileUnboxedByte_From(0);
	else if (x < y)
		return SmileUnboxedByte_From((Byte)(SByte)-1);
	else
		return SmileUnboxedByte_From(+1);
}

//-------------------------------------------------------------------------------------------------

void SmileByte_Setup(SmileUserObject base)
{
	SmileUnboxedByte_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("int64", ToInt64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("int32", ToByte, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("int16", ToInt16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("byte", ToByte, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("parse", Parse, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 1, 1, _parseChecks);

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupSynonym("+", "+~");
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupSynonym("-", "-~");
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("*~", UStar, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);

	SetupFunction("/", Slash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("/!", Slash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("/~", USlash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("/!~", USlash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("div", Div, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("div!", Div, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("div~", USlash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("div!~", USlash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("mod", Mod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("mod!", Mod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("mod~", UMod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("mod!~", UMod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("rem", Rem, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("rem!", Rem, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("rem~", UMod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("rem!~", UMod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _byteChecks);
	SetupFunction("clip~", UClip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _byteChecks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("min~", UMin, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("max~", UMax, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("^", Power, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("sqrt", Sqrt, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("sqrt!", Sqrt, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("pow2?", Pow2Q, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("next-pow2", NextPow2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("int-lg", IntLg, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("int-lg!", IntLg, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("band", BitAnd, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("bor", BitOr, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("bxor", BitXor, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("~", BitNot, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("<<<", LogicalShiftLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction(">>>", LogicalShiftRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("<<", ArithmeticShiftLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction(">>", ArithmeticShiftRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("<<+", RotateLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("+>>", RotateRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);

	SetupFunction("count-ones", CountOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("count-zeros", CountZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("parity", Parity, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("reverse-bits", ReverseBits, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("reverse-bytes", ReverseBytes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("count-right-zeros", CountRightZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("count-right-ones", CountRightOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("count-left-zeros", CountLeftZeros , NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("count-left-ones", CountLeftOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("<~", ULt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction(">~", UGt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("<=~", ULe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction(">=~", UGe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupSynonym("compare", "cmp");
	SetupFunction("compare~", UCompare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupSynonym("compare~", "cmp~");
}
