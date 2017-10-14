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
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _real32Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
};

static Byte _real32ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
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
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL32)
		return SmileUnboxedBool_From(!Real32_IsZero(argv[0].unboxed.r32));

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL32)
		return SmileUnboxedInteger64_From(Real32_ToInt64(argv[0].unboxed.r32));

	return SmileUnboxedReal32_From(Real32_Zero);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(real32, "Real32");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_REAL32) {
		return SmileArg_From((SmileObject)(Real32_ToStringEx(argv[0].unboxed.r32, 0, 0, False)));
	}

	return SmileArg_From((SmileObject)real32);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileReal32 obj = (SmileReal32)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_REAL32)
		return SmileUnboxedInteger64_From((UInt32)(*(UInt32 *)&obj->value));

	return SmileUnboxedInteger64_From((UInt32)((PtrInt)obj ^ Smile_HashOracle));
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion.

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	return SmileUnboxedByte_From(Real32_ToByte(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return SmileUnboxedInteger16_From(Real32_ToInt16(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return SmileUnboxedInteger32_From(Real32_ToInt32(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From(Real32_ToInt64(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(ToReal64)
{
#if 32 == 64
	return argv[0];
#else
	return SmileUnboxedReal64_From(Real32_ToReal64(argv[0].unboxed.r32));
#endif
}

SMILE_EXTERNAL_FUNCTION(ToReal32)
{
#if 32 == 64
	return SmileUnboxedReal32_From(Real32_ToReal32(argv[0].unboxed.r32));
#else
	return argv[0];
#endif
}

SMILE_EXTERNAL_FUNCTION(ToFloat64)
{
	return SmileUnboxedFloat64_From(Real32_ToFloat64(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(ToFloat32)
{
	return SmileUnboxedFloat32_From(Real32_ToFloat32(argv[0].unboxed.r32));
}

//-------------------------------------------------------------------------------------------------
// Basic arithmetic

SMILE_EXTERNAL_FUNCTION(Plus)
{
	Real32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.r32;
			x = Real32_Add(x, argv[1].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		case 3:
			x = argv[0].unboxed.r32;
			x = Real32_Add(x, argv[1].unboxed.r32);
			x = Real32_Add(x, argv[2].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		case 4:
			x = argv[0].unboxed.r32;
			x = Real32_Add(x, argv[1].unboxed.r32);
			x = Real32_Add(x, argv[2].unboxed.r32);
			x = Real32_Add(x, argv[3].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		default:
			x = argv[0].unboxed.r32;
			for (i = 1; i < argc; i++) {
				x = Real32_Add(x, argv[i].unboxed.r32);
			}
			return SmileUnboxedReal32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	Real32 x;
	Int i;

	switch (argc) {
		case 1:
			x = argv[0].unboxed.r32;
			x = Real32_Neg(x);
			return SmileUnboxedReal32_From(x);

		case 2:
			x = argv[0].unboxed.r32;
			x = Real32_Sub(x, argv[1].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		case 3:
			x = argv[0].unboxed.r32;
			x = Real32_Sub(x, argv[1].unboxed.r32);
			x = Real32_Sub(x, argv[2].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		case 4:
			x = argv[0].unboxed.r32;
			x = Real32_Sub(x, argv[1].unboxed.r32);
			x = Real32_Sub(x, argv[2].unboxed.r32);
			x = Real32_Sub(x, argv[3].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		default:
			x = argv[0].unboxed.r32;
			for (i = 1; i < argc; i++) {
				x = Real32_Sub(x, argv[i].unboxed.r32);
			}
			return SmileUnboxedReal32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Star)
{
	Real32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.r32;
			x = Real32_Mul(x, argv[1].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		case 3:
			x = argv[0].unboxed.r32;
			x = Real32_Mul(x, argv[1].unboxed.r32);
			x = Real32_Mul(x, argv[2].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		case 4:
			x = argv[0].unboxed.r32;
			x = Real32_Mul(x, argv[1].unboxed.r32);
			x = Real32_Mul(x, argv[2].unboxed.r32);
			x = Real32_Mul(x, argv[3].unboxed.r32);
			return SmileUnboxedReal32_From(x);

		default:
			x = argv[0].unboxed.r32;
			for (i = 1; i < argc; i++) {
				x = Real32_Mul(x, argv[i].unboxed.r32);
			}
			return SmileUnboxedReal32_From(x);
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

	return SmileUnboxedReal32_From(Real32_Zero);
}

SMILE_EXTERNAL_FUNCTION(Slash)
{
	Real32 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.r32;
			if (Real32_IsZero(y = argv[1].unboxed.r32))
				return DivideByZero(param);
			x = Real32_Div(x, y);
			return SmileUnboxedReal32_From(x);

		case 3:
			x = argv[0].unboxed.r32;
			if (Real32_IsZero(y = argv[1].unboxed.r32))
				return DivideByZero(param);
			x = Real32_Div(x, y);
			if (Real32_IsZero(y = argv[2].unboxed.r32))
				return DivideByZero(param);
			x = Real32_Div(x, y);
			return SmileUnboxedReal32_From(x);

		case 4:
			x = argv[0].unboxed.r32;
			if (Real32_IsZero(y = argv[1].unboxed.r32))
				return DivideByZero(param);
			x = Real32_Div(x, y);
			if (Real32_IsZero(y = argv[2].unboxed.r32))
				return DivideByZero(param);
			x = Real32_Div(x, y);
			if (Real32_IsZero(y = argv[3].unboxed.r32))
				return DivideByZero(param);
			x = Real32_Div(x, y);
			return SmileUnboxedReal32_From(x);

		default:
			x = argv[0].unboxed.r32;
			for (i = 1; i < argc; i++) {
				if (Real32_IsZero(y = argv[i].unboxed.r32))
					return DivideByZero(param);
				x = Real32_Div(x, y);
			}
			return SmileUnboxedReal32_From(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Real32 MathematiciansModulus(Real32 x, Real32 y)
{
	Real32 rem;

	if (Real32_IsNeg(x)) {
		if (Real32_IsNeg(y))
			return Real32_Neg(Real32_Mod(Real32_Neg(x), Real32_Neg(y)));
		else {
			rem = Real32_Mod(Real32_Neg(x), y);
			return !Real32_IsZero(rem) ? Real32_Sub(y, rem) : Real32_Zero;
		}
	}
	else if (Real32_IsNeg(y)) {
		rem = Real32_Mod(x, Real32_Neg(y));
		return !Real32_IsZero(rem) ? Real32_Add(y, rem) : Real32_Zero;
	}
	else
		return Real32_Mod(x, y);
}

/// <summary>
/// Perform remainder, in which the result has the same sign as the dividend (x).
/// </summary>
Inline Real32 MathematiciansRemainder(Real32 x, Real32 y)
{
	Real32 rem;

	if (Real32_IsNeg(x)) {
		if (Real32_IsNeg(y)) {
			rem = Real32_Mod(Real32_Neg(x), Real32_Neg(y));
			return !Real32_IsZero(rem) ? Real32_Add(rem, y) : Real32_Zero;
		}
		else
			return Real32_Neg(Real32_Mod(Real32_Neg(x), y));
	}
	else if (Real32_IsNeg(y))
		return Real32_Mod(x, Real32_Neg(y));
	else {
		rem = Real32_Mod(x, y);
		return !Real32_IsZero(rem) ? Real32_Sub(rem, y) : Real32_Zero;
	}
}

SMILE_EXTERNAL_FUNCTION(Mod)
{
	Real32 x = argv[0].unboxed.r32;
	Real32 y = argv[1].unboxed.r32;

	if (Real32_IsZero(y))
		return DivideByZero(param);

	return SmileUnboxedReal32_From(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	Real32 x = argv[0].unboxed.r32;
	Real32 y = argv[1].unboxed.r32;

	if (Real32_IsZero(y))
		return DivideByZero(param);

	return SmileUnboxedReal32_From(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	return SmileUnboxedReal32_From(Real32_Sign(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
	return SmileUnboxedReal32_From(Real32_Abs(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Clip)
{
	Real32 value = argv[0].unboxed.r32;
	Real32 min = argv[1].unboxed.r32;
	Real32 max = argv[2].unboxed.r32;

	if (Real32_Gt(value, max)) {
		value = max;
		return Real32_Lt(value, min) ? argv[1] : argv[2];
	}
	else if (Real32_Lt(value, min)) {
		return argv[1];
	}
	else return argv[0];
}

SMILE_EXTERNAL_FUNCTION(Min)
{
	Real32 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.r32;
			if (Real32_Lt((y = argv[1].unboxed.r32), x)) x = y;
			return SmileUnboxedReal32_From(x);

		case 3:
			x = argv[0].unboxed.r32;
			if (Real32_Lt((y = argv[1].unboxed.r32), x)) x = y;
			if (Real32_Lt((y = argv[2].unboxed.r32), x)) x = y;
			return SmileUnboxedReal32_From(x);

		case 4:
			x = argv[0].unboxed.r32;
			if (Real32_Lt((y = argv[1].unboxed.r32), x)) x = y;
			if (Real32_Lt((y = argv[2].unboxed.r32), x)) x = y;
			if (Real32_Lt((y = argv[3].unboxed.r32), x)) x = y;
			return SmileUnboxedReal32_From(x);

		default:
			x = argv[0].unboxed.r32;
			for (i = 1; i < argc; i++) {
				if (Real32_Lt((y = argv[i].unboxed.r32), x)) x = y;
			}
			return SmileUnboxedReal32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Max)
{
	Real32 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.r32;
			if (Real32_Gt((y = argv[1].unboxed.r32), x)) x = y;
			return SmileUnboxedReal32_From(x);

		case 3:
			x = argv[0].unboxed.r32;
			if (Real32_Gt((y = argv[1].unboxed.r32), x)) x = y;
			if (Real32_Gt((y = argv[2].unboxed.r32), x)) x = y;
			return SmileUnboxedReal32_From(x);

		case 4:
			x = argv[0].unboxed.r32;
			if (Real32_Gt((y = argv[1].unboxed.r32), x)) x = y;
			if (Real32_Gt((y = argv[2].unboxed.r32), x)) x = y;
			if (Real32_Gt((y = argv[3].unboxed.r32), x)) x = y;
			return SmileUnboxedReal32_From(x);

		default:
			x = argv[0].unboxed.r32;
			for (i = 1; i < argc; i++) {
				if (Real32_Gt((y = argv[i].unboxed.r32), x)) x = y;
			}
			return SmileUnboxedReal32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Sqrt)
{
	Real32 value = argv[0].unboxed.r32;

	if (Real32_IsNeg(value)) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);
		return SmileUnboxedReal32_From(Real32_Zero);
	}

	return SmileUnboxedReal32_From(Real32_Sqrt(value));
}

//-------------------------------------------------------------------------------------------------
// Rounding/truncation operations

SMILE_EXTERNAL_FUNCTION(Ceil)
{
	return SmileUnboxedReal32_From(Real32_Ceil(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Floor)
{
	return SmileUnboxedReal32_From(Real32_Floor(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Trunc)
{
	return SmileUnboxedReal32_From(Real32_Trunc(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Round)
{
	return SmileUnboxedReal32_From(Real32_Round(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(BankRound)
{
	return SmileUnboxedReal32_From(Real32_BankRound(argv[0].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Modf)
{
	Real32 intPart, fracPart;
	SmileList list;

	fracPart = Real32_Modf(argv[0].unboxed.r32, &intPart);

	list = SmileList_Cons((SmileObject)SmileReal32_Create(intPart),
		(SmileObject)SmileList_Cons((SmileObject)SmileReal32_Create(fracPart),
		NullObject));

	return SmileArg_From((SmileObject)list);
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_REAL32
		&& Real32_Eq(argv[0].unboxed.r32, argv[1].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_REAL32
		|| Real32_Ne(argv[0].unboxed.r32, argv[1].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(Real32_Lt(argv[0].unboxed.r32, argv[1].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(Real32_Gt(argv[0].unboxed.r32, argv[1].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(Real32_Le(argv[0].unboxed.r32, argv[1].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(Real32_Ge(argv[0].unboxed.r32, argv[1].unboxed.r32));
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Real32 x = argv[0].unboxed.r32;
	Real32 y = argv[1].unboxed.r32;

	if (Real32_Lt(x, y))
		return SmileUnboxedInteger64_From(-1);
	else if (Real32_Gt(x, y))
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

Inline Bool IsPositive(Real32 value)
{
	return (Real32_IsInf(value) && !Real32_IsNeg(value))
		|| (Real32_IsFinite(value) && !Real32_IsNeg(value) && !Real32_IsZero(value));
}

Inline Bool IsNegative(Real32 value)
{
	return (Real32_IsInf(value) && Real32_IsNeg(value))
		|| (Real32_IsFinite(value) && Real32_IsNeg(value));
}

SMILE_EXTERNAL_FUNCTION(ValueTest)
{
	Real32 value = argv[0].unboxed.r32;

	switch ((PtrInt)param) {
		case ZERO_TEST:
			return SmileUnboxedBool_From(Real32_IsZero(value));
		case ONE_TEST:
			return SmileUnboxedBool_From(Real32_Eq(value, Real32_One));
		case NONZERO_TEST:
			return SmileUnboxedBool_From(!Real32_IsZero(value));
		case POS_TEST:
			return SmileUnboxedBool_From(IsPositive(value));
		case NONPOS_TEST:
			return SmileUnboxedBool_From(!IsPositive(value));
		case NEG_TEST:
			return SmileUnboxedBool_From(IsNegative(value));
		case NONNEG_TEST:
			return SmileUnboxedBool_From(!IsNegative(value));
		case ODD_TEST:
			return SmileUnboxedBool_From(Real32_Eq(Real32_Mod(value, Real32_Two), Real32_One));
		case EVEN_TEST:
			return SmileUnboxedBool_From(Real32_IsZero(Real32_Mod(value, Real32_Two)));
		case INF_TEST:
			return SmileUnboxedBool_From(Real32_IsInf(value));
		case NAN_TEST:
			return SmileUnboxedBool_From(Real32_IsNaN(value));
		case FINITE_TEST:
			return SmileUnboxedBool_From(Real32_IsFinite(value));
		default:
			return SmileArg_From(NullObject);
	}
}

//-------------------------------------------------------------------------------------------------

void SmileReal32_Setup(SmileUserObject base)
{
	SmileUnboxedReal32_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("float32", ToFloat32, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("float64", ToFloat64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("float", ToFloat64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real32", ToReal32, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real64", ToReal64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("real", ToReal64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("byte", ToByte, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int16", ToInt16, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int32", ToInt32, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int64", ToInt64, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _real32Checks);
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _real32Checks);
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _real32Checks);
	SetupFunction("/", Slash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _real32Checks);
	SetupFunction("/!", Slash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _real32Checks);
	SetupSynonym("/", "div");
	SetupSynonym("/!", "div!");
	SetupFunction("mod", Mod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);
	SetupFunction("mod!", Mod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);
	SetupFunction("rem", Rem, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);
	SetupFunction("rem!", Rem, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);
	SetupFunction("sqrt", Sqrt, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("sqrt!", Sqrt, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _real32Checks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _real32Checks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _real32Checks);

	SetupFunction("ceil", Ceil, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("floor", Floor, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("trunc", Trunc, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("round", Round, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("bank-round", BankRound, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("modf", Modf, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);

	SetupFunction("odd?", ValueTest, (void *)ODD_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("even?", ValueTest, (void *)EVEN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("zero?", ValueTest, (void *)ZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("one?", ValueTest, (void *)ONE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("nonzero?", ValueTest, (void *)NONZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("positive?", ValueTest, (void *)POS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupSynonym("positive?", "pos?");
	SetupFunction("finite?", ValueTest, (void *)FINITE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("infinite?", ValueTest, (void *)INF_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupSynonym("infinite?", "inf?");
	SetupFunction("nan?", ValueTest, (void *)NAN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupFunction("nonpositive?", ValueTest, (void *)NONPOS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupSynonym("nonpositive?", "nonpos?");
	SetupFunction("negative?", ValueTest, (void *)NEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupSynonym("negative?", "neg?");
	SetupFunction("nonnegative?", ValueTest, (void *)NONNEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _real32Checks);
	SetupSynonym("nonnegative?", "nonneg?");

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32ComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32ComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _real32Checks);
	SetupSynonym("compare", "cmp");

	SetupData("inf", SmileReal32_Create(Real32_Inf));
}
