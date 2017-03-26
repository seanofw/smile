#ifndef __SMILE_NUMERIC_REAL128_H__
#define __SMILE_NUMERIC_REAL128_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

//-------------------------------------------------------------------------------------------------
// Predefined constants.
//
// Note that the NaN constants are only one of many possible NaNs of each sign; these NaNs
// are not the only NaNs that can exist.  (Also note that these are Quiet NaNs, not Signaling
// NaNs; Smile does not support Signaling NaNs).

SMILE_API_DATA Real128 Real128_NegNaN;
SMILE_API_DATA Real128 Real128_NegInf;
SMILE_API_DATA Real128 Real128_NegSixteen;
SMILE_API_DATA Real128 Real128_NegTen;
SMILE_API_DATA Real128 Real128_NegTwo;
SMILE_API_DATA Real128 Real128_NegOne;
SMILE_API_DATA Real128 Real128_NegZero;
SMILE_API_DATA Real128 Real128_Zero;
SMILE_API_DATA Real128 Real128_One;
SMILE_API_DATA Real128 Real128_Two;
SMILE_API_DATA Real128 Real128_Ten;
SMILE_API_DATA Real128 Real128_Sixteen;
SMILE_API_DATA Real128 Real128_Inf;
SMILE_API_DATA Real128 Real128_NaN;

//-------------------------------------------------------------------------------------------------
// External functions.

SMILE_API_FUNC Real128 Real128_FromInt32(Int32 int32);
SMILE_API_FUNC Real128 Real128_FromInt64(Int64 int64);
SMILE_API_FUNC Real128 Real128_FromFloat32(Float32 float32);
SMILE_API_FUNC Real128 Real128_FromFloat64(Float64 float64);

SMILE_API_FUNC Float32 Real128_ToFloat32(Real128 real128);
SMILE_API_FUNC Float64 Real128_ToFloat64(Real128 real128);
SMILE_API_FUNC Real32 Real128_ToReal32(Real128 real128);
SMILE_API_FUNC Real64 Real128_ToReal64(Real128 real128);

SMILE_API_FUNC Bool Real128_TryParseInternal(const Byte *text, Int length, Real128 *result);
SMILE_API_FUNC String Real128_ToFixedString(Real128 real128, Int minIntDigits, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC String Real128_ToExpString(Real128 real128, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC String Real128_ToStringEx(Real128 real128, Int minIntDigits, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC Int32 Real128_Decompose(Byte *str, Int32 *exp, Int32 *kind, Real128 real128);

SMILE_API_FUNC Real128 Real128_Add(Real128 a, Real128 b);
SMILE_API_FUNC Real128 Real128_Sub(Real128 a, Real128 b);
SMILE_API_FUNC Real128 Real128_Mul(Real128 a, Real128 b);
SMILE_API_FUNC Real128 Real128_Div(Real128 a, Real128 b);
SMILE_API_FUNC Real128 Real128_Mod(Real128 a, Real128 b);
SMILE_API_FUNC Real128 Real128_Rem(Real128 a, Real128 b);
SMILE_API_FUNC Real128 Real128_IeeeRem(Real128 a, Real128 b);

SMILE_API_FUNC Real128 Real128_Neg(Real128 real128);
SMILE_API_FUNC Real128 Real128_Abs(Real128 real128);

SMILE_API_FUNC Real128 Real128_Ceil(Real128 real128);
SMILE_API_FUNC Real128 Real128_Floor(Real128 real128);
SMILE_API_FUNC Real128 Real128_Trunc(Real128 real128);
SMILE_API_FUNC Real128 Real128_Modf(Real128 real128, Real128 *intPart);
SMILE_API_FUNC Real128 Real128_Round(Real128 real128);
SMILE_API_FUNC Real128 Real128_BankRound(Real128 real128);

SMILE_API_FUNC Real128 Real128_Sqrt(Real128 real128);

SMILE_API_FUNC Bool Real128_IsInf(Real128 real128);
SMILE_API_FUNC Bool Real128_IsNaN(Real128 real128);
SMILE_API_FUNC Bool Real128_IsNeg(Real128 real128);
SMILE_API_FUNC Bool Real128_IsZero(Real128 real128);
SMILE_API_FUNC Bool Real128_IsFinite(Real128 real128);
SMILE_API_FUNC Bool Real128_IsOrderable(Real128 a, Real128 b);

SMILE_API_FUNC Int Real128_Compare(Real128 a, Real128 b);

SMILE_API_FUNC Bool Real128_Eq(Real128 a, Real128 b);
SMILE_API_FUNC Bool Real128_Ne(Real128 a, Real128 b);
SMILE_API_FUNC Bool Real128_Lt(Real128 a, Real128 b);
SMILE_API_FUNC Bool Real128_Gt(Real128 a, Real128 b);
SMILE_API_FUNC Bool Real128_Le(Real128 a, Real128 b);
SMILE_API_FUNC Bool Real128_Ge(Real128 a, Real128 b);

//-------------------------------------------------------------------------------------------------
// Inline functions.

Inline Real128 Real128_Sign(Real128 x)
{
	return Real128_IsZero(x) ? Real128_Zero
		: Real128_IsNeg(x) ? Real128_NegOne
		: Real128_One;
}

Inline Int Real128_IntSign(Real128 x)
{
	return Real128_IsZero(x) ? 0
		: Real128_IsNeg(x) ? -1
		: +1;
}

Inline Bool Real128_TryParse(String str, Real128 *result)
{
	return Real128_TryParseInternal(String_GetBytes(str), String_Length(str), result);
}

Inline Real128 Real128_Parse(String str)
{
	Real128 result;
	Real128_TryParseInternal(String_GetBytes(str), String_Length(str), &result);
	return result;
}

Inline Bool Real128_TryParseC(const char *str, Real128 *result)
{
	return Real128_TryParseInternal((const Byte *)str, StrLen(str), result);
}

Inline Real128 Real128_ParseC(const char *str)
{
	Real128 result;
	Real128_TryParseInternal((const Byte *)str, StrLen(str), &result);
	return result;
}

Inline String Real128_ToString(Real128 real128)
{
	return Real128_ToStringEx(real128, 0, 0, False);
}

#endif
