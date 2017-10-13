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

#include <smile/numeric/float64.h>
#include <smile/stringbuilder.h>
#include <smile/internal/staticstring.h>

STATIC_STRING(Float_String_Zero, "0");
STATIC_STRING(Float_String_PosZero, "+0");
STATIC_STRING(Float_String_NegZero, "-0");

STATIC_STRING(Float_String_Inf, "inf");
STATIC_STRING(Float_String_PosInf, "+inf");
STATIC_STRING(Float_String_NegInf, "-inf");

STATIC_STRING(Float_String_NaN, "NaN");
STATIC_STRING(Float_String_PosNaN, "+NaN");
STATIC_STRING(Float_String_NegNaN, "-NaN");

STATIC_STRING(Float_String_SNaN, "SNaN");
STATIC_STRING(Float_String_PosSNaN, "+SNaN");
STATIC_STRING(Float_String_NegSNaN, "-SNaN");

Int32 Float64_Decompose(Byte *str, Int32 *exp, Int32 *kind, Float64 float64)
{
	UInt64 floatBits;
	UInt64 exponent;
	UInt64 mantissa;
	Bool sign;
	const int bias = 1023;

	floatBits = *(UInt64 *)(Float64 *)&float64;

	sign = (Bool)((floatBits >> 63) & 1);
	exponent = (floatBits >> 52) & 0x7FF;
	mantissa = floatBits & ((1ULL << 52) - 1);

	if (exponent == 0) {
		// One of the signed zero forms, or possibly a subnormal value.
		if (mantissa == 0) {
			// Zero.
			*kind = sign ? FLOAT_KIND_NEG_ZERO : FLOAT_KIND_POS_ZERO;
			*exp = 0;
			*str++ = '0';
			*str = '\0';
			return 1;
		}
		else {
			// Subnormal values.
			*kind = sign ? FLOAT_KIND_NEG_NUM : FLOAT_KIND_POS_NUM;
			*exp = -bias;
			*str++ = '1';
			*str = '\0';
			return 1;
		}
	}
	else if (exponent == 0x7FF) {
		// NaN or infinity.
		if (mantissa == 0) {
			// Infinity.
			*kind = sign ? FLOAT_KIND_NEG_INF : FLOAT_KIND_POS_INF;
			*exp = 0;
			*str = '\0';
			return 0;
		}
		else {
			// NaN (quiet or signalling).
			if (mantissa & (1ULL << 51)) {
				// QNaN.
				*kind = sign ? FLOAT_KIND_NEG_QNAN : FLOAT_KIND_POS_QNAN;
				*exp = 0;
				*str = '\0';
				return 0;
			}
			else {
				// SNaN.
				*kind = sign ? FLOAT_KIND_NEG_SNAN : FLOAT_KIND_POS_SNAN;
				*exp = 0;
				*str = '\0';
				return 0;
			}
		}
	}

	// Normal values.
	mantissa |= (1ULL << 53);
	*kind = sign ? FLOAT_KIND_NEG_NUM : FLOAT_KIND_POS_NUM;
	*exp = (Int32)exponent - bias;
	*str++ = '1';
	*str = '\0';
	return 1;
}

String Float_ToFixedString(Byte *buffer, Int32 len, Int32 exp, Int32 kind, Int32 minIntDigits, Int32 minFracDigits, Bool forceSign)
{
	DECLARE_INLINE_STRINGBUILDER(numBuilder, 256);

	INIT_INLINE_STRINGBUILDER(numBuilder);

	switch (kind) {
		case FLOAT_KIND_POS_INF:
			return forceSign ? Float_String_PosInf : Float_String_Inf;
		case FLOAT_KIND_NEG_INF:
			return Float_String_NegInf;
		case FLOAT_KIND_POS_QNAN:
			return forceSign ? Float_String_PosNaN : Float_String_NaN;
		case FLOAT_KIND_NEG_QNAN:
			return Float_String_NegNaN;
		case FLOAT_KIND_POS_SNAN:
			return forceSign ? Float_String_PosSNaN : Float_String_SNaN;
		case FLOAT_KIND_NEG_SNAN:
			return Float_String_NegSNaN;
		case FLOAT_KIND_POS_ZERO:
		case FLOAT_KIND_POS_NUM:
			if (forceSign) {
				StringBuilder_AppendByte(numBuilder, '+');
			}
			break;
		case FLOAT_KIND_NEG_ZERO:
		case FLOAT_KIND_NEG_NUM:
			StringBuilder_AppendByte(numBuilder, '-');
			break;
	}

	if (exp >= 0) {

		// All digits are integer; none are fractional.
		if (len + exp < minIntDigits) {
			// Need to prepend initial zeros.
			StringBuilder_AppendRepeat(numBuilder, '0', minIntDigits - (len + exp));
		}

		// Copy all the integer digits.
		StringBuilder_Append(numBuilder, buffer, 0, len);

		if (exp > 0) {
			// Emit trailing zeros.
			StringBuilder_AppendRepeat(numBuilder, '0', exp);
		}

		if (minFracDigits > 0) {
			// Need to append trailing fractional zeros.
			StringBuilder_AppendByte(numBuilder, '.');
			StringBuilder_AppendRepeat(numBuilder, '0', minFracDigits);
		}
	}
	else if (-exp >= len) {

		// All digits are fractional; none are integer.
		if (minIntDigits > 0) {
			// Need to prepend initial zeros.
			StringBuilder_AppendRepeat(numBuilder, '0', minIntDigits);
		}

		// Emit the decimal point.
		StringBuilder_AppendByte(numBuilder, '.');

		if (-exp - len > 0) {
			// Emit leading zeros.
			StringBuilder_AppendRepeat(numBuilder, '0', -exp - len);
		}

		// Copy the fractional digits.
		StringBuilder_Append(numBuilder, buffer, 0, len);

		if (minFracDigits > -exp) {
			// Emit trailing zeros.
			StringBuilder_AppendRepeat(numBuilder, '0', minFracDigits - -exp);
		}
	}
	else {
		// Negative exponent, and split across the decimal point: Some integer, some fractional digits.
		Int32 numIntDigits = len - -exp, numFracDigits = -exp;

		if (numIntDigits < minIntDigits) {
			// Need to prepend initial zeros.
			StringBuilder_AppendRepeat(numBuilder, '0', minIntDigits - numIntDigits);
		}

		// Copy all the integer digits.
		StringBuilder_Append(numBuilder, buffer, 0, numIntDigits);

		// Emit the decimal point.
		StringBuilder_AppendByte(numBuilder, '.');

		// Copy the fractional digits.
		StringBuilder_Append(numBuilder, buffer, numIntDigits, numFracDigits);

		if (numFracDigits < minFracDigits) {
			// Need to append trailing zeros.
			StringBuilder_AppendRepeat(numBuilder, '0', minFracDigits - numFracDigits);
		}
	}

	return StringBuilder_ToString(numBuilder);
}

String Float_ToExpString(Byte *buffer, Int32 len, Int32 exp, Int32 kind, Int32 minFracDigits, Bool forceSign)
{
	DECLARE_INLINE_STRINGBUILDER(numBuilder, 256);
	Int32 numFracDigits;

	INIT_INLINE_STRINGBUILDER(numBuilder);

	switch (kind) {
		case FLOAT_KIND_POS_INF:
			return forceSign ? Float_String_PosInf : Float_String_Inf;
		case FLOAT_KIND_NEG_INF:
			return Float_String_NegInf;
		case FLOAT_KIND_POS_QNAN:
			return forceSign ? Float_String_PosNaN : Float_String_NaN;
		case FLOAT_KIND_NEG_QNAN:
			return Float_String_NegNaN;
		case FLOAT_KIND_POS_SNAN:
			return forceSign ? Float_String_PosSNaN : Float_String_SNaN;
		case FLOAT_KIND_NEG_SNAN:
			return Float_String_NegSNaN;
		case FLOAT_KIND_POS_ZERO:
		case FLOAT_KIND_POS_NUM:
			if (forceSign) {
				StringBuilder_AppendByte(numBuilder, '+');
			}
			break;
		case FLOAT_KIND_NEG_ZERO:
		case FLOAT_KIND_NEG_NUM:
			StringBuilder_AppendByte(numBuilder, '-');
			break;
	}

	// Output the digits.
	StringBuilder_AppendByte(numBuilder, buffer[0]);
	numFracDigits = len - 1;
	if (numFracDigits > 0 || numFracDigits < minFracDigits) {
		StringBuilder_AppendByte(numBuilder, '.');
	}
	if (numFracDigits > 0) {
		StringBuilder_Append(numBuilder, buffer, 1, numFracDigits);
	}
	exp += numFracDigits;

	// Pad any missing trailing zeros.  This is unrolled for speed.
	while (numFracDigits + 16 <= minFracDigits) {
		StringBuilder_AppendC(numBuilder, "0000000000000000", 0, 16);
		numFracDigits += 16;
	}
	if (numFracDigits <= minFracDigits) {
		StringBuilder_AppendC(numBuilder, "0000000000000000", 0, minFracDigits - numFracDigits);
		numFracDigits += (minFracDigits - numFracDigits);
	}

	// Output the exponent, always including 'e+' or 'e-' before it.  The only exception
	// to this is if we're outputting a zero.
	if (kind != FLOAT_KIND_POS_ZERO && kind != FLOAT_KIND_NEG_ZERO) {
		StringBuilder_AppendC(numBuilder, exp < 0 ? "e-" : "e+", 0, 2);
		StringBuilder_AppendFormat(numBuilder, "%d", exp < 0 ? -exp : exp);
	}

	// And we're done!
	return StringBuilder_ToString(numBuilder);
}

String Float64_ToFixedString(Float64 float64, Int minIntDigits, Int minFracDigits, Bool forceSign)
{
	Byte buffer[64];
	Int32 buflen, exp, kind;

	buflen = Float64_Decompose(buffer, &exp, &kind, float64);

	return Float_ToFixedString(buffer, buflen, exp, kind, (Int32)minIntDigits, (Int32)minFracDigits, forceSign);
}

String Float64_ToExpString(Float64 float64, Int minFracDigits, Bool forceSign)
{
	Byte buffer[64];
	Int32 buflen, exp, kind;

	buflen = Float64_Decompose(buffer, &exp, &kind, float64);

	return Float_ToExpString(buffer, buflen, exp, kind, (Int32)minFracDigits, forceSign);
}

#include <stdio.h>

String Float64_ToStringEx(Float64 float64, Int minIntDigits, Int minFracDigits, Bool forceSign)
{
	Byte buffer[64];
	//Int32 buflen, exp, kind;

	UNUSED(minIntDigits);
	UNUSED(minFracDigits);
	UNUSED(forceSign);

	sprintf_s(buffer, sizeof(buffer), "%g", float64);
	return String_FromC(buffer);
/*
	buflen = Float64_Decompose(buffer, &exp, &kind, float64);

	if (exp + buflen - 1 > 9 || exp + buflen - 1 < -6) {
		// Very large (1'000'000'000 or larger), or very small (smaller than 0.00001), so
		// print in exponential notation.
		return Float_ToExpString(buffer, buflen, exp, kind, (Int32)minFracDigits, forceSign);
	}
	else {
		// Moderate range:  In (1'000'000'000, 0.00001], so print it as a traditional decimal string.
		return Float_ToFixedString(buffer, buflen, exp, kind, (Int32)minIntDigits, (Int32)minFracDigits, forceSign);
	}
*/
}
