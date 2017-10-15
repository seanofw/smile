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
#include <smile/smiletypes/range/smilebyterange.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
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

typedef struct MathInfoStruct {
	Bool isLoud;
} *MathInfo;

static struct MathInfoStruct _loudMath[] = { True };
static struct MathInfoStruct _quietMath[] = { False };

STATIC_STRING(_divideByZero, "Divide by zero error");
STATIC_STRING(_negativeLog, "Logarithm of negative or zero value");
STATIC_STRING(_negativeSqrt, "Square root of negative number");

STATIC_STRING(_invalidTypeError, "All arguments to 'Byte.%s' must be of type 'Byte'.");

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Byte'");
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
		return SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.i8);

	return SmileUnboxedByte_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	Byte numericBase;
	STATIC_STRING(byte, "Byte");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BYTE) {
		if (argc == 2) {
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_BYTE)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)argv[1].unboxed.i8;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;

		return SmileArg_From((SmileObject)String_CreateFromInteger(argv[0].unboxed.i8, (Int)numericBase, False));
	}

	return SmileArg_From((SmileObject)byte);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileByte obj = (SmileByte)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_BYTE)
		return SmileUnboxedInteger64_From((UInt32)obj->value);

	return SmileUnboxedInteger64_From((UInt32)((PtrInt)obj ^ Smile_HashOracle));
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(SignExtend64)
{
	return SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ZeroExtend64)
{
	return SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return SmileUnboxedInteger32_From((Int32)(UInt32)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(SignExtend32)
{
	return SmileUnboxedInteger32_From((Int32)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ZeroExtend32)
{
	return SmileUnboxedInteger32_From((Int32)(UInt32)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return SmileUnboxedInteger16_From((Int16)(UInt16)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(SignExtend16)
{
	return SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ZeroExtend16)
{
	return SmileUnboxedInteger16_From((Int16)(UInt16)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(SignExtend8)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(ZeroExtend8)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(ToReal32)
{
	return SmileUnboxedReal32_From(Real32_FromInt64((Int64)argv[0].unboxed.i8));
}

SMILE_EXTERNAL_FUNCTION(ToReal64)
{
	return SmileUnboxedReal64_From(Real64_FromInt64((Int64)argv[0].unboxed.i8));
}

SMILE_EXTERNAL_FUNCTION(ToFloat32)
{
	return SmileUnboxedFloat32_From((Float32)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ToFloat64)
{
	return SmileUnboxedFloat64_From((Float64)argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(RangeTo)
{
	Byte start, end, step;

	start = argv[0].unboxed.i8;
	end = argv[1].unboxed.i8;
	step = end >= start ? +1 : -1;

	return SmileArg_From((SmileObject)SmileByteRange_Create(start, end, step));
}

SMILE_EXTERNAL_FUNCTION(ToChar)
{
	return SmileUnboxedChar_From(argv[0].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ToUni)
{
	return SmileUnboxedUni_From(argv[0].unboxed.i8);
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
			if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING && SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_BYTE) {
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
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_STRING || SMILE_KIND(argv[2].obj) != SMILE_KIND_UNBOXED_BYTE)
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
	Byte x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

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

SMILE_EXTERNAL_FUNCTION(UStar)
{
	Byte x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (Byte)argv[0].unboxed.i8;
			x *= (Byte)argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (Byte)argv[0].unboxed.i8;
			x *= (Byte)argv[1].unboxed.i8;
			x *= (Byte)argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (Byte)argv[0].unboxed.i8;
			x *= (Byte)argv[1].unboxed.i8;
			x *= (Byte)argv[2].unboxed.i8;
			x *= (Byte)argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = (Byte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x *= (Byte)argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(FMA)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;
	Byte z = argv[2].unboxed.i8;

	return SmileUnboxedByte_From(x + y * z);
}

SMILE_EXTERNAL_FUNCTION(UFMA)
{
	Byte x = (Byte)argv[0].unboxed.i8;
	Byte y = (Byte)argv[1].unboxed.i8;
	Byte z = (Byte)argv[2].unboxed.i8;

	return SmileUnboxedByte_From((Byte)(x + y * z));
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
Inline Byte CDiv(Byte dividend, Byte divisor)
{
	if (dividend < 0) {
		if (divisor < 0)
			return (Byte)((Byte)-dividend / (Byte)-divisor);
		else
			return -(Byte)((Byte)-dividend / (Byte)divisor);
	}
	else if (divisor < 0)
		return -(Byte)((Byte)dividend / (Byte)-divisor);
	else
		return (Byte)((Byte)dividend / (Byte)divisor);
}

/// <summary>
/// Perform division like mathematicians expect, which rounds toward negative infinity (always).
/// </summary>
Inline Byte MathematiciansDiv(Byte dividend, Byte divisor)
{
	if (dividend < 0) {
		if (divisor < 0) {
			return (Byte)((Byte)-dividend / (Byte)-divisor);
		}
		else {
			Byte positiveQuotient = (Byte)-dividend / (Byte)divisor;
			Byte positiveRemainder = (Byte)-dividend % (Byte)divisor;
			return positiveRemainder == 0 ? -(Byte)positiveQuotient : -(Byte)positiveQuotient - 1;
		}
	}
	else if (divisor < 0) {
		Byte positiveQuotient = (Byte)dividend / (Byte)-divisor;
		Byte positiveRemainder = (Byte)dividend % (Byte)-divisor;
		return positiveRemainder == 0 ? -(Byte)positiveQuotient : -(Byte)positiveQuotient - 1;
	}
	else {
		return dividend / divisor;
	}
}

SMILE_EXTERNAL_FUNCTION(Slash)
{
	Byte x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedByte_From(x);

		case 3:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			if ((y = argv[3].unboxed.i8) == 0)
				return DivideByZero(param);
			x = MathematiciansDiv(x, y);
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i8) == 0)
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
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (Byte)argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (Byte)argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			if ((y = (Byte)argv[3].unboxed.i8) == 0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedByte_From(x);

		default:
			x = (Byte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = (Byte)argv[i].unboxed.i8) == 0)
					return DivideByZero(param);
				x /= y;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Div)
{
	Byte x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedByte_From(x);

		case 3:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedByte_From(x);

		case 4:
			x = argv[0].unboxed.i8;
			if ((y = argv[1].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[2].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			if ((y = argv[3].unboxed.i8) == 0)
				return DivideByZero(param);
			x = CDiv(x, y);
			return SmileUnboxedByte_From(x);

		default:
			x = argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.i8) == 0)
					return DivideByZero(param);
				x = CDiv(x, y);
			}
			return SmileUnboxedByte_From(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Byte MathematiciansModulus(Byte x, Byte y)
{
	Byte rem;

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
Inline Byte MathematiciansRemainder(Byte x, Byte y)
{
	Byte rem;

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
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedByte_From(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(UMod)
{
	Byte x = (Byte)argv[0].unboxed.i8;
	Byte y = (Byte)argv[1].unboxed.i8;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedByte_From(x % y);
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	if (y == 0)
		return DivideByZero(param);

	return SmileUnboxedByte_From(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	Byte value = argv[0].unboxed.i8;

	return value == 0 ? SmileUnboxedByte_From(0) : SmileUnboxedByte_From(1);
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
#if 8 > 8
	Byte value = argv[0].unboxed.i8;

	return value < 0 ? SmileUnboxedByte_From(-value) : argv[0];
#else
	return argv[0];
#endif
}

SMILE_EXTERNAL_FUNCTION(Clip)
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

SMILE_EXTERNAL_FUNCTION(UClip)
{
	Byte value = (Byte)argv[0].unboxed.i8;
	Byte min = (Byte)argv[1].unboxed.i8;
	Byte max = (Byte)argv[2].unboxed.i8;

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

SMILE_EXTERNAL_FUNCTION(UMin)
{
	Byte x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) < x) x = y;
			if ((y = (Byte)argv[2].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) < x) x = y;
			if ((y = (Byte)argv[2].unboxed.i8) < x) x = y;
			if ((y = (Byte)argv[3].unboxed.i8) < x) x = y;
			return SmileUnboxedByte_From(x);

		default:
			x = (Byte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = (Byte)argv[i].unboxed.i8) < x) x = y;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Max)
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

SMILE_EXTERNAL_FUNCTION(UMax)
{
	Byte x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) > x) x = y;
			if ((y = (Byte)argv[2].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (Byte)argv[0].unboxed.i8;
			if ((y = (Byte)argv[1].unboxed.i8) > x) x = y;
			if ((y = (Byte)argv[2].unboxed.i8) > x) x = y;
			if ((y = (Byte)argv[3].unboxed.i8) > x) x = y;
			return SmileUnboxedByte_From(x);

		default:
			x = (Byte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				if ((y = (Byte)argv[i].unboxed.i8) > x) x = y;
			}
			return SmileUnboxedByte_From(x);
	}
}

Inline Byte IntPower(Byte value, Byte exponent)
{
	if (exponent < 0) return 0;

	Byte result = 1;

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
	Byte x;
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

#if 8 == 64
	bit =
		(value >= 0x100000000UL) ? (1ULL << 62)
		: (value >= 0x10000UL) ? (1ULL << 30)
		: (1ULL << 14);
#elif 8 == 32
	bit = (value >= 0x10000U) ? (1U << 30) : (1U << 14);
#elif 8 == 16
	bit = (1U << 14);
#elif 8 == 8
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
	Byte value = argv[0].unboxed.i8;

	if (value < 0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);
		return SmileUnboxedByte_From(0);
	}

	return SmileUnboxedByte_From(IntSqrt(value));
}

SMILE_EXTERNAL_FUNCTION(Sqr)
{
	Byte value = argv[0].unboxed.i8;
	return SmileUnboxedByte_From(value * value);
}

SMILE_EXTERNAL_FUNCTION(Cube)
{
	Byte value = argv[0].unboxed.i8;
	return SmileUnboxedByte_From(value * value * value);
}

SMILE_EXTERNAL_FUNCTION(Pow2Q)
{
	Byte value = argv[0].unboxed.i8;

	return SmileUnboxedBool_From(value > 0 && (value & (value - 1)) == 0);
}

SMILE_EXTERNAL_FUNCTION(NextPow2)
{
	Byte value = (Byte)argv[0].unboxed.i8;
	Byte uvalue = (Byte)value;

	if (value < 0) return SmileUnboxedByte_From(0);
	if (value == 0) return SmileUnboxedByte_From(1);

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
#if 8 >= 16
	uvalue |= uvalue >> 8;
#endif
#if 8 >= 32
	uvalue |= uvalue >> 16;
#endif
#if 8 >= 64
	uvalue |= uvalue >> 32;
#endif
	uvalue++;

	return SmileUnboxedByte_From(uvalue);
}

SMILE_EXTERNAL_FUNCTION(IntLg)
{
	Byte value = (Byte)argv[0].unboxed.i8;
	Byte uvalue = (Byte)value;
	Byte log;

	if (value <= 0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeLog);
		return SmileUnboxedByte_From(0);
	}

	log = 0;
#if 8 >= 64
	if ((uvalue & 0xFFFFFFFF00000000) != 0) uvalue >>= 32, log += 32;
#endif
#if 8 >= 32
	if ((uvalue & 0x00000000FFFF0000) != 0) uvalue >>= 16, log += 16;
#endif
#if 8 >= 16
	if ((uvalue & 0x000000000000FF00) != 0) uvalue >>= 8, log += 8;
#endif
	if ((uvalue & 0x00000000000000F0) != 0) uvalue >>= 4, log += 4;
	if ((uvalue & 0x000000000000000C) != 0) uvalue >>= 2, log += 2;
	if ((uvalue & 0x0000000000000002) != 0) uvalue >>= 1, log += 1;

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
			x = (Byte)argv[0].unboxed.i8;
			x &= (Byte)argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (Byte)argv[0].unboxed.i8;
			x &= (Byte)argv[1].unboxed.i8;
			x &= (Byte)argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (Byte)argv[0].unboxed.i8;
			x &= (Byte)argv[1].unboxed.i8;
			x &= (Byte)argv[2].unboxed.i8;
			x &= (Byte)argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = (Byte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x &= (Byte)argv[i].unboxed.i8;
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
			x = (Byte)argv[0].unboxed.i8;
			x |= (Byte)argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (Byte)argv[0].unboxed.i8;
			x |= (Byte)argv[1].unboxed.i8;
			x |= (Byte)argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (Byte)argv[0].unboxed.i8;
			x |= (Byte)argv[1].unboxed.i8;
			x |= (Byte)argv[2].unboxed.i8;
			x |= (Byte)argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = (Byte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x |= (Byte)argv[i].unboxed.i8;
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
			x = (Byte)argv[0].unboxed.i8;
			x ^= (Byte)argv[1].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 3:
			x = (Byte)argv[0].unboxed.i8;
			x ^= (Byte)argv[1].unboxed.i8;
			x ^= (Byte)argv[2].unboxed.i8;
			return SmileUnboxedByte_From(x);

		case 4:
			x = (Byte)argv[0].unboxed.i8;
			x ^= (Byte)argv[1].unboxed.i8;
			x ^= (Byte)argv[2].unboxed.i8;
			x ^= (Byte)argv[3].unboxed.i8;
			return SmileUnboxedByte_From(x);

		default:
			x = (Byte)argv[0].unboxed.i8;
			for (i = 1; i < argc; i++) {
				x ^= (Byte)argv[i].unboxed.i8;
			}
			return SmileUnboxedByte_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(BitNot)
{
	Byte value = (Byte)argv[0].unboxed.i8;

	return SmileUnboxedByte_From(~value);
}

//-------------------------------------------------------------------------------------------------
// Shift/rotate operators

SMILE_EXTERNAL_FUNCTION(LogicalShiftLeft)
{
	Byte x = (Byte)argv[0].unboxed.i8;
	Byte y = (Byte)argv[1].unboxed.i8;

	return SmileUnboxedByte_From((Byte)(x << y));
}

SMILE_EXTERNAL_FUNCTION(LogicalShiftRight)
{
	Byte x = (Byte)argv[0].unboxed.i8;
	Byte y = (Byte)argv[1].unboxed.i8;

	return SmileUnboxedByte_From((Byte)(x >> y));
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftLeft)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	return SmileUnboxedByte_From(x << y);
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftRight)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	return SmileUnboxedByte_From(x >> y);
}

SMILE_EXTERNAL_FUNCTION(RotateLeft)
{
	Byte x = (Byte)argv[0].unboxed.i8;
	Byte y = (Byte)argv[1].unboxed.i8;

	return SmileUnboxedByte_From(Smile_RotateLeft64(x, y));
}

SMILE_EXTERNAL_FUNCTION(RotateRight)
{
	Byte x = (Byte)argv[0].unboxed.i8;
	Byte y = (Byte)argv[1].unboxed.i8;

	return SmileUnboxedByte_From(Smile_RotateRight64(x, y));
}

//-------------------------------------------------------------------------------------------------
// Bit twiddling

Inline Byte CountBitsSet(Byte value)
{
#if 8 == 64
	value = ((value >>  1) & 0x5555555555555555ULL) + (value & 0x5555555555555555ULL);
	value = ((value >>  2) & 0x3333333333333333ULL) + (value & 0x3333333333333333ULL);
	value = ((value >>  4) & 0x0F0F0F0F0F0F0F0FULL) + (value & 0x0F0F0F0F0F0F0F0FULL);
	value = ((value >>  8) & 0x00FF00FF00FF00FFULL) + (value & 0x00FF00FF00FF00FFULL);
	value = ((value >> 16) & 0x0000FFFF0000FFFFULL) + (value & 0x0000FFFF0000FFFFULL);
	value = ((value >> 32) & 0x00000000FFFFFFFFULL) + (value & 0x00000000FFFFFFFFULL);
#elif 8 == 32
	value = ((value >>  1) & 0x55555555U) + (value & 0x55555555U);
	value = ((value >>  2) & 0x33333333U) + (value & 0x33333333U);
	value = ((value >>  4) & 0x0F0F0F0FU) + (value & 0x0F0F0F0FU);
	value = ((value >>  8) & 0x00FF00FFU) + (value & 0x00FF00FFU);
	value = ((value >> 16) & 0x0000FFFFU) + (value & 0x0000FFFFU);
#elif 8 == 16
	value = ((value >>  1) & 0x5555U) + (value & 0x5555U);
	value = ((value >>  2) & 0x3333U) + (value & 0x3333U);
	value = ((value >>  4) & 0x0F0FU) + (value & 0x0F0FU);
	value = ((value >>  8) & 0x00FFU) + (value & 0x00FFU);
#elif 8 == 8
	value = ((value >>  1) & 0x55U) + (value & 0x55U);
	value = ((value >>  2) & 0x33U) + (value & 0x33U);
	value = ((value >>  4) & 0x0FU) + (value & 0x0FU);
#endif
	return value;
}

Inline Byte ComputeReverseBits(Byte value)
{
#if 8 == 64
	value = ((value >>  1) & 0x5555555555555555ULL) | ((value & 0x5555555555555555ULL) <<  1);
	value = ((value >>  2) & 0x3333333333333333ULL) | ((value & 0x3333333333333333ULL) <<  2);
	value = ((value >>  4) & 0x0F0F0F0F0F0F0F0FULL) | ((value & 0x0F0F0F0F0F0F0F0FULL) <<  4);
	value = ((value >>  8) & 0x00FF00FF00FF00FFULL) | ((value & 0x00FF00FF00FF00FFULL) <<  8);
	value = ((value >> 16) & 0x0000FFFF0000FFFFULL) | ((value & 0x0000FFFF0000FFFFULL) << 16);
	value = ( value >> 32                         ) | ( value                          << 32);
#elif 8 == 32
	value = ((value >>  1) & 0x55555555) | ((value & 0x55555555) <<  1);
	value = ((value >>  2) & 0x33333333) | ((value & 0x33333333) <<  2);
	value = ((value >>  4) & 0x0F0F0F0F) | ((value & 0x0F0F0F0F) <<  4);
	value = ((value >>  8) & 0x00FF00FF) | ((value & 0x00FF00FF) <<  8);
	value = ( value >> 16              ) | ( value               << 16);
#elif 8 == 16
	value = ((value >>  1) & 0x5555) | ((value & 0x5555) <<  1);
	value = ((value >>  2) & 0x3333) | ((value & 0x3333) <<  2);
	value = ((value >>  4) & 0x0F0F) | ((value & 0x0F0F) <<  4);
	value = ( value >>  8          ) | ( value           <<  8);
#elif 8 == 8
	value = ((value >>  1) & 0x55) | ((value & 0x55) <<  1);
	value = ((value >>  2) & 0x33) | ((value & 0x33) <<  2);
	value = ( value >>  4        ) | ( value         <<  4);
#endif
	return value;
}

Inline Byte ComputeCountOfRightZeros(Byte value)
{
	Byte c = 8;
	value &= ~value + 1;
	if (value != 0) c--;

#if 8 == 64
	if ((value & 0x00000000FFFFFFFF) != 0) c -= 32;
	if ((value & 0x0000FFFF0000FFFF) != 0) c -= 16;
	if ((value & 0x00FF00FF00FF00FF) != 0) c -= 8;
	if ((value & 0x0F0F0F0F0F0F0F0F) != 0) c -= 4;
	if ((value & 0x3333333333333333) != 0) c -= 2;
	if ((value & 0x5555555555555555) != 0) c -= 1;
#elif 8 == 32
	if ((value & 0x0000FFFF) != 0) c -= 16;
	if ((value & 0x00FF00FF) != 0) c -= 8;
	if ((value & 0x0F0F0F0F) != 0) c -= 4;
	if ((value & 0x33333333) != 0) c -= 2;
	if ((value & 0x55555555) != 0) c -= 1;
#elif 8 == 16
	if ((value & 0x00FF) != 0) c -= 8;
	if ((value & 0x0F0F) != 0) c -= 4;
	if ((value & 0x3333) != 0) c -= 2;
	if ((value & 0x5555) != 0) c -= 1;
#elif 8 == 8
	if ((value & 0x0F) != 0) c -= 4;
	if ((value & 0x33) != 0) c -= 2;
	if ((value & 0x55) != 0) c -= 1;
#endif

	return c;
}

SMILE_EXTERNAL_FUNCTION(CountOnes)
{
	Byte value = (Byte)argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)CountBitsSet(value));
}

SMILE_EXTERNAL_FUNCTION(CountZeros)
{
	Byte value = (Byte)argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)CountBitsSet(~value));
}

SMILE_EXTERNAL_FUNCTION(Parity)
{
	Byte value = (Byte)argv[0].unboxed.i8;

#if 8 >= 64
	value ^= value >> 32;
#endif
#if 8 >= 32
	value ^= value >> 16;
#endif
#if 8 >= 16
	value ^= value >> 8;
#endif
#if 8 >= 8
	value ^= value >> 4;
#endif
	value &= 0xF;
	value = (0x6996 >> value) & 1;

	return SmileUnboxedByte_From((Byte)value);
}

SMILE_EXTERNAL_FUNCTION(ReverseBits)
{
	Byte value = (Byte)argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)ComputeReverseBits(value));
}

SMILE_EXTERNAL_FUNCTION(ReverseBytes)
{
	Byte value = (Byte)argv[0].unboxed.i8;

#if 8 == 64
	value = (Byte)( ((value >> 56) & 0x00000000000000FFULL)
						| ((value >> 40) & 0x000000000000FF00ULL)
						| ((value >> 24) & 0x0000000000FF0000ULL)
						| ((value >>  8) & 0x00000000FF000000ULL)
						| ((value <<  8) & 0x000000FF00000000ULL)
						| ((value << 24) & 0x0000FF0000000000ULL)
						| ((value << 40) & 0x00FF000000000000ULL)
						| ((value << 56) & 0xFF00000000000000ULL) );
#elif 8 == 32
	value = (Byte)( ((value >> 24) & 0x000000FFU)
						| ((value >>  8) & 0x0000FF00U)
						| ((value <<  8) & 0x00FF0000U)
						| ((value << 24) & 0xFF000000U) );
#elif 8 == 16
	value = (Byte)( ((value >>  8) & 0x00FFU)
						| ((value <<  8) & 0xFF00U) );
#endif

	return SmileUnboxedByte_From((Byte)value);
}

SMILE_EXTERNAL_FUNCTION(CountRightZeros)
{
	Byte value = (Byte)argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)ComputeCountOfRightZeros(value));
}

SMILE_EXTERNAL_FUNCTION(CountRightOnes)
{
	Byte value = (Byte)argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)ComputeCountOfRightZeros(~value));
}

SMILE_EXTERNAL_FUNCTION(CountLeftZeros)
{
	Byte value = (Byte)argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)ComputeCountOfRightZeros(ComputeReverseBits(value)));
}

SMILE_EXTERNAL_FUNCTION(CountLeftOnes)
{
	Byte value = (Byte)argv[0].unboxed.i8;

	return SmileUnboxedByte_From((Byte)ComputeCountOfRightZeros(~ComputeReverseBits(value)));
}

//-------------------------------------------------------------------------------------------------
// Number Theoretics

/// <summary>
/// Calculate the greatest common divisor of two numbers.  This implementation is very similar to
/// the binary GCD described on Wikipedia (https://en.wikipedia.org/wiki/Binary_GCD_algorithm), and
/// runs in O(max(lg a + lg b)^2) time, or at most 4095 iterations for unsigned 64-bit integers.
/// </summary>
static Byte CalculateBinaryGcd(Byte a, Byte b)
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
			Byte temp = a;
			a = b;
			b = temp;
		}
	} while ((b -= a) != 0);

	return a << shift;
}

SMILE_EXTERNAL_FUNCTION(Gcd)
{
	Byte value1 = argv[0].unboxed.i8;
	Byte value2 = argv[1].unboxed.i8;
	Byte gcd;
	Byte a, b;

	if (value1 <= 0 || value2 <= 0) return SmileUnboxedByte_From(0);
	
	a = (Byte)value1, b = (Byte)value2;
	gcd = CalculateBinaryGcd(a, b);

	return SmileUnboxedByte_From((Byte)gcd);
}

SMILE_EXTERNAL_FUNCTION(IsCoprime)
{
	Byte value1 = argv[0].unboxed.i8;
	Byte value2 = argv[1].unboxed.i8;
	Byte gcd;
	Byte a, b;

	if (value1 <= 0 || value2 <= 0) return SmileUnboxedBool_From(False);

	a = (Byte)value1, b = (Byte)value2;
	gcd = CalculateBinaryGcd(a, b);

	return SmileUnboxedBool_From(gcd == 1);
}

SMILE_EXTERNAL_FUNCTION(Lcm)
{
	Byte value1 = argv[0].unboxed.i8;
	Byte value2 = argv[1].unboxed.i8;
	Byte gcd;
	Byte a, b;

	if (value1 <= 0 || value2 <= 0) return SmileUnboxedByte_From(0);

	a = (Byte)value1, b = (Byte)value2;
	gcd = CalculateBinaryGcd(a, b);

	return SmileUnboxedByte_From((Byte)((a / gcd) * b));
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
	return SmileUnboxedBool_From(argv[0].unboxed.i8 < argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i8 > argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i8 <= argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(argv[0].unboxed.i8 >= argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ULt)
{
	return SmileUnboxedBool_From((Byte)argv[0].unboxed.i8 < (Byte)argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(UGt)
{
	return SmileUnboxedBool_From((Byte)argv[0].unboxed.i8 >(Byte)argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(ULe)
{
	return SmileUnboxedBool_From((Byte)argv[0].unboxed.i8 <= (Byte)argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(UGe)
{
	return SmileUnboxedBool_From((Byte)argv[0].unboxed.i8 >= (Byte)argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Byte x = argv[0].unboxed.i8;
	Byte y = argv[1].unboxed.i8;

	if (x == y)
		return SmileUnboxedInteger64_From(0);
	else if (x < y)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

SMILE_EXTERNAL_FUNCTION(UCompare)
{
	Byte x = (Byte)argv[0].unboxed.i8;
	Byte y = (Byte)argv[1].unboxed.i8;

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
	Byte value = (Byte)argv[0].unboxed.i8;

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
			return SmileUnboxedBool_From(value == ByteMax);
		case MIN_TEST:
			return SmileUnboxedBool_From(value == ByteMin);
		case UMAX_TEST:
			return SmileUnboxedBool_From((Byte)value == ByteMax);
		case UMIN_TEST:
			return SmileUnboxedBool_From(value == 0);
		default:
			return SmileArg_From(NullObject);
	}
}

//-------------------------------------------------------------------------------------------------

void SmileByte_Setup(SmileUserObject base)
{
	SmileUnboxedByte_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("int64", ToInt64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("int32", ToInt32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("int16", ToInt16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("byte", ToByte, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("sign-extend-64", SignExtend64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("sign-extend-32", SignExtend32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("sign-extend-16", SignExtend16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("sign-extend-8", SignExtend8, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("zero-extend-64", ZeroExtend64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("zero-extend-32", ZeroExtend32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("zero-extend-16", ZeroExtend16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("zero-extend-8", ZeroExtend8, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("float32", ToFloat32, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("float64", ToFloat64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("float", ToFloat64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real32", ToReal32, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real64", ToReal64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real", ToReal64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("char", ToChar, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("uni", ToUni, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("parse", Parse, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 3, 0, NULL);

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupSynonym("+", "+~");
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupSynonym("-", "-~");
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("*~", UStar, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("+*", FMA, NULL, "augend multiplier multiplicand", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _byteChecks);
	SetupFunction("+*~", UFMA, NULL, "augend multiplier multiplicand", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _byteChecks);

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
	SetupFunction("sqr", Sqr, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("cube", Cube, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
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
	SetupFunction("count-left-zeros", CountLeftZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("count-left-ones", CountLeftOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

	SetupFunction("gcd", Gcd, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("lcm", Lcm, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("coprime?", IsCoprime, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);

	SetupFunction("odd?", ValueTest, (void *)ODD_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("even?", ValueTest, (void *)EVEN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("zero?", ValueTest, (void *)ZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("one?", ValueTest, (void *)ONE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("nonzero?", ValueTest, (void *)NONZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("positive?", ValueTest, (void *)POS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupSynonym("positive?", "pos?");
	SetupFunction("nonpositive?", ValueTest, (void *)NONPOS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupSynonym("nonpositive?", "nonpos?");
	SetupFunction("negative?", ValueTest, (void *)NEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupSynonym("negative?", "neg?");
	SetupFunction("nonnegative?", ValueTest, (void *)NONNEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupSynonym("nonnegative?", "nonneg?");
	SetupFunction("max?", ValueTest, (void *)MAX_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("min?", ValueTest, (void *)MIN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("max~?", ValueTest, (void *)UMAX_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("min~?", ValueTest, (void *)UMIN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

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

	SetupFunction("range-to", RangeTo, NULL, "start end", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);

#if 8 == 8
	SetupData("max-value", Smile_KnownObjects.Bytes[255]);
	SetupData("min-value", Smile_KnownObjects.ZeroByte);
	SetupData("max-value~", Smile_KnownObjects.Bytes[255]);
	SetupData("min-value~", Smile_KnownObjects.ZeroByte);
#else
	SetupData("max-value", SmileByte_CreateInternal(ByteMax));
	SetupData("min-value", SmileByte_CreateInternal(ByteMin));
	SetupData("max-value~", SmileByte_CreateInternal(ByteMax));
	SetupData("min-value~", Smile_KnownObjects.ZeroByte);
#endif
}
