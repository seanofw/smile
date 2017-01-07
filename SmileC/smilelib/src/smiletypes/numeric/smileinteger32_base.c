//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/smilefunction.h>

#define Setup(__name__, __value__) \
	(SmileUserObject_QuickSet(base, (__name__), (__value__)))

#define SetupFunction(__name__, __function__, __param__, __argNames__, __argCheckFlags__, __minArgs__, __maxArgs__, __numArgsToTypeCheck__, __argTypeChecks__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), (__argCheckFlags__), (__minArgs__), (__maxArgs__), (__numArgsToTypeCheck__), (__argTypeChecks__))))

#define SetupSimpleFunction(__name__, __function__, __argNames__, __numArgs__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), ARG_CHECK_EXACT, (__numArgs__), (__numArgs__), NULL)))

static Byte _integer32Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
};

static Byte _integer32ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER32,
	0, 0,
};

STATIC_STRING(_divideByZero, "Divide by zero error");

STATIC_STRING(_invalidTypeError, "All arguments to 'Integer32.%s' must be of type 'Integer32'.");

static SmileObject Plus(Int argc, SmileObject *argv, void *param)
{
	Int32 x, i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger32)argv[0])->value;
			x += ((SmileInteger32)argv[1])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		case 3:
			x = ((SmileInteger32)argv[0])->value;
			x += ((SmileInteger32)argv[1])->value;
			x += ((SmileInteger32)argv[2])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			x += ((SmileInteger32)argv[1])->value;
			x += ((SmileInteger32)argv[2])->value;
			x += ((SmileInteger32)argv[3])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x += ((SmileInteger32)argv[i])->value;
			}
			return (SmileObject)SmileInteger32_Create(x);
	}
}

static SmileObject Minus(Int argc, SmileObject *argv, void *param)
{
	Int32 x, i;

	UNUSED(param);

	switch (argc) {
		case 1:
			x = ((SmileInteger32)argv[0])->value;
			return (SmileObject)SmileInteger32_Create(-x);

		case 2:
			x = ((SmileInteger32)argv[0])->value;
			x -= ((SmileInteger32)argv[1])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 3:
			x = ((SmileInteger32)argv[0])->value;
			x -= ((SmileInteger32)argv[1])->value;
			x -= ((SmileInteger32)argv[2])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			x -= ((SmileInteger32)argv[1])->value;
			x -= ((SmileInteger32)argv[2])->value;
			x -= ((SmileInteger32)argv[3])->value;
			return (SmileObject)SmileInteger32_Create(x);

		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x -= ((SmileInteger32)argv[i])->value;
			}
			return (SmileObject)SmileInteger32_Create(x);
	}
}

static SmileObject Star(Int argc, SmileObject *argv, void *param)
{
	Int32 x, i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger32)argv[0])->value;
			x *= ((SmileInteger32)argv[1])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 3:
			x = ((SmileInteger32)argv[0])->value;
			x *= ((SmileInteger32)argv[1])->value;
			x *= ((SmileInteger32)argv[2])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			x *= ((SmileInteger32)argv[1])->value;
			x *= ((SmileInteger32)argv[2])->value;
			x *= ((SmileInteger32)argv[3])->value;
			return (SmileObject)SmileInteger32_Create(x);

		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x *= ((SmileInteger32)argv[i])->value;
			}
			return (SmileObject)SmileInteger32_Create(x);
	}
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

static SmileObject Slash(Int argc, SmileObject *argv, void *param)
{
	Int32 x, y, i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger32)argv[0])->value;
			if ((y = ((SmileInteger32)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger32_Create(x);

		case 3:
			x = ((SmileInteger32)argv[0])->value;
			if ((y = ((SmileInteger32)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger32)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			if ((y = ((SmileInteger32)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger32)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			if ((y = ((SmileInteger32)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = MathematiciansDiv(x, y);
			return (SmileObject)SmileInteger32_Create(x);

		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileInteger32)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = MathematiciansDiv(x, y);
			}
			return (SmileObject)SmileInteger32_Create(x);
	}
}

static SmileObject Div(Int argc, SmileObject *argv, void *param)
{
	Int32 x, y, i;

	UNUSED(param);

	switch (argc) {
		case 2:
			x = ((SmileInteger32)argv[0])->value;
			if ((y = ((SmileInteger32)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger32_Create(x);

		case 3:
			x = ((SmileInteger32)argv[0])->value;
			if ((y = ((SmileInteger32)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger32)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			if ((y = ((SmileInteger32)argv[1])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger32)argv[2])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			if ((y = ((SmileInteger32)argv[3])->value) == 0)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
			x = CDiv(x, y);
			return (SmileObject)SmileInteger32_Create(x);

		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				if ((y = ((SmileInteger32)argv[i])->value) == 0)
					Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);
				x = CDiv(x, y);
			}
			return (SmileObject)SmileInteger32_Create(x);
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

static SmileObject Mod(Int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger32_Create(MathematiciansModulus(x, y));
}

static SmileObject Rem(Int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	if (y == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _divideByZero);

	return (SmileObject)SmileInteger32_Create(MathematiciansRemainder(x, y));
}

static SmileObject Sign(Int argc, SmileObject *argv, void *param)
{
	Int32 value = ((SmileInteger32)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value == 0 ? (SmileObject)Smile_KnownObjects.ZeroInt32
		: value > 0 ? (SmileObject)Smile_KnownObjects.OneInt32
		: (SmileObject)Smile_KnownObjects.NegOneInt32;
}

static SmileObject Abs(Int argc, SmileObject *argv, void *param)
{
	Int32 value = ((SmileInteger32)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value < 0 ? (SmileObject)SmileInteger32_Create(-value) : argv[0];
}

static SmileObject BitAnd(Int argc, SmileObject *argv, void *param)
{
	Int32 x, i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger32)argv[0])->value;
			x &= ((SmileInteger32)argv[1])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		case 3:
			x = ((SmileInteger32)argv[0])->value;
			x &= ((SmileInteger32)argv[1])->value;
			x &= ((SmileInteger32)argv[2])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			x &= ((SmileInteger32)argv[1])->value;
			x &= ((SmileInteger32)argv[2])->value;
			x &= ((SmileInteger32)argv[3])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x &= ((SmileInteger32)argv[i])->value;
			}
			return (SmileObject)SmileInteger32_Create(x);
	}
}

static SmileObject BitOr(Int argc, SmileObject *argv, void *param)
{
	Int32 x, i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger32)argv[0])->value;
			x |= ((SmileInteger32)argv[1])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		case 3:
			x = ((SmileInteger32)argv[0])->value;
			x |= ((SmileInteger32)argv[1])->value;
			x |= ((SmileInteger32)argv[2])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			x |= ((SmileInteger32)argv[1])->value;
			x |= ((SmileInteger32)argv[2])->value;
			x |= ((SmileInteger32)argv[3])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x |= ((SmileInteger32)argv[i])->value;
			}
			return (SmileObject)SmileInteger32_Create(x);
	}
}

static SmileObject BitXor(Int argc, SmileObject *argv, void *param)
{
	Int32 x, i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = ((SmileInteger32)argv[0])->value;
			x ^= ((SmileInteger32)argv[1])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		case 3:
			x = ((SmileInteger32)argv[0])->value;
			x ^= ((SmileInteger32)argv[1])->value;
			x ^= ((SmileInteger32)argv[2])->value;
			return (SmileObject)SmileInteger32_Create(x);

		case 4:
			x = ((SmileInteger32)argv[0])->value;
			x ^= ((SmileInteger32)argv[1])->value;
			x ^= ((SmileInteger32)argv[2])->value;
			x ^= ((SmileInteger32)argv[3])->value;
			return (SmileObject)SmileInteger32_Create(x);
		
		default:
			x = ((SmileInteger32)argv[0])->value;
			for (i = 1; i < argc; i++) {
				x ^= ((SmileInteger32)argv[i])->value;
			}
			return (SmileObject)SmileInteger32_Create(x);
	}
}

static SmileObject BitNot(Int argc, SmileObject *argv, void *param)
{
	Int32 value = ((SmileInteger32)argv[0])->value;

	UNUSED(argc);
	UNUSED(param);

	return value < 0 ? (SmileObject)SmileInteger32_Create(~value) : argv[0];
}

static SmileObject LogicalShiftLeft(Int argc, SmileObject *argv, void *param)
{
	UInt32 x = (UInt32)((SmileInteger32)argv[0])->value;
	UInt32 y = (UInt32)((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create((Int32)(x << y));
}

static SmileObject LogicalShiftRight(Int argc, SmileObject *argv, void *param)
{
	UInt32 x = (UInt32)((SmileInteger32)argv[0])->value;
	UInt32 y = (UInt32)((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create((Int32)(x >> y));
}

static SmileObject ArithmeticShiftLeft(Int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create(x << y);
}

static SmileObject ArithmeticShiftRight(Int argc, SmileObject *argv, void *param)
{
	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create(x >> y);
}

static SmileObject RotateLeft(Int argc, SmileObject *argv, void *param)
{
	UInt32 x = (UInt32)((SmileInteger32)argv[0])->value;
	UInt32 y = (UInt32)((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create((Int32)Smile_RotateLeft32(x, y));
}

static SmileObject RotateRight(Int argc, SmileObject *argv, void *param)
{
	UInt32 x = (UInt32)((SmileInteger32)argv[0])->value;
	UInt32 y = (UInt32)((SmileInteger32)argv[1])->value;

	UNUSED(argc);
	UNUSED(param);

	return (SmileObject)SmileInteger32_Create((Int32)Smile_RotateRight32(x, y));
}

static SmileObject Eq(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER32
		|| ((SmileInteger32)argv[0])->value != ((SmileInteger32)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject Ne(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_INTEGER32
		|| ((SmileInteger32)argv[0])->value != ((SmileInteger32)argv[1])->value)
		return (SmileObject)Smile_KnownObjects.TrueObj;

	return (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Lt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger32)argv[0])->value < ((SmileInteger32)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Gt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger32)argv[0])->value > ((SmileInteger32)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Le(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger32)argv[0])->value <= ((SmileInteger32)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Ge(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileInteger32)argv[0])->value >= ((SmileInteger32)argv[1])->value
		? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Compare(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	Int32 x = ((SmileInteger32)argv[0])->value;
	Int32 y = ((SmileInteger32)argv[1])->value;

	if (x == y)
		return (SmileObject)Smile_KnownObjects.ZeroInt32;
	else if (x < y)
		return (SmileObject)Smile_KnownObjects.NegOneInt32;
	else
		return (SmileObject)Smile_KnownObjects.OneInt32;
}

void SmileInteger32_Setup(SmileUserObject base)
{
	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _integer32Checks);
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("/", Slash, NULL, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("div", Div, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 0, 8, _integer32Checks);
	SetupFunction("mod", Mod, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("rem", Rem, NULL, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _integer32Checks);

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

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32ComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32ComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer32Checks);
}
