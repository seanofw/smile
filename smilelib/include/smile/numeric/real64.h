#ifndef __SMILE_NUMERIC_REAL64_H__
#define __SMILE_NUMERIC_REAL64_H__

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

SMILE_API_DATA Real64 Real64_NegNaN;
SMILE_API_DATA Real64 Real64_NegInf;
SMILE_API_DATA Real64 Real64_NegSixteen;
SMILE_API_DATA Real64 Real64_NegTen;
SMILE_API_DATA Real64 Real64_NegTwo;
SMILE_API_DATA Real64 Real64_NegOne;
SMILE_API_DATA Real64 Real64_NegZero;
SMILE_API_DATA Real64 Real64_Zero;
SMILE_API_DATA Real64 Real64_One;
SMILE_API_DATA Real64 Real64_Two;
SMILE_API_DATA Real64 Real64_Ten;
SMILE_API_DATA Real64 Real64_Sixteen;
SMILE_API_DATA Real64 Real64_Inf;
SMILE_API_DATA Real64 Real64_NaN;

//-------------------------------------------------------------------------------------------------
// External functions.

SMILE_API_FUNC Real64 Real64_FromInt32(Int32 int32);
SMILE_API_FUNC Real64 Real64_FromInt64(Int64 int64);
SMILE_API_FUNC Real64 Real64_FromFloat32(Float32 float32);
SMILE_API_FUNC Real64 Real64_FromFloat64(Float64 float64);

SMILE_API_FUNC Real32 Real64_ToReal32(Real64 real64);
SMILE_API_FUNC Real128 Real64_ToReal128(Real64 real64);
SMILE_API_FUNC Float32 Real64_ToFloat32(Real64 real64);
SMILE_API_FUNC Float64 Real64_ToFloat64(Real64 real64);

SMILE_API_FUNC Bool Real64_TryParseInternal(const Byte *text, Int length, Real64 *result);
SMILE_API_FUNC String Real64_ToFixedString(Real64 real64, Int minIntDigits, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC String Real64_ToExpString(Real64 real64, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC String Real64_ToStringEx(Real64 real64, Int minIntDigits, Int minFracDigits, Bool forceSign);
SMILE_API_FUNC Int32 Real64_Decompose(Byte *str, Int32 *exp, Int32 *kind, Real64 real64);

SMILE_API_FUNC Real64 Real64_Add(Real64 a, Real64 b);
SMILE_API_FUNC Real64 Real64_Sub(Real64 a, Real64 b);
SMILE_API_FUNC Real64 Real64_Mul(Real64 a, Real64 b);
SMILE_API_FUNC Real64 Real64_Div(Real64 a, Real64 b);
SMILE_API_FUNC Real64 Real64_Mod(Real64 a, Real64 b);
SMILE_API_FUNC Real64 Real64_Rem(Real64 a, Real64 b);
SMILE_API_FUNC Real64 Real64_IeeeRem(Real64 a, Real64 b);

SMILE_API_FUNC Real64 Real64_Neg(Real64 real64);
SMILE_API_FUNC Real64 Real64_Abs(Real64 real64);

SMILE_API_FUNC Real64 Real64_Ceil(Real64 real64);
SMILE_API_FUNC Real64 Real64_Floor(Real64 real64);
SMILE_API_FUNC Real64 Real64_Trunc(Real64 real64);
SMILE_API_FUNC Real64 Real64_Modf(Real64 real64, Real64 *intPart);
SMILE_API_FUNC Real64 Real64_Round(Real64 real64);
SMILE_API_FUNC Real64 Real64_BankRound(Real64 real64);

SMILE_API_FUNC Real64 Real64_Sqrt(Real64 real64);

SMILE_API_FUNC Bool Real64_IsInf(Real64 real64);
SMILE_API_FUNC Bool Real64_IsNaN(Real64 real64);
SMILE_API_FUNC Bool Real64_IsNeg(Real64 real64);
SMILE_API_FUNC Bool Real64_IsZero(Real64 real64);
SMILE_API_FUNC Bool Real64_IsFinite(Real64 real64);
SMILE_API_FUNC Bool Real64_IsOrderable(Real64 a, Real64 b);

SMILE_API_FUNC Int Real64_Compare(Real64 a, Real64 b);

SMILE_API_FUNC Bool Real64_Eq(Real64 a, Real64 b);
SMILE_API_FUNC Bool Real64_Ne(Real64 a, Real64 b);
SMILE_API_FUNC Bool Real64_Lt(Real64 a, Real64 b);
SMILE_API_FUNC Bool Real64_Gt(Real64 a, Real64 b);
SMILE_API_FUNC Bool Real64_Le(Real64 a, Real64 b);
SMILE_API_FUNC Bool Real64_Ge(Real64 a, Real64 b);

//-------------------------------------------------------------------------------------------------
// Inline functions.

Inline Real64 Real64_Sign(Real64 x)
{
	return Real64_IsZero(x) ? Real64_Zero
		: Real64_IsNeg(x) ? Real64_NegOne
		: Real64_One;
}

Inline Int Real64_IntSign(Real64 x)
{
	return Real64_IsZero(x) ? 0
		: Real64_IsNeg(x) ? -1
		: +1;
}

Inline Bool Real64_TryParse(String str, Real64 *result)
{
	return Real64_TryParseInternal(String_GetBytes(str), String_Length(str), result);
}

Inline Real64 Real64_Parse(String str)
{
	Real64 result;
	Real64_TryParseInternal(String_GetBytes(str), String_Length(str), &result);
	return result;
}

Inline Bool Real64_TryParseC(const char *str, Real64 *result)
{
	return Real64_TryParseInternal((const Byte *)str, StrLen(str), result);
}

Inline Real64 Real64_ParseC(const char *str)
{
	Real64 result;
	Real64_TryParseInternal((const Byte *)str, StrLen(str), &result);
	return result;
}

Inline String Real64_ToString(Real64 real64)
{
	return Real64_ToStringEx(real64, 0, 0, False);
}

#endif
