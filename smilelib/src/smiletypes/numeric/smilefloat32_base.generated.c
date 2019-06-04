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

static Byte _float32Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
};

static Byte _float32ComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_FLOAT32,
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
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_FLOAT32)
		return SmileUnboxedBool_From(argv[0].unboxed.f32 != 0.0f);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_FLOAT32)
		return SmileUnboxedInteger64_From((Int64)argv[0].unboxed.f32);

	return SmileUnboxedFloat32_From(0.0f);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(float32, "Float32");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_FLOAT32) {
		return SmileArg_From((SmileObject)(Float64_ToStringEx((Float64)argv[0].unboxed.f32, 0, 0, False)));
	}

	return SmileArg_From((SmileObject)float32);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileFloat32 obj = (SmileFloat32)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_FLOAT32)
		return SmileUnboxedInteger64_From(Smile_ApplyHashOracle(*(UInt32 *)&obj->value));

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((UInt64)(PtrInt)obj));
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion.

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	return SmileUnboxedByte_From((Byte)argv[0].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return SmileUnboxedInteger16_From((Int16)argv[0].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return SmileUnboxedInteger32_From((Int32)argv[0].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From((Int64)argv[0].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(ToFloat64)
{
#if 32 == 64
	return argv[0];
#else
	return SmileUnboxedFloat64_From((Float64)argv[0].unboxed.f32);
#endif
}

SMILE_EXTERNAL_FUNCTION(ToFloat32)
{
#if 32 == 64
	return SmileUnboxedFloat32_From((Float32)argv[0].unboxed.f32);
#else
	return argv[0];
#endif
}

SMILE_EXTERNAL_FUNCTION(ToReal64)
{
	return SmileUnboxedReal64_From(Real64_FromFloat32(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(ToReal32)
{
	return SmileUnboxedReal32_From(Real32_FromFloat32(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(RangeTo)
{
	Float32 start, end, step;

	start = argv[0].unboxed.f32;
	end = argv[1].unboxed.f32;
	step = end >= start ? (Float32)+1 : (Float32)-1;

	return SmileArg_From((SmileObject)SmileFloat32Range_Create(start, end, step));
}

//-------------------------------------------------------------------------------------------------
// Basic arithmetic

SMILE_EXTERNAL_FUNCTION(Plus)
{
	Float32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.f32;
			x += argv[1].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		case 3:
			x = argv[0].unboxed.f32;
			x += argv[1].unboxed.f32;
			x += argv[2].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		case 4:
			x = argv[0].unboxed.f32;
			x += argv[1].unboxed.f32;
			x += argv[2].unboxed.f32;
			x += argv[3].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		default:
			x = argv[0].unboxed.f32;
			for (i = 1; i < argc; i++) {
				x += argv[i].unboxed.f32;
			}
			return SmileUnboxedFloat32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	Float32 x;
	Int i;

	switch (argc) {
		case 1:
			x = argv[0].unboxed.f32;
			x = -x;
			return SmileUnboxedFloat32_From(x);

		case 2:
			x = argv[0].unboxed.f32;
			x -= argv[1].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		case 3:
			x = argv[0].unboxed.f32;
			x -= argv[1].unboxed.f32;
			x -= argv[2].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		case 4:
			x = argv[0].unboxed.f32;
			x -= argv[1].unboxed.f32;
			x -= argv[2].unboxed.f32;
			x -= argv[3].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		default:
			x = argv[0].unboxed.f32;
			for (i = 1; i < argc; i++) {
				x -= argv[i].unboxed.f32;
			}
			return SmileUnboxedFloat32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Star)
{
	Float32 x;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.f32;
			x *= argv[1].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		case 3:
			x = argv[0].unboxed.f32;
			x *= argv[1].unboxed.f32;
			x *= argv[2].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		case 4:
			x = argv[0].unboxed.f32;
			x *= argv[1].unboxed.f32;
			x *= argv[2].unboxed.f32;
			x *= argv[3].unboxed.f32;
			return SmileUnboxedFloat32_From(x);

		default:
			x = argv[0].unboxed.f32;
			for (i = 1; i < argc; i++) {
				x *= argv[i].unboxed.f32;
			}
			return SmileUnboxedFloat32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(FMA)
{
	Float32 x, y, z;

	x = argv[0].unboxed.f32;
	y = argv[1].unboxed.f32;
	z = argv[2].unboxed.f32;

#if 32 == 64
	return SmileUnboxedFloat32_From(fma(x, y, z));	// C99, C++ 11
#else
	return SmileUnboxedFloat32_From(fmaf(x, y, z));	// C99, C++ 11
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

	return SmileUnboxedFloat32_From(0.0f);
}

SMILE_EXTERNAL_FUNCTION(Slash)
{
	Float32 x, y;
	Int i;

	switch (argc) {
		case 2:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) == 0.0f)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedFloat32_From(x);

		case 3:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) == 0.0f)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[2].unboxed.f32) == 0.0f)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedFloat32_From(x);

		case 4:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) == 0.0f)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[2].unboxed.f32) == 0.0f)
				return DivideByZero(param);
			x /= y;
			if ((y = argv[3].unboxed.f32) == 0.0f)
				return DivideByZero(param);
			x /= y;
			return SmileUnboxedFloat32_From(x);

		default:
			x = argv[0].unboxed.f32;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.f32) == 0.0f)
					return DivideByZero(param);
				x /= y;
			}
			return SmileUnboxedFloat32_From(x);
	}
}

/// <summary>
/// Perform modulus like mathematicians expect, in which the modulus has the same sign as the divisor (y).
/// </summary>
Inline Float32 MathematiciansModulus(Float32 x, Float32 y)
{
	Float32 rem;

	if (x < 0.0f) {
		if (y < 0.0f)
			return (Float32)-fmod(-x, -y);
		else {
			rem = (Float32)fmod(-x, y);
			return rem != 0.0f ? y - rem : 0.0f;
		}
	}
	else if (y < 0.0f) {
		rem = (Float32)fmod(x, -y);
		return rem != 0.0f ? y + rem : 0.0f;
	}
	else
		return (Float32)fmod(x, y);
}

/// <summary>
/// Perform remainder, in which the result has the same sign as the dividend (x).
/// </summary>
Inline Float32 MathematiciansRemainder(Float32 x, Float32 y)
{
	Float32 rem;

	if (x < 0.0f) {
		if (y < 0.0f) {
			rem = (Float32)fmod(-x, -y);
			return rem != 0 ? rem + y : 0.0f;
		}
		else
			return -((Float32)fmod(-x, y));
	}
	else if (y < 0.0f)
		return (Float32)fmod(x, -y);
	else {
		rem = (Float32)fmod(x, y);
		return rem != 0.0f ? rem - y : 0.0f;
	}
}

SMILE_EXTERNAL_FUNCTION(Mod)
{
	Float32 x = argv[0].unboxed.f32;
	Float32 y = argv[1].unboxed.f32;

	if (y == 0.0f)
		return DivideByZero(param);

	return SmileUnboxedFloat32_From(MathematiciansModulus(x, y));
}

SMILE_EXTERNAL_FUNCTION(Rem)
{
	Float32 x = argv[0].unboxed.f32;
	Float32 y = argv[1].unboxed.f32;

	if (y == 0.0f)
		return DivideByZero(param);

	return SmileUnboxedFloat32_From(MathematiciansRemainder(x, y));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic extensions

SMILE_EXTERNAL_FUNCTION(Sign)
{
	Float32 value;

	value = argv[0].unboxed.f32;

	if (value < 0.0f)
		return SmileUnboxedFloat32_From((Float32)-1.0);
	else if (value > 0.0f)
		return SmileUnboxedFloat32_From((Float32)+1.0);
	else if (value == 0.0f)
		return SmileUnboxedFloat32_From((Float32)+0.0);
	else // NaN
		return SmileUnboxedFloat32_From(value);
}

SMILE_EXTERNAL_FUNCTION(Abs)
{
	return SmileUnboxedFloat32_From((Float32)fabs(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Clip)
{
	Float32 value = argv[0].unboxed.f32;
	Float32 min = argv[1].unboxed.f32;
	Float32 max = argv[2].unboxed.f32;

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
	Float32 value = argv[0].unboxed.f32;

	return value >= 0 ? argv[0] : SmileUnboxedFloat32_From(0);
}

SMILE_EXTERNAL_FUNCTION(Heaviside)
{
	Float32 value = argv[0].unboxed.f32;

	return value < 0 ? SmileUnboxedFloat32_From(0)
		: value > 0 ? SmileUnboxedFloat32_From(1)
		: SmileUnboxedFloat32_From(0.5f);
}

SMILE_EXTERNAL_FUNCTION(Rect)
{
	Float32 value = argv[0].unboxed.f32;
	value = (Float32)fabs(value);

	return value > 0.5f ? SmileUnboxedFloat32_From(0)
		: value < 0.5f ? SmileUnboxedFloat32_From(1)
		: SmileUnboxedFloat32_From(0.5f);
}

SMILE_EXTERNAL_FUNCTION(Tri)
{
	Float32 value = argv[0].unboxed.f32;
	value = (Float32)fabs(value);

	return value >= 1 ? SmileUnboxedFloat32_From(0)
		: SmileUnboxedFloat32_From(1 - value);
}

SMILE_EXTERNAL_FUNCTION(Min)
{
	Float32 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) < x) x = y;
			return SmileUnboxedFloat32_From(x);

		case 3:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) < x) x = y;
			if ((y = argv[2].unboxed.f32) < x) x = y;
			return SmileUnboxedFloat32_From(x);

		case 4:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) < x) x = y;
			if ((y = argv[2].unboxed.f32) < x) x = y;
			if ((y = argv[3].unboxed.f32) < x) x = y;
			return SmileUnboxedFloat32_From(x);

		default:
			x = argv[0].unboxed.f32;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.f32) < x) x = y;
			}
			return SmileUnboxedFloat32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Max)
{
	Float32 x, y;
	Int i;

	switch (argc) {
		case 1:
			return argv[0];

		case 2:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) > x) x = y;
			return SmileUnboxedFloat32_From(x);

		case 3:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) > x) x = y;
			if ((y = argv[2].unboxed.f32) > x) x = y;
			return SmileUnboxedFloat32_From(x);

		case 4:
			x = argv[0].unboxed.f32;
			if ((y = argv[1].unboxed.f32) > x) x = y;
			if ((y = argv[2].unboxed.f32) > x) x = y;
			if ((y = argv[3].unboxed.f32) > x) x = y;
			return SmileUnboxedFloat32_From(x);

		default:
			x = argv[0].unboxed.f32;
			for (i = 1; i < argc; i++) {
				if ((y = argv[i].unboxed.f32) > x) x = y;
			}
			return SmileUnboxedFloat32_From(x);
	}
}

SMILE_EXTERNAL_FUNCTION(Sqrt)
{
	Float32 value = argv[0].unboxed.f32;

	if (value < 0.0f) {
		MathInfo mathInfo = (MathInfo)param;
		if (mathInfo->isLoud)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _negativeSqrt);
		return SmileUnboxedFloat32_From(0.0f);
	}

	return SmileUnboxedFloat32_From((Float32)sqrt(value));
}

SMILE_EXTERNAL_FUNCTION(Sqr)
{
	Float32 value = argv[0].unboxed.f32;
	return SmileUnboxedFloat32_From(value * value);
}

SMILE_EXTERNAL_FUNCTION(Cube)
{
	Float32 value = argv[0].unboxed.f32;
	return SmileUnboxedFloat32_From(value * value * value);
}

SMILE_EXTERNAL_FUNCTION(CubeRoot)
{
	Float32 value = argv[0].unboxed.f32;
	return SmileUnboxedFloat32_From(cbrtf(value));
}

SMILE_EXTERNAL_FUNCTION(Hypotenuse)
{
	Float32 a = argv[0].unboxed.f32;
	Float32 b = argv[1].unboxed.f32;
	return SmileUnboxedFloat32_From(hypotf(a, b));
}

SMILE_EXTERNAL_FUNCTION(Half)
{
	Float32 value = argv[0].unboxed.f32;
	return SmileUnboxedFloat32_From(value * 0.5f);
}

SMILE_EXTERNAL_FUNCTION(Double)
{
	Float32 value = argv[0].unboxed.f32;
	return SmileUnboxedFloat32_From(value * 2.0f);
}

//-------------------------------------------------------------------------------------------------
// Rounding/truncation operations

SMILE_EXTERNAL_FUNCTION(Ceil)
{
	return SmileUnboxedFloat32_From((Float32)ceil(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Floor)
{
	return SmileUnboxedFloat32_From((Float32)floor(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Trunc)
{
	return SmileUnboxedFloat32_From((Float32)trunc(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Round)
{
	return SmileUnboxedFloat32_From((Float32)floor(argv[0].unboxed.f32 + 0.5));
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
	return SmileUnboxedFloat32_From((Float32)BankRoundInternal(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Modf)
{
	Float64 intPart, fracPart;
	SmileList list;

	fracPart = modf((Float64)argv[0].unboxed.f32, &intPart);

	list = SmileList_Cons((SmileObject)SmileFloat32_Create((Float32)intPart),
		(SmileObject)SmileList_Cons((SmileObject)SmileFloat32_Create((Float32)fracPart),
		NullObject));

	return SmileArg_From((SmileObject)list);
}

//-------------------------------------------------------------------------------------------------
// Powers, exponents, and logarithms

SMILE_EXTERNAL_FUNCTION(Log)
{
	if (argc < 2)
		return SmileUnboxedFloat32_From(log10f(argv[0].unboxed.f32));
	else {
		Float32 value = logf(argv[1].unboxed.f32);
		Float32 base = logf(argv[0].unboxed.f32);
		Float32 result = value / base;
		return SmileUnboxedFloat32_From(result);
	}
}

SMILE_EXTERNAL_FUNCTION(Log2)
{
	return SmileUnboxedFloat32_From(log2f(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Ln)
{
	return SmileUnboxedFloat32_From(logf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Ln1p)
{
	return SmileUnboxedFloat32_From(log1pf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Exp)
{
	return SmileUnboxedFloat32_From(expf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Exp2)
{
	return SmileUnboxedFloat32_From(exp2f(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Exp10)
{
	return SmileUnboxedFloat32_From(powf(10, argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Expm1)
{
	return SmileUnboxedFloat32_From(expm1f(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Pow)
{
	return SmileUnboxedFloat32_From(powf(argv[0].unboxed.f32, argv[1].unboxed.f32));
}

//-------------------------------------------------------------------------------------------------
// The Gamma function (like Factorial(N), but well-defined for all N).

SMILE_EXTERNAL_FUNCTION(Gamma)
{
	return SmileUnboxedFloat32_From(tgammaf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(LnGamma)
{
	return SmileUnboxedFloat32_From(lgammaf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Factorial)
{
	return SmileUnboxedFloat32_From(tgammaf(argv[0].unboxed.f32 + 1.0f));
}

SMILE_EXTERNAL_FUNCTION(LnFactorial)
{
	return SmileUnboxedFloat32_From(lgammaf(argv[0].unboxed.f32 + 1.0f));
}

//-------------------------------------------------------------------------------------------------
// Trigonometry

SMILE_EXTERNAL_FUNCTION(Sin)
{
	return SmileUnboxedFloat32_From(sinf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Cos)
{
	return SmileUnboxedFloat32_From(cosf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Tan)
{
	return SmileUnboxedFloat32_From(tanf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(ATan)
{
	if (argc < 2)
		return SmileUnboxedFloat32_From(atanf(argv[0].unboxed.f32));
	else
		return SmileUnboxedFloat32_From(atan2f(argv[0].unboxed.f32, argv[1].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(ASin)
{
	return SmileUnboxedFloat32_From(asinf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(ACos)
{
	return SmileUnboxedFloat32_From(acosf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(DegToRad)
{
	return SmileUnboxedFloat32_From(argv[0].unboxed.f32 * (Float32)(3.14159265358979323846264338327950288 / 180.0));
}

SMILE_EXTERNAL_FUNCTION(RadToDeg)
{
	return SmileUnboxedFloat32_From(argv[0].unboxed.f32 * (Float32)(180.0 / 3.14159265358979323846264338327950288));
}

//-------------------------------------------------------------------------------------------------
// Hyperbolic functions

SMILE_EXTERNAL_FUNCTION(Sinh)
{
	return SmileUnboxedFloat32_From(sinhf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Cosh)
{
	return SmileUnboxedFloat32_From(coshf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(Tanh)
{
	return SmileUnboxedFloat32_From(tanhf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(ATanh)
{
	return SmileUnboxedFloat32_From(atanhf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(ASinh)
{
	return SmileUnboxedFloat32_From(asinhf(argv[0].unboxed.f32));
}

SMILE_EXTERNAL_FUNCTION(ACosh)
{
	return SmileUnboxedFloat32_From(acoshf(argv[0].unboxed.f32));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_FLOAT32
		&& argv[0].unboxed.f32 == argv[1].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_FLOAT32
		|| argv[0].unboxed.f32 != argv[1].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.f32 < argv[1].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.f32 > argv[1].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(argv[0].unboxed.f32 <= argv[1].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(argv[0].unboxed.f32 >= argv[1].unboxed.f32);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Float32 x = argv[0].unboxed.f32;
	Float32 y = argv[1].unboxed.f32;

	if (x < y)
		return SmileUnboxedInteger64_From(-1);
	else if (x > y)
		return SmileUnboxedInteger64_From(+1);
	else
		return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(RandomFunc)
{
	SmileFloat32 obj = (SmileFloat32)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_FLOAT32) {
		Float32 value = argv[0].unboxed.f32;
		return SmileUnboxedFloat32_From(Random_Float32(Random_Shared) * value);
	}
	else {
		return SmileUnboxedFloat32_From(Random_Float32(Random_Shared));
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

Inline Bool IsPositive(Float32 value)
{
	switch (Float32_GetKind(value)) {
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

Inline Bool IsNegative(Float32 value)
{
	switch (Float32_GetKind(value)) {
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

Inline Bool IsInfinity(Float32 value)
{
	switch (Float32_GetKind(value)) {
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

Inline Bool IsNaN(Float32 value)
{
	switch (Float32_GetKind(value)) {
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

Inline Bool IsFinite(Float32 value)
{
	switch (Float32_GetKind(value)) {
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
	Float32 value = argv[0].unboxed.f32;

	switch ((PtrInt)param) {
		case ZERO_TEST:
			return SmileUnboxedBool_From(value == 0.0f);
		case ONE_TEST:
			return SmileUnboxedBool_From(value == 1.0f);
		case NONZERO_TEST:
			return SmileUnboxedBool_From(value != 0.0f);
		case POS_TEST:
			return SmileUnboxedBool_From(IsPositive(value));
		case NONPOS_TEST:
			return SmileUnboxedBool_From(!IsPositive(value));
		case NEG_TEST:
			return SmileUnboxedBool_From(IsNegative(value));
		case NONNEG_TEST:
			return SmileUnboxedBool_From(!IsNegative(value));
		case ODD_TEST:
			return SmileUnboxedBool_From(fabs(fmod(value, 2.0f)) == 1.0f);
		case EVEN_TEST:
			return SmileUnboxedBool_From(fabs(fmod(value, 2.0f)) == 0.0f);
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

void SmileFloat32_Setup(SmileUserObject base)
{
	const UInt32 infValue = 0x7F800000U; Float32 inf = *(Float32 *)&infValue;
	SmileUnboxedFloat32_Instance->base = (SmileObject)base;

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

	SetupFunction("+", Plus, NULL, "augend addend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _float32Checks);
	SetupFunction("-", Minus, NULL, "minuend subtrahend", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _float32Checks);
	SetupFunction("*", Star, NULL, "multiplier multiplicand", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _float32Checks);
	SetupFunction("+*", FMA, NULL, "augend multiplier multiplicand", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _float32Checks);
	SetupFunction("/", Slash, &_quietMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _float32Checks);
	SetupFunction("/!", Slash, &_loudMath, "dividend divisor", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 0, 8, _float32Checks);
	SetupSynonym("/", "div");
	SetupSynonym("/!", "div!");
	SetupFunction("mod", Mod, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupFunction("mod!", Mod, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupFunction("rem", Rem, &_quietMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupFunction("rem!", Rem, &_loudMath, "dividend divisor", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);

	SetupFunction("sign", Sign, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("abs", Abs, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("clip", Clip, NULL, "value min max", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _float32Checks);
	SetupFunction("ramp", Ramp, NULL, "value", ARG_CHECK_EXACT, 1, 1, 1, _float32Checks);
	SetupFunction("heaviside", Heaviside, NULL, "value", ARG_CHECK_EXACT, 1, 1, 1, _float32Checks);
	SetupFunction("rect", Rect, NULL, "value", ARG_CHECK_EXACT, 1, 1, 1, _float32Checks);
	SetupFunction("tri", Tri, NULL, "value", ARG_CHECK_EXACT, 1, 1, 1, _float32Checks);
	SetupFunction("min", Min, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _float32Checks);
	SetupFunction("max", Max, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _float32Checks);

	SetupFunction("sqrt", Sqrt, &_quietMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("sqrt!", Sqrt, &_loudMath, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("sqr", Sqr, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("cube", Cube, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("cube-root", CubeRoot, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("hypotenuse", Hypotenuse, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupFunction("half", Half, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("double", Double, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupSynonym("double", "dbl");

	SetupFunction("ceil", Ceil, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("floor", Floor, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("trunc", Trunc, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("round", Round, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("bank-round", BankRound, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("modf", Modf, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);

	SetupFunction("log", Log, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _float32Checks);
	SetupFunction("log2", Log2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("ln", Ln, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("ln1p", Ln1p, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("exp", Exp, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("exp2", Exp2, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("exp10", Exp10, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("expm1", Expm1, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("^", Pow, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupSynonym("log2", "lg");

	SetupFunction("gamma", Gamma, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("ln-gamma", LnGamma, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("factorial", Factorial, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("ln-factorial", LnFactorial, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);

	SetupFunction("sin", Sin, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("cos", Cos, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("tan", Tan, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("atan", ATan, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _float32Checks);
	SetupSynonym("atan", "atan2");
	SetupFunction("asin", ASin, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("acos", ACos, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("deg-to-rad", DegToRad, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("rad-to-deg", RadToDeg, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);

	SetupFunction("sinh", Sinh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("cosh", Cosh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("tanh", Tanh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("atanh", ATanh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("asinh", ASinh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("acosh", ACosh, NULL, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);

	SetupFunction("odd?", ValueTest, (void *)ODD_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("even?", ValueTest, (void *)EVEN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("zero?", ValueTest, (void *)ZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("one?", ValueTest, (void *)ONE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("nonzero?", ValueTest, (void *)NONZERO_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("positive?", ValueTest, (void *)POS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupSynonym("positive?", "pos?");
	SetupFunction("finite?", ValueTest, (void *)FINITE_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("infinite?", ValueTest, (void *)INF_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupSynonym("infinite?", "inf?");
	SetupFunction("nan?", ValueTest, (void *)NAN_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupFunction("nonpositive?", ValueTest, (void *)NONPOS_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupSynonym("nonpositive?", "nonpos?");
	SetupFunction("negative?", ValueTest, (void *)NEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupSynonym("negative?", "neg?");
	SetupFunction("nonnegative?", ValueTest, (void *)NONNEG_TEST, "value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _float32Checks);
	SetupSynonym("nonnegative?", "nonneg?");

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32ComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32ComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);
	SetupSynonym("compare", "cmp");

	SetupFunction("range-to", RangeTo, NULL, "start end", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _float32Checks);

	SetupFunction("random", RandomFunc, base, "count", 0, 1, 1, 0, NULL);

	SetupData("inf",   SmileFloat32_Create(inf));

	SetupData("pi",    SmileFloat32_Create((Float32)3.14159265358979323846264338327950288));
	SetupData("e",     SmileFloat32_Create((Float32)2.71828182845904523536028747135266249));
	SetupData("tau",   SmileFloat32_Create((Float32)6.28318530717958647692528676655900576));
	SetupData("sqrt2", SmileFloat32_Create((Float32)1.41421356237309504880168872420969807));
	SetupData("sqrt3", SmileFloat32_Create((Float32)1.73205080756887729352744634150587236));
	SetupData("sqrt5", SmileFloat32_Create((Float32)2.23606797749978969640917366873127623));
	SetupData("phi",   SmileFloat32_Create((Float32)1.61803398874989484820458683436563811));

	SetupData("G",     SmileFloat32_Create((Float32)6.67408e-11));
	SetupData("c",     SmileFloat32_Create((Float32)299792458.0));
	SetupData("h",     SmileFloat32_Create((Float32)6.626070040e-34));

	SetupData("g",     SmileFloat32_Create((Float32)9.80665));
}
