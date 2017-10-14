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

#include <stdio.h>
#include <math.h>

STATIC_STRING(Float_String_Zero, "0.0");
STATIC_STRING(Float_String_PosZero, "+0.0");
STATIC_STRING(Float_String_NegZero, "-0.0");

STATIC_STRING(Float_String_Inf, "inf");
STATIC_STRING(Float_String_PosInf, "+inf");
STATIC_STRING(Float_String_NegInf, "-inf");

STATIC_STRING(Float_String_NaN, "NaN");
STATIC_STRING(Float_String_PosNaN, "+NaN");
STATIC_STRING(Float_String_NegNaN, "-NaN");

STATIC_STRING(Float_String_SNaN, "SNaN");
STATIC_STRING(Float_String_PosSNaN, "+SNaN");
STATIC_STRING(Float_String_NegSNaN, "-SNaN");

Int Float32_GetKind(Float32 float32)
{
	UInt32 floatBits;
	UInt32 exponent;
	UInt32 mantissa;
	Bool sign;

	floatBits = *(UInt32 *)(Float32 *)&float32;

	sign = (Bool)((floatBits >> 31) & 1);
	exponent = (floatBits >> 23) & 0xFF;

	if (exponent == 0) {
		// One of the signed zero forms, or possibly a subnormal value.
		mantissa = floatBits & ((1U << 23) - 1);
		if (mantissa == 0) {
			// Zero.
			return sign ? FLOAT_KIND_NEG_ZERO : FLOAT_KIND_POS_ZERO;
		}
		else {
			// Subnormal values.
			return sign ? FLOAT_KIND_NEG_NUM : FLOAT_KIND_POS_NUM;
		}
	}
	else if (exponent == 0xFF) {
		// NaN or infinity.
		mantissa = floatBits & ((1U << 23) - 1);
		if (mantissa == 0) {
			// Infinity.
			return sign ? FLOAT_KIND_NEG_INF : FLOAT_KIND_POS_INF;
		}
		else {
			// NaN (quiet or signalling).
			if (mantissa & (1U << 22)) {
				// QNaN.
				return sign ? FLOAT_KIND_NEG_QNAN : FLOAT_KIND_POS_QNAN;
			}
			else {
				// SNaN.
				return sign ? FLOAT_KIND_NEG_SNAN : FLOAT_KIND_POS_SNAN;
			}
		}
	}

	// Normal values.
	return sign ? FLOAT_KIND_NEG_NUM : FLOAT_KIND_POS_NUM;
}

Int Float64_GetKind(Float64 float64)
{
	UInt64 floatBits;
	UInt64 exponent;
	UInt64 mantissa;
	Bool sign;

	floatBits = *(UInt64 *)(Float64 *)&float64;

	sign = (Bool)((floatBits >> 63) & 1);
	exponent = (floatBits >> 52) & 0x7FF;

	if (exponent == 0) {
		// One of the signed zero forms, or possibly a subnormal value.
		mantissa = floatBits & ((1ULL << 52) - 1);
		if (mantissa == 0) {
			// Zero.
			return sign ? FLOAT_KIND_NEG_ZERO : FLOAT_KIND_POS_ZERO;
		}
		else {
			// Subnormal values.
			return sign ? FLOAT_KIND_NEG_NUM : FLOAT_KIND_POS_NUM;
		}
	}
	else if (exponent == 0x7FF) {
		// NaN or infinity.
		mantissa = floatBits & ((1ULL << 52) - 1);
		if (mantissa == 0) {
			// Infinity.
			return sign ? FLOAT_KIND_NEG_INF : FLOAT_KIND_POS_INF;
		}
		else {
			// NaN (quiet or signalling).
			if (mantissa & (1ULL << 51)) {
				// QNaN.
				return sign ? FLOAT_KIND_NEG_QNAN : FLOAT_KIND_POS_QNAN;
			}
			else {
				// SNaN.
				return sign ? FLOAT_KIND_NEG_SNAN : FLOAT_KIND_POS_SNAN;
			}
		}
	}

	// Normal values.
	return sign ? FLOAT_KIND_NEG_NUM : FLOAT_KIND_POS_NUM;
}

String Float64_ToFixedString(Float64 value, Int minIntDigits, Int maxFracDigits, Bool forceSign)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	char numberBuffer[256];
	char *start = numberBuffer;
	char *dot, *end;
	Int kind;
	Int digitsPrinted;

	kind = Float64_GetKind(value);

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
			if (forceSign) *start++ = '+';
			break;
		case FLOAT_KIND_NEG_ZERO:
		case FLOAT_KIND_NEG_NUM:
			*start++ = '-';
			value = -value;
			break;
	}

	// Use sprintf to actually format the number.
	sprintf_s(start, numberBuffer + sizeof(numberBuffer) - start, "%.*f",
		(Int32)maxFracDigits, value);

	// Trim any trailing zeros, but ensure that there's always one digit after the decimal point.
	end = start + strlen(start);
	while (end > start && end[-1] == '0') end--;
	if (end == start) end++;
	else if (end[-1] == '.') {
		*end++ = '0';
	}
	*end = '\0';

	// Find out where the decimal point ended up.
	dot = strchr(start, '.');

	// If we printed enough digits before the decimal point to satisfy
	// 'minIntDigits', then we're done.
	digitsPrinted = (dot == NULL ? strlen(start) : dot - start);
	if (digitsPrinted >= minIntDigits)
		return String_FromC(numberBuffer);

	// Padding is required.  So first, make a place to put the padding.
	INIT_INLINE_STRINGBUILDER(stringBuilder);

	// Copy into the StringBuilder anything before the 'start' position.
	if (start > numberBuffer)
		StringBuilder_Append(stringBuilder, (const Byte *)numberBuffer, 0, start - numberBuffer);

	// Now add the padding.
	StringBuilder_AppendRepeat(stringBuilder, '0', minIntDigits - (dot - start));

	// Finally, add the rest of the number.
	StringBuilder_Append(stringBuilder, start, 0, strlen(start));

	// All done.
	return StringBuilder_ToString(stringBuilder);
}

String Float64_ToExpString(Float64 value, Int maxFracDigits, Bool forceSign)
{
	char numberBuffer[256];
	char *start = numberBuffer;
	Int kind;

	kind = Float64_GetKind(value);

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
		if (forceSign) *start++ = '+';
		break;
	case FLOAT_KIND_NEG_ZERO:
	case FLOAT_KIND_NEG_NUM:
		*start++ = '-';
		value = -value;
		break;
	}

	// Now use sprintf to actually format the number.
	sprintf_s(start, numberBuffer + sizeof(numberBuffer) - start, "%.*e",
		(maxFracDigits > Int32Max ? Int32Max : (Int32)maxFracDigits), value);

	// And we're done.
	return String_FromC(numberBuffer);
}

String Float64_ToStringEx(Float64 float64, Int minIntDigits, Int maxFracDigits, Bool forceSign)
{
	Int kind;
	Float64 absValue = fabs(float64);

	if (absValue == 0.0) {
		if (forceSign) {
			kind = Float64_GetKind(float64);
			if (kind == FLOAT_KIND_POS_ZERO)
				return Float_String_PosZero;
			if (kind == FLOAT_KIND_NEG_ZERO)
				return Float_String_NegZero;
		}
		return Float_String_Zero;
	}

	if (absValue > 1000000000.0 || absValue < 0.00001) {
		// Very large (1'000'000'000 or larger), or very small (smaller than 0.00001), so
		// print in exponential notation.
		return Float64_ToExpString(float64, (Int32)maxFracDigits, forceSign);
	}
	else {
		// Moderate range:  In (1'000'000'000, 0.00001], so print it as a traditional decimal string.
		return Float64_ToFixedString(float64, (Int32)minIntDigits, (Int32)maxFracDigits, forceSign);
	}
}
