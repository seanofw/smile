//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

#include <smile/numeric/realshared.h>
#include <smile/stringbuilder.h>

STATIC_STRING(Real_String_Zero, "0");
STATIC_STRING(Real_String_PosZero, "+0");
STATIC_STRING(Real_String_NegZero, "-0");

STATIC_STRING(Real_String_Inf, "inf");
STATIC_STRING(Real_String_PosInf, "+inf");
STATIC_STRING(Real_String_NegInf, "-inf");

STATIC_STRING(Real_String_NaN, "NaN");
STATIC_STRING(Real_String_PosNaN, "+NaN");
STATIC_STRING(Real_String_NegNaN, "-NaN");

STATIC_STRING(Real_String_SNaN, "SNaN");
STATIC_STRING(Real_String_PosSNaN, "+SNaN");
STATIC_STRING(Real_String_NegSNaN, "-SNaN");

String Real_ToFixedString(Byte *buffer, Int32 len, Int32 exp, Int32 kind, Int minIntDigits, Int minFracDigits, Bool forceSign)
{
	DECLARE_INLINE_STRINGBUILDER(numBuilder, 256);

	INIT_INLINE_STRINGBUILDER(numBuilder);

	switch (kind) {
	case REAL_KIND_POS_INF:
		return forceSign ? Real_String_PosInf : Real_String_Inf;
	case REAL_KIND_NEG_INF:
		return Real_String_NegInf;
	case REAL_KIND_POS_QNAN:
		return forceSign ? Real_String_PosNaN : Real_String_NaN;
	case REAL_KIND_NEG_QNAN:
		return Real_String_NegNaN;
	case REAL_KIND_POS_SNAN:
		return forceSign ? Real_String_PosSNaN : Real_String_SNaN;
	case REAL_KIND_NEG_SNAN:
		return Real_String_NegSNaN;
	case REAL_KIND_POS_ZERO:
	case REAL_KIND_POS_NUM:
		if (forceSign) {
			StringBuilder_AppendByte(numBuilder, '+');
		}
		break;
	case REAL_KIND_NEG_ZERO:
	case REAL_KIND_NEG_NUM:
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
		Int numIntDigits = len - -exp, numFracDigits = -exp;

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

String Real_ToExpString(Byte *buffer, Int32 len, Int32 exp, Int32 kind, Int minFracDigits, Bool forceSign)
{
	DECLARE_INLINE_STRINGBUILDER(numBuilder, 256);
	Int numFracDigits;

	INIT_INLINE_STRINGBUILDER(numBuilder);

	switch (kind) {
	case REAL_KIND_POS_INF:
		return forceSign ? Real_String_PosInf : Real_String_Inf;
	case REAL_KIND_NEG_INF:
		return Real_String_NegInf;
	case REAL_KIND_POS_QNAN:
		return forceSign ? Real_String_PosNaN : Real_String_NaN;
	case REAL_KIND_NEG_QNAN:
		return Real_String_NegNaN;
	case REAL_KIND_POS_SNAN:
		return forceSign ? Real_String_PosSNaN : Real_String_SNaN;
	case REAL_KIND_NEG_SNAN:
		return Real_String_NegSNaN;
	case REAL_KIND_POS_ZERO:
	case REAL_KIND_POS_NUM:
		if (forceSign) {
			StringBuilder_AppendByte(numBuilder, '+');
		}
		break;
	case REAL_KIND_NEG_ZERO:
	case REAL_KIND_NEG_NUM:
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
		StringBuilder_Append(numBuilder, "0000000000000000", 0, 16);
		numFracDigits += 16;
	}
	if (numFracDigits <= minFracDigits) {
		StringBuilder_Append(numBuilder, "0000000000000000", 0, minFracDigits - numFracDigits);
		numFracDigits += (minFracDigits - numFracDigits);
	}

	// Output the exponent, always including 'e+' or 'e-' before it.  The only exception
	// to this is if we're outputting a zero.
	if (kind != REAL_KIND_POS_ZERO && kind != REAL_KIND_NEG_ZERO) {
		StringBuilder_Append(numBuilder, exp < 0 ? "e-" : "e+", 0, 2);
		StringBuilder_AppendFormat(numBuilder, "%d", exp < 0 ? -exp : exp);
	}

	// And we're done!
	return StringBuilder_ToString(numBuilder);
}
