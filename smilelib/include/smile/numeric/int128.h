#ifndef __SMILE_NUMERIC_INT128_H__
#define __SMILE_NUMERIC_INT128_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

SMILE_API_FUNC void UInt64_MulExtended128_External(UInt64 x, UInt64 y, UInt64 *lo, UInt64 *hi);
SMILE_API_FUNC void Int64_MulExtended128_External(Int64 x, Int64 y, Int64 *lo, Int64 *hi);

Inline void UInt128_Add(UInt64 x_lo, UInt64 x_hi, UInt64 y_lo, UInt64 y_hi,
	UInt64 *result_lo, UInt64 *result_hi)
{
	*result_lo = x_lo + y_lo;
	*result_hi = x_hi + y_hi + (x_lo + y_lo < x_lo);
}

Inline void UInt128_Sub(UInt64 x_lo, UInt64 x_hi, UInt64 y_lo, UInt64 y_hi,
	UInt64 * result_lo, UInt64 * result_hi)
{
	*result_lo = x_lo - y_lo;
	*result_hi = x_hi - y_hi - (x_lo - y_lo > x_lo);
}

Inline void UInt128_IncInPlace(UInt64 *lo, UInt64 *hi)
{
	if ((*lo)++ == 0)
		(*hi)++;
}

Inline void UInt128_DecInPlace(UInt64 *lo, UInt64 *hi)
{
	if ((*lo)-- == (UInt64)(Int64)-1)
		(*hi)--;
}

Inline void Int128_IncInPlace(Int64 *lo, Int64 *hi)
{
	if ((*lo)++ == 0)
		(*hi)++;
}

Inline void Int128_DecInPlace(Int64 *lo, Int64 *hi)
{
	if ((*lo)-- == -1)
		(*hi)--;
}

Inline void Int128_NegInPlace(Int64 *lo, Int64 *hi)
{
	*lo = ~*lo;
	*hi = ~*hi;

	if (*lo == 0)
		(*hi)++;
}

Inline void UInt64_MulExtended128(UInt64 x, UInt64 y, UInt64 *lo, UInt64 *hi)
{
#	if SMILE_COMPILER_HAS_UINT128
		UInt128 result = (UInt128)x * (UInt128)y;
		*lo = (UInt64)result;
		*hi = (UInt64)(result >> 64);
#	else
		UInt64_MulExtended128_External(x, y, lo, hi);
#	endif
}

Inline void Int64_MulExtended128(Int64 x, Int64 y, Int64 *lo, Int64 *hi)
{
#	if SMILE_COMPILER_HAS_UINT128
		Int128 result = (Int128)x * (Int128)y;
		*lo = (Int64)result;
		*hi = (Int64)(result >> 64);
#	else
		Int64_MulExtended128_External(x, y, lo, hi);
#	endif
}

#endif
