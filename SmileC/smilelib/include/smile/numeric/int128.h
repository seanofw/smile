#ifndef __SMILE_NUMERIC_INT128_H__
#define __SMILE_NUMERIC_INT128_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

//-------------------------------------------------------------------------------------------------
// Predefined constants.

SMILE_API_DATA Int128 Int128_Min;
SMILE_API_DATA Int128 Int128_NegSixteen;
SMILE_API_DATA Int128 Int128_NegTen;
SMILE_API_DATA Int128 Int128_NegTwo;
SMILE_API_DATA Int128 Int128_NegOne;
SMILE_API_DATA Int128 Int128_NegZero;
SMILE_API_DATA Int128 Int128_Zero;
SMILE_API_DATA Int128 Int128_One;
SMILE_API_DATA Int128 Int128_Two;
SMILE_API_DATA Int128 Int128_Ten;
SMILE_API_DATA Int128 Int128_Sixteen;
SMILE_API_DATA Int128 Int128_Max;

//-------------------------------------------------------------------------------------------------
// External functions.

SMILE_API_FUNC Int128 Int128_FromInt32(Int32 int32);
SMILE_API_FUNC Int128 Int128_FromInt64(Int64 int64);

SMILE_API_FUNC Bool Int128_TryParse(String str, Int128 *result);
SMILE_API_FUNC String Int128_ToStringEx(Int128 int128, Int minIntDigits, Bool forceSign);

SMILE_API_FUNC Int128 Int128_Add(Int128 a, Int128 b);
SMILE_API_FUNC Int128 Int128_Sub(Int128 a, Int128 b);
SMILE_API_FUNC Int128 Int128_Mul(Int128 a, Int128 b);
SMILE_API_FUNC Int128 Int128_Div(Int128 a, Int128 b);
SMILE_API_FUNC Int128 Int128_Mod(Int128 a, Int128 b);
SMILE_API_FUNC void Int128_DivMo(Int128 a, Int128 b, Int128 *div, Int128 *mod);
SMILE_API_FUNC Int128 Int128_Rem(Int128 a, Int128 b);

SMILE_API_FUNC Int128 Int128_Neg(Int128 int128);
SMILE_API_FUNC Int128 Int128_Abs(Int128 int128);

SMILE_API_FUNC Bool Int128_IsNeg(Int128 int128);
SMILE_API_FUNC Bool Int128_IsZero(Int128 int128);

SMILE_API_FUNC Int Int128_Compare(Int128 a, Int128 b);

SMILE_API_FUNC Bool Int128_Eq(Int128 a, Int128 b);
SMILE_API_FUNC Bool Int128_Ne(Int128 a, Int128 b);
SMILE_API_FUNC Bool Int128_Lt(Int128 a, Int128 b);
SMILE_API_FUNC Bool Int128_Gt(Int128 a, Int128 b);
SMILE_API_FUNC Bool Int128_Le(Int128 a, Int128 b);
SMILE_API_FUNC Bool Int128_Ge(Int128 a, Int128 b);

//-------------------------------------------------------------------------------------------------
// Inline functions.

Inline Int128 Int128_Sign(Int128 x)
{
	return x.hi == 0 && x.lo == 0 ? Int128_Zero
		: x.hi < 0 ? Int128_NegOne
		: Int128_One;
}

Inline Int Int128_IntSign(Int128 x)
{
	return x.hi == 0 && x.lo == 0 ? 0
		: x.hi < 0 ? -1
		: +1;
}

Inline Int128 Int128_Parse(String str)
{
	Int128 result;
	Int128_TryParse(str, &result);
	return result;
}

Inline Bool Int128_TryParseC(const char *str, Int128 *result)
{
	DECLARE_TEMP_C_STRING(string);
	INIT_TEMP_C_STRING(string, str);

	return Int128_TryParse(string, result);
}

Inline Int128 Int128_ParseC(const char *str)
{
	Int128 result;
	DECLARE_TEMP_C_STRING(string);
	INIT_TEMP_C_STRING(string, str);

	Int128_TryParse(string, &result);
	return result;
}

Inline String Int128_ToString(Real128 real128)
{
	return Int128_ToStringEx(real128, 0, False);
}

#endif
