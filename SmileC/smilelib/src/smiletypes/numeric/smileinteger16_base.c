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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>

static Byte _integer16Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
};

static Byte _integer16ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16,
	0, 0,
};

static Byte _parseChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
};

STATIC_STRING(_divideByZero, "Divide by zero error");
STATIC_STRING(_negativeLog, "Logarithm of negative or zero value");
STATIC_STRING(_negativeSqrt, "Square root of negative number");

STATIC_STRING(_invalidTypeError, "All arguments to 'Integer16.%s' must be of type 'Integer16'");

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Integer64'");
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
STATIC_STRING(_parseArguments, "Illegal arguments to 'parse' function");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

static SmileObject ToBool(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_INTEGER16)
		return ((SmileInteger16)argv[0])->value ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject ToInt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_INTEGER16)
		return (SmileObject)SmileInteger64_Create(((SmileInteger16)argv[0])->value);

	return (SmileObject)Smile_KnownObjects.ZeroInt64;
}

STATIC_STRING(_Integer16, "Integer16");

static SmileObject ToString(Int argc, SmileObject *argv, void *param)
{
	Int64 numericBase;

	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_INTEGER16) {
		if (argc == 2) {
			if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER64)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)((SmileInteger64)argv[1])->value;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;

		return (SmileObject)SmileString_Create(String_CreateFromInteger((Int64)((SmileInteger16)argv[0])->value, (Int)numericBase, False));
	}

	return (SmileObject)SmileString_Create(_Integer16);
}

static SmileObject Hash(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_INTEGER16)
		return (SmileObject)SmileInteger64_Create(((SmileInteger16)argv[0])->value);

	return (SmileObject)SmileInteger64_Create(((PtrInt)argv[0]) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

static SmileObject ToInt64(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create(((SmileInteger16)argv[0])->value);
}

static SmileObject ToInt32(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create(((SmileInteger16)argv[0])->value);
}

static SmileObject ToInt16(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return argv[0];
}

static SmileObject ToByte(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create((Byte)((SmileInteger16)argv[0])->value);
}

//-------------------------------------------------------------------------------------------------
// Parsing

static SmileObject Parse(Int argc, SmileObject *argv, void *param)
{
	Int64 numericBase;
	Int64 value;

	UNUSED(param);

	switch (argc) {

		case 1:
			// The form [parse string].
			if (SMILE_KIND(argv[0]) != SMILE_KIND_STRING)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _parseArguments);
			if (!String_ParseInteger(SmileString_GetString((SmileString)argv[0]), 10, &value))
				return NullObject;
			return (SmileObject)SmileInteger16_Create((Int16)value);

		case 2:
			// Either the form [parse string base] or [obj.parse string].
			if (SMILE_KIND(argv[0]) == SMILE_KIND_STRING && SMILE_KIND(argv[1]) == SMILE_KIND_INTEGER64) {
				// The form [parse string base].
				numericBase = (Int)((SmileInteger64)argv[1])->value;
				if (numericBase < 2 || numericBase > 36)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
				if (!String_ParseInteger(SmileString_GetString((SmileString)argv[0]), (Int)numericBase, &value))
					return NullObject;
				return (SmileObject)SmileInteger16_Create((Int16)value);
			}
			else if (SMILE_KIND(argv[1]) == SMILE_KIND_STRING) {
				// The form [obj.parse string].
				if (!String_ParseInteger(SmileString_GetString((SmileString)argv[1]), 10, &value))
					return NullObject;
				return (SmileObject)SmileInteger16_Create((Int16)value);
			}
			else {
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _parseArguments);
			}

		case 3:
			// The form [obj.parse string base].
			if (SMILE_KIND(argv[1]) != SMILE_KIND_STRING || SMILE_KIND(argv[2]) != SMILE_KIND_INTEGER64)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _parseArguments);
			numericBase = (Int)((SmileInteger64)argv[2])->value;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
			if (!String_ParseInteger(SmileString_GetString((SmileString)argv[1]), (Int)numericBase, &value))
				return NullObject;
			return (SmileObject)SmileInteger16_Create((Int16)value);
	}

	return NullObject;	// Can't get here, but the compiler doesn't know that.
}


//-------------------------------------------------------------------------------------------------
// Arithmetic operators

static SmileObject Plus(Int argc, SmileObject *argv, void *param)
{
	Int16 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger16)argv[0])->value;
			x += ((SmileInteger16)argv[1])->value;
			return (SmileObject)SmileInteger16_Create(x);
		
		case 3:
			x = ((SmileInteger16)argv[0])->value;
			x += ((SmileInteger16)argv[1])->value;
			x += ((SmileInteger16)argv[2])->value;
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			x += ((SmileInteger16)argv[1])->value;
			x += ((SmileInteger16)argv[2])->value;
			x += ((SmileInteger16)argv[3])->value;
			return (SmileObject)SmileInteger16_Create(x);
		
		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x += ((SmileInteger16)argv[i])->value;
			}
			return (SmileObject)SmileInteger16_Create(x);
	}
}

static SmileObject Minus(Int argc, SmileObject *argv, void *param)
{
	Int16 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			x = ((SmileInteger16)argv[0])->value;
			return (SmileObject)SmileInteger16_Create(-x);

		case 2:
			x = ((SmileInteger16)argv[0])->value;
			x -= ((SmileInteger16)argv[1])->value;
			return (SmileObject)SmileInteger16_Create(x);

		case 3:
			x = ((SmileInteger16)argv[0])->value;
			x -= ((SmileInteger16)argv[1])->value;
			x -= ((SmileInteger16)argv[2])->value;
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			x -= ((SmileInteger16)argv[1])->value;
			x -= ((SmileInteger16)argv[2])->value;
			x -= ((SmileInteger16)argv[3])->value;
			return (SmileObject)SmileInteger16_Create(x);

		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x -= ((SmileInteger16)argv[i])->value;
			}
			return (SmileObject)SmileInteger16_Create(x);
	}
}

static SmileObject Star(Int argc, SmileObject *argv, void *param)
{
	Int16 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger16)argv[0])->value;
			x *= ((SmileInteger16)argv[1])->value;
			return (SmileObject)SmileInteger16_Create(x);

		case 3:
			x = ((SmileInteger16)argv[0])->value;
			x *= ((SmileInteger16)argv[1])->value;
			x *= ((SmileInteger16)argv[2])->value;
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			x *= ((SmileInteger16)argv[1])->value;
			x *= ((SmileInteger16)argv[2])->value;
			x *= ((SmileInteger16)argv[3])->value;
			return (SmileObject)SmileInteger16_Create(x);

		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x *= ((SmileInteger16)argv[i])->value;
			}
			return (SmileObject)SmileInteger16_Create(x);
	}
}

static SmileObject UStar(Int argc, SmileObject *argv, void *param)
{
	UInt16 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			x *= (UInt16)((SmileInteger16)argv[1])->value;
			return (SmileObject)SmileInteger16_Create((Int16)x);

		case 3:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			x *= (UInt16)((SmileInteger16)argv[1])->value;
			x *= (UInt16)((SmileInteger16)argv[2])->value;
			return (SmileObject)SmileInteger16_Create((Int16)x);

		case 4:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			x *= (UInt16)((SmileInteger16)argv[1])->value;
			x *= (UInt16)((SmileInteger16)argv[2])->value;
			x *= (UInt16)((SmileInteger16)argv[3])->value;
			return (SmileObject)SmileInteger16_Create((Int16)x);

		default:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x *= (UInt16)((SmileInteger16)argv[i])->value;
			}
			return (SmileObject)SmileInteger16_Create((Int16)x);
	}
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

static SmileObject Slash(Int argc, SmileObject *argv, void *param)
{
	Int16 x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		case 3:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger16)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger16)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger16)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileInteger16)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = MathematiciansDiv(x, y);
			}
			return (SmileObject)SmileInteger16_Create(x);
	}
}

static SmileObject USlash(Int argc, SmileObject *argv, void *param)
{
	UInt16 x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			if ((y = (UInt16)((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileInteger16_Create((Int16)x);

		case 3:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			if ((y = (UInt16)((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (UInt16)((SmileInteger16)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileInteger16_Create((Int16)x);

		case 4:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			if ((y = (UInt16)((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (UInt16)((SmileInteger16)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (UInt16)((SmileInteger16)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileInteger16_Create((Int16)x);

		default:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = (UInt16)((SmileInteger16)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x /= y;
			}
			return (SmileObject)SmileInteger16_Create((Int16)x);
	}
}

static SmileObject Div(Int argc, SmileObject *argv, void *param)
{
	Int16 x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		case 3:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger16)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger16)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger16)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileInteger16)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = CDiv(x, y);
			}
			return (SmileObject)SmileInteger16_Create(x);
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

static SmileObject Mod(Int argc, SmileObject *argv, void *param)
{
	Int16 x = ((SmileInteger16)argv[0])->value;
	Int16 y = ((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger16_Create(MathematiciansModulus(x, y));
}

static SmileObject UMod(Int argc, SmileObject *argv, void *param)
{
	UInt16 x = (UInt16)((SmileInteger16)argv[0])->value;
	UInt16 y = (UInt16)((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger16_Create((Int16)(x % y));
}

static SmileObject Rem(Int argc, SmileObject *argv, void *param)
{
	Int16 x = ((SmileInteger16)argv[0])->value;
	Int16 y = ((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger16_Create(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

static SmileObject Sign(Int argc, SmileObject *argv, void *param)
{
	Int16 value = ((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value == 0 ? (SmileObject)Smile_KnownObjects.ZeroInt16
		: value > 0 ? (SmileObject)Smile_KnownObjects.OneInt16
		: (SmileObject)Smile_KnownObjects.NegOneInt16;
}

static SmileObject Abs(Int argc, SmileObject *argv, void *param)
{
	Int16 value = ((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value < 0 ? (SmileObject)SmileInteger16_Create(-value) : argv[0];
}

static SmileObject Clip(Int argc, SmileObject *argv, void *param)
{
	Int16 value = ((SmileInteger16)argv[0])->value;
	Int16 min = ((SmileInteger16)argv[1])->value;
	Int16 max = ((SmileInteger16)argv[2])->value;

	UNUSED(argc);
	UNUSED(param);

	if (value > max) {
		value = max;
		return value < min ? argv[1] : argv[2];
	}
	else if (value < min) {
		return argv[1];
	}
	else return argv[0];
}

static SmileObject UClip(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = (UInt16)((SmileInteger16)argv[0])->value;
	UInt16 min = (UInt16)((SmileInteger16)argv[1])->value;
	UInt16 max = (UInt16)((SmileInteger16)argv[2])->value;

	UNUSED(argc);
	UNUSED(param);

	if (value > max) {
		value = max;
		return value < min ? argv[1] : argv[2];
	}
	else if (value < min) {
		return argv[1];
	}
	else return argv[0];
}

static SmileObject Min(Int argc, SmileObject *argv, void *param)
{
	Int16 x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileInteger16)argv[0])->value;
			i = 0;
			y = ((SmileInteger16)argv[1])->value;
			if (y < x) i = 1;
			return argv[i];

		case 3:
			x = ((SmileInteger16)argv[0])->value;
			i = 0;
			y = ((SmileInteger16)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = ((SmileInteger16)argv[2])->value;
			if (y < x) i = 2, x = y;
			return argv[i];

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			i = 0;
			y = ((SmileInteger16)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = ((SmileInteger16)argv[2])->value;
			if (y < x) i = 2, x = y;
			y = ((SmileInteger16)argv[3])->value;
			if (y < x) i = 3, x = y;
			return argv[i];

		default:
			x = ((SmileInteger16)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = ((SmileInteger16)argv[i])->value;
				if (y < x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject UMin(Int argc, SmileObject *argv, void *param)
{
	UInt16 x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			i = 0;
			y = (UInt16)((SmileInteger16)argv[1])->value;
			if (y < x) i = 1;
			return argv[i];

		case 3:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			i = 0;
			y = (UInt16)((SmileInteger16)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = (UInt16)((SmileInteger16)argv[2])->value;
			if (y < x) i = 2, x = y;
			return argv[i];

		case 4:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			i = 0;
			y = (UInt16)((SmileInteger16)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = (UInt16)((SmileInteger16)argv[2])->value;
			if (y < x) i = 2, x = y;
			y = (UInt16)((SmileInteger16)argv[3])->value;
			if (y < x) i = 3, x = y;
			return argv[i];

		default:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = (UInt16)((SmileInteger16)argv[i])->value;
				if (y < x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject Max(Int argc, SmileObject *argv, void *param)
{
	Int16 x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileInteger16)argv[0])->value;
			i = 0;
			y = ((SmileInteger16)argv[1])->value;
			if (y > x) i = 1;
			return argv[i];

		case 3:
			x = ((SmileInteger16)argv[0])->value;
			i = 0;
			y = ((SmileInteger16)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = ((SmileInteger16)argv[2])->value;
			if (y > x) i = 2, x = y;
			return argv[i];

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			i = 0;
			y = ((SmileInteger16)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = ((SmileInteger16)argv[2])->value;
			if (y > x) i = 2, x = y;
			y = ((SmileInteger16)argv[3])->value;
			if (y > x) i = 3, x = y;
			return argv[i];

		default:
			x = ((SmileInteger16)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = ((SmileInteger16)argv[i])->value;
				if (y > x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject UMax(Int argc, SmileObject *argv, void *param)
{
	UInt16 x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			i = 0;
			y = (UInt16)((SmileInteger16)argv[1])->value;
			if (y > x) i = 1;
			return argv[i];

		case 3:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			i = 0;
			y = (UInt16)((SmileInteger16)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = (UInt16)((SmileInteger16)argv[2])->value;
			if (y > x) i = 2, x = y;
			return argv[i];

		case 4:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			i = 0;
			y = (UInt16)((SmileInteger16)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = (UInt16)((SmileInteger16)argv[2])->value;
			if (y > x) i = 2, x = y;
			y = (UInt16)((SmileInteger16)argv[3])->value;
			if (y > x) i = 3, x = y;
			return argv[i];

		default:
			x = (UInt16)((SmileInteger16)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = (UInt16)((SmileInteger16)argv[i])->value;
				if (y > x) j = i, x = y;
			}
			return argv[j];
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

static SmileObject Power(Int argc, SmileObject *argv, void *param)
{
	Int16 x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		case 3:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileInteger16)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			if ((y = ((SmileInteger16)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileInteger16)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileInteger16)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileInteger16_Create(x);

		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileInteger16)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = IntPower(x, y);
			}
			return (SmileObject)SmileInteger16_Create(x);
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

static SmileObject Sqrt(Int argc, SmileObject *argv, void *param)
{
	Int16 value = ((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	if (value < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);

	return (SmileObject)SmileInteger16_Create((Int16)IntSqrt((UInt16)value));
}

static SmileObject Pow2Q(Int argc, SmileObject *argv, void *param)
{
	Int16 value = ((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value > 0 && (value & (value - 1)) == 0 ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject NextPow2(Int argc, SmileObject *argv, void *param)
{
	Int16 value = ((SmileInteger16)argv[0])->value;
	UInt16 uvalue = (UInt16)value;

	UNUSED(argc);
	UNUSED(param);

	if (value <= 0) return (SmileObject)Smile_KnownObjects.OneInt16;

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue |= uvalue >> 8;
	uvalue++;

	return (SmileObject)SmileInteger16_Create((Int16)uvalue);
}

static SmileObject IntLg(Int argc, SmileObject *argv, void *param)
{
	Int16 value = ((SmileInteger16)argv[0])->value;
	UInt16 uvalue = (UInt16)value;
	UInt16 log;

	UNUSED(argc);
	UNUSED(param);

	if (value <= 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeLog);

	log = 0;
	if ((uvalue & 0xFF00) != 0) uvalue >>= 8, log += 8;
	if ((uvalue & 0x00F0) != 0) uvalue >>= 4, log += 4;
	if ((uvalue & 0x000C) != 0) uvalue >>= 2, log += 2;
	if ((uvalue & 0x0002) != 0) uvalue >>= 1, log += 1;

	return (SmileObject)SmileInteger16_Create((Int16)log);
}

//-------------------------------------------------------------------------------------------------
// Bitwise operators

static SmileObject BitAnd(Int argc, SmileObject *argv, void *param)
{
	Int16 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger16)argv[0])->value;
			x &= ((SmileInteger16)argv[1])->value;
			return (SmileObject)SmileInteger16_Create(x);
		
		case 3:
			x = ((SmileInteger16)argv[0])->value;
			x &= ((SmileInteger16)argv[1])->value;
			x &= ((SmileInteger16)argv[2])->value;
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			x &= ((SmileInteger16)argv[1])->value;
			x &= ((SmileInteger16)argv[2])->value;
			x &= ((SmileInteger16)argv[3])->value;
			return (SmileObject)SmileInteger16_Create(x);
		
		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x &= ((SmileInteger16)argv[i])->value;
			}
			return (SmileObject)SmileInteger16_Create(x);
	}
}

static SmileObject BitOr(Int argc, SmileObject *argv, void *param)
{
	Int16 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger16)argv[0])->value;
			x |= ((SmileInteger16)argv[1])->value;
			return (SmileObject)SmileInteger16_Create(x);
		
		case 3:
			x = ((SmileInteger16)argv[0])->value;
			x |= ((SmileInteger16)argv[1])->value;
			x |= ((SmileInteger16)argv[2])->value;
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			x |= ((SmileInteger16)argv[1])->value;
			x |= ((SmileInteger16)argv[2])->value;
			x |= ((SmileInteger16)argv[3])->value;
			return (SmileObject)SmileInteger16_Create(x);
		
		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x |= ((SmileInteger16)argv[i])->value;
			}
			return (SmileObject)SmileInteger16_Create(x);
	}
}

static SmileObject BitXor(Int argc, SmileObject *argv, void *param)
{
	Int16 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger16)argv[0])->value;
			x ^= ((SmileInteger16)argv[1])->value;
			return (SmileObject)SmileInteger16_Create(x);
		
		case 3:
			x = ((SmileInteger16)argv[0])->value;
			x ^= ((SmileInteger16)argv[1])->value;
			x ^= ((SmileInteger16)argv[2])->value;
			return (SmileObject)SmileInteger16_Create(x);

		case 4:
			x = ((SmileInteger16)argv[0])->value;
			x ^= ((SmileInteger16)argv[1])->value;
			x ^= ((SmileInteger16)argv[2])->value;
			x ^= ((SmileInteger16)argv[3])->value;
			return (SmileObject)SmileInteger16_Create(x);
		
		default:
			x = ((SmileInteger16)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x ^= ((SmileInteger16)argv[i])->value;
			}
			return (SmileObject)SmileInteger16_Create(x);
	}
}

static SmileObject BitNot(Int argc, SmileObject *argv, void *param)
{
	Int16 value = ((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value < 0 ? (SmileObject)SmileInteger16_Create(~value) : argv[0];
}

//-------------------------------------------------------------------------------------------------
// Shift/rotate operators

static SmileObject LogicalShiftLeft(Int argc, SmileObject *argv, void *param)
{
	UInt16 x = (UInt16)((SmileInteger16)argv[0])->value;
	UInt16 y = (UInt16)((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)(x << y));
}

static SmileObject LogicalShiftRight(Int argc, SmileObject *argv, void *param)
{
	UInt16 x = (UInt16)((SmileInteger16)argv[0])->value;
	UInt16 y = (UInt16)((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)(x >> y));
}

static SmileObject ArithmeticShiftLeft(Int argc, SmileObject *argv, void *param)
{
	Int16 x = ((SmileInteger16)argv[0])->value;
	Int16 y = ((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create(x << y);
}

static SmileObject ArithmeticShiftRight(Int argc, SmileObject *argv, void *param)
{
	Int16 x = ((SmileInteger16)argv[0])->value;
	Int16 y = ((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create(x >> y);
}

static SmileObject RotateLeft(Int argc, SmileObject *argv, void *param)
{
	UInt16 x = (UInt16)((SmileInteger16)argv[0])->value;
	UInt16 y = (UInt16)((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)Smile_RotateLeft32(x, y));
}

static SmileObject RotateRight(Int argc, SmileObject *argv, void *param)
{
	UInt16 x = (UInt16)((SmileInteger16)argv[0])->value;
	UInt16 y = (UInt16)((SmileInteger16)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)Smile_RotateRight32(x, y));
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

static SmileObject CountOnes(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = ((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)CountBitsSet((UInt16)value));
}

static SmileObject CountZeros(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = ((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)CountBitsSet(~(UInt16)value));
}

static SmileObject Parity(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = ((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	value ^= value >> 8;
	value ^= value >> 4;
	value &= 0xF;
	value = (0x6996 >> value) & 1;

	return (SmileObject)SmileInteger16_Create((Int16)value);
}

static SmileObject ReverseBits(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = (UInt16)((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)ComputeReverseBits(value));
}

static SmileObject ReverseBytes(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = (UInt16)((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	value = (UInt16)( ((value >>  8) & 0x00FFU)
						| ((value <<  8) & 0xFF00U) );

	return (SmileObject)SmileInteger16_Create((Int16)value);
}

static SmileObject CountRightZeros(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = (UInt16)((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)ComputeCountOfRightZeros(value));
}

static SmileObject CountRightOnes(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = (UInt16)((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)ComputeCountOfRightZeros(~value));
}

static SmileObject CountLeftZeros(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = (UInt16)((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)ComputeCountOfRightZeros(ComputeReverseBits(value)));
}

static SmileObject CountLeftOnes(Int argc, SmileObject *argv, void *param)
{
	UInt16 value = (UInt16)((SmileInteger16)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((Int16)ComputeCountOfRightZeros(~ComputeReverseBits(value)));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

static SmileObject Eq(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER16
		|| ((SmileInteger16)argv[0])->value != ((SmileInteger16)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject Ne(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER16
		|| ((SmileInteger16)argv[0])->value != ((SmileInteger16)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.TrueObj;

	return (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Lt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger16)argv[0])->value < ((SmileInteger16)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Gt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger16)argv[0])->value > ((SmileInteger16)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Le(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger16)argv[0])->value <= ((SmileInteger16)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Ge(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger16)argv[0])->value >= ((SmileInteger16)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject ULt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (UInt16)((SmileInteger16)argv[0])->value < (UInt16)((SmileInteger16)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject UGt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (UInt16)((SmileInteger16)argv[0])->value > (UInt16)((SmileInteger16)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject ULe(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (UInt16)((SmileInteger16)argv[0])->value <= (UInt16)((SmileInteger16)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject UGe(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (UInt16)((SmileInteger16)argv[0])->value >= (UInt16)((SmileInteger16)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Compare(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	Int16 x = ((SmileInteger16)argv[0])->value;
	Int16 y = ((SmileInteger16)argv[1])->value;

	if (x == y)
		return (SmileObject)Smile_KnownObjects.ZeroInt64;
	else if (x < y)
		return (SmileObject)Smile_KnownObjects.NegOneInt64;
	else
		return (SmileObject)Smile_KnownObjects.OneInt64;
}

static SmileObject UCompare(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	UInt16 x = (UInt16)((SmileInteger16)argv[0])->value;
	UInt16 y = (UInt16)((SmileInteger16)argv[1])->value;

	if (x == y)
		return (SmileObject)Smile_KnownObjects.ZeroInt64;
	else if (x < y)
		return (SmileObject)Smile_KnownObjects.NegOneInt64;
	else
		return (SmileObject)Smile_KnownObjects.OneInt64;
}

//-------------------------------------------------------------------------------------------------

void SmileInteger16_Setup(SmileUserObject base)
{
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
	SetupFunction("/", Slash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("/~", USlash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("div", Div, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("div~", USlash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer16Checks);
	SetupFunction("mod", Mod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("mod~", UMod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("rem", Rem, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("rem~", UMod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer16Checks);
	SetupFunction("clip~", UClip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer16Checks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("min~", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("max~", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("^", Power, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer16Checks);
	SetupFunction("sqrt", Sqrt, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("pow2?", Pow2Q, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("next-pow2", NextPow2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);
	SetupFunction("int-lg", IntLg, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer16Checks);

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
