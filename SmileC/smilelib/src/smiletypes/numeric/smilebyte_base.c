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

SMILE_IGNORE_UNUSED_VARIABLES

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

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0]) == SMILE_KIND_BYTE)
		return ((SmileByte)argv[0])->value ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0]) == SMILE_KIND_BYTE)
		return (SmileObject)SmileInteger64_Create(((SmileByte)argv[0])->value);

	return (SmileObject)Smile_KnownObjects.ZeroInt64;
}

STATIC_STRING(_Integer32, "Integer32");

SMILE_EXTERNAL_FUNCTION(ToString)
{
	Int64 numericBase;

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

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0]) == SMILE_KIND_BYTE)
		return (SmileObject)SmileInteger64_Create(((SmileByte)argv[0])->value);

	return (SmileObject)SmileInteger64_Create(((PtrInt)argv[0]) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return (SmileObject)SmileInteger64_Create(((SmileByte)argv[0])->value);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return (SmileObject)SmileInteger32_Create(((SmileByte)argv[0])->value);
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return (SmileObject)SmileInteger16_Create(((SmileByte)argv[0])->value);
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

SMILE_EXTERNAL_FUNCTION(Plus)
{
	Byte x;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(Minus)
{
	Byte x;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(Star)
{
	SByte x;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(UStar)
{
	Byte x;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(Slash)
{
	SByte x, y;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(USlash)
{
	Byte x, y;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(Div)
{
	SByte x, y;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(Mod)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileByte_Create(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(UMod)
{
	Byte x = (Byte)((SmileByte)argv[0])->value;
	Byte y = (Byte)((SmileByte)argv[1])->value;

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileByte_Create((Byte)(x % y));
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileByte_Create(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	SByte value = ((SmileByte)argv[0])->value;

	return value == 0 ? (SmileObject)Smile_KnownObjects.ZeroByte
		: value > 0 ? (SmileObject)Smile_KnownObjects.OneByte
		: (SmileObject)Smile_KnownObjects.NegOneByte;
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
	SByte value = ((SmileByte)argv[0])->value;

	return value < 0 ? (SmileObject)SmileByte_Create(-value) : argv[0];
}

SMILE_EXTERNAL_FUNCTION(Clip)
{
	SByte value = ((SmileByte)argv[0])->value;
	SByte min = ((SmileByte)argv[1])->value;
	SByte max = ((SmileByte)argv[2])->value;

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
	Byte value = (Byte)((SmileByte)argv[0])->value;
	Byte min = (Byte)((SmileByte)argv[1])->value;
	Byte max = (Byte)((SmileByte)argv[2])->value;

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
	Int i, j;

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

SMILE_EXTERNAL_FUNCTION(UMin)
{
	Byte x, y;
	Int i, j;

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

SMILE_EXTERNAL_FUNCTION(Max)
{
	SByte x, y;
	Int i, j;

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

SMILE_EXTERNAL_FUNCTION(UMax)
{
	Byte x, y;
	Int i, j;

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

SMILE_EXTERNAL_FUNCTION(Power)
{
	SByte x, y;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(Sqrt)
{
	SByte value = ((SmileByte)argv[0])->value;

	if (value < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);

	return (SmileObject)SmileByte_Create(IntSqrt(value));
}

SMILE_EXTERNAL_FUNCTION(Pow2Q)
{
	Byte value = ((SmileByte)argv[0])->value;

	return value > 0 && (value & (value - 1)) == 0 ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(NextPow2)
{
	SByte value = ((SmileByte)argv[0])->value;
	Byte uvalue = (Byte)value;

	if (value <= 0) return (SmileObject)Smile_KnownObjects.OneByte;

	uvalue--;
	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue++;

	return (SmileObject)SmileByte_Create((Byte)uvalue);
}

SMILE_EXTERNAL_FUNCTION(IntLg)
{
	SByte value = ((SmileByte)argv[0])->value;
	Byte uvalue = (Byte)value;
	Byte log;

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

SMILE_EXTERNAL_FUNCTION(BitAnd)
{
	Byte x;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(BitOr)
{
	Byte x;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(BitXor)
{
	Byte x;
	Int i;

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

SMILE_EXTERNAL_FUNCTION(BitNot)
{
	Byte value = ((SmileByte)argv[0])->value;

	return value < 0 ? (SmileObject)SmileByte_Create(~value) : argv[0];
}

//-------------------------------------------------------------------------------------------------
// Shift/rotate operators

SMILE_EXTERNAL_FUNCTION(LogicalShiftLeft)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	return (SmileObject)SmileByte_Create(x << y);
}

SMILE_EXTERNAL_FUNCTION(LogicalShiftRight)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	return (SmileObject)SmileByte_Create(x >> y);
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftLeft)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	return (SmileObject)SmileByte_Create((Byte)(x << y));
}

SMILE_EXTERNAL_FUNCTION(ArithmeticShiftRight)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	return (SmileObject)SmileByte_Create((Byte)(x >> y));
}

SMILE_EXTERNAL_FUNCTION(RotateLeft)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	return (SmileObject)SmileByte_Create((Byte)Smile_RotateLeft8(x, y));
}

SMILE_EXTERNAL_FUNCTION(RotateRight)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

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

SMILE_EXTERNAL_FUNCTION(CountOnes)
{
	Byte value = ((SmileByte)argv[0])->value;

	return (SmileObject)SmileByte_Create((Byte)CountBitsSet(value));
}

SMILE_EXTERNAL_FUNCTION(CountZeros)
{
	Byte value = ((SmileByte)argv[0])->value;

	return (SmileObject)SmileByte_Create((Byte)CountBitsSet(~value));
}

SMILE_EXTERNAL_FUNCTION(Parity)
{
	Byte value = ((SmileByte)argv[0])->value;

	value ^= value >> 4;
	value &= 0xF;
	value = (0x6996 >> value) & 1;

	return (SmileObject)SmileByte_Create(value);
}

SMILE_EXTERNAL_FUNCTION(ReverseBits)
{
	Byte value = (Byte)((SmileByte)argv[0])->value;

	return (SmileObject)SmileByte_Create(ComputeReverseBits(value));
}

SMILE_EXTERNAL_FUNCTION(ReverseBytes)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(CountRightZeros)
{
	Byte value = ((SmileByte)argv[0])->value;

	return (SmileObject)SmileByte_Create(ComputeCountOfRightZeros(value));
}

SMILE_EXTERNAL_FUNCTION(CountRightOnes)
{
	Byte value = ((SmileByte)argv[0])->value;

	return (SmileObject)SmileByte_Create(ComputeCountOfRightZeros(~value));
}

SMILE_EXTERNAL_FUNCTION(CountLeftZeros)
{
	Byte value = ((SmileByte)argv[0])->value;

	return (SmileObject)SmileByte_Create(ComputeCountOfRightZeros(ComputeReverseBits(value)));
}

SMILE_EXTERNAL_FUNCTION(CountLeftOnes)
{
	Byte value = ((SmileByte)argv[0])->value;

	return (SmileObject)SmileByte_Create(ComputeCountOfRightZeros(~ComputeReverseBits(value)));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	if (SMILE_KIND(argv[1]) != SMILE_KIND_BYTE
		|| ((SmileByte)argv[0])->value != ((SmileByte)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	if (SMILE_KIND(argv[1]) != SMILE_KIND_BYTE
		|| ((SmileByte)argv[0])->value != ((SmileByte)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.TrueObj;

	return (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return (SByte)((SmileByte)argv[0])->value < (SByte)((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return (SByte)((SmileByte)argv[0])->value > (SByte)((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return (SByte)((SmileByte)argv[0])->value <= (SByte)((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return (SByte)((SmileByte)argv[0])->value >= (SByte)((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(ULt)
{
	return ((SmileByte)argv[0])->value < ((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(UGt)
{
	return ((SmileByte)argv[0])->value > ((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(ULe)
{
	return ((SmileByte)argv[0])->value <= ((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(UGe)
{
	return ((SmileByte)argv[0])->value >= ((SmileByte)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	SByte x = ((SmileByte)argv[0])->value;
	SByte y = ((SmileByte)argv[1])->value;

	if (x == y)
		return (SmileObject)Smile_KnownObjects.ZeroInt64;
	else if (x < y)
		return (SmileObject)Smile_KnownObjects.NegOneInt64;
	else
		return (SmileObject)Smile_KnownObjects.OneInt64;
}

SMILE_EXTERNAL_FUNCTION(UCompare)
{
	Byte x = ((SmileByte)argv[0])->value;
	Byte y = ((SmileByte)argv[1])->value;

	if (x == y)
		return (SmileObject)Smile_KnownObjects.ZeroInt64;
	else if (x < y)
		return (SmileObject)Smile_KnownObjects.NegOneInt64;
	else
		return (SmileObject)Smile_KnownObjects.OneInt64;
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
	SetupSynonym("+", "+~");
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupSynonym("-", "-~");
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("*~", UStar, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("/", Slash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("/~", USlash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("div", Div, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("div~", USlash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _byteChecks);
	SetupFunction("mod", Mod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("mod~", UMod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("rem", Rem, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);
	SetupFunction("rem~", UMod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _byteChecks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _byteChecks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _byteChecks);
	SetupFunction("clip~", UClip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _byteChecks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("min~", UMin, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
	SetupFunction("max~", UMax, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _byteChecks);
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
