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
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _real64Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
};

static Byte _real64ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
	0, 0,
};

typedef struct MathInfoStruct {
	Bool isLoud;
} *MathInfo;

static struct MathInfoStruct _loudMath[] = { True };
static struct MathInfoStruct _quietMath[] = { False };

STATIC_STRING(_divideByZero, "Divide by zero error");
STATIC_STRING(_negativeSqrt, "Square root of negative number");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL64)
		return SmileUnboxedBool_From(!Real64_IsZero(argv[0].unboxed.r64));

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL64)
		return SmileUnboxedInteger64_From(Real64_ToInt64(argv[0].unboxed.r64));

	return SmileUnboxedReal64_From(Real64_Zero);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(real64, "Real64");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL64) {
		return SmileArg_From((SmileObject)(Real64_ToStringEx(argv[0].unboxed.r64, 0, 0, False)));
	}

	return SmileArg_From((SmileObject)real64);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileReal64 obj = (SmileReal64)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_REAL64)
		return SmileUnboxedInteger64_From((UInt32)(*(UInt64 *)&obj->value ^ (*(UInt64 *)&obj->value >> 32)));

	return SmileUnboxedInteger64_From((UInt32)((PtrInt)obj ^ Smile_HashOracle));
}

//-------------------------------------------------------------------------------------------------
// Basic arithmetic

SMILE_EXTERNAL_FUNCTION(Plus)
{
	Real64 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.r64;
			x = Real64_Add(x, argv[1].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		case 3:
			x = argv[0].unboxed.r64;
			x = Real64_Add(x, argv[1].unboxed.r64);
			x = Real64_Add(x, argv[2].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		case 4:
			x = argv[0].unboxed.r64;
			x = Real64_Add(x, argv[1].unboxed.r64);
			x = Real64_Add(x, argv[2].unboxed.r64);
			x = Real64_Add(x, argv[3].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		default:
			x = argv[0].unboxed.r64;
			for (i = 1; i < argc; i++) {
				x = Real64_Add(x, argv[i].unboxed.r64);
			}
			return SmileUnboxedReal64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	Real64 x;
	Int i;

	switch (argc) {
		case 1:
			x = argv[0].unboxed.r64;
			x = Real64_Neg(x);
			return SmileUnboxedReal64_From(x);

		case 2:
			x = argv[0].unboxed.r64;
			x = Real64_Sub(x, argv[1].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		case 3:
			x = argv[0].unboxed.r64;
			x = Real64_Sub(x, argv[1].unboxed.r64);
			x = Real64_Sub(x, argv[2].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		case 4:
			x = argv[0].unboxed.r64;
			x = Real64_Sub(x, argv[1].unboxed.r64);
			x = Real64_Sub(x, argv[2].unboxed.r64);
			x = Real64_Sub(x, argv[3].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		default:
			x = argv[0].unboxed.r64;
			for (i = 1; i < argc; i++) {
				x = Real64_Sub(x, argv[i].unboxed.r64);
			}
			return SmileUnboxedReal64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Star)
{
	Real64 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.r64;
			x = Real64_Mul(x, argv[1].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		case 3:
			x = argv[0].unboxed.r64;
			x = Real64_Mul(x, argv[1].unboxed.r64);
			x = Real64_Mul(x, argv[2].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		case 4:
			x = argv[0].unboxed.r64;
			x = Real64_Mul(x, argv[1].unboxed.r64);
			x = Real64_Mul(x, argv[2].unboxed.r64);
			x = Real64_Mul(x, argv[3].unboxed.r64);
			return SmileUnboxedReal64_From(x);

		default:
			x = argv[0].unboxed.r64;
			for (i = 1; i < argc; i++) {
				x = Real64_Mul(x, argv[i].unboxed.r64);
			}
			return SmileUnboxedReal64_From(x);
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

	return SmileUnboxedReal64_From(Real64_Zero);
}

SMILE_EXTERNAL_FUNCTION(Slash)
{
	Real64 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.r64;
			if (Real64_IsZero(y = argv[1].unboxed.r64))
				return DivideByZero(param);
			x = Real64_Div(x, y);
			return SmileUnboxedReal64_From(x);

		case 3:
			x = argv[0].unboxed.r64;
			if (Real64_IsZero(y = argv[1].unboxed.r64))
				return DivideByZero(param);
			x = Real64_Div(x, y);
			if (Real64_IsZero(y = argv[2].unboxed.r64))
				return DivideByZero(param);
			x = Real64_Div(x, y);
			return SmileUnboxedReal64_From(x);

		case 4:
			x = argv[0].unboxed.r64;
			if (Real64_IsZero(y = argv[1].unboxed.r64))
				return DivideByZero(param);
			x = Real64_Div(x, y);
			if (Real64_IsZero(y = argv[2].unboxed.r64))
				return DivideByZero(param);
			x = Real64_Div(x, y);
			if (Real64_IsZero(y = argv[3].unboxed.r64))
				return DivideByZero(param);
			x = Real64_Div(x, y);
			return SmileUnboxedReal64_From(x);

		default:
			x = argv[0].unboxed.r64;
			for (i = 1; i < argc; i++) {
				if (Real64_IsZero(y = argv[i].unboxed.r64))
					return DivideByZero(param);
				x = Real64_Div(x, y);
			}
			return SmileUnboxedReal64_From(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Real64 MathematiciansModulus(Real64 x, Real64 y)
{
	Real64 rem;

	if (Real64_IsNeg(x)) {
		if (Real64_IsNeg(y))
			return Real64_Neg(Real64_Mod(Real64_Neg(x), Real64_Neg(y)));
		else {
			rem = Real64_Mod(Real64_Neg(x), y);
			return !Real64_IsZero(rem) ? Real64_Sub(y, rem) : Real64_Zero;
		}
	}
	else if (Real64_IsNeg(y)) {
		rem = Real64_Mod(x, Real64_Neg(y));
		return !Real64_IsZero(rem) ? Real64_Add(y, rem) : Real64_Zero;
	}
	else
		return Real64_Mod(x, y);
}

/// <summary>
/// Perform remainder, in which the result has the same sign as the dividend (x).
/// </summary>
Inline Real64 MathematiciansRemainder(Real64 x, Real64 y)
{
	Real64 rem;

	if (Real64_IsNeg(x)) {
		if (Real64_IsNeg(y)) {
			rem = Real64_Mod(Real64_Neg(x), Real64_Neg(y));
			return !Real64_IsZero(rem) ? Real64_Add(rem, y) : Real64_Zero;
		}
		else
			return Real64_Neg(Real64_Mod(Real64_Neg(x), y));
	}
	else if (Real64_IsNeg(y))
		return Real64_Mod(x, Real64_Neg(y));
	else {
		rem = Real64_Mod(x, y);
		return !Real64_IsZero(rem) ? Real64_Sub(rem, y) : Real64_Zero;
	}
}

SMILE_EXTERNAL_FUNCTION(Mod)
{
	Real64 x = argv[0].unboxed.r64;
	Real64 y = argv[1].unboxed.r64;

	if (Real64_IsZero(y))
		return DivideByZero(param);

	return SmileUnboxedReal64_From(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	Real64 x = argv[0].unboxed.r64;
	Real64 y = argv[1].unboxed.r64;

	if (Real64_IsZero(y))
		return DivideByZero(param);

	return SmileUnboxedReal64_From(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	return SmileUnboxedReal64_From(Real64_Sign(argv[0].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
	return SmileUnboxedReal64_From(Real64_Abs(argv[0].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Clip)
{
	Real64 value = argv[0].unboxed.r64;
	Real64 min = argv[1].unboxed.r64;
	Real64 max = argv[2].unboxed.r64;

	if (Real64_Gt(value, max)) {
		value = max;
		return Real64_Lt(value, min) ? argv[1] : argv[2];
	}
	else if (Real64_Lt(value, min)) {
		return argv[1];
	}
	else return argv[0];
}

SMILE_EXTERNAL_FUNCTION(Min)
{
	Real64 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.r64;
			if (Real64_Lt((y = argv[1].unboxed.r64), x)) x = y;
			return SmileUnboxedReal64_From(x);

		case 3:
			x = argv[0].unboxed.r64;
			if (Real64_Lt((y = argv[1].unboxed.r64), x)) x = y;
			if (Real64_Lt((y = argv[2].unboxed.r64), x)) x = y;
			return SmileUnboxedReal64_From(x);

		case 4:
			x = argv[0].unboxed.r64;
			if (Real64_Lt((y = argv[1].unboxed.r64), x)) x = y;
			if (Real64_Lt((y = argv[2].unboxed.r64), x)) x = y;
			if (Real64_Lt((y = argv[3].unboxed.r64), x)) x = y;
			return SmileUnboxedReal64_From(x);

		default:
			x = argv[0].unboxed.r64;
			for (i = 1; i < argc; i++) {
				if (Real64_Lt((y = argv[i].unboxed.r64), x)) x = y;
			}
			return SmileUnboxedReal64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Max)
{
	Real64 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.r64;
			if (Real64_Gt((y = argv[1].unboxed.r64), x)) x = y;
			return SmileUnboxedReal64_From(x);

		case 3:
			x = argv[0].unboxed.r64;
			if (Real64_Gt((y = argv[1].unboxed.r64), x)) x = y;
			if (Real64_Gt((y = argv[2].unboxed.r64), x)) x = y;
			return SmileUnboxedReal64_From(x);

		case 4:
			x = argv[0].unboxed.r64;
			if (Real64_Gt((y = argv[1].unboxed.r64), x)) x = y;
			if (Real64_Gt((y = argv[2].unboxed.r64), x)) x = y;
			if (Real64_Gt((y = argv[3].unboxed.r64), x)) x = y;
			return SmileUnboxedReal64_From(x);

		default:
			x = argv[0].unboxed.r64;
			for (i = 1; i < argc; i++) {
				if (Real64_Gt((y = argv[i].unboxed.r64), x)) x = y;
			}
			return SmileUnboxedReal64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Sqrt)
{
	Real64 value = argv[0].unboxed.r64;

	if (Real64_IsNeg(value)) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);
		return SmileUnboxedReal64_From(Real64_Zero);
	}

	return SmileUnboxedReal64_From(Real64_Sqrt(value));
}

//-------------------------------------------------------------------------------------------------
// Rounding/truncation operations

SMILE_EXTERNAL_FUNCTION(Ceil)
{
	return SmileUnboxedReal64_From(Real64_Ceil(argv[0].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Floor)
{
	return SmileUnboxedReal64_From(Real64_Floor(argv[0].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Trunc)
{
	return SmileUnboxedReal64_From(Real64_Trunc(argv[0].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Round)
{
	return SmileUnboxedReal64_From(Real64_Round(argv[0].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(BankRound)
{
	return SmileUnboxedReal64_From(Real64_BankRound(argv[0].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Modf)
{
	Real64 intPart, fracPart;
	SmileList list;

	fracPart = Real64_Modf(argv[0].unboxed.r64, &intPart);

	list = SmileList_Cons((SmileObject)SmileReal64_Create(intPart),
		(SmileObject)SmileList_Cons((SmileObject)SmileReal64_Create(fracPart),
		NullObject));

	return SmileArg_From((SmileObject)list);
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_REAL64
		&& Real64_Eq(argv[0].unboxed.r64, argv[1].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_REAL64
		|| Real64_Ne(argv[0].unboxed.r64, argv[1].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(Real64_Lt(argv[0].unboxed.r64, argv[1].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(Real64_Gt(argv[0].unboxed.r64, argv[1].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(Real64_Le(argv[0].unboxed.r64, argv[1].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(Real64_Ge(argv[0].unboxed.r64, argv[1].unboxed.r64));
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Real64 x = argv[0].unboxed.r64;
	Real64 y = argv[1].unboxed.r64;

	if (Real64_Lt(x, y))
		return SmileUnboxedInteger64_From(-1);
	else if (Real64_Gt(x, y))
		return SmileUnboxedInteger64_From(+1);
	else
		return SmileUnboxedInteger64_From(0);
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
	INF_TEST,
	NAN_TEST,
	FINITE_TEST,
};

Inline Bool IsPositive(Real64 value)
{
	return (Real64_IsInf(value) && !Real64_IsNeg(value))
		|| (Real64_IsFinite(value) && !Real64_IsNeg(value) && !Real64_IsZero(value));
}

Inline Bool IsNegative(Real64 value)
{
	return (Real64_IsInf(value) && Real64_IsNeg(value))
		|| (Real64_IsFinite(value) && Real64_IsNeg(value));
}

SMILE_EXTERNAL_FUNCTION(ValueTest)
{
	Real64 value = argv[0].unboxed.r64;

	switch ((PtrInt)param) {
		case ZERO_TEST:
			return SmileUnboxedBool_From(Real64_IsZero(value));
		case ONE_TEST:
			return SmileUnboxedBool_From(Real64_Eq(value, Real64_One));
		case NONZERO_TEST:
			return SmileUnboxedBool_From(!Real64_IsZero(value));
		case POS_TEST:
			return SmileUnboxedBool_From(IsPositive(value));
		case NONPOS_TEST:
			return SmileUnboxedBool_From(!IsPositive(value));
		case NEG_TEST:
			return SmileUnboxedBool_From(IsNegative(value));
		case NONNEG_TEST:
			return SmileUnboxedBool_From(!IsNegative(value));
		case ODD_TEST:
			return SmileUnboxedBool_From(Real64_Eq(Real64_Mod(value, Real64_Two), Real64_One));
		case EVEN_TEST:
			return SmileUnboxedBool_From(Real64_IsZero(Real64_Mod(value, Real64_Two)));
		case INF_TEST:
			return SmileUnboxedBool_From(Real64_IsInf(value));
		case NAN_TEST:
			return SmileUnboxedBool_From(Real64_IsNaN(value));
		case FINITE_TEST:
			return SmileUnboxedBool_From(Real64_IsFinite(value));
		default:
			return SmileArg_From(NullObject);
	}
}

//-------------------------------------------------------------------------------------------------

void SmileReal64_Setup(SmileUserObject base)
{
	SmileUnboxedReal64_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _real64Checks);
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _real64Checks);
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _real64Checks);
	SetupFunction("/", Slash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _real64Checks);
	SetupFunction("/!", Slash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _real64Checks);
	SetupSynonym("/", "div");
	SetupSynonym("/!", "div!");
	SetupFunction("mod", Mod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);
	SetupFunction("mod!", Mod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);
	SetupFunction("rem", Rem, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);
	SetupFunction("rem!", Rem, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);
	SetupFunction("sqrt", Sqrt, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("sqrt!", Sqrt, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _real64Checks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _real64Checks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _real64Checks);

	SetupFunction("ceil", Ceil, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("floor", Floor, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("trunc", Trunc, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("round", Round, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("bank-round", BankRound, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("modf", Modf, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);

	SetupFunction("odd?", ValueTest, (void *)ODD_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("even?", ValueTest, (void *)EVEN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("zero?", ValueTest, (void *)ZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("one?", ValueTest, (void *)ONE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("nonzero?", ValueTest, (void *)NONZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("positive?", ValueTest, (void *)POS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupSynonym("positive?", "pos?");
	SetupFunction("finite?", ValueTest, (void *)FINITE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("infinite?", ValueTest, (void *)INF_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupSynonym("infinite?", "inf?");
	SetupFunction("nan?", ValueTest, (void *)NAN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupFunction("nonpositive?", ValueTest, (void *)NONPOS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupSynonym("nonpositive?", "nonpos?");
	SetupFunction("negative?", ValueTest, (void *)NEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupSynonym("negative?", "neg?");
	SetupFunction("nonnegative?", ValueTest, (void *)NONNEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real64Checks);
	SetupSynonym("nonnegative?", "nonneg?");

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64ComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64ComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real64Checks);
	SetupSynonym("compare", "cmp");

	SetupData("inf", SmileReal64_Create(Real64_Inf));
}
