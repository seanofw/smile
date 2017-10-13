#ifndef __SMILE_NUMERIC_REAL32_H__
#define __SMILE_NUMERIC_REAL32_H__

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

SMILE_API_DATA Real32 Real32_NegNaN;
SMILE_API_DATA Real32 Real32_NegInf;
SMILE_API_DATA Real32 Real32_NegSixteen;
SMILE_API_DATA Real32 Real32_NegTen;
SMILE_API_DATA Real32 Real32_NegTwo;
SMILE_API_DATA Real32 Real32_NegOne;
SMILE_API_DATA Real32 Real32_NegZero;
SMILE_API_DATA Real32 Real32_Zero;
SMILE_API_DATA Real32 Real32_One;
SMILE_API_DATA Real32 Real32_Two;
SMILE_API_DATA Real32 Real32_Ten;
SMILE_API_DATA Real32 Real32_Sixteen;
SMILE_API_DATA Real32 Real32_Inf;
SMILE_API_DATA Real32 Real32_NaN;

//-------------------------------------------------------------------------------------------------
// External functions.

SMILE_API_FUNC Real32 Real32_FromInt32(Int32 int32);
SMILE_API_FUNC Real32 Real32_FromInt64(Int64 int64);
SMILE_API_FUNC Real32 Real32_FromFloat32(Float32 float32);
SMILE_API_FUNC Real32 Real32_FromFloat64(Float64 float64);

SMILE_API_FUNC Real64 Real32_ToReal64(Real32 real32);
SMILE_API_FUNC Real128 Real32_ToReal128(Real32 real32);
SMILE_API_FUNC Float32 Real32_ToFloat32(Real32 real32);
SMILE_API_FUNC Float64 Real32_ToFloat64(Real32 real32);
SMILE_API_FUNC Int64 Real32_ToInt64(Real32 real32);

SMILE_API_FUNC Bool Real32_TryParseInternal(const Byte *text, Int length, Real32 *result);
SMILE_API_FUNC String Real32_ToFixedString(Real32 real32, Int minIntDigits, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC String Real32_ToExpString(Real32 real32, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC String Real32_ToStringEx(Real32 real32, Int minIntDigits, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC Int32 Real32_Decompose(Byte *str, Int32 *exp, Int32 *kind, Real32 real32);

SMILE_API_FUNC Real32 Real32_Add(Real32 a, Real32 b);
SMILE_API_FUNC Real32 Real32_Sub(Real32 a, Real32 b);
SMILE_API_FUNC Real32 Real32_Mul(Real32 a, Real32 b);
SMILE_API_FUNC Real32 Real32_Div(Real32 a, Real32 b);
SMILE_API_FUNC Real32 Real32_Mod(Real32 a, Real32 b);
SMILE_API_FUNC Real32 Real32_Rem(Real32 a, Real32 b);
SMILE_API_FUNC Real32 Real32_IeeeRem(Real32 a, Real32 b);

SMILE_API_FUNC Real32 Real32_Neg(Real32 real32);
SMILE_API_FUNC Real32 Real32_Abs(Real32 real32);

SMILE_API_FUNC Real32 Real32_Ceil(Real32 real32);
SMILE_API_FUNC Real32 Real32_Floor(Real32 real32);
SMILE_API_FUNC Real32 Real32_Trunc(Real32 real32);
SMILE_API_FUNC Real32 Real32_Modf(Real32 real32, Real32 *intPart);
SMILE_API_FUNC Real32 Real32_Round(Real32 real32);
SMILE_API_FUNC Real32 Real32_BankRound(Real32 real32);

SMILE_API_FUNC Real32 Real32_Sqrt(Real32 real32);

SMILE_API_FUNC Bool Real32_IsInf(Real32 real32);
SMILE_API_FUNC Bool Real32_IsNaN(Real32 real32);
SMILE_API_FUNC Bool Real32_IsNeg(Real32 real32);
SMILE_API_FUNC Bool Real32_IsZero(Real32 real32);
SMILE_API_FUNC Bool Real32_IsFinite(Real32 real32);
SMILE_API_FUNC Bool Real32_IsOrderable(Real32 a, Real32 b);

SMILE_API_FUNC Int Real32_Compare(Real32 a, Real32 b);

SMILE_API_FUNC Bool Real32_Eq(Real32 a, Real32 b);
SMILE_API_FUNC Bool Real32_Ne(Real32 a, Real32 b);
SMILE_API_FUNC Bool Real32_Lt(Real32 a, Real32 b);
SMILE_API_FUNC Bool Real32_Gt(Real32 a, Real32 b);
SMILE_API_FUNC Bool Real32_Le(Real32 a, Real32 b);
SMILE_API_FUNC Bool Real32_Ge(Real32 a, Real32 b);

//-------------------------------------------------------------------------------------------------
// Inline functions.

Inline Real32 Real32_Sign(Real32 x)
{
	return Real32_IsZero(x) ? Real32_Zero
		: Real32_IsNeg(x) ? Real32_NegOne
		: Real32_One;
}

Inline Int Real32_IntSign(Real32 x)
{
	return Real32_IsZero(x) ? 0
		: Real32_IsNeg(x) ? -1
		: +1;
}

Inline Bool Real32_TryParse(String str, Real32 *result)
{
	return Real32_TryParseInternal(String_GetBytes(str), String_Length(str), result);
}

Inline Real32 Real32_Parse(String str)
{
	Real32 result;
	Real32_TryParseInternal(String_GetBytes(str), String_Length(str), &result);
	return result;
}

Inline Bool Real32_TryParseC(const char *str, Real32 *result)
{
	return Real32_TryParseInternal((const Byte *)str, StrLen(str), result);
}

Inline Real32 Real32_ParseC(const char *str)
{
	Real32 result;
	Real32_TryParseInternal((const Byte *)str, StrLen(str), &result);
	return result;
}

Inline String Real32_ToString(Real32 real32)
{
	return Real32_ToStringEx(real32, 0, 0, False);
}

#endif
