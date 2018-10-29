// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

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
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/range/smileinteger32range.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _integer32Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
};

static Byte _integer32ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER32,
	0, 0,
};

typedef struct MathInfoStruct {
	Bool isLoud;
} *MathInfo;

static struct MathInfoStruct _loudMath[] = { True };
static struct MathInfoStruct _quietMath[] = { False };

STATIC_STRING(_divideByZero, "Divide by zero error");
STATIC_STRING(_negativeLog, "Logarithm of negative or zero value");
STATIC_STRING(_negativeSqrt, "Square root of negative number");

STATIC_STRING(_invalidTypeError, "All arguments to 'Integer32.%s' must be of type 'Integer32'.");

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Integer32'");
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
STATIC_STRING(_parseArguments, "Illegal arguments to 'parse' function");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_INTEGER32)
		return SmileUnboxedBool_From(!!argv[0].unboxed.i32);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_INTEGER32)
		return SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i32);

	return SmileUnboxedInteger32_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	Int32 numericBase;
	STATIC_STRING(integer32, "Integer32");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_INTEGER32) {
		if (argc == 2) {
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_INTEGER32)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)argv[1].unboxed.i32;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;

		return SmileArg_From((SmileObject)String_CreateFromInteger(argv[0].unboxed.i32, (Int)numericBase, False));
	}

	return SmileArg_From((SmileObject)integer32);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileInteger32 obj = (SmileInteger32)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_INTEGER32)
		return SmileUnboxedInteger64_From((UInt32)obj->value);

	return SmileUnboxedInteger64_From((UInt32)((PtrInt)obj ^ Smile_HashOracle));
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(SignExtend64)
{
	return SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ZeroExtend64)
{
	return SmileUnboxedInteger64_From((Int64)(UInt64)(UInt32)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(SignExtend32)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(ZeroExtend32)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(SignExtend16)
{
	return SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ZeroExtend16)
{
	return SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	return SmileUnboxedByte_From((Byte)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(SignExtend8)
{
	return SmileUnboxedByte_From((Byte)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ZeroExtend8)
{
	return SmileUnboxedByte_From((Byte)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ToReal32)
{
	return SmileUnboxedReal32_From(Real32_FromInt64((Int64)argv[0].unboxed.i32));
}

SMILE_EXTERNAL_FUNCTION(ToReal64)
{
	return SmileUnboxedReal64_From(Real64_FromInt64((Int64)argv[0].unboxed.i32));
}

SMILE_EXTERNAL_FUNCTION(ToFloat32)
{
	return SmileUnboxedFloat32_From((Float32)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ToFloat64)
{
	return SmileUnboxedFloat64_From((Float64)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(RangeTo)
{
	Int32 start, end, step;

	start = argv[0].unboxed.i32;
	end = argv[1].unboxed.i32;
	step = end >= start ? +1 : -1;

	return SmileArg_From((SmileObject)SmileInteger32Range_Create(start, end, step));
}

SMILE_EXTERNAL_FUNCTION(ToChar)
{
	return SmileUnboxedChar_From((Byte)argv[0].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ToUni)
{
	return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.i32);
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
			return SmileUnboxedInteger32_From((Int32)value);

		case 2:
			// Either the form [parse string base] or [obj.parse string].
			if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING && SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_INTEGER32) {
				// The form [parse string base].
				numericBase = (Int)argv[1].unboxed.i64;
				if (numericBase < 2 || numericBase > 36)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
				if (!String_ParseInteger((String)argv[0].obj, (Int)numericBase, &value))
					return SmileArg_From(NullObject);
				return SmileUnboxedInteger32_From((Int32)value);
			}
			else if (SMILE_KIND(argv[1].obj) == SMILE_KIND_STRING) {
				// The form [obj.parse string].
				if (!String_ParseInteger((String)argv[1].obj, 10, &value))
					return SmileArg_From(NullObject);
				return SmileUnboxedInteger32_From((Int32)value);
			}
			else {
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _parseArguments);
			}

		case 3:
			// The form [obj.parse string base].
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_STRING || SMILE_KIND(argv[2].obj) != SMILE_KIND_UNBOXED_INTEGER32)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _parseArguments);
			numericBase = (Int)argv[2].unboxed.i64;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
			if (!String_ParseInteger((String)argv[1].obj, (Int)numericBase, &value))
				return SmileArg_From(NullObject);
			return SmileUnboxedInteger32_From((Int32)value);
	}

	return SmileArg_From(NullObject);	// Can't get here, but the compiler doesn't know that.
}

//-------------------------------------------------------------------------------------------------
// Arithmetic operators

SMILE_EXTERNAL_FUNCTION(Plus)
{
	Int32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.i32;
			x += argv[1].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = argv[0].unboxed.i32;
			x += argv[1].unboxed.i32;
			x += argv[2].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = argv[0].unboxed.i32;
			x += argv[1].unboxed.i32;
			x += argv[2].unboxed.i32;
			x += argv[3].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		default:
			x = argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				x += argv[i].unboxed.i32;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	Int32 x;
	Int i;

	switch (argc) {
		case 1:
			x = -argv[0].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 2:
			x = argv[0].unboxed.i32;
			x -= argv[1].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = argv[0].unboxed.i32;
			x -= argv[1].unboxed.i32;
			x -= argv[2].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = argv[0].unboxed.i32;
			x -= argv[1].unboxed.i32;
			x -= argv[2].unboxed.i32;
			x -= argv[3].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		default:
			x = argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				x -= argv[i].unboxed.i32;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Star)
{
	Int32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.i32;
			x *= argv[1].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = argv[0].unboxed.i32;
			x *= argv[1].unboxed.i32;
			x *= argv[2].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = argv[0].unboxed.i32;
			x *= argv[1].unboxed.i32;
			x *= argv[2].unboxed.i32;
			x *= argv[3].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		default:
			x = argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				x *= argv[i].unboxed.i32;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UStar)
{
	UInt32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt32)argv[0].unboxed.i32;
			x *= (UInt32)argv[1].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = (UInt32)argv[0].unboxed.i32;
			x *= (UInt32)argv[1].unboxed.i32;
			x *= (UInt32)argv[2].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = (UInt32)argv[0].unboxed.i32;
			x *= (UInt32)argv[1].unboxed.i32;
			x *= (UInt32)argv[2].unboxed.i32;
			x *= (UInt32)argv[3].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		default:
			x = (UInt32)argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				x *= (UInt32)argv[i].unboxed.i32;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(FMA)
{
	Int32 x = argv[0].unboxed.i32;
	Int32 y = argv[1].unboxed.i32;
	Int32 z = argv[2].unboxed.i32;

	return SmileUnboxedInteger32_From(x + y * z);
}

SMILE_EXTERNAL_FUNCTION(UFMA)
{
	UInt32 x = (UInt32)argv[0].unboxed.i32;
	UInt32 y = (UInt32)argv[1].unboxed.i32;
	UInt32 z = (UInt32)argv[2].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)(x + y * z));
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

	return SmileUnboxedInteger32_From(0);
}

/// <summary>
/// Perform division classic-C-style, which rounds towards zero no matter what the sign is.
/// </summary>
Inline Int32 CDiv(Int32 dividend, Int32 divisor)
{
	if (dividend < 0) {
		if (divisor < 0)
			return (Int32)((UInt32)-dividend / (UInt32)-divisor);
		else
			return -(Int32)((UInt32)-dividend / (UInt32)divisor);
	}
	else if (divisor < 0)
		return -(Int32)((UInt32)dividend / (UInt32)-divisor);
	else
		return (Int32)((UInt32)dividend / (UInt32)divisor);
}

/// <summary>
/// Perform division like mathematicians expect, which rounds toward negative infinity (always).
/// </summary>
Inline Int32 MathematiciansDiv(Int32 dividend, Int32 divisor)
{
	if (dividend < 0) {
		if (divisor < 0) {
			return (Int32)((UInt32)-dividend / (UInt32)-divisor);
		}
		else {
			UInt32 positiveQuotient = (UInt32)-dividend / (UInt32)divisor;
			UInt32 positiveRemainder = (UInt32)-dividend % (UInt32)divisor;
			return positiveRemainder == 0 ? -(Int32)positiveQuotient : -(Int32)positiveQuotient - 1;
		}
	}
	else if (divisor < 0) {
		UInt32 positiveQuotient = (UInt32)dividend / (UInt32)-divisor;
		UInt32 positiveRemainder = (UInt32)dividend % (UInt32)-divisor;
		return positiveRemainder == 0 ? -(Int32)positiveQuotient : -(Int32)positiveQuotient - 1;
	}
	else {
		return dividend / divisor;
	}
}

SMILE_EXTERNAL_FUNCTION(Slash)
{
	Int32 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[2].unboxed.i32) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[2].unboxed.i32) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[3].unboxed.i32) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedInteger32_From(x);

		default:
			x = argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i32) == 0)
					return DivideByZero(param);
				x = MathematiciansDiv(x, y);
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(USlash)
{
	UInt32 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (UInt32)argv[2].unboxed.i32) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (UInt32)argv[2].unboxed.i32) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (UInt32)argv[3].unboxed.i32) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedInteger32_From(x);

		default:
			x = (UInt32)argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				if ((y = (UInt32)argv[i].unboxed.i32) == 0)
					return DivideByZero(param);
				x /= y;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Div)
{
	Int32 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[2].unboxed.i32) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[2].unboxed.i32) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[3].unboxed.i32) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedInteger32_From(x);

		default:
			x = argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i32) == 0)
					return DivideByZero(param);
				x = CDiv(x, y);
			}
			return SmileUnboxedInteger32_From(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Int32 MathematiciansModulus(Int32 x, Int32 y)
{
	Int32 rem;

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
Inline Int32 MathematiciansRemainder(Int32 x, Int32 y)
{
	Int32 rem;

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
	Int32 x = argv[0].unboxed.i32;
	Int32 y = argv[1].unboxed.i32;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedInteger32_From(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(UMod)
{
	UInt32 x = (UInt32)argv[0].unboxed.i32;
	UInt32 y = (UInt32)argv[1].unboxed.i32;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedInteger32_From(x % y);
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	Int32 x = argv[0].unboxed.i32;
	Int32 y = argv[1].unboxed.i32;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedInteger32_From(MathematiciansRemainder(x, y));
}

SMILE_EXTERNAL_FUNCTION(IsDiv)
{
	Int32 x = argv[0].unboxed.i32;
	Int32 y = argv[1].unboxed.i32;

	return SmileUnboxedBool_From(y == 0 || x % y == 0);
}

SMILE_EXTERNAL_FUNCTION(IsUDiv)
{
	UInt32 x = (UInt32)argv[0].unboxed.i32;
	UInt32 y = (UInt32)argv[1].unboxed.i32;

	return SmileUnboxedBool_From(y == 0 || x % y == 0);
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	Int32 value = argv[0].unboxed.i32;

	return value == 0 ? SmileUnboxedInteger32_From(0) : value > 0 ? SmileUnboxedInteger32_From(1) : SmileUnboxedInteger32_From(-1);
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
#if 32 > 8
	Int32 value = argv[0].unboxed.i32;

	return value < 0 ? SmileUnboxedInteger32_From(-value) : argv[0];
#else
	return argv[0];
#endif
}

SMILE_EXTERNAL_FUNCTION(Clip)
{
	Int32 value = argv[0].unboxed.i32;
	Int32 min = argv[1].unboxed.i32;
	Int32 max = argv[2].unboxed.i32;

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
	UInt32 value = (UInt32)argv[0].unboxed.i32;
	UInt32 min = (UInt32)argv[1].unboxed.i32;
	UInt32 max = (UInt32)argv[2].unboxed.i32;

	if (value > max) {
		value = max;
		return value < min ? argv[1] : argv[2];
	}
	else if (value < min) {
		return argv[1];
	}
	else return argv[0];
}

SMILE_EXTERNAL_FUNCTION(Ramp)
{
#if 32 > 8
	Int32 value = argv[0].unboxed.i32;

	return value >= 0 ? argv[0] : SmileUnboxedInteger32_From(0);
#else
	return argv[0];
#endif
}

SMILE_EXTERNAL_FUNCTION(Heaviside)
{
	Int32 value = argv[0].unboxed.i32;

	return value <= 0 ? SmileUnboxedInteger32_From(0) : SmileUnboxedInteger32_From(1);
}

SMILE_EXTERNAL_FUNCTION(RectTri)
{
	Int32 value = argv[0].unboxed.i32;
	value = value < 0 ? -value : value;

	return value > 0 ? SmileUnboxedInteger32_From(0) : SmileUnboxedInteger32_From(1);
}

SMILE_EXTERNAL_FUNCTION(Min)
{
	Int32 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) < x) x = y;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) < x) x = y;
			if ((y = argv[2].unboxed.i32) < x) x = y;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) < x) x = y;
			if ((y = argv[2].unboxed.i32) < x) x = y;
			if ((y = argv[3].unboxed.i32) < x) x = y;
			return SmileUnboxedInteger32_From(x);

		default:
			x = argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i32) < x) x = y;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UMin)
{
	UInt32 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) < x) x = y;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) < x) x = y;
			if ((y = (UInt32)argv[2].unboxed.i32) < x) x = y;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) < x) x = y;
			if ((y = (UInt32)argv[2].unboxed.i32) < x) x = y;
			if ((y = (UInt32)argv[3].unboxed.i32) < x) x = y;
			return SmileUnboxedInteger32_From(x);

		default:
			x = (UInt32)argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				if ((y = (UInt32)argv[i].unboxed.i32) < x) x = y;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Max)
{
	Int32 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) > x) x = y;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) > x) x = y;
			if ((y = argv[2].unboxed.i32) > x) x = y;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = argv[0].unboxed.i32;
			if ((y = argv[1].unboxed.i32) > x) x = y;
			if ((y = argv[2].unboxed.i32) > x) x = y;
			if ((y = argv[3].unboxed.i32) > x) x = y;
			return SmileUnboxedInteger32_From(x);

		default:
			x = argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i32) > x) x = y;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(UMax)
{
	UInt32 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) > x) x = y;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) > x) x = y;
			if ((y = (UInt32)argv[2].unboxed.i32) > x) x = y;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = (UInt32)argv[0].unboxed.i32;
			if ((y = (UInt32)argv[1].unboxed.i32) > x) x = y;
			if ((y = (UInt32)argv[2].unboxed.i32) > x) x = y;
			if ((y = (UInt32)argv[3].unboxed.i32) > x) x = y;
			return SmileUnboxedInteger32_From(x);

		default:
			x = (UInt32)argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				if ((y = (UInt32)argv[i].unboxed.i32) > x) x = y;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

Inline Int32 IntPower(Int32 value, Int32 exponent)
{
	if (exponent < 0) return 0;

	Int32 result = 1;

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
	Int32 x;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i32;
			x = IntPower(x, argv[1].unboxed.i32);
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = argv[0].unboxed.i32;
			x = IntPower(x, argv[1].unboxed.i32);
			x = IntPower(x, argv[2].unboxed.i32);
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = argv[0].unboxed.i32;
			x = IntPower(x, argv[1].unboxed.i32);
			x = IntPower(x, argv[2].unboxed.i32);
			x = IntPower(x, argv[3].unboxed.i32);
			return SmileUnboxedInteger32_From(x);

		default:
			x = argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				x = IntPower(x, argv[i].unboxed.i32);
			}
			return SmileUnboxedInteger32_From(x);
	}
}

Inline UInt32 IntSqrt(UInt32 value)
{
	UInt32 root, bit, trial;

	root = 0;

#if 32 == 64
	bit =
		(value >= 0x100000000UL) ? (1ULL << 62)
		: (value >= 0x10000UL) ? (1ULL << 30)
		: (1ULL << 14);
#elif 32 == 32
	bit = (value >= 0x10000U) ? (1U << 30) : (1U << 14);
#elif 32 == 16
	bit = (1U << 14);
#elif 32 == 8
	bit = (1U << 7);
#endif

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
	Int32 value = argv[0].unboxed.i32;

	if (value < 0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);
		return SmileUnboxedInteger32_From(0);
	}

	return SmileUnboxedInteger32_From(IntSqrt(value));
}

SMILE_EXTERNAL_FUNCTION(Sqr)
{
	Int32 value = argv[0].unboxed.i32;
	return SmileUnboxedInteger32_From(value * value);
}

SMILE_EXTERNAL_FUNCTION(Cube)
{
	Int32 value = argv[0].unboxed.i32;
	return SmileUnboxedInteger32_From(value * value * value);
}

SMILE_EXTERNAL_FUNCTION(Pow2Q)
{
	Int32 value = argv[0].unboxed.i32;

	return SmileUnboxedBool_From(value > 0 && (value & (value - 1)) == 0);
}

SMILE_EXTERNAL_FUNCTION(NextPow2)
{
	Int32 value = (Int32)argv[0].unboxed.i32;
	UInt32 uvalue = (UInt32)value;

	if (value < 0) return SmileUnboxedInteger32_From(0);
	if (value == 0) return SmileUnboxedInteger32_From(1);

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
#if 32 >= 16
	uvalue |= uvalue >> 8;
#endif
#if 32 >= 32
	uvalue |= uvalue >> 16;
#endif
#if 32 >= 64
	uvalue |= uvalue >> 32;
#endif
	uvalue++;

	return SmileUnboxedInteger32_From(uvalue);
}

SMILE_EXTERNAL_FUNCTION(IntLg)
{
	Int32 value = (Int32)argv[0].unboxed.i32;
	UInt32 uvalue = (UInt32)value;
	UInt32 log;

	if (value <= 0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeLog);
		return SmileUnboxedInteger32_From(0);
	}

	log = 0;
#if 32 >= 64
	if ((uvalue & 0xFFFFFFFF00000000) != 0) uvalue >>= 32, log += 32;
#endif
#if 32 >= 32
	if ((uvalue & 0x00000000FFFF0000) != 0) uvalue >>= 16, log += 16;
#endif
#if 32 >= 16
	if ((uvalue & 0x000000000000FF00) != 0) uvalue >>= 8, log += 8;
#endif
	if ((uvalue & 0x00000000000000F0) != 0) uvalue >>= 4, log += 4;
	if ((uvalue & 0x000000000000000C) != 0) uvalue >>= 2, log += 2;
	if ((uvalue & 0x0000000000000002) != 0) uvalue >>= 1, log += 1;

	return SmileUnboxedInteger32_From((Int32)log);
}

SMILE_EXTERNAL_FUNCTION(Half)
{
	Int32 value = (Int32)argv[0].unboxed.i32;
	return SmileUnboxedInteger32_From(value >> 1);
}

SMILE_EXTERNAL_FUNCTION(UHalf)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;
	return SmileUnboxedInteger32_From((Int32)(value >> 1));
}

SMILE_EXTERNAL_FUNCTION(Double)
{
	Int32 value = (Int32)argv[0].unboxed.i32;
	return SmileUnboxedInteger32_From(value << 1);
}

//-------------------------------------------------------------------------------------------------
// Bitwise operators

SMILE_EXTERNAL_FUNCTION(BitAnd)
{
	UInt32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt32)argv[0].unboxed.i32;
			x &= (UInt32)argv[1].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = (UInt32)argv[0].unboxed.i32;
			x &= (UInt32)argv[1].unboxed.i32;
			x &= (UInt32)argv[2].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = (UInt32)argv[0].unboxed.i32;
			x &= (UInt32)argv[1].unboxed.i32;
			x &= (UInt32)argv[2].unboxed.i32;
			x &= (UInt32)argv[3].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		default:
			x = (UInt32)argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				x &= (UInt32)argv[i].unboxed.i32;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitOr)
{
	UInt32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt32)argv[0].unboxed.i32;
			x |= (UInt32)argv[1].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = (UInt32)argv[0].unboxed.i32;
			x |= (UInt32)argv[1].unboxed.i32;
			x |= (UInt32)argv[2].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = (UInt32)argv[0].unboxed.i32;
			x |= (UInt32)argv[1].unboxed.i32;
			x |= (UInt32)argv[2].unboxed.i32;
			x |= (UInt32)argv[3].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		default:
			x = (UInt32)argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				x |= (UInt32)argv[i].unboxed.i32;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitXor)
{
	UInt32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt32)argv[0].unboxed.i32;
			x ^= (UInt32)argv[1].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 3:
			x = (UInt32)argv[0].unboxed.i32;
			x ^= (UInt32)argv[1].unboxed.i32;
			x ^= (UInt32)argv[2].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		case 4:
			x = (UInt32)argv[0].unboxed.i32;
			x ^= (UInt32)argv[1].unboxed.i32;
			x ^= (UInt32)argv[2].unboxed.i32;
			x ^= (UInt32)argv[3].unboxed.i32;
			return SmileUnboxedInteger32_From(x);

		default:
			x = (UInt32)argv[0].unboxed.i32;
			for (i = 1; i < argc; i++) {
				x ^= (UInt32)argv[i].unboxed.i32;
			}
			return SmileUnboxedInteger32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitNot)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

	return SmileUnboxedInteger32_From(~value);
}

//-------------------------------------------------------------------------------------------------
// Shift/rotate operators

SMILE_EXTERNAL_FUNCTION(LogicalShiftLeft)
{
	UInt32 x = (UInt32)argv[0].unboxed.i32;
	UInt32 y = (UInt32)argv[1].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)(x << y));
}

SMILE_EXTERNAL_FUNCTION(LogicalShiftRight)
{
	UInt32 x = (UInt32)argv[0].unboxed.i32;
	UInt32 y = (UInt32)argv[1].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)(x >> y));
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftLeft)
{
	Int32 x = argv[0].unboxed.i32;
	Int32 y = argv[1].unboxed.i32;

	return SmileUnboxedInteger32_From(x << y);
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftRight)
{
	Int32 x = argv[0].unboxed.i32;
	Int32 y = argv[1].unboxed.i32;

	return SmileUnboxedInteger32_From(x >> y);
}

SMILE_EXTERNAL_FUNCTION(RotateLeft)
{
	UInt32 x = (UInt32)argv[0].unboxed.i32;
	UInt32 y = (UInt32)argv[1].unboxed.i32;

	return SmileUnboxedInteger32_From(Smile_RotateLeft64(x, y));
}

SMILE_EXTERNAL_FUNCTION(RotateRight)
{
	UInt32 x = (UInt32)argv[0].unboxed.i32;
	UInt32 y = (UInt32)argv[1].unboxed.i32;

	return SmileUnboxedInteger32_From(Smile_RotateRight64(x, y));
}

//-------------------------------------------------------------------------------------------------
// Bit twiddling

Inline UInt32 CountBitsSet(UInt32 value)
{
#if 32 == 64
	value = ((value >>  1) & 0x5555555555555555ULL) + (value & 0x5555555555555555ULL);
	value = ((value >>  2) & 0x3333333333333333ULL) + (value & 0x3333333333333333ULL);
	value = ((value >>  4) & 0x0F0F0F0F0F0F0F0FULL) + (value & 0x0F0F0F0F0F0F0F0FULL);
	value = ((value >>  8) & 0x00FF00FF00FF00FFULL) + (value & 0x00FF00FF00FF00FFULL);
	value = ((value >> 16) & 0x0000FFFF0000FFFFULL) + (value & 0x0000FFFF0000FFFFULL);
	value = ((value >> 32) & 0x00000000FFFFFFFFULL) + (value & 0x00000000FFFFFFFFULL);
#elif 32 == 32
	value = ((value >>  1) & 0x55555555U) + (value & 0x55555555U);
	value = ((value >>  2) & 0x33333333U) + (value & 0x33333333U);
	value = ((value >>  4) & 0x0F0F0F0FU) + (value & 0x0F0F0F0FU);
	value = ((value >>  8) & 0x00FF00FFU) + (value & 0x00FF00FFU);
	value = ((value >> 16) & 0x0000FFFFU) + (value & 0x0000FFFFU);
#elif 32 == 16
	value = ((value >>  1) & 0x5555U) + (value & 0x5555U);
	value = ((value >>  2) & 0x3333U) + (value & 0x3333U);
	value = ((value >>  4) & 0x0F0FU) + (value & 0x0F0FU);
	value = ((value >>  8) & 0x00FFU) + (value & 0x00FFU);
#elif 32 == 8
	value = ((value >>  1) & 0x55U) + (value & 0x55U);
	value = ((value >>  2) & 0x33U) + (value & 0x33U);
	value = ((value >>  4) & 0x0FU) + (value & 0x0FU);
#endif
	return value;
}

Inline UInt32 ComputeReverseBits(UInt32 value)
{
#if 32 == 64
	value = ((value >>  1) & 0x5555555555555555ULL) | ((value & 0x5555555555555555ULL) <<  1);
	value = ((value >>  2) & 0x3333333333333333ULL) | ((value & 0x3333333333333333ULL) <<  2);
	value = ((value >>  4) & 0x0F0F0F0F0F0F0F0FULL) | ((value & 0x0F0F0F0F0F0F0F0FULL) <<  4);
	value = ((value >>  8) & 0x00FF00FF00FF00FFULL) | ((value & 0x00FF00FF00FF00FFULL) <<  8);
	value = ((value >> 16) & 0x0000FFFF0000FFFFULL) | ((value & 0x0000FFFF0000FFFFULL) << 16);
	value = ( value >> 32                         ) | ( value                          << 32);
#elif 32 == 32
	value = ((value >>  1) & 0x55555555) | ((value & 0x55555555) <<  1);
	value = ((value >>  2) & 0x33333333) | ((value & 0x33333333) <<  2);
	value = ((value >>  4) & 0x0F0F0F0F) | ((value & 0x0F0F0F0F) <<  4);
	value = ((value >>  8) & 0x00FF00FF) | ((value & 0x00FF00FF) <<  8);
	value = ( value >> 16              ) | ( value               << 16);
#elif 32 == 16
	value = ((value >>  1) & 0x5555) | ((value & 0x5555) <<  1);
	value = ((value >>  2) & 0x3333) | ((value & 0x3333) <<  2);
	value = ((value >>  4) & 0x0F0F) | ((value & 0x0F0F) <<  4);
	value = ( value >>  8          ) | ( value           <<  8);
#elif 32 == 8
	value = ((value >>  1) & 0x55) | ((value & 0x55) <<  1);
	value = ((value >>  2) & 0x33) | ((value & 0x33) <<  2);
	value = ( value >>  4        ) | ( value         <<  4);
#endif
	return value;
}

Inline UInt32 ComputeCountOfRightZeros(UInt32 value)
{
	UInt32 c = 32;
	value &= ~value + 1;
	if (value != 0) c--;

#if 32 == 64
	if ((value & 0x00000000FFFFFFFF) != 0) c -= 32;
	if ((value & 0x0000FFFF0000FFFF) != 0) c -= 16;
	if ((value & 0x00FF00FF00FF00FF) != 0) c -= 8;
	if ((value & 0x0F0F0F0F0F0F0F0F) != 0) c -= 4;
	if ((value & 0x3333333333333333) != 0) c -= 2;
	if ((value & 0x5555555555555555) != 0) c -= 1;
#elif 32 == 32
	if ((value & 0x0000FFFF) != 0) c -= 16;
	if ((value & 0x00FF00FF) != 0) c -= 8;
	if ((value & 0x0F0F0F0F) != 0) c -= 4;
	if ((value & 0x33333333) != 0) c -= 2;
	if ((value & 0x55555555) != 0) c -= 1;
#elif 32 == 16
	if ((value & 0x00FF) != 0) c -= 8;
	if ((value & 0x0F0F) != 0) c -= 4;
	if ((value & 0x3333) != 0) c -= 2;
	if ((value & 0x5555) != 0) c -= 1;
#elif 32 == 8
	if ((value & 0x0F) != 0) c -= 4;
	if ((value & 0x33) != 0) c -= 2;
	if ((value & 0x55) != 0) c -= 1;
#endif

	return c;
}

SMILE_EXTERNAL_FUNCTION(CountOnes)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)CountBitsSet(value));
}

SMILE_EXTERNAL_FUNCTION(CountZeros)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)CountBitsSet(~value));
}

SMILE_EXTERNAL_FUNCTION(Parity)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

#if 32 >= 64
	value ^= value >> 32;
#endif
#if 32 >= 32
	value ^= value >> 16;
#endif
#if 32 >= 16
	value ^= value >> 8;
#endif
#if 32 >= 8
	value ^= value >> 4;
#endif
	value &= 0xF;
	value = (0x6996 >> value) & 1;

	return SmileUnboxedInteger32_From((Int32)value);
}

SMILE_EXTERNAL_FUNCTION(ReverseBits)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)ComputeReverseBits(value));
}

SMILE_EXTERNAL_FUNCTION(ReverseBytes)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

#if 32 == 64
	value = (UInt32)( ((value >> 56) & 0x00000000000000FFULL)
						| ((value >> 40) & 0x000000000000FF00ULL)
						| ((value >> 24) & 0x0000000000FF0000ULL)
						| ((value >>  8) & 0x00000000FF000000ULL)
						| ((value <<  8) & 0x000000FF00000000ULL)
						| ((value << 24) & 0x0000FF0000000000ULL)
						| ((value << 40) & 0x00FF000000000000ULL)
						| ((value << 56) & 0xFF00000000000000ULL) );
#elif 32 == 32
	value = (UInt32)( ((value >> 24) & 0x000000FFU)
						| ((value >>  8) & 0x0000FF00U)
						| ((value <<  8) & 0x00FF0000U)
						| ((value << 24) & 0xFF000000U) );
#elif 32 == 16
	value = (UInt32)( ((value >>  8) & 0x00FFU)
						| ((value <<  8) & 0xFF00U) );
#endif

	return SmileUnboxedInteger32_From((Int32)value);
}

SMILE_EXTERNAL_FUNCTION(CountRightZeros)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)ComputeCountOfRightZeros(value));
}

SMILE_EXTERNAL_FUNCTION(CountRightOnes)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)ComputeCountOfRightZeros(~value));
}

SMILE_EXTERNAL_FUNCTION(CountLeftZeros)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)ComputeCountOfRightZeros(ComputeReverseBits(value)));
}

SMILE_EXTERNAL_FUNCTION(CountLeftOnes)
{
	UInt32 value = (UInt32)argv[0].unboxed.i32;

	return SmileUnboxedInteger32_From((Int32)ComputeCountOfRightZeros(~ComputeReverseBits(value)));
}

//-------------------------------------------------------------------------------------------------
// Number Theoretics

/// <summary>
/// Calculate the greatest common divisor of two numbers.  This implementation is very similar to
/// the binary GCD described on Wikipedia (https://en.wikipedia.org/wiki/Binary_GCD_algorithm), and
/// runs in O(max(lg a + lg b)^2) time, or at most 4095 iterations for unsigned 64-bit integers.
/// </summary>
static UInt32 CalculateBinaryGcd(UInt32 a, UInt32 b)
{
	UInt shift = 0;

	while (((a | b) & 1) == 0)
		a >>= 1, b >>= 1, shift++;

	while ((a & 1) == 0)
		a >>= 1;

	do {
		while ((b & 1) == 0)
			b >>= 1;

		if (a > b) {
			UInt32 temp = a;
			a = b;
			b = temp;
		}
	} while ((b -= a) != 0);

	return a << shift;
}

SMILE_EXTERNAL_FUNCTION(Gcd)
{
	Int32 value1 = argv[0].unboxed.i32;
	Int32 value2 = argv[1].unboxed.i32;
	UInt32 gcd;
	UInt32 a, b;

	if (value1 <= 0 || value2 <= 0) return SmileUnboxedInteger32_From(0);
	
	a = (UInt32)value1, b = (UInt32)value2;
	gcd = CalculateBinaryGcd(a, b);

	return SmileUnboxedInteger32_From((Int32)gcd);
}

SMILE_EXTERNAL_FUNCTION(IsCoprime)
{
	Int32 value1 = argv[0].unboxed.i32;
	Int32 value2 = argv[1].unboxed.i32;
	UInt32 gcd;
	UInt32 a, b;

	if (value1 <= 0 || value2 <= 0) return SmileUnboxedBool_From(False);

	a = (UInt32)value1, b = (UInt32)value2;
	gcd = CalculateBinaryGcd(a, b);

	return SmileUnboxedBool_From(gcd == 1);
}

SMILE_EXTERNAL_FUNCTION(Lcm)
{
	Int32 value1 = argv[0].unboxed.i32;
	Int32 value2 = argv[1].unboxed.i32;
	UInt32 gcd;
	UInt32 a, b;

	if (value1 <= 0 || value2 <= 0) return SmileUnboxedInteger32_From(0);

	a = (UInt32)value1, b = (UInt32)value2;
	gcd = CalculateBinaryGcd(a, b);

	return SmileUnboxedInteger32_From((Int32)((a / gcd) * b));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_INTEGER32
		&& argv[0].unboxed.i32 == argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_INTEGER32
		|| argv[0].unboxed.i32 != argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i32 < argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i32 > argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i32 <= argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i32 >= argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ULt)
{
	return SmileUnboxedBool_From((UInt32)argv[0].unboxed.i32 < (UInt32)argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(UGt)
{
	return SmileUnboxedBool_From((UInt32)argv[0].unboxed.i32 >(UInt32)argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(ULe)
{
	return SmileUnboxedBool_From((UInt32)argv[0].unboxed.i32 <= (UInt32)argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(UGe)
{
	return SmileUnboxedBool_From((UInt32)argv[0].unboxed.i32 >= (UInt32)argv[1].unboxed.i32);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Int32 x = argv[0].unboxed.i32;
	Int32 y = argv[1].unboxed.i32;

	if (x == y)
		return SmileUnboxedInteger64_From(0);
	else if (x < y)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

SMILE_EXTERNAL_FUNCTION(UCompare)
{
	UInt32 x = (UInt32)argv[0].unboxed.i32;
	UInt32 y = (UInt32)argv[1].unboxed.i32;

	if (x == y)
		return SmileUnboxedInteger64_From(0);
	else if (x < y)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

//-------------------------------------------------------------------------------------------------

enum {
	ZERO_TEST,
	ONE_TEST,
	NONZERO_TEST,
	POS_TEST,
	NONPOS_TEST,
	NEG_TEST,
	NONNEG_TEST,
	ODD_TEST,
	EVEN_TEST,
	MAX_TEST,
	MIN_TEST,
	UMAX_TEST,
	UMIN_TEST,
};

SMILE_EXTERNAL_FUNCTION(ValueTest)
{
	Int32 value = (UInt32)argv[0].unboxed.i32;

	switch ((PtrInt)param) {
		case ZERO_TEST:
			return SmileUnboxedBool_From(value == 0);
		case ONE_TEST:
			return SmileUnboxedBool_From(value == 1);
		case NONZERO_TEST:
			return SmileUnboxedBool_From(value != 0);
		case POS_TEST:
			return SmileUnboxedBool_From(value > 0);
		case NONPOS_TEST:
			return SmileUnboxedBool_From(value <= 0);
		case NEG_TEST:
			return SmileUnboxedBool_From(value < 0);
		case NONNEG_TEST:
			return SmileUnboxedBool_From(value >= 0);
		case ODD_TEST:
			return SmileUnboxedBool_From((value & 1) != 0);
		case EVEN_TEST:
			return SmileUnboxedBool_From((value & 1) == 0);
		case MAX_TEST:
			return SmileUnboxedBool_From(value == Int32Max);
		case MIN_TEST:
			return SmileUnboxedBool_From(value == Int32Min);
		case UMAX_TEST:
			return SmileUnboxedBool_From((UInt32)value == UInt32Max);
		case UMIN_TEST:
			return SmileUnboxedBool_From(value == 0);
		default:
			return SmileArg_From(NullObject);
	}
}

//-------------------------------------------------------------------------------------------------

void SmileInteger32_Setup(SmileUserObject base)
{
	SmileUnboxedInteger32_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("int64", ToInt64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("int32", ToInt32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("int16", ToInt16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("byte", ToByte, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);

	SetupFunction("sign-extend-64", SignExtend64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("sign-extend-32", SignExtend32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("sign-extend-16", SignExtend16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("sign-extend-8", SignExtend8, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);

	SetupFunction("zero-extend-64", ZeroExtend64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("zero-extend-32", ZeroExtend32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("zero-extend-16", ZeroExtend16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("zero-extend-8", ZeroExtend8, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);

	SetupFunction("float32", ToFloat32, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("float64", ToFloat64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("float", ToFloat64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real32", ToReal32, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real64", ToReal64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real", ToReal64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("char", ToChar, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("uni", ToUni, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);

	SetupFunction("parse", Parse, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 3, 0, NULL);

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupSynonym("+", "+~");
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupSynonym("-", "-~");
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("*~", UStar, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("+*", FMA, NULL, "augend multiplier multiplicand", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer32Checks);
	SetupFunction("+*~", UFMA, NULL, "augend multiplier multiplicand", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer32Checks);

	SetupFunction("/", Slash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("/!", Slash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("/~", USlash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("/!~", USlash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("div", Div, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("div!", Div, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("div~", USlash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("div!~", USlash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("mod", Mod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("mod!", Mod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("mod~", UMod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("mod!~", UMod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("rem", Rem, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("rem!", Rem, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("rem~", UMod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("rem!~", UMod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer32Checks);
	SetupFunction("clip~", UClip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer32Checks);
	SetupFunction("ramp", Ramp, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("heaviside", Heaviside, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("rect", RectTri, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("tri", RectTri, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("min~", UMin, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("max~", UMax, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);

	SetupFunction("^", Power, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("sqrt", Sqrt, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("sqrt!", Sqrt, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("sqr", Sqr, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("cube", Cube, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("pow2?", Pow2Q, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("next-pow2", NextPow2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("int-lg", IntLg, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("int-lg!", IntLg, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("half", Half, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("half~", UHalf, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("double", Double, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupSynonym("double", "dbl");

	SetupFunction("band", BitAnd, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("bor", BitOr, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("bxor", BitXor, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("~", BitNot, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);

	SetupFunction("<<<", LogicalShiftLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction(">>>", LogicalShiftRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("<<", ArithmeticShiftLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction(">>", ArithmeticShiftRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("<<+", RotateLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("+>>", RotateRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);

	SetupFunction("count-ones", CountOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("count-zeros", CountZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("parity", Parity, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("reverse-bits", ReverseBits, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("reverse-bytes", ReverseBytes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("count-right-zeros", CountRightZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("count-right-ones", CountRightOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("count-left-zeros", CountLeftZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("count-left-ones", CountLeftOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);

	SetupFunction("gcd", Gcd, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("lcm", Lcm, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("coprime?", IsCoprime, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);

	SetupFunction("odd?", ValueTest, (void *)ODD_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("even?", ValueTest, (void *)EVEN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("zero?", ValueTest, (void *)ZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("one?", ValueTest, (void *)ONE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("nonzero?", ValueTest, (void *)NONZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("positive?", ValueTest, (void *)POS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupSynonym("positive?", "pos?");
	SetupFunction("nonpositive?", ValueTest, (void *)NONPOS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupSynonym("nonpositive?", "nonpos?");
	SetupFunction("negative?", ValueTest, (void *)NEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupSynonym("negative?", "neg?");
	SetupFunction("nonnegative?", ValueTest, (void *)NONNEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupSynonym("nonnegative?", "nonneg?");
	SetupFunction("max?", ValueTest, (void *)MAX_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("min?", ValueTest, (void *)MIN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("max~?", ValueTest, (void *)UMAX_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("min~?", ValueTest, (void *)UMIN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("div?", IsDiv, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("div~?", IsUDiv, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32ComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32ComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("<~", ULt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction(">~", UGt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("<=~", ULe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction(">=~", UGe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupSynonym("compare", "cmp");
	SetupFunction("compare~", UCompare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupSynonym("compare~", "cmp~");

	SetupFunction("range-to", RangeTo, NULL, "start end", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);

#if 32 == 8
	SetupData("max-value", Smile_KnownObjects.Bytes[255]);
	SetupData("min-value", Smile_KnownObjects.ZeroByte);
	SetupData("max-value~", Smile_KnownObjects.Bytes[255]);
	SetupData("min-value~", Smile_KnownObjects.ZeroByte);
#else
	SetupData("max-value", SmileInteger32_CreateInternal(Int32Max));
	SetupData("min-value", SmileInteger32_CreateInternal(Int32Min));
	SetupData("max-value~", SmileInteger32_CreateInternal(UInt32Max));
	SetupData("min-value~", Smile_KnownObjects.ZeroInt32);
#endif
}
