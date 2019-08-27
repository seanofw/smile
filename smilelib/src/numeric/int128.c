
#include <smile/types.h>
#include <smile/numeric/int128.h>

void UInt64_MulExtended128_External(UInt64 x, UInt64 y, UInt64 *lo, UInt64 *hi)
{
	UInt64 x_lo = (UInt32)x;
	UInt64 x_hi = x >> 32;
	UInt64 y_lo = (UInt32)y;
	UInt64 y_hi = y >> 32;

	UInt64 xy_hi  = x_hi * y_hi;
	UInt64 xy_mid = x_hi * y_lo;
	UInt64 yx_mid = y_hi * x_lo;
	UInt64 xy_lo  = x_lo * y_lo;

	UInt64 carry_bit = ((UInt64)(UInt32)xy_mid + (UInt64)(UInt32)yx_mid + (xy_lo >> 32)) >> 32;
	*hi = xy_hi + (xy_mid >> 32) + (yx_mid >> 32) + carry_bit;
	*lo = x * y;
}

void Int64_MulExtended128_External(Int64 x, Int64 y, Int64 *lo, Int64 *hi)
{
	Bool negativeResult = ((x ^ y) >> 63) & 1;
	if (x < 0) x = -x;
	if (y < 0) y = -y;

	UInt64_MulExtended128_External((UInt64)x, (UInt64)y, (UInt64 *)&lo, (UInt64 *)&hi);

	if (negativeResult)
		Int128_NegInPlace(lo, hi);
}
