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

#define Setup(__name__, __value__) \
	(SmileUserObject_QuickSet(base, (__name__), (__value__)))

#define SetupFunction(__name__, __function__, __param__, __argNames__, __argCheckFlags__, __minArgs__, __maxArgs__, __numArgsToTypeCheck__, __argTypeChecks__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), (__argCheckFlags__), (__minArgs__), (__maxArgs__), (__numArgsToTypeCheck__), (__argTypeChecks__))))

#define SetupSimpleFunction(__name__, __function__, __argNames__, __numArgs__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), ARG_CHECK_EXACT, (__numArgs__), (__numArgs__), NULL)))

static Byte _byteChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
};

static Byte _byteComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_BYTE,
	0, 0,
};

static Byte _parseChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
};

STATIC_STRING(_divideByZero, "Divide by zero error");
STATIC_STRING(_negativeLog, "Logarithm of negative or zero value");
STATIC_STRING(_negativeSqrt, "Square root of negative number");

STATIC_STRING(_invalidTypeError, "All arguments to 'Byte.%s' must be of type 'Byte'");

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Integer64'");
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
STATIC_STRING(_parseArguments, "Illegal arguments to 'parse' function");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

static SmileObject ToBool(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_BYTE)
		return ((SmileByte)argv[0])->value ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject ToInt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_BYTE)
		return (SmileObject)SmileInteger64_Create(((SmileByte)argv[0])->value);

	return (SmileObject)Smile_KnownObjects.ZeroInt64;
}

STATIC_STRING(_Integer32, "Integer32");

static SmileObject ToString(Int argc, SmileObject *argv, void *param)
{
	Int64 numericBase;

	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_BYTE) {
		if (argc == 2) {
			if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER64)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)((SmileInteger64)argv[1])->value;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;

		return (SmileObject)SmileString_Create(String_CreateFromInteger((Int64)((SmileByte)argv[0])->value, (Int)numericBase, False));
	}

	return (SmileObject)SmileString_Create(_Integer32);
}

static SmileObject Hash(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_BYTE)
		return (SmileObject)SmileInteger64_Create(((SmileByte)argv[0])->value);

	return (SmileObject)SmileInteger64_Create(((PtrInt)argv[0]) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

static SmileObject ToInt64(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger64_Create(((SmileByte)argv[0])->value);
}

static SmileObject ToInt32(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create(((SmileByte)argv[0])->value);
}

static SmileObject ToInt16(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger16_Create(((SmileByte)argv[0])->value);
}

static SmileObject ToByte(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return argv[0];
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
			return (SmileObject)SmileByte_Create((Byte)value);

		case 2:
			// Either the form [parse string base] or [obj.parse string].
			if (SMILE_KIND(argv[0]) == SMILE_KIND_STRING && SMILE_KIND(argv[1]) == SMILE_KIND_INTEGER64) {
				// The form [parse string base].
				numericBase = (Int)((SmileInteger64)argv[1])->value;
				if (numericBase < 2 || numericBase > 36)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
				if (!String_ParseInteger(SmileString_GetString((SmileString)argv[0]), (Int)numericBase, &value))
					return NullObject;
				return (SmileObject)SmileByte_Create((Byte)value);
			}
			else if (SMILE_KIND(argv[1]) == SMILE_KIND_STRING) {
				// The form [obj.parse string].
				if (!String_ParseInteger(SmileString_GetString((SmileString)argv[1]), 10, &value))
					return NullObject;
				return (SmileObject)SmileByte_Create((Byte)value);
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
			return (SmileObject)SmileByte_Create((Byte)value);
	}

	return NullObject;	// Can't get here, but the compiler doesn't know that.
}

//-------------------------------------------------------------------------------------------------
// Arithmetic operators

static SmileObject Plus(Int argc, SmileObject *argv, void *param)
{
	Byte x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileByte)argv[0])->value;
			x += ((SmileByte)argv[1])->value;
			return (SmileObject)SmileByte_Create(x);
		
		case 3:
			x = ((SmileByte)argv[0])->value;
			x += ((SmileByte)argv[1])->value;
			x += ((SmileByte)argv[2])->value;
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			x += ((SmileByte)argv[1])->value;
			x += ((SmileByte)argv[2])->value;
			x += ((SmileByte)argv[3])->value;
			return (SmileObject)SmileByte_Create(x);
		
		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x += ((SmileByte)argv[i])->value;
			}
			return (SmileObject)SmileByte_Create(x);
	}
}

static SmileObject Minus(Int argc, SmileObject *argv, void *param)
{
	Byte x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			x = ((SmileByte)argv[0])->value;
			return (SmileObject)SmileByte_Create(-x);

		case 2:
			x = ((SmileByte)argv[0])->value;
			x -= ((SmileByte)argv[1])->value;
			return (SmileObject)SmileByte_Create(x);

		case 3:
			x = ((SmileByte)argv[0])->value;
			x -= ((SmileByte)argv[1])->value;
			x -= ((SmileByte)argv[2])->value;
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			x -= ((SmileByte)argv[1])->value;
			x -= ((SmileByte)argv[2])->value;
			x -= ((SmileByte)argv[3])->value;
			return (SmileObject)SmileByte_Create(x);

		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x -= ((SmileByte)argv[i])->value;
			}
			return (SmileObject)SmileByte_Create(x);
	}
}

static SmileObject Star(Int argc, SmileObject *argv, void *param)
{
	SByte x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileByte)argv[0])->value;
			x *= ((SmileByte)argv[1])->value;
			return (SmileObject)SmileByte_Create(x);

		case 3:
			x = ((SmileByte)argv[0])->value;
			x *= ((SmileByte)argv[1])->value;
			x *= ((SmileByte)argv[2])->value;
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			x *= ((SmileByte)argv[1])->value;
			x *= ((SmileByte)argv[2])->value;
			x *= ((SmileByte)argv[3])->value;
			return (SmileObject)SmileByte_Create(x);

		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x *= ((SmileByte)argv[i])->value;
			}
			return (SmileObject)SmileByte_Create(x);
	}
}

static SmileObject UStar(Int argc, SmileObject *argv, void *param)
{
	Byte x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = (Byte)((SmileByte)argv[0])->value;
			x *= (Byte)((SmileByte)argv[1])->value;
			return (SmileObject)SmileByte_Create((Byte)x);

		case 3:
			x = (Byte)((SmileByte)argv[0])->value;
			x *= (Byte)((SmileByte)argv[1])->value;
			x *= (Byte)((SmileByte)argv[2])->value;
			return (SmileObject)SmileByte_Create((Byte)x);

		case 4:
			x = (Byte)((SmileByte)argv[0])->value;
			x *= (Byte)((SmileByte)argv[1])->value;
			x *= (Byte)((SmileByte)argv[2])->value;
			x *= (Byte)((SmileByte)argv[3])->value;
			return (SmileObject)SmileByte_Create((Byte)x);

		default:
			x = (Byte)((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x *= (Byte)((SmileByte)argv[i])->value;
			}
			return (SmileObject)SmileByte_Create((Byte)x);
	}
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

static SmileObject Slash(Int argc, SmileObject *argv, void *param)
{
	SByte x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileByte_Create(x);

		case 3:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileByte)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileByte)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileByte)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileByte_Create(x);

		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileByte)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = MathematiciansDiv(x, y);
			}
			return (SmileObject)SmileByte_Create(x);
	}
}

static SmileObject USlash(Int argc, SmileObject *argv, void *param)
{
	Byte x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = (Byte)((SmileByte)argv[0])->value;
			if ((y = (Byte)((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileByte_Create((Byte)x);

		case 3:
			x = (Byte)((SmileByte)argv[0])->value;
			if ((y = (Byte)((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (Byte)((SmileByte)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileByte_Create((Byte)x);

		case 4:
			x = (Byte)((SmileByte)argv[0])->value;
			if ((y = (Byte)((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (Byte)((SmileByte)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			if ((y = (Byte)((SmileByte)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x /= y;
			return (SmileObject)SmileByte_Create((Byte)x);

		default:
			x = (Byte)((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = (Byte)((SmileByte)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x /= y;
			}
			return (SmileObject)SmileByte_Create((Byte)x);
	}
}

static SmileObject Div(Int argc, SmileObject *argv, void *param)
{
	SByte x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileByte_Create(x);

		case 3:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileByte)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileByte)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileByte)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileByte_Create(x);

		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileByte)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = CDiv(x, y);
			}
			return (SmileObject)SmileByte_Create(x);
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

static SmileObject Mod(Int argc, SmileObject *argv, void *param)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileByte_Create(MathematiciansModulus(x, y));
}

static SmileObject UMod(Int argc, SmileObject *argv, void *param)
{
	Byte x = (Byte)((SmileByte)argv[0])->value;
	Byte y = (Byte)((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileByte_Create((Byte)(x % y));
}

static SmileObject Rem(Int argc, SmileObject *argv, void *param)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileByte_Create(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

static SmileObject Sign(Int argc, SmileObject *argv, void *param)
{
	SByte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value == 0 ? (SmileObject)Smile_KnownObjects.ZeroByte
		: value > 0 ? (SmileObject)Smile_KnownObjects.OneByte
		: (SmileObject)Smile_KnownObjects.NegOneByte;
}

static SmileObject Abs(Int argc, SmileObject *argv, void *param)
{
	SByte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value < 0 ? (SmileObject)SmileByte_Create(-value) : argv[0];
}

static SmileObject Clip(Int argc, SmileObject *argv, void *param)
{
	SByte value = ((SmileByte)argv[0])->value;
	SByte min = ((SmileByte)argv[1])->value;
	SByte max = ((SmileByte)argv[2])->value;

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
	Byte value = (Byte)((SmileByte)argv[0])->value;
	Byte min = (Byte)((SmileByte)argv[1])->value;
	Byte max = (Byte)((SmileByte)argv[2])->value;

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
	SByte x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileByte)argv[0])->value;
			i = 0;
			y = ((SmileByte)argv[1])->value;
			if (y < x) i = 1;
			return argv[i];

		case 3:
			x = ((SmileByte)argv[0])->value;
			i = 0;
			y = ((SmileByte)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = ((SmileByte)argv[2])->value;
			if (y < x) i = 2, x = y;
			return argv[i];

		case 4:
			x = ((SmileByte)argv[0])->value;
			i = 0;
			y = ((SmileByte)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = ((SmileByte)argv[2])->value;
			if (y < x) i = 2, x = y;
			y = ((SmileByte)argv[3])->value;
			if (y < x) i = 3, x = y;
			return argv[i];

		default:
			x = ((SmileByte)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = ((SmileByte)argv[i])->value;
				if (y < x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject UMin(Int argc, SmileObject *argv, void *param)
{
	Byte x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (Byte)((SmileByte)argv[0])->value;
			i = 0;
			y = (Byte)((SmileByte)argv[1])->value;
			if (y < x) i = 1;
			return argv[i];

		case 3:
			x = (Byte)((SmileByte)argv[0])->value;
			i = 0;
			y = (Byte)((SmileByte)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = (Byte)((SmileByte)argv[2])->value;
			if (y < x) i = 2, x = y;
			return argv[i];

		case 4:
			x = (Byte)((SmileByte)argv[0])->value;
			i = 0;
			y = (Byte)((SmileByte)argv[1])->value;
			if (y < x) i = 1, x = y;
			y = (Byte)((SmileByte)argv[2])->value;
			if (y < x) i = 2, x = y;
			y = (Byte)((SmileByte)argv[3])->value;
			if (y < x) i = 3, x = y;
			return argv[i];

		default:
			x = (Byte)((SmileByte)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = (Byte)((SmileByte)argv[i])->value;
				if (y < x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject Max(Int argc, SmileObject *argv, void *param)
{
	SByte x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = ((SmileByte)argv[0])->value;
			i = 0;
			y = ((SmileByte)argv[1])->value;
			if (y > x) i = 1;
			return argv[i];

		case 3:
			x = ((SmileByte)argv[0])->value;
			i = 0;
			y = ((SmileByte)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = ((SmileByte)argv[2])->value;
			if (y > x) i = 2, x = y;
			return argv[i];

		case 4:
			x = ((SmileByte)argv[0])->value;
			i = 0;
			y = ((SmileByte)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = ((SmileByte)argv[2])->value;
			if (y > x) i = 2, x = y;
			y = ((SmileByte)argv[3])->value;
			if (y > x) i = 3, x = y;
			return argv[i];

		default:
			x = ((SmileByte)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = ((SmileByte)argv[i])->value;
				if (y > x) j = i, x = y;
			}
			return argv[j];
	}
}

static SmileObject UMax(Int argc, SmileObject *argv, void *param)
{
	Byte x, y;
	Int i, j;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = (Byte)((SmileByte)argv[0])->value;
			i = 0;
			y = (Byte)((SmileByte)argv[1])->value;
			if (y > x) i = 1;
			return argv[i];

		case 3:
			x = (Byte)((SmileByte)argv[0])->value;
			i = 0;
			y = (Byte)((SmileByte)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = (Byte)((SmileByte)argv[2])->value;
			if (y > x) i = 2, x = y;
			return argv[i];

		case 4:
			x = (Byte)((SmileByte)argv[0])->value;
			i = 0;
			y = (Byte)((SmileByte)argv[1])->value;
			if (y > x) i = 1, x = y;
			y = (Byte)((SmileByte)argv[2])->value;
			if (y > x) i = 2, x = y;
			y = (Byte)((SmileByte)argv[3])->value;
			if (y > x) i = 3, x = y;
			return argv[i];

		default:
			x = (Byte)((SmileByte)argv[0])->value;
			j = 0;
			for (i = 1; i < argc; i++) {
				y = (Byte)((SmileByte)argv[i])->value;
				if (y > x) j = i, x = y;
			}
			return argv[j];
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

static SmileObject Power(Int argc, SmileObject *argv, void *param)
{
	SByte x, y;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileByte_Create(x);

		case 3:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileByte)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			if ((y = ((SmileByte)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileByte)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			if ((y = ((SmileByte)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = IntPower(x, y);
			return (SmileObject)SmileByte_Create(x);

		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileByte)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = IntPower(x, y);
			}
			return (SmileObject)SmileByte_Create(x);
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

static SmileObject Sqrt(Int argc, SmileObject *argv, void *param)
{
	SByte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	if (value < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);

	return (SmileObject)SmileByte_Create(IntSqrt(value));
}

static SmileObject Pow2Q(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value > 0 && (value & (value - 1)) == 0 ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject NextPow2(Int argc, SmileObject *argv, void *param)
{
	SByte value = ((SmileByte)argv[0])->value;
	Byte uvalue = (Byte)value;

	UNUSED(argc);
	UNUSED(param);

	if (value <= 0) return (SmileObject)Smile_KnownObjects.OneByte;

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue++;

	return (SmileObject)SmileByte_Create((Byte)uvalue);
}

static SmileObject IntLg(Int argc, SmileObject *argv, void *param)
{
	SByte value = ((SmileByte)argv[0])->value;
	Byte uvalue = (Byte)value;
	Byte log;

	UNUSED(argc);
	UNUSED(param);

	if (value <= 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeLog);

	log = 0;
	if ((uvalue & 0x000000F0) != 0) uvalue >>= 4, log += 4;
	if ((uvalue & 0x0000000C) != 0) uvalue >>= 2, log += 2;
	if ((uvalue & 0x00000002) != 0) uvalue >>= 1, log += 1;

	return (SmileObject)SmileByte_Create((Byte)log);
}

//-------------------------------------------------------------------------------------------------
// Bitwise operators

static SmileObject BitAnd(Int argc, SmileObject *argv, void *param)
{
	Byte x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileByte)argv[0])->value;
			x &= ((SmileByte)argv[1])->value;
			return (SmileObject)SmileByte_Create(x);
		
		case 3:
			x = ((SmileByte)argv[0])->value;
			x &= ((SmileByte)argv[1])->value;
			x &= ((SmileByte)argv[2])->value;
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			x &= ((SmileByte)argv[1])->value;
			x &= ((SmileByte)argv[2])->value;
			x &= ((SmileByte)argv[3])->value;
			return (SmileObject)SmileByte_Create(x);
		
		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x &= ((SmileByte)argv[i])->value;
			}
			return (SmileObject)SmileByte_Create(x);
	}
}

static SmileObject BitOr(Int argc, SmileObject *argv, void *param)
{
	Byte x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileByte)argv[0])->value;
			x |= ((SmileByte)argv[1])->value;
			return (SmileObject)SmileByte_Create(x);
		
		case 3:
			x = ((SmileByte)argv[0])->value;
			x |= ((SmileByte)argv[1])->value;
			x |= ((SmileByte)argv[2])->value;
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			x |= ((SmileByte)argv[1])->value;
			x |= ((SmileByte)argv[2])->value;
			x |= ((SmileByte)argv[3])->value;
			return (SmileObject)SmileByte_Create(x);
		
		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x |= ((SmileByte)argv[i])->value;
			}
			return (SmileObject)SmileByte_Create(x);
	}
}

static SmileObject BitXor(Int argc, SmileObject *argv, void *param)
{
	Byte x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileByte)argv[0])->value;
			x ^= ((SmileByte)argv[1])->value;
			return (SmileObject)SmileByte_Create(x);
		
		case 3:
			x = ((SmileByte)argv[0])->value;
			x ^= ((SmileByte)argv[1])->value;
			x ^= ((SmileByte)argv[2])->value;
			return (SmileObject)SmileByte_Create(x);

		case 4:
			x = ((SmileByte)argv[0])->value;
			x ^= ((SmileByte)argv[1])->value;
			x ^= ((SmileByte)argv[2])->value;
			x ^= ((SmileByte)argv[3])->value;
			return (SmileObject)SmileByte_Create(x);
		
		default:
			x = ((SmileByte)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x ^= ((SmileByte)argv[i])->value;
			}
			return (SmileObject)SmileByte_Create(x);
	}
}

static SmileObject BitNot(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value < 0 ? (SmileObject)SmileByte_Create(~value) : argv[0];
}

//-------------------------------------------------------------------------------------------------
// Shift/rotate operators

static SmileObject LogicalShiftLeft(Int argc, SmileObject *argv, void *param)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create(x << y);
}

static SmileObject LogicalShiftRight(Int argc, SmileObject *argv, void *param)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create(x >> y);
}

static SmileObject ArithmeticShiftLeft(Int argc, SmileObject *argv, void *param)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create((Byte)(x << y));
}

static SmileObject ArithmeticShiftRight(Int argc, SmileObject *argv, void *param)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create((Byte)(x >> y));
}

static SmileObject RotateLeft(Int argc, SmileObject *argv, void *param)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create((Byte)Smile_RotateLeft8(x, y));
}

static SmileObject RotateRight(Int argc, SmileObject *argv, void *param)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create((Byte)Smile_RotateRight8(x, y));
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

static SmileObject CountOnes(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create((Byte)CountBitsSet(value));
}

static SmileObject CountZeros(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create((Byte)CountBitsSet(~value));
}

static SmileObject Parity(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	value ^= value >> 4;
	value &= 0xF;
	value = (0x6996 >> value) & 1;

	return (SmileObject)SmileByte_Create(value);
}

static SmileObject ReverseBits(Int argc, SmileObject *argv, void *param)
{
	Byte value = (Byte)((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create(ComputeReverseBits(value));
}

static SmileObject ReverseBytes(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return argv[0];
}

static SmileObject CountRightZeros(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create(ComputeCountOfRightZeros(value));
}

static SmileObject CountRightOnes(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create(ComputeCountOfRightZeros(~value));
}

static SmileObject CountLeftZeros(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create(ComputeCountOfRightZeros(ComputeReverseBits(value)));
}

static SmileObject CountLeftOnes(Int argc, SmileObject *argv, void *param)
{
	Byte value = ((SmileByte)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileByte_Create(ComputeCountOfRightZeros(~ComputeReverseBits(value)));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

static SmileObject Eq(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_BYTE
		|| ((SmileByte)argv[0])->value != ((SmileByte)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject Ne(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_BYTE
		|| ((SmileByte)argv[0])->value != ((SmileByte)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.TrueObj;

	return (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Lt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SByte)((SmileByte)argv[0])->value < (SByte)((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Gt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SByte)((SmileByte)argv[0])->value > (SByte)((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Le(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SByte)((SmileByte)argv[0])->value <= (SByte)((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Ge(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return (SByte)((SmileByte)argv[0])->value >= (SByte)((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject ULt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileByte)argv[0])->value < ((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject UGt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileByte)argv[0])->value > ((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject ULe(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileByte)argv[0])->value <= ((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject UGe(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileByte)argv[0])->value >= ((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Compare(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	if (x == y)
		return (SmileObject)Smile_KnownObjects.ZeroByte;
	else if (x < y)
		return (SmileObject)Smile_KnownObjects.NegOneByte;
	else
		return (SmileObject)Smile_KnownObjects.OneByte;
}

static SmileObject UCompare(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	if (x == y)
		return (SmileObject)Smile_KnownObjects.ZeroByte;
	else if (x < y)
		return (SmileObject)Smile_KnownObjects.NegOneByte;
	else
		return (SmileObject)Smile_KnownObjects.OneByte;
}

//-------------------------------------------------------------------------------------------------

void SmileByte_Setup(SmileUserObject base)
{
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
	SetupFunction("~+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("~-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("~*", UStar, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("/", Slash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("~/", USlash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("div", Div, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("udiv", USlash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("mod", Mod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("umod", UMod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("rem", Rem, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("urem", UMod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _byteChecks);
	SetupFunction("uclip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _byteChecks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("umin", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("umax", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("^", Power, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("sqrt", Sqrt, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("pow2?", Pow2Q, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("next-pow2", NextPow2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("int-lg", IntLg, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);

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
	SetupFunction("~<", ULt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("~>", UGt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("~<=", ULe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("~>=", UGe, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("ucompare", UCompare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
}
