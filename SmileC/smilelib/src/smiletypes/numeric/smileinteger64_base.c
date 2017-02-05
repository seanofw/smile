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

#include <stdio.h>

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilefunction.h>

#define Setup(__name__, __value__) \
	(SmileUserObject_QuickSet(base, (__name__), (__value__)))

#define SetupFunction(__name__, __function__, __param__, __argNames__, __argCheckFlags__, __minArgs__, __maxArgs__, __numArgsToTypeCheck__, __argTypeChecks__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), (__argCheckFlags__), (__minArgs__), (__maxArgs__), (__numArgsToTypeCheck__), (__argTypeChecks__))))

#define SetupSimpleFunction(__name__, __function__, __argNames__, __numArgs__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), ARG_CHECK_EXACT, (__numArgs__), (__numArgs__), NULL)))

static Byte _integer64Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
};

static Byte _integer64ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	0, 0,
};

STATIC_STRING(_divideByZero, "Divide by zero error");
STATIC_STRING(_negativeLog, "Logarithm of negative or zero value");
STATIC_STRING(_negativeSqrt, "Square root of negative number");

STATIC_STRING(_invalidTypeError, "All arguments to 'Integer64.%s' must be of type 'Integer64'.");

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Integer64'");
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
STATIC_STRING(_parseArguments, "Illegal arguments to 'parse' function");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

static SmileObject ToBool(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_INTEGER64)
		return ((SmileInteger64)argv[0])->value ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject ToInt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_INTEGER64)
		return argv[0];

	return (SmileObject)Smile_KnownObjects.ZeroInt64;
}

STATIC_STRING(_Integer64, "Integer64");

static SmileObject ToString(Int argc, SmileObject *argv, void *param)
{
	Int64 numericBase;

	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_INTEGER64) {
		if (argc == 2) {
			if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER64)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)((SmileInteger64)argv[1])->value;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;

		return (SmileObject)SmileString_Create(String_CreateFromInteger(((SmileInteger64)argv[0])->value, (Int)numericBase, False));
	}

	return (SmileObject)SmileString_Create(_Integer64);
}

static SmileObject Hash(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_INTEGER64)
		return argv[0];

	return (SmileObject)SmileInteger64_Create(((PtrInt)argv[0]) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

static SmileObject ToInt64(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return argv[0];
}

static SmileObject ToInt32(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create((UInt32)((SmileInteger64)argv[0])->value);
}

static SmileObject ToInt16(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create((UInt16)((SmileInteger64)argv[0])->value);
}

static SmileObject ToByte(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create((Byte)((SmileInteger64)argv[0])->value);
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
			return (SmileObject)SmileInteger64_Create(value);
		
		case 2:
			// Either the form [parse string base] or [obj.parse string].
			if (SMILE_KIND(argv[0]) == SMILE_KIND_STRING && SMILE_KIND(argv[1]) == SMILE_KIND_INTEGER64) {
				// The form [parse string base].
				numericBase = (Int)((SmileInteger64)argv[1])->value;
				if (numericBase < 2 || numericBase > 36)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
				if (!String_ParseInteger(SmileString_GetString((SmileString)argv[0]), (Int)numericBase, &value))
					return NullObject;
				return (SmileObject)SmileInteger64_Create(value);
			}
			else if (SMILE_KIND(argv[1]) == SMILE_KIND_STRING) {
				// The form [obj.parse string].
				if (!String_ParseInteger(SmileString_GetString((SmileString)argv[1]), 10, &value))
					return NullObject;
				return (SmileObject)SmileInteger64_Create(value);
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
			return (SmileObject)SmileInteger64_Create(value);
	}

	return NullObject;	// Can't get here, but the compiler doesn't know that.
}

//-------------------------------------------------------------------------------------------------
// Arithmetic operators

static SmileObject Plus(Int argc, SmileObject *argv, void *param)
{
	Int64 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileInteger64)argv[0])->value;
			x += ((SmileInteger64)argv[1])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			x += ((SmileInteger64)argv[1])->value;
			x += ((SmileInteger64)argv[2])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			x += ((SmileInteger64)argv[1])->value;
			x += ((SmileInteger64)argv[2])->value;
			x += ((SmileInteger64)argv[3])->value;
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x += ((SmileInteger64)argv[i])->value;
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

static SmileObject Minus(Int argc, SmileObject *argv, void *param)
{
	Int64 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			x = ((SmileInteger64)argv[0])->value;
			return (SmileObject)SmileInteger64_Create(-x);

		case 2:
			x = ((SmileInteger64)argv[0])->value;
			x -= ((SmileInteger64)argv[1])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			x -= ((SmileInteger64)argv[1])->value;
			x -= ((SmileInteger64)argv[2])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			x -= ((SmileInteger64)argv[1])->value;
			x -= ((SmileInteger64)argv[2])->value;
			x -= ((SmileInteger64)argv[3])->value;
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x -= ((SmileInteger64)argv[i])->value;
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

static SmileObject Star(Int argc, SmileObject *argv, void *param)
{
	Int64 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger64)argv[0])->value;
			x *= ((SmileInteger64)argv[1])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			x *= ((SmileInteger64)argv[1])->value;
			x *= ((SmileInteger64)argv[2])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			x *= ((SmileInteger64)argv[1])->value;
			x *= ((SmileInteger64)argv[2])->value;
			x *= ((SmileInteger64)argv[3])->value;
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x *= ((SmileInteger64)argv[i])->value;
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

static SmileObject UStar(Int argc, SmileObject *argv, void *param)
{
	UInt64 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			x *= (UInt64)((SmileInteger64)argv[1])->value;
			return (SmileObject)SmileInteger64_Create((Int64)x);

		case 3:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			x *= (UInt64)((SmileInteger64)argv[1])->value;
			x *= (UInt64)((SmileInteger64)argv[2])->value;
			return (SmileObject)SmileInteger64_Create((Int64)x);

		case 4:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			x *= (UInt64)((SmileInteger64)argv[1])->value;
			x *= (UInt64)((SmileInteger64)argv[2])->value;
			x *= (UInt64)((SmileInteger64)argv[3])->value;
			return (SmileObject)SmileInteger64_Create((Int64)x);

		default:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x *= (UInt64)((SmileInteger64)argv[i])->value;
			}
			return (SmileObject)SmileInteger64_Create((Int64)x);
	}
}

/// <summary>
/// Perform division classic-C-style, which rounds towards zero no matter what the sign is.
/// </summary>
Inline Int64 CDiv(Int64 dividend, Int64 divisor)
{
	if (dividend < 0) {
		if (divisor < 0)
			return (Int64)((UInt64)-dividend / (UInt64)-divisor);
		else
			return -(Int64)((UInt64)-dividend / (UInt64)divisor);
	}
	else if (divisor < 0)
		return -(Int64)((UInt64)dividend / (UInt64)-divisor);
	else
		return (Int64)((UInt64)dividend / (UInt64)divisor);
}

/// <summary>
/// Perform division like mathematicians expect, which rounds toward negative infinity (always).
/// </summary>
Inline Int64 MathematiciansDiv(Int64 dividend, Int64 divisor)
{
	if (dividend < 0) {
		if (divisor < 0) {
			return (Int64)((UInt64)-dividend / (UInt64)-divisor);
		}
		else {
			UInt64 positiveQuotient = (UInt64)-dividend / (UInt64)divisor;
			UInt64 positiveRemainder = (UInt64)-dividend % (UInt64)divisor;
			return positiveRemainder == 0 ? -(Int64)positiveQuotient : -(Int64)positiveQuotient - 1;
		}
	}
	else if (divisor < 0) {
		UInt64 positiveQuotient = (UInt64)dividend / (UInt64)-divisor;
		UInt64 positiveRemainder = (UInt64)dividend % (UInt64)-divisor;
		return positiveRemainder == 0 ? -(Int64)positiveQuotient : -(Int64)positiveQuotient - 1;
	}
	else {
		return dividend / divisor;
	}
}

static SmileObject Slash(Int argc, SmileObject *argv, void *param)
{
	Int64 x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger64)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger64)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger64)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileInteger64)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = MathematiciansDiv(x, y);
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

static SmileObject USlash(Int argc, SmileObject *argv, void *param)
{
	UInt64 x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			if ((y = (UInt64)((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileInteger64_Create((Int64)x);

		case 3:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			if ((y = (UInt64)((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (UInt64)((SmileInteger64)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileInteger64_Create((Int64)x);

		case 4:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			if ((y = (UInt64)((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (UInt64)((SmileInteger64)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (UInt64)((SmileInteger64)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileInteger64_Create((Int64)x);

		default:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = (UInt64)((SmileInteger64)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x /= y;
			}
			return (SmileObject)SmileInteger64_Create((Int64)x);
	}
}

static SmileObject Div(Int argc, SmileObject *argv, void *param)
{
	Int64 x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger64)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger64)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger64)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileInteger64)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = CDiv(x, y);
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Int64 MathematiciansModulus(Int64 x, Int64 y)
{
	Int64 rem;

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
Inline Int64 MathematiciansRemainder(Int64 x, Int64 y)
{
	Int64 rem;

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
	Int64 x = ((SmileInteger64)argv[0])->value;
	Int64 y = ((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger64_Create(MathematiciansModulus(x, y));
}

static SmileObject UMod(Int argc, SmileObject *argv, void *param)
{
	UInt64 x = (UInt64)((SmileInteger64)argv[0])->value;
	UInt64 y = (UInt64)((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger64_Create((Int64)(x % y));
}

static SmileObject Rem(Int argc, SmileObject *argv, void *param)
{
	Int64 x = ((SmileInteger64)argv[0])->value;
	Int64 y = ((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger64_Create(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

static SmileObject Sign(Int argc, SmileObject *argv, void *param)
{
	Int64 value = ((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value == 0 ? (SmileObject)Smile_KnownObjects.ZeroInt64
		: value > 0 ? (SmileObject)Smile_KnownObjects.OneInt64
		: (SmileObject)Smile_KnownObjects.NegOneInt64;
}

static SmileObject Abs(Int argc, SmileObject *argv, void *param)
{
	Int64 value = ((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value < 0 ? (SmileObject)SmileInteger64_Create(-value) : argv[0];
}

static SmileObject Clip(Int argc, SmileObject *argv, void *param)
{
	Int64 value = ((SmileInteger64)argv[0])->value;
	Int64 min = ((SmileInteger64)argv[1])->value;
	Int64 max = ((SmileInteger64)argv[2])->value;

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
	UInt64 value = (UInt64)((SmileInteger64)argv[0])->value;
	UInt64 min = (UInt64)((SmileInteger64)argv[1])->value;
	UInt64 max = (UInt64)((SmileInteger64)argv[2])->value;

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
	Int64 x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileInteger64)argv[0])->value;
			i = 0;
			y = ((SmileInteger64)argv[1])->value;
			if (y < x) i = 1;
			return argv[i];

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			i = 0;
			y = ((SmileInteger64)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = ((SmileInteger64)argv[2])->value;
			if (y < x) i = 2, x = y;
			return argv[i];

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			i = 0;
			y = ((SmileInteger64)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = ((SmileInteger64)argv[2])->value;
			if (y < x) i = 2, x = y;
			y = ((SmileInteger64)argv[3])->value;
			if (y < x) i = 3, x = y;
			return argv[i];

		default:
			x = ((SmileInteger64)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = ((SmileInteger64)argv[i])->value;
				if (y < x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject UMin(Int argc, SmileObject *argv, void *param)
{
	UInt64 x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			i = 0;
			y = (UInt64)((SmileInteger64)argv[1])->value;
			if (y < x) i = 1;
			return argv[i];

		case 3:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			i = 0;
			y = (UInt64)((SmileInteger64)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = (UInt64)((SmileInteger64)argv[2])->value;
			if (y < x) i = 2, x = y;
			return argv[i];

		case 4:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			i = 0;
			y = (UInt64)((SmileInteger64)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = (UInt64)((SmileInteger64)argv[2])->value;
			if (y < x) i = 2, x = y;
			y = (UInt64)((SmileInteger64)argv[3])->value;
			if (y < x) i = 3, x = y;
			return argv[i];

		default:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = (UInt64)((SmileInteger64)argv[i])->value;
				if (y < x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject Max(Int argc, SmileObject *argv, void *param)
{
	Int64 x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileInteger64)argv[0])->value;
			i = 0;
			y = ((SmileInteger64)argv[1])->value;
			if (y > x) i = 1;
			return argv[i];

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			i = 0;
			y = ((SmileInteger64)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = ((SmileInteger64)argv[2])->value;
			if (y > x) i = 2, x = y;
			return argv[i];

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			i = 0;
			y = ((SmileInteger64)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = ((SmileInteger64)argv[2])->value;
			if (y > x) i = 2, x = y;
			y = ((SmileInteger64)argv[3])->value;
			if (y > x) i = 3, x = y;
			return argv[i];

		default:
			x = ((SmileInteger64)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = ((SmileInteger64)argv[i])->value;
				if (y > x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject UMax(Int argc, SmileObject *argv, void *param)
{
	UInt64 x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			i = 0;
			y = (UInt64)((SmileInteger64)argv[1])->value;
			if (y > x) i = 1;
			return argv[i];

		case 3:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			i = 0;
			y = (UInt64)((SmileInteger64)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = (UInt64)((SmileInteger64)argv[2])->value;
			if (y > x) i = 2, x = y;
			return argv[i];

		case 4:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			i = 0;
			y = (UInt64)((SmileInteger64)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = (UInt64)((SmileInteger64)argv[2])->value;
			if (y > x) i = 2, x = y;
			y = (UInt64)((SmileInteger64)argv[3])->value;
			if (y > x) i = 3, x = y;
			return argv[i];

		default:
			x = (UInt64)((SmileInteger64)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = (UInt64)((SmileInteger64)argv[i])->value;
				if (y > x) j = i, x = y;
			}
			return argv[j];
	}
}

Inline Int64 IntPower(Int64 value, Int64 exponent)
{
	if (exponent < 0) return 0;

	Int64 result = 1;

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
	Int64 x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileInteger64)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			if ((y = ((SmileInteger64)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileInteger64)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileInteger64)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileInteger64)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = IntPower(x, y);
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

Inline UInt64 IntSqrt(UInt64 value)
{
	UInt64 root, bit, trial;

	root = 0;

	bit =
		(value >= 0x100000000UL) ? (1ULL << 62)
		: (value >= 0x10000UL) ? (1ULL << 30)
		: (1ULL << 14);

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
	Int64 value = ((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	if (value < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);

	return (SmileObject)SmileInteger64_Create((Int64)IntSqrt((UInt64)value));
}

static SmileObject Pow2Q(Int argc, SmileObject *argv, void *param)
{
	Int64 value = ((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value > 0 && (value & (value - 1)) == 0 ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject NextPow2(Int argc, SmileObject *argv, void *param)
{
	Int64 value = ((SmileInteger64)argv[0])->value;
	UInt64 uvalue = (UInt64)value;

	UNUSED(argc);
	UNUSED(param);

	if (value <= 0) return (SmileObject)Smile_KnownObjects.OneInt64;

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue |= uvalue >> 8;
	uvalue |= uvalue >> 16;
	uvalue |= uvalue >> 32;
	uvalue++;

	return (SmileObject)SmileInteger64_Create((Int64)uvalue);
}

static SmileObject IntLg(Int argc, SmileObject *argv, void *param)
{
	Int64 value = ((SmileInteger64)argv[0])->value;
	UInt64 uvalue = (UInt64)value;
	UInt64 log;

	UNUSED(argc);
	UNUSED(param);

	if (value <= 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeLog);

	log = 0;
	if ((uvalue & 0xFFFFFFFF00000000) != 0) uvalue >>= 32, log += 32;
	if ((uvalue & 0x00000000FFFF0000) != 0) uvalue >>= 16, log += 16;
	if ((uvalue & 0x000000000000FF00) != 0) uvalue >>= 8, log += 8;
	if ((uvalue & 0x00000000000000F0) != 0) uvalue >>= 4, log += 4;
	if ((uvalue & 0x000000000000000C) != 0) uvalue >>= 2, log += 2;
	if ((uvalue & 0x0000000000000002) != 0) uvalue >>= 1, log += 1;

	return (SmileObject)SmileInteger64_Create((Int64)log);
}

//-------------------------------------------------------------------------------------------------
// Bitwise operators

static SmileObject BitAnd(Int argc, SmileObject *argv, void *param)
{
	Int64 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileInteger64)argv[0])->value;
			x &= ((SmileInteger64)argv[1])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			x &= ((SmileInteger64)argv[1])->value;
			x &= ((SmileInteger64)argv[2])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			x &= ((SmileInteger64)argv[1])->value;
			x &= ((SmileInteger64)argv[2])->value;
			x &= ((SmileInteger64)argv[3])->value;
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x &= ((SmileInteger64)argv[i])->value;
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

static SmileObject BitOr(Int argc, SmileObject *argv, void *param)
{
	Int64 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileInteger64)argv[0])->value;
			x |= ((SmileInteger64)argv[1])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			x |= ((SmileInteger64)argv[1])->value;
			x |= ((SmileInteger64)argv[2])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			x |= ((SmileInteger64)argv[1])->value;
			x |= ((SmileInteger64)argv[2])->value;
			x |= ((SmileInteger64)argv[3])->value;
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x |= ((SmileInteger64)argv[i])->value;
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

static SmileObject BitXor(Int argc, SmileObject *argv, void *param)
{
	Int64 x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileInteger64)argv[0])->value;
			x ^= ((SmileInteger64)argv[1])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 3:
			x = ((SmileInteger64)argv[0])->value;
			x ^= ((SmileInteger64)argv[1])->value;
			x ^= ((SmileInteger64)argv[2])->value;
			return (SmileObject)SmileInteger64_Create(x);

		case 4:
			x = ((SmileInteger64)argv[0])->value;
			x ^= ((SmileInteger64)argv[1])->value;
			x ^= ((SmileInteger64)argv[2])->value;
			x ^= ((SmileInteger64)argv[3])->value;
			return (SmileObject)SmileInteger64_Create(x);

		default:
			x = ((SmileInteger64)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x ^= ((SmileInteger64)argv[i])->value;
			}
			return (SmileObject)SmileInteger64_Create(x);
	}
}

static SmileObject BitNot(Int argc, SmileObject *argv, void *param)
{
	Int64 value = ((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value < 0 ? (SmileObject)SmileInteger64_Create(~value) : argv[0];
}

//-------------------------------------------------------------------------------------------------
// Shift/rotate operators

static SmileObject LogicalShiftLeft(Int argc, SmileObject *argv, void *param)
{
	UInt64 x = (UInt64)((SmileInteger64)argv[0])->value;
	UInt64 y = (UInt64)((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)(x << y));
}

static SmileObject LogicalShiftRight(Int argc, SmileObject *argv, void *param)
{
	UInt64 x = (UInt64)((SmileInteger64)argv[0])->value;
	UInt64 y = (UInt64)((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)(x >> y));
}

static SmileObject ArithmeticShiftLeft(Int argc, SmileObject *argv, void *param)
{
	Int64 x = ((SmileInteger64)argv[0])->value;
	Int64 y = ((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create(x << y);
}

static SmileObject ArithmeticShiftRight(Int argc, SmileObject *argv, void *param)
{
	Int64 x = ((SmileInteger64)argv[0])->value;
	Int64 y = ((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create(x >> y);
}

static SmileObject RotateLeft(Int argc, SmileObject *argv, void *param)
{
	UInt64 x = (UInt64)((SmileInteger64)argv[0])->value;
	UInt64 y = (UInt64)((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)Smile_RotateLeft64(x, y));
}

static SmileObject RotateRight(Int argc, SmileObject *argv, void *param)
{
	UInt64 x = (UInt64)((SmileInteger64)argv[0])->value;
	UInt64 y = (UInt64)((SmileInteger64)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)Smile_RotateRight64(x, y));
}

//-------------------------------------------------------------------------------------------------
// Bit twiddling

Inline UInt32 CountBitsSet32(UInt32 value)
{
	value = value - ((value >> 1) & 0x55555555);
	value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
	return ((value + (value >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

Inline UInt64 CountBitsSet(UInt64 value)
{
	return CountBitsSet32((UInt32)value) + CountBitsSet32((UInt32)(value >> 32));
}

Inline UInt64 ComputeReverseBits(UInt64 value)
{
	value = ((value >>  1) & 0x5555555555555555ULL) | ((value & 0x5555555555555555ULL) <<  1);
	value = ((value >>  2) & 0x3333333333333333ULL) | ((value & 0x3333333333333333ULL) <<  2);
	value = ((value >>  4) & 0x0F0F0F0F0F0F0F0FULL) | ((value & 0x0F0F0F0F0F0F0F0FULL) <<  4);
	value = ((value >>  8) & 0x00FF00FF00FF00FFULL) | ((value & 0x00FF00FF00FF00FFULL) <<  8);
	value = ((value >> 16) & 0x0000FFFF0000FFFFULL) | ((value & 0x0000FFFF0000FFFFULL) << 16);
	value = ( value >> 32                         ) | ( value                          << 32);
	return value;
}

Inline UInt64 ComputeCountOfRightZeros(UInt64 value)
{
	UInt64 c = 64;
	value &= ~value + 1;
	if (value != 0) c--;
	if ((value & 0x00000000FFFFFFFF) != 0) c -= 32;
	if ((value & 0x0000FFFF0000FFFF) != 0) c -= 16;
	if ((value & 0x00FF00FF00FF00FF) != 0) c -= 8;
	if ((value & 0x0F0F0F0F0F0F0F0F) != 0) c -= 4;
	if ((value & 0x3333333333333333) != 0) c -= 2;
	if ((value & 0x5555555555555555) != 0) c -= 1;
	return c;
}

static SmileObject CountOnes(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = ((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)CountBitsSet((UInt64)value));
}

static SmileObject CountZeros(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = ((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)CountBitsSet(~(UInt64)value));
}

static SmileObject Parity(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = ((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	value ^= value >> 32;
	value ^= value >> 16;
	value ^= value >> 8;
	value ^= value >> 4;
	value &= 0xF;
	value = (0x6996 >> value) & 1;

	return (SmileObject)SmileInteger64_Create((Int64)value);
}

static SmileObject ReverseBits(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = (UInt64)((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)ComputeReverseBits(value));
}

static SmileObject ReverseBytes(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = (UInt64)((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	value = (UInt64)( ((value >> 56) & 0x00000000000000FFULL)
						| ((value >> 40) & 0x000000000000FF00ULL)
						| ((value >> 24) & 0x0000000000FF0000ULL)
						| ((value >>  8) & 0x00000000FF000000ULL)
						| ((value <<  8) & 0x000000FF00000000ULL)
						| ((value << 24) & 0x0000FF0000000000ULL)
						| ((value << 40) & 0x00FF000000000000ULL)
						| ((value << 56) & 0xFF00000000000000ULL) );

	return (SmileObject)SmileInteger64_Create((Int64)value);
}

static SmileObject CountRightZeros(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = (UInt64)((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)ComputeCountOfRightZeros(value));
}

static SmileObject CountRightOnes(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = (UInt64)((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)ComputeCountOfRightZeros(~value));
}

static SmileObject CountLeftZeros(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = (UInt64)((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)ComputeCountOfRightZeros(ComputeReverseBits(value)));
}

static SmileObject CountLeftOnes(Int argc, SmileObject *argv, void *param)
{
	UInt64 value = (UInt64)((SmileInteger64)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create((Int64)ComputeCountOfRightZeros(~ComputeReverseBits(value)));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

static SmileObject Eq(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER64
		|| ((SmileInteger64)argv[0])->value != ((SmileInteger64)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject Ne(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER64
		|| ((SmileInteger64)argv[0])->value != ((SmileInteger64)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.TrueObj;

	return (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Lt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger64)argv[0])->value < ((SmileInteger64)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Gt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger64)argv[0])->value >((SmileInteger64)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Le(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger64)argv[0])->value <= ((SmileInteger64)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Ge(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger64)argv[0])->value >= ((SmileInteger64)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject ULt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (UInt64)((SmileInteger64)argv[0])->value < (UInt64)((SmileInteger64)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject UGt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (UInt64)((SmileInteger64)argv[0])->value >(UInt64)((SmileInteger64)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject ULe(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (UInt64)((SmileInteger64)argv[0])->value <= (UInt64)((SmileInteger64)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject UGe(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (UInt64)((SmileInteger64)argv[0])->value >= (UInt64)((SmileInteger64)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Compare(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	Int64 x = ((SmileInteger64)argv[0])->value;
	Int64 y = ((SmileInteger64)argv[1])->value;

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

	UInt64 x = (UInt64)((SmileInteger64)argv[0])->value;
	UInt64 y = (UInt64)((SmileInteger64)argv[1])->value;

	if (x == y)
		return (SmileObject)Smile_KnownObjects.ZeroInt64;
	else if (x < y)
		return (SmileObject)Smile_KnownObjects.NegOneInt64;
	else
		return (SmileObject)Smile_KnownObjects.OneInt64;
}

//-------------------------------------------------------------------------------------------------

void SmileInteger64_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("int64", ToInt64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("int32", ToInt32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("int16", ToInt16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("byte", ToByte, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);

	SetupFunction("parse", Parse, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 3, 0, NULL);

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("~+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("~-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer64Checks);
	SetupFunction("~*", UStar, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer64Checks);
	SetupFunction("/", Slash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer64Checks);
	SetupFunction("~/", USlash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer64Checks);
	SetupFunction("div", Div, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer64Checks);
	SetupFunction("udiv", USlash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer64Checks);
	SetupFunction("mod", Mod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("umod", UMod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("rem", Rem, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("urem", UMod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer64Checks);
	SetupFunction("uclip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _integer64Checks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("umin", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("umax", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("^", Power, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("sqrt", Sqrt, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("pow2?", Pow2Q, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("next-pow2", NextPow2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("int-lg", IntLg, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);

	SetupFunction("band", BitAnd, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("bor", BitOr, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("bxor", BitXor, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer64Checks);
	SetupFunction("~", BitNot, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);

	SetupFunction("<<<", LogicalShiftLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction(">>>", LogicalShiftRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("<<", ArithmeticShiftLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction(">>", ArithmeticShiftRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("<<+", RotateLeft, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("+>>", RotateRight, NULL, "value count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);

	SetupFunction("count-ones", CountOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("count-zeros", CountZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("parity", Parity, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("reverse-bits", ReverseBits, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("reverse-bytes", ReverseBytes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("count-right-zeros", CountRightZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("count-right-ones", CountRightOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("count-left-zeros", CountLeftZeros, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);
	SetupFunction("count-left-ones", CountLeftOnes, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer64Checks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64ComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64ComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("~<", ULt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("~>", UGt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("~<=", ULe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("~>=", UGe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
	SetupFunction("ucompare", UCompare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer64Checks);
}
