#define BID_128RES
#include <stdio.h>
#include "bid_internal.h"
#include "bid128_2_str.h"
#include "bid128_2_str_macros.h"

#if DECIMAL_CALL_BY_REFERENCE

int bid128_decompose_to_string (char *coeff, int *rexp, int *kind, BID_UINT128 *px
	_EXC_FLAGS_PARAM _EXC_MASKS_PARAM _EXC_INFO_PARAM)
{
	BID_UINT128 x;
#else

int bid128_decompose_to_string(char *str, int *rexp, int *kind, BID_UINT128 x
	_EXC_FLAGS_PARAM _EXC_MASKS_PARAM _EXC_INFO_PARAM)
{
#endif
	BID_UINT64 x_sign;
	BID_UINT64 x_exp;
	int exp; 	// unbiased exponent
	// Note: C1.w[1], C1.w[0] represent x_signif_hi, x_signif_lo (all are BID_UINT64)
	BID_UINT128 C1;
	unsigned int k = 0; // pointer in the string
	BID_UINT64 HI_18Dig, LO_18Dig, Tmp;
	BID_UINT32 MiDi[12], *ptr;
	char *c_ptr_start, *c_ptr;
	int midi_ind, k_lcv, len;
	int save_fpsf;

#if DECIMAL_CALL_BY_REFERENCE
	x = *px;
#endif

	save_fpsf = *pfpsf; // dummy

	BID_SWAP128(x);
	// check for NaN or Infinity
	if ((x.w[1] & MASK_SPECIAL) == MASK_SPECIAL) {
		// x is special
		if ((x.w[1] & MASK_NAN) == MASK_NAN) { // x is NAN
			if ((x.w[1] & MASK_SNAN) == MASK_SNAN) { // x is SNAN
				// set invalid flag
				*kind = ((BID_SINT64)x.w[1] < 0) ? BID_KIND_NEG_SNAN : BID_KIND_POS_SNAN;
			}
			else { // x is QNaN
				*kind = ((BID_SINT64)x.w[1] < 0) ? BID_KIND_NEG_QNAN : BID_KIND_POS_QNAN;
			}
		}
		else { // x is not a NaN, so it must be infinity
			if ((x.w[1] & MASK_SIGN) == 0x0ull) { // x is +inf
				*kind = BID_KIND_POS_INF;
			}
			else { // x is -inf 
				*kind = BID_KIND_NEG_INF;
			}
		}
		*rexp = 0;
		*str = '\0';
		return 0;
	}
	else if (((x.w[1] & MASK_COEFF) == 0x0ull) && (x.w[0] == 0x0ull)) {
		// x is 0
		len = 0;

		//determine if +/-
		if (x.w[1] & MASK_SIGN)
			*kind = BID_KIND_NEG_ZERO;
		else
			*kind = BID_KIND_POS_ZERO;
		str[0] = '0';
		str[1] = '\0';

		// extract the exponent
		exp = (int)(((x.w[1] & MASK_EXP) >> 49) - 6176);
		if (exp > (((0x5ffe) >> 1) - (6176))) {
			exp = (int)((((x.w[1] << 2) & MASK_EXP) >> 49) - 6176);
		}
		*rexp = exp;
		return 1;
	}

	// x is not special and is not zero
	// unpack x
	x_sign = x.w[1] & MASK_SIGN;// 0 for positive, MASK_SIGN for negative
	x_exp = x.w[1] & MASK_EXP;// biased and shifted left 49 bit positions
	if ((x.w[1] & 0x6000000000000000ull) == 0x6000000000000000ull)
		x_exp = (x.w[1] << 2) & MASK_EXP;// biased and shifted left 49 bit positions
	C1.w[1] = x.w[1] & MASK_COEFF;
	C1.w[0] = x.w[0];
	exp = (x_exp >> 49) - 6176;
	*rexp = exp;

	// determine sign's representation as a char
	if (x_sign)
		*kind = BID_KIND_NEG_NUM;
	else
		*kind = BID_KIND_POS_NUM;

	// determine coefficient's representation as a decimal string

	// if zero or non-canonical, set coefficient to '0'
	if ((C1.w[1] > 0x0001ed09bead87c0ull) ||
		(C1.w[1] == 0x0001ed09bead87c0ull &&
		(C1.w[0] > 0x378d8e63ffffffffull)) ||
		((x.w[1] & 0x6000000000000000ull) == 0x6000000000000000ull) ||
		((C1.w[1] == 0) && (C1.w[0] == 0))) {
		str[k++] = '0';
	}
	else {
		/* ****************************************************
			This takes a bid coefficient in C1.w[1],C1.w[0]
			and put the converted character sequence at location
			starting at &(str[k]). The function returns the number
			of MiDi returned. Note that the character sequence
			does not have leading zeros EXCEPT when the input is of
			zero value. It will then output 1 character '0'
			The algorithm essentailly tries first to get a sequence of
			Millenial Digits "MiDi" and then uses table lookup to get the
			character strings of these MiDis.
			**************************************************** */
		/* Algorithm first decompose possibly 34 digits in hi and lo
			18 digits. (The high can have at most 16 digits). It then
			uses macro that handle 18 digit portions.
			The first step is to get hi and lo such that
			2^(64) C1.w[1] + C1.w[0] = hi * 10^18  + lo,   0 <= lo < 10^18.
			We use a table lookup method to obtain the hi and lo 18 digits.
			[C1.w[1],C1.w[0]] = c_8 2^(107) + c_7 2^(101) + ... + c_0 2^(59) + d
			where 0 <= d < 2^59 and each c_j has 6 bits. Because d fits in
			18 digits,  we set hi = 0, and lo = d to begin with.
			We then retrieve from a table, for j = 0, 1, ..., 8
			that gives us A and B where c_j 2^(59+6j) = A * 10^18 + B.
			hi += A ; lo += B; After each accumulation into lo, we normalize
			immediately. So at the end, we have the decomposition as we need. */

		Tmp = C1.w[0] >> 59;
		LO_18Dig = (C1.w[0] << 5) >> 5;
		Tmp += (C1.w[1] << 5);
		HI_18Dig = 0;
		k_lcv = 0;
		// Tmp = {C1.w[1]{49:0}, C1.w[0]{63:59}}
		// Lo_18Dig = {C1.w[0]{58:0}}

		while (Tmp) {
			midi_ind = (int)(Tmp & 0x000000000000003FLL);
			midi_ind <<= 1;
			Tmp >>= 6;
			HI_18Dig += mod10_18_tbl[k_lcv][midi_ind++];
			LO_18Dig += mod10_18_tbl[k_lcv++][midi_ind];
			__L0_Normalize_10to18(HI_18Dig, LO_18Dig);
		}
		ptr = MiDi;
		if (HI_18Dig == 0LL) {
			__L1_Split_MiDi_6_Lead(LO_18Dig, ptr);
		}
		else {
			__L1_Split_MiDi_6_Lead(HI_18Dig, ptr);
			__L1_Split_MiDi_6(LO_18Dig, ptr);
		}
		len = ptr - MiDi;
		c_ptr_start = &(str[k]);
		c_ptr = c_ptr_start;

		/* now convert the MiDi into character strings */
		__L0_MiDi2Str_Lead(MiDi[0], c_ptr);
		for (k_lcv = 1; k_lcv < len; k_lcv++) {
			__L0_MiDi2Str(MiDi[k_lcv], c_ptr);
		}
		k = k + (c_ptr - c_ptr_start);
	}
	str[k] = '\0';
	return k;
}


#if DECIMAL_CALL_BY_REFERENCE

int bid64_decompose_to_string(char *coeff, int *rexp, int *kind, BID_UINT64 *px
	_EXC_FLAGS_PARAM _EXC_MASKS_PARAM _EXC_INFO_PARAM)
{
	BID_UINT64 x;
#else

int bid64_decompose_to_string(char *str, int *rexp, int *kind, BID_UINT64 x
	_EXC_FLAGS_PARAM _EXC_MASKS_PARAM _EXC_INFO_PARAM)
{
#endif
	// the destination string (pointed to by ps) must be pre-allocated
	BID_UINT64 sign_x, coefficient_x, D, ER10;
	int istart, exponent_x, j, digits_x, bin_expon_cx;
	int_float tempx;
	BID_UINT32 MiDi[12], *ptr;
	BID_UINT64 HI_18Dig, LO_18Dig, Tmp;
	char *c_ptr_start, *c_ptr;
	int midi_ind, k_lcv, len;
	unsigned int save_fpsf;

#if DECIMAL_CALL_BY_REFERENCE
	x = *px;
#endif

	save_fpsf = *pfpsf; // place holder only
	// unpack arguments, check for NaN or Infinity
	if (!unpack_BID64(&sign_x, &exponent_x, &coefficient_x, x)) {
		// x is Inf. or NaN or 0

		// Inf or NaN?
		if ((x & 0x7800000000000000ull) == 0x7800000000000000ull) {
			if ((x & 0x7c00000000000000ull) == 0x7c00000000000000ull) {
				if (sign_x) {
					*kind = ((x & MASK_SNAN) == MASK_SNAN) ? BID_KIND_NEG_SNAN : BID_KIND_NEG_QNAN;
				}
				else {
					*kind = ((x & MASK_SNAN) == MASK_SNAN) ? BID_KIND_POS_SNAN : BID_KIND_POS_QNAN;
				}
				*rexp = 0;
				*str = '\0';
				return 0;
			}
			// x is Inf
			*kind = sign_x ? BID_KIND_NEG_INF : BID_KIND_POS_INF;
			*rexp = 0;
			*str = '\0';
			return 0;
		}
		// 0
		*kind = sign_x ? BID_KIND_NEG_ZERO : BID_KIND_POS_ZERO;
		*rexp = exponent_x - 398;
		str[0] = '0';
		str[1] = '\0';
		return 1;
	}

	// convert expon, coeff to ASCII
	exponent_x -= DECIMAL_EXPONENT_BIAS;

	ER10 = 0x1999999a;

	*kind = sign_x ? BID_KIND_NEG_NUM : BID_KIND_POS_NUM;
	*rexp = exponent_x;

	// if zero or non-canonical, set coefficient to '0'
	istart = 0;
	if ((coefficient_x > 9999999999999999ull) ||	// non-canonical
		((coefficient_x == 0))	// significand is zero
		) {
		str[istart++] = '0';
	}
	else {
		/* ****************************************************
		   This takes a bid coefficient in C1.w[1],C1.w[0]
		   and put the converted character sequence at location
		   starting at &(str[k]). The function returns the number
		   of MiDi returned. Note that the character sequence
		   does not have leading zeros EXCEPT when the input is of
		   zero value. It will then output 1 character '0'
		   The algorithm essentailly tries first to get a sequence of
		   Millenial Digits "MiDi" and then uses table lookup to get the
		   character strings of these MiDis.
		   **************************************************** */
		/* Algorithm first decompose possibly 34 digits in hi and lo
		   18 digits. (The high can have at most 16 digits). It then
		   uses macro that handle 18 digit portions.
		   The first step is to get hi and lo such that
		   2^(64) C1.w[1] + C1.w[0] = hi * 10^18  + lo,   0 <= lo < 10^18.
		   We use a table lookup method to obtain the hi and lo 18 digits.
		   [C1.w[1],C1.w[0]] = c_8 2^(107) + c_7 2^(101) + ... + c_0 2^(59) + d
		   where 0 <= d < 2^59 and each c_j has 6 bits. Because d fits in
		   18 digits,  we set hi = 0, and lo = d to begin with.
		   We then retrieve from a table, for j = 0, 1, ..., 8
		   that gives us A and B where c_j 2^(59+6j) = A * 10^18 + B.
		   hi += A ; lo += B; After each accumulation into lo, we normalize
		   immediately. So at the end, we have the decomposition as we need. */

		Tmp = coefficient_x >> 59;
		LO_18Dig = (coefficient_x << 5) >> 5;
		HI_18Dig = 0;
		k_lcv = 0;

		while (Tmp) {
			midi_ind = (int)(Tmp & 0x000000000000003FLL);
			midi_ind <<= 1;
			Tmp >>= 6;
			HI_18Dig += mod10_18_tbl[k_lcv][midi_ind++];
			LO_18Dig += mod10_18_tbl[k_lcv++][midi_ind];
			__L0_Normalize_10to18(HI_18Dig, LO_18Dig);
		}

		ptr = MiDi;
		__L1_Split_MiDi_6_Lead(LO_18Dig, ptr);
		len = ptr - MiDi;
		c_ptr_start = &(str[istart]);
		c_ptr = c_ptr_start;

		/* now convert the MiDi into character strings */
		__L0_MiDi2Str_Lead(MiDi[0], c_ptr);
		for (k_lcv = 1; k_lcv < len; k_lcv++) {
			__L0_MiDi2Str(MiDi[k_lcv], c_ptr);
		}
		istart = istart + (c_ptr - c_ptr_start);
	}

	str[istart] = '\0';

	return istart;
}


#if DECIMAL_CALL_BY_REFERENCE

int bid32_decompose_to_string(char *coeff, int *rexp, int *kind, BID_UINT32 *px
	_EXC_FLAGS_PARAM _EXC_MASKS_PARAM _EXC_INFO_PARAM)
{
	BID_UINT32 x;
#else

int bid32_decompose_to_string(char *str, int *rexp, int *kind, BID_UINT32 x
	_EXC_FLAGS_PARAM _EXC_MASKS_PARAM _EXC_INFO_PARAM)
{
#endif
	// the destination string (pointed to by ps) must be pre-allocated
	BID_UINT64 CT;
	int d, j, istart, istart0;
	BID_UINT32 sign_x, coefficient_x;
	int exponent_x;
	unsigned int save_fpsf;

#if DECIMAL_CALL_BY_REFERENCE
	x = *px;
#endif

	save_fpsf = *pfpsf; // place holder only
	// unpack arguments, check for NaN or Infinity
	if (!unpack_BID32(&sign_x, &exponent_x, &coefficient_x, x)) {
		// x is Inf. or NaN or 0
		if ((x & NAN_MASK32) == NAN_MASK32)  {
			if (sign_x) {
				*kind = ((x & SNAN_MASK32) == SNAN_MASK32) ? BID_KIND_NEG_SNAN : BID_KIND_NEG_QNAN;
			}
			else {
				*kind = ((x & SNAN_MASK32) == SNAN_MASK32) ? BID_KIND_POS_SNAN : BID_KIND_POS_QNAN;
			}
			*rexp = 0;
			*str = '\0';
			return 0;
		}
		if ((x & INFINITY_MASK32) == INFINITY_MASK32) {
			*kind = sign_x ? BID_KIND_NEG_INF : BID_KIND_POS_INF;
			*rexp = 0;
			*str = '\0';
			return 0;
		}

		*kind = sign_x ? BID_KIND_NEG_ZERO : BID_KIND_POS_ZERO;
		*rexp = exponent_x - DECIMAL_EXPONENT_BIAS_32;
		str[0] = '0';
		str[1] = '\0';
		return 1;
	}

	// x is not special
	*kind = sign_x ? BID_KIND_NEG_NUM : BID_KIND_POS_NUM;
	*rexp = exponent_x - DECIMAL_EXPONENT_BIAS_32;

	istart = 0;
	if (coefficient_x >= 1000000)
	{
		CT = (BID_UINT64)coefficient_x * 0x431BDE83ull;
		CT >>= 32;
		d = CT >> (50 - 32);
		str[istart++] = d + '0';

		coefficient_x -= d * 1000000;

		// get lower 6 digits
		CT = (BID_UINT64)coefficient_x * 0x20C49BA6ull;
		CT >>= 32;
		d = CT >> (39 - 32);
		str[istart++] = bid_midi_tbl[d][0];
		str[istart++] = bid_midi_tbl[d][1];
		str[istart++] = bid_midi_tbl[d][2];

		d = coefficient_x - d * 1000;

		str[istart++] = bid_midi_tbl[d][0];
		str[istart++] = bid_midi_tbl[d][1];
		str[istart++] = bid_midi_tbl[d][2];
		//str[istart] = 0;
	}
	else if (coefficient_x >= 1000) {
		CT = (BID_UINT64)coefficient_x * 0x20C49BA6ull;
		CT >>= 32;
		d = CT >> (39 - 32);

		istart0 = istart;
		str[istart] = bid_midi_tbl[d][0]; if (str[istart] != '0') istart++;
		str[istart] = bid_midi_tbl[d][1];
		if ((str[istart] != '0') || (istart != istart0)) istart++;
		str[istart++] = bid_midi_tbl[d][2];

		d = coefficient_x - d * 1000;

		str[istart++] = bid_midi_tbl[d][0];
		str[istart++] = bid_midi_tbl[d][1];
		str[istart++] = bid_midi_tbl[d][2];
		//str[istart] = 0;
	}
	else {
		d = coefficient_x;

		istart0 = istart;
		str[istart] = bid_midi_tbl[d][0];  if (str[istart] != '0') istart++;
		str[istart] = bid_midi_tbl[d][1];
		if ((str[istart] != '0') || (istart != istart0)) istart++;
		str[istart++] = bid_midi_tbl[d][2];
	}
	str[istart] = '\0';

	return istart;
}
