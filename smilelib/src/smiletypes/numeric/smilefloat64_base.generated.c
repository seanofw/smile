// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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

#include <smile/numeric/float64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/range/smilefloat32range.h>
#include <smile/smiletypes/range/smilefloat64range.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>
#include <smile/numeric/random.h>

#include <math.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _float64Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
};

static Byte _float64ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT64,
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
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_FLOAT64)
		return SmileUnboxedBool_From(argv[0].unboxed.f64 != 0.0);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_FLOAT64)
		return SmileUnboxedInteger64_From((Int64)argv[0].unboxed.f64);

	return SmileUnboxedFloat64_From(0.0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(float64, "Float64");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_FLOAT64) {
		return SmileArg_From((SmileObject)(Float64_ToStringEx(argv[0].unboxed.f64, 0, 0, False)));
	}

	return SmileArg_From((SmileObject)float64);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileFloat64 obj = (SmileFloat64)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_FLOAT64)
		return SmileUnboxedInteger64_From(Smile_ApplyHashOracle(*(UInt64 *)&obj->value));

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((UInt64)(PtrInt)obj));
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion.

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	return SmileUnboxedByte_From((Byte)argv[0].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return SmileUnboxedInteger16_From((Int16)argv[0].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return SmileUnboxedInteger32_From((Int32)argv[0].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From((Int64)argv[0].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(ToFloat64)
{
#if 64 == 64
	return argv[0];
#else
	return SmileUnboxedFloat64_From((Float64)argv[0].unboxed.f64);
#endif
}

SMILE_EXTERNAL_FUNCTION(ToFloat32)
{
#if 64 == 64
	return SmileUnboxedFloat32_From((Float32)argv[0].unboxed.f64);
#else
	return argv[0];
#endif
}

SMILE_EXTERNAL_FUNCTION(ToReal64)
{
	return SmileUnboxedReal64_From(Real64_FromFloat64(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(ToReal32)
{
	return SmileUnboxedReal32_From(Real32_FromFloat64(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(ToRawBits)
{
	return SmileUnboxedInteger64_From(*(Int64 *)&argv[0].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(FromRawBits)
{
	return SmileUnboxedFloat64_From(*(Float64 *)&argv[0].unboxed.i64);
}

SMILE_EXTERNAL_FUNCTION(RangeTo)
{
	Float64 start, end, step;

	start = argv[0].unboxed.f64;
	end = argv[1].unboxed.f64;
	step = end >= start ? (Float64)+1 : (Float64)-1;

	return SmileArg_From((SmileObject)SmileFloat64Range_Create(start, end, step));
}

//-------------------------------------------------------------------------------------------------
// Basic arithmetic

SMILE_EXTERNAL_FUNCTION(Plus)
{
	Float64 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.f64;
			x += argv[1].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		case 3:
			x = argv[0].unboxed.f64;
			x += argv[1].unboxed.f64;
			x += argv[2].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		case 4:
			x = argv[0].unboxed.f64;
			x += argv[1].unboxed.f64;
			x += argv[2].unboxed.f64;
			x += argv[3].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		default:
			x = argv[0].unboxed.f64;
			for (i = 1; i < argc; i++) {
				x += argv[i].unboxed.f64;
			}
			return SmileUnboxedFloat64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	Float64 x;
	Int i;

	switch (argc) {
		case 1:
			x = argv[0].unboxed.f64;
			x = -x;
			return SmileUnboxedFloat64_From(x);

		case 2:
			x = argv[0].unboxed.f64;
			x -= argv[1].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		case 3:
			x = argv[0].unboxed.f64;
			x -= argv[1].unboxed.f64;
			x -= argv[2].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		case 4:
			x = argv[0].unboxed.f64;
			x -= argv[1].unboxed.f64;
			x -= argv[2].unboxed.f64;
			x -= argv[3].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		default:
			x = argv[0].unboxed.f64;
			for (i = 1; i < argc; i++) {
				x -= argv[i].unboxed.f64;
			}
			return SmileUnboxedFloat64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Star)
{
	Float64 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.f64;
			x *= argv[1].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		case 3:
			x = argv[0].unboxed.f64;
			x *= argv[1].unboxed.f64;
			x *= argv[2].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		case 4:
			x = argv[0].unboxed.f64;
			x *= argv[1].unboxed.f64;
			x *= argv[2].unboxed.f64;
			x *= argv[3].unboxed.f64;
			return SmileUnboxedFloat64_From(x);

		default:
			x = argv[0].unboxed.f64;
			for (i = 1; i < argc; i++) {
				x *= argv[i].unboxed.f64;
			}
			return SmileUnboxedFloat64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(FMA)
{
	Float64 x, y, z;

	x = argv[0].unboxed.f64;
	y = argv[1].unboxed.f64;
	z = argv[2].unboxed.f64;

#if 64 == 64
	return SmileUnboxedFloat64_From(fma(x, y, z));	// C99, C++ 11
#else
	return SmileUnboxedFloat64_From(fmaf(x, y, z));	// C99, C++ 11
#endif
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

	return SmileUnboxedFloat64_From(0.0);
}

SMILE_EXTERNAL_FUNCTION(Slash)
{
	Float64 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) == 0.0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedFloat64_From(x);

		case 3:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) == 0.0)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[2].unboxed.f64) == 0.0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedFloat64_From(x);

		case 4:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) == 0.0)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[2].unboxed.f64) == 0.0)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[3].unboxed.f64) == 0.0)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedFloat64_From(x);

		default:
			x = argv[0].unboxed.f64;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.f64) == 0.0)
					return DivideByZero(param);
				x /= y;
			}
			return SmileUnboxedFloat64_From(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Float64 MathematiciansModulus(Float64 x, Float64 y)
{
	Float64 rem;

	if (x < 0.0) {
		if (y < 0.0)
			return (Float64)-fmod(-x, -y);
		else {
			rem = (Float64)fmod(-x, y);
			return rem != 0.0 ? y - rem : 0.0;
		}
	}
	else if (y < 0.0) {
		rem = (Float64)fmod(x, -y);
		return rem != 0.0 ? y + rem : 0.0;
	}
	else
		return (Float64)fmod(x, y);
}

/// <summary>
/// Perform remainder, in which the result has the same sign as the dividend (x).
/// </summary>
Inline Float64 MathematiciansRemainder(Float64 x, Float64 y)
{
	Float64 rem;

	if (x < 0.0) {
		if (y < 0.0) {
			rem = (Float64)fmod(-x, -y);
			return rem != 0 ? rem + y : 0.0;
		}
		else
			return -((Float64)fmod(-x, y));
	}
	else if (y < 0.0)
		return (Float64)fmod(x, -y);
	else {
		rem = (Float64)fmod(x, y);
		return rem != 0.0 ? rem - y : 0.0;
	}
}

SMILE_EXTERNAL_FUNCTION(Mod)
{
	Float64 x = argv[0].unboxed.f64;
	Float64 y = argv[1].unboxed.f64;

	if (y == 0.0)
		return DivideByZero(param);

	return SmileUnboxedFloat64_From(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	Float64 x = argv[0].unboxed.f64;
	Float64 y = argv[1].unboxed.f64;

	if (y == 0.0)
		return DivideByZero(param);

	return SmileUnboxedFloat64_From(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	Float64 value;

	value = argv[0].unboxed.f64;

	if (value < 0.0)
		return SmileUnboxedFloat64_From((Float64)-1.0);
	else if (value > 0.0)
		return SmileUnboxedFloat64_From((Float64)+1.0);
	else if (value == 0.0)
		return SmileUnboxedFloat64_From((Float64)+0.0);
	else // NaN
		return SmileUnboxedFloat64_From(value);
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
	return SmileUnboxedFloat64_From((Float64)fabs(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Clip)
{
	Float64 value = argv[0].unboxed.f64;
	Float64 min = argv[1].unboxed.f64;
	Float64 max = argv[2].unboxed.f64;

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
	Float64 value = argv[0].unboxed.f64;

	return value >= 0 ? argv[0] : SmileUnboxedFloat64_From(0);
}

SMILE_EXTERNAL_FUNCTION(Heaviside)
{
	Float64 value = argv[0].unboxed.f64;

	return value < 0 ? SmileUnboxedFloat64_From(0)
		: value > 0 ? SmileUnboxedFloat64_From(1)
		: SmileUnboxedFloat64_From(0.5f);
}

SMILE_EXTERNAL_FUNCTION(Rect)
{
	Float64 value = argv[0].unboxed.f64;
	value = (Float64)fabs(value);

	return value > 0.5f ? SmileUnboxedFloat64_From(0)
		: value < 0.5f ? SmileUnboxedFloat64_From(1)
		: SmileUnboxedFloat64_From(0.5f);
}

SMILE_EXTERNAL_FUNCTION(Tri)
{
	Float64 value = argv[0].unboxed.f64;
	value = (Float64)fabs(value);

	return value >= 1 ? SmileUnboxedFloat64_From(0)
		: SmileUnboxedFloat64_From(1 - value);
}

SMILE_EXTERNAL_FUNCTION(Min)
{
	Float64 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) < x) x = y;
			return SmileUnboxedFloat64_From(x);

		case 3:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) < x) x = y;
			if ((y = argv[2].unboxed.f64) < x) x = y;
			return SmileUnboxedFloat64_From(x);

		case 4:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) < x) x = y;
			if ((y = argv[2].unboxed.f64) < x) x = y;
			if ((y = argv[3].unboxed.f64) < x) x = y;
			return SmileUnboxedFloat64_From(x);

		default:
			x = argv[0].unboxed.f64;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.f64) < x) x = y;
			}
			return SmileUnboxedFloat64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Max)
{
	Float64 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) > x) x = y;
			return SmileUnboxedFloat64_From(x);

		case 3:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) > x) x = y;
			if ((y = argv[2].unboxed.f64) > x) x = y;
			return SmileUnboxedFloat64_From(x);

		case 4:
			x = argv[0].unboxed.f64;
			if ((y = argv[1].unboxed.f64) > x) x = y;
			if ((y = argv[2].unboxed.f64) > x) x = y;
			if ((y = argv[3].unboxed.f64) > x) x = y;
			return SmileUnboxedFloat64_From(x);

		default:
			x = argv[0].unboxed.f64;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.f64) > x) x = y;
			}
			return SmileUnboxedFloat64_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Sqrt)
{
	Float64 value = argv[0].unboxed.f64;

	if (value < 0.0) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);
		return SmileUnboxedFloat64_From(0.0);
	}

	return SmileUnboxedFloat64_From((Float64)sqrt(value));
}

SMILE_EXTERNAL_FUNCTION(Sqr)
{
	Float64 value = argv[0].unboxed.f64;
	return SmileUnboxedFloat64_From(value * value);
}

SMILE_EXTERNAL_FUNCTION(Cube)
{
	Float64 value = argv[0].unboxed.f64;
	return SmileUnboxedFloat64_From(value * value * value);
}

SMILE_EXTERNAL_FUNCTION(CubeRoot)
{
	Float64 value = argv[0].unboxed.f64;
	return SmileUnboxedFloat64_From(cbrt(value));
}

SMILE_EXTERNAL_FUNCTION(Hypotenuse)
{
	Float64 a = argv[0].unboxed.f64;
	Float64 b = argv[1].unboxed.f64;
	return SmileUnboxedFloat64_From(hypot(a, b));
}

SMILE_EXTERNAL_FUNCTION(Half)
{
	Float64 value = argv[0].unboxed.f64;
	return SmileUnboxedFloat64_From(value * 0.5);
}

SMILE_EXTERNAL_FUNCTION(Double)
{
	Float64 value = argv[0].unboxed.f64;
	return SmileUnboxedFloat64_From(value * 2.0);
}

//-------------------------------------------------------------------------------------------------
// Rounding/truncation operations

SMILE_EXTERNAL_FUNCTION(Ceil)
{
	return SmileUnboxedFloat64_From((Float64)ceil(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Floor)
{
	return SmileUnboxedFloat64_From((Float64)floor(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Trunc)
{
	return SmileUnboxedFloat64_From((Float64)trunc(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Round)
{
	return SmileUnboxedFloat64_From((Float64)floor(argv[0].unboxed.f64 + 0.5));
}

static Float64 BankRoundInternal(Float64 value)
{
	Float64 intPart, fracPart;
	Float64 intTail;

	// Negative numbers do the same algorithm upside-down.
	if (value < 0.0) return -BankRoundInternal(-value);

	// Split into integer and fractional parts.
	fracPart = modf(value, &intPart);

	// Easy rounding cases.
	if (fracPart < 0.5)
		return intPart;
	else if (fracPart > 0.5)
		return intPart + 1;

	// Exactly 0.5, so we need to use the integer part to decide which way to go.
	intTail = fmod(intPart, 2.0);
	if (intTail < 0.5 || intTail > 1.5) {
		// The integer part is even, so round down.
		return intPart;
	}
	else {
		// The integer part is odd, so round up.
		return intPart + 1;
	}	
}

SMILE_EXTERNAL_FUNCTION(BankRound)
{
	return SmileUnboxedFloat64_From((Float64)BankRoundInternal(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Modf)
{
	Float64 intPart, fracPart;
	SmileList list;

	fracPart = modf((Float64)argv[0].unboxed.f64, &intPart);

	list = SmileList_Cons((SmileObject)SmileFloat64_Create((Float64)intPart),
		(SmileObject)SmileList_Cons((SmileObject)SmileFloat64_Create((Float64)fracPart),
		NullObject));

	return SmileArg_From((SmileObject)list);
}

//-------------------------------------------------------------------------------------------------
// Powers, exponents, and logarithms

SMILE_EXTERNAL_FUNCTION(Log)
{
	if (argc < 2)
		return SmileUnboxedFloat64_From(log10(argv[0].unboxed.f64));
	else {
		Float64 value = log(argv[1].unboxed.f64);
		Float64 base = log(argv[0].unboxed.f64);
		Float64 result = value / base;
		return SmileUnboxedFloat64_From(result);
	}
}

SMILE_EXTERNAL_FUNCTION(Log2)
{
	return SmileUnboxedFloat64_From(log2(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Ln)
{
	return SmileUnboxedFloat64_From(log(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Ln1p)
{
	return SmileUnboxedFloat64_From(log1p(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Exp)
{
	return SmileUnboxedFloat64_From(exp(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Exp2)
{
	return SmileUnboxedFloat64_From(exp2(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Exp10)
{
	return SmileUnboxedFloat64_From(pow(10, argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Expm1)
{
	return SmileUnboxedFloat64_From(expm1(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Pow)
{
	return SmileUnboxedFloat64_From(pow(argv[0].unboxed.f64, argv[1].unboxed.f64));
}

//-------------------------------------------------------------------------------------------------
// The Gamma function (like Factorial(N), but well-defined for all N).

SMILE_EXTERNAL_FUNCTION(Gamma)
{
	return SmileUnboxedFloat64_From(tgamma(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(LnGamma)
{
	return SmileUnboxedFloat64_From(lgamma(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Factorial)
{
	return SmileUnboxedFloat64_From(tgamma(argv[0].unboxed.f64 + 1.0));
}

SMILE_EXTERNAL_FUNCTION(LnFactorial)
{
	return SmileUnboxedFloat64_From(lgamma(argv[0].unboxed.f64 + 1.0));
}

//-------------------------------------------------------------------------------------------------
// Trigonometry

SMILE_EXTERNAL_FUNCTION(Sin)
{
	return SmileUnboxedFloat64_From(sin(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Cos)
{
	return SmileUnboxedFloat64_From(cos(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Tan)
{
	return SmileUnboxedFloat64_From(tan(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(ATan)
{
	if (argc < 2)
		return SmileUnboxedFloat64_From(atan(argv[0].unboxed.f64));
	else
		return SmileUnboxedFloat64_From(atan2(argv[0].unboxed.f64, argv[1].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(ASin)
{
	return SmileUnboxedFloat64_From(asin(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(ACos)
{
	return SmileUnboxedFloat64_From(acos(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(DegToRad)
{
	return SmileUnboxedFloat64_From(argv[0].unboxed.f64 * (Float64)(3.14159265358979323846264338327950288 / 180.0));
}

SMILE_EXTERNAL_FUNCTION(RadToDeg)
{
	return SmileUnboxedFloat64_From(argv[0].unboxed.f64 * (Float64)(180.0 / 3.14159265358979323846264338327950288));
}

//-------------------------------------------------------------------------------------------------
// Hyperbolic functions

SMILE_EXTERNAL_FUNCTION(Sinh)
{
	return SmileUnboxedFloat64_From(sinh(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Cosh)
{
	return SmileUnboxedFloat64_From(cosh(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(Tanh)
{
	return SmileUnboxedFloat64_From(tanh(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(ATanh)
{
	return SmileUnboxedFloat64_From(atanh(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(ASinh)
{
	return SmileUnboxedFloat64_From(asinh(argv[0].unboxed.f64));
}

SMILE_EXTERNAL_FUNCTION(ACosh)
{
	return SmileUnboxedFloat64_From(acosh(argv[0].unboxed.f64));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_FLOAT64
		&& argv[0].unboxed.f64 == argv[1].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_FLOAT64
		|| argv[0].unboxed.f64 != argv[1].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.f64 < argv[1].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.f64 > argv[1].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(argv[0].unboxed.f64 <= argv[1].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(argv[0].unboxed.f64 >= argv[1].unboxed.f64);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Float64 x = argv[0].unboxed.f64;
	Float64 y = argv[1].unboxed.f64;

	if (x < y)
		return SmileUnboxedInteger64_From(-1);
	else if (x > y)
		return SmileUnboxedInteger64_From(+1);
	else
		return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(RandomFunc)
{
	SmileFloat64 obj = (SmileFloat64)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_FLOAT64) {
		Float64 value = argv[0].unboxed.f64;
		return SmileUnboxedFloat64_From(Random_Float64(Random_Shared) * value);
	}
	else {
		return SmileUnboxedFloat64_From(Random_Float64(Random_Shared));
	}
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

Inline Bool IsPositive(Float64 value)
{
	switch (Float64_GetKind(value)) {
		case FLOAT_KIND_POS_ZERO: return False;
		case FLOAT_KIND_NEG_ZERO: return False;
		case FLOAT_KIND_POS_NUM: return True;
		case FLOAT_KIND_NEG_NUM: return False;
		case FLOAT_KIND_POS_INF: return True;
		case FLOAT_KIND_NEG_INF: return False;
		case FLOAT_KIND_POS_QNAN: return False;
		case FLOAT_KIND_NEG_QNAN: return False;
		case FLOAT_KIND_POS_SNAN: return False;
		case FLOAT_KIND_NEG_SNAN: return False;
		default: return False;
	}
}

Inline Bool IsNegative(Float64 value)
{
	switch (Float64_GetKind(value)) {
		case FLOAT_KIND_POS_ZERO: return False;
		case FLOAT_KIND_NEG_ZERO: return False;
		case FLOAT_KIND_POS_NUM: return False;
		case FLOAT_KIND_NEG_NUM: return True;
		case FLOAT_KIND_POS_INF: return False;
		case FLOAT_KIND_NEG_INF: return True;
		case FLOAT_KIND_POS_QNAN: return False;
		case FLOAT_KIND_NEG_QNAN: return False;
		case FLOAT_KIND_POS_SNAN: return False;
		case FLOAT_KIND_NEG_SNAN: return False;
		default: return False;
	}
}

Inline Bool IsInfinity(Float64 value)
{
	switch (Float64_GetKind(value)) {
		case FLOAT_KIND_POS_ZERO: return False;
		case FLOAT_KIND_NEG_ZERO: return False;
		case FLOAT_KIND_POS_NUM: return False;
		case FLOAT_KIND_NEG_NUM: return False;
		case FLOAT_KIND_POS_INF: return True;
		case FLOAT_KIND_NEG_INF: return True;
		case FLOAT_KIND_POS_QNAN: return False;
		case FLOAT_KIND_NEG_QNAN: return False;
		case FLOAT_KIND_POS_SNAN: return False;
		case FLOAT_KIND_NEG_SNAN: return False;
		default: return False;
	}
}

Inline Bool IsNaN(Float64 value)
{
	switch (Float64_GetKind(value)) {
		case FLOAT_KIND_POS_ZERO: return False;
		case FLOAT_KIND_NEG_ZERO: return False;
		case FLOAT_KIND_POS_NUM: return False;
		case FLOAT_KIND_NEG_NUM: return False;
		case FLOAT_KIND_POS_INF: return False;
		case FLOAT_KIND_NEG_INF: return False;
		case FLOAT_KIND_POS_QNAN: return True;
		case FLOAT_KIND_NEG_QNAN: return True;
		case FLOAT_KIND_POS_SNAN: return True;
		case FLOAT_KIND_NEG_SNAN: return True;
		default: return False;
	}
}

Inline Bool IsFinite(Float64 value)
{
	switch (Float64_GetKind(value)) {
		case FLOAT_KIND_POS_ZERO: return True;
		case FLOAT_KIND_NEG_ZERO: return True;
		case FLOAT_KIND_POS_NUM: return True;
		case FLOAT_KIND_NEG_NUM: return True;
		case FLOAT_KIND_POS_INF: return False;
		case FLOAT_KIND_NEG_INF: return False;
		case FLOAT_KIND_POS_QNAN: return False;
		case FLOAT_KIND_NEG_QNAN: return False;
		case FLOAT_KIND_POS_SNAN: return False;
		case FLOAT_KIND_NEG_SNAN: return False;
		default: return False;
	}
}

SMILE_EXTERNAL_FUNCTION(ValueTest)
{
	Float64 value = argv[0].unboxed.f64;

	switch ((PtrInt)param) {
		case ZERO_TEST:
			return SmileUnboxedBool_From(value == 0.0);
		case ONE_TEST:
			return SmileUnboxedBool_From(value == 1.0);
		case NONZERO_TEST:
			return SmileUnboxedBool_From(value != 0.0);
		case POS_TEST:
			return SmileUnboxedBool_From(IsPositive(value));
		case NONPOS_TEST:
			return SmileUnboxedBool_From(!IsPositive(value));
		case NEG_TEST:
			return SmileUnboxedBool_From(IsNegative(value));
		case NONNEG_TEST:
			return SmileUnboxedBool_From(!IsNegative(value));
		case ODD_TEST:
			return SmileUnboxedBool_From(fabs(fmod(value, 2.0)) == 1.0);
		case EVEN_TEST:
			return SmileUnboxedBool_From(fabs(fmod(value, 2.0)) == 0.0);
		case INF_TEST:
			return SmileUnboxedBool_From(IsInfinity(value));
		case NAN_TEST:
			return SmileUnboxedBool_From(IsNaN(value));
		case FINITE_TEST:
			return SmileUnboxedBool_From(IsFinite(value));
		default:
			return SmileArg_From(NullObject);
	}
}

//-------------------------------------------------------------------------------------------------

void SmileFloat64_Setup(SmileUserObject base)
{
	const UInt64 infValue = 0x7FF0000000000000ULL; Float64 inf = *(Float64 *)&infValue;
	SmileUnboxedFloat64_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("float32", ToFloat32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("float64", ToFloat64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("float", ToFloat64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("real32", ToReal32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("real64", ToReal64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("real", ToReal64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("byte", ToByte, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("int16", ToInt16, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("int32", ToInt32, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("int64", ToInt64, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);

	SetupFunction("raw-bits", ToRawBits, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("from-raw-bits", FromRawBits, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _float64Checks);
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _float64Checks);
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _float64Checks);
	SetupFunction("+*", FMA, NULL, "augend multiplier multiplicand", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _float64Checks);
	SetupFunction("/", Slash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _float64Checks);
	SetupFunction("/!", Slash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _float64Checks);
	SetupSynonym("/", "div");
	SetupSynonym("/!", "div!");
	SetupFunction("mod", Mod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupFunction("mod!", Mod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupFunction("rem", Rem, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupFunction("rem!", Rem, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _float64Checks);
	SetupFunction("ramp", Ramp, NULL, "value", ARG_CHECK_EXACT, 1, 1, 1, _float64Checks);
	SetupFunction("heaviside", Heaviside, NULL, "value", ARG_CHECK_EXACT, 1, 1, 1, _float64Checks);
	SetupFunction("rect", Rect, NULL, "value", ARG_CHECK_EXACT, 1, 1, 1, _float64Checks);
	SetupFunction("tri", Tri, NULL, "value", ARG_CHECK_EXACT, 1, 1, 1, _float64Checks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _float64Checks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _float64Checks);

	SetupFunction("sqrt", Sqrt, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("sqrt!", Sqrt, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("sqr", Sqr, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("cube", Cube, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("cube-root", CubeRoot, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("hypotenuse", Hypotenuse, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupFunction("half", Half, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("double", Double, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupSynonym("double", "dbl");

	SetupFunction("ceil", Ceil, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("floor", Floor, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("trunc", Trunc, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("round", Round, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("bank-round", BankRound, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("modf", Modf, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);

	SetupFunction("log", Log, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _float64Checks);
	SetupFunction("log2", Log2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("ln", Ln, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("ln1p", Ln1p, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("exp", Exp, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("exp2", Exp2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("exp10", Exp10, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("expm1", Expm1, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("^", Pow, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupSynonym("log2", "lg");

	SetupFunction("gamma", Gamma, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("ln-gamma", LnGamma, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("factorial", Factorial, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("ln-factorial", LnFactorial, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);

	SetupFunction("sin", Sin, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("cos", Cos, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("tan", Tan, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("atan", ATan, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _float64Checks);
	SetupSynonym("atan", "atan2");
	SetupFunction("asin", ASin, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("acos", ACos, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("deg-to-rad", DegToRad, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("rad-to-deg", RadToDeg, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);

	SetupFunction("sinh", Sinh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("cosh", Cosh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("tanh", Tanh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("atanh", ATanh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("asinh", ASinh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("acosh", ACosh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);

	SetupFunction("odd?", ValueTest, (void *)ODD_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("even?", ValueTest, (void *)EVEN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("zero?", ValueTest, (void *)ZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("one?", ValueTest, (void *)ONE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("nonzero?", ValueTest, (void *)NONZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("positive?", ValueTest, (void *)POS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupSynonym("positive?", "pos?");
	SetupFunction("finite?", ValueTest, (void *)FINITE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("infinite?", ValueTest, (void *)INF_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupSynonym("infinite?", "inf?");
	SetupFunction("nan?", ValueTest, (void *)NAN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupFunction("nonpositive?", ValueTest, (void *)NONPOS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupSynonym("nonpositive?", "nonpos?");
	SetupFunction("negative?", ValueTest, (void *)NEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupSynonym("negative?", "neg?");
	SetupFunction("nonnegative?", ValueTest, (void *)NONNEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float64Checks);
	SetupSynonym("nonnegative?", "nonneg?");

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64ComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64ComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);
	SetupSynonym("compare", "cmp");

	SetupFunction("range-to", RangeTo, NULL, "start end", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float64Checks);

	SetupFunction("random", RandomFunc, base, "count", 0, 1, 1, 0, NULL);

	SetupData("inf",   SmileFloat64_Create(inf));

	SetupData("pi",    SmileFloat64_Create((Float64)3.14159265358979323846264338327950288));
	SetupData("e",     SmileFloat64_Create((Float64)2.71828182845904523536028747135266249));
	SetupData("tau",   SmileFloat64_Create((Float64)6.28318530717958647692528676655900576));
	SetupData("sqrt2", SmileFloat64_Create((Float64)1.41421356237309504880168872420969807));
	SetupData("sqrt3", SmileFloat64_Create((Float64)1.73205080756887729352744634150587236));
	SetupData("sqrt5", SmileFloat64_Create((Float64)2.23606797749978969640917366873127623));
	SetupData("phi",   SmileFloat64_Create((Float64)1.61803398874989484820458683436563811));

	SetupData("G",     SmileFloat64_Create((Float64)6.67408e-11));
	SetupData("c",     SmileFloat64_Create((Float64)299792458.0));
	SetupData("h",     SmileFloat64_Create((Float64)6.626070040e-34));

	SetupData("g",     SmileFloat64_Create((Float64)9.80665));
}
