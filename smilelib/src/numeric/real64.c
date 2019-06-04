//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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
#include <smile/numeric/real64.h>
#include <smile/stringbuilder.h>

extern String Real_ToFixedString(Byte *buffer, Int32 len, Int32 exp, Int32 kind, Int minIntDigits, Int minFracDigits, Bool forceSign);
extern String Real_ToExpString(Byte *buffer, Int32 len, Int32 exp, Int32 kind, Int minFracDigits, Bool forceSign);

extern SMILE_THREAD_LOCAL UInt32 Real_RoundingMode;
extern SMILE_THREAD_LOCAL UInt32 Real_Flags;

extern Real64 Real64_FromRawCString(const char *str);

Real64 Real64_NegNaN =		{ 0xFC00000000000000ULL };
Real64 Real64_NegInf =		{ 0xF800000000000000ULL };
Real64 Real64_NegSixteen =	{ 0xB1C0000000000010ULL };
Real64 Real64_NegTen =		{ 0xB1C000000000000AULL };
Real64 Real64_NegTwo =		{ 0xB1C0000000000002ULL };
Real64 Real64_NegOne =		{ 0xB1C0000000000001ULL };
Real64 Real64_NegOneHalf =	{ 0xB1A0000000000005ULL };
Real64 Real64_NegZero =		{ 0xB1C0000000000000ULL };
Real64 Real64_Zero =		{ 0x31C0000000000000ULL };
Real64 Real64_OneHalf =		{ 0x31A0000000000005ULL };
Real64 Real64_One =			{ 0x31C0000000000001ULL };
Real64 Real64_Two =			{ 0x31C0000000000002ULL };
Real64 Real64_Ten =			{ 0x31C000000000000AULL };
Real64 Real64_Sixteen =		{ 0x31C0000000000010ULL };
Real64 Real64_Inf =			{ 0x7800000000000000ULL };
Real64 Real64_NaN =			{ 0x7C00000000000000ULL };

SMILE_API_FUNC Bool Real64_TryParseInternal(const Byte *src, Int length, Real64 *result)
{
	DECLARE_INLINE_STRINGBUILDER(cleanString, 256);
	const Byte *end, *start;
	Byte ch;

	end = src + length;

	INIT_INLINE_STRINGBUILDER(cleanString);

	// We need to clean the Smile-isms out of the string so that it's just raw digits,
	// decimal points, and possibly 'E' and signs.  Then we can pass it to the native
	// parsing function.

	// Skip initial whitespace.
	while (src < end && *src <= '\x20') src++;

	// If there's no content, this is a fail.
	if (src >= end) {
		*result = Real64_Zero;
		return False;
	}

	// Trim off trailing whitespace.
	while (end > src && end[-1] <= '\x20') end--;

	// Check for named numeric values like "inf" and "nan".  We only allow quiet NaNs, since
	// nothing in Smile's numerics supports signaling NaNs.  We have to check for these up front,
	// since the underlying parser can't indicate the difference beween a failed parse and the
	// user actually requesting "NaN".  It also allows "infinity", fully-spelled-out, which we
	// do not.
	//
	// Note: These tests are carefully ordered so that the compiler's optimizer can easily
	// perform CSE on them; these read cleanly, but they optimize down to the most-efficient way
	// of testing for this.  Don't reorder these without a good reason.
	if (src + 3 == end
		&& (((ch = src[0]) == 'i' || ch == 'I')
		&& ((ch = src[1]) == 'n' || ch == 'N')
		&& ((ch = src[2]) == 'f' || ch == 'F'))) {
		*result = Real64_Inf;
		return True;
	}
	else if (src + 3 == end
		&& (((ch = src[0]) == 'n' || ch == 'N')
		&& ((ch = src[1]) == 'a' || ch == 'A')
		&& ((ch = src[2]) == 'n' || ch == 'N'))) {
		*result = Real64_NaN;
		return True;
	}
	else if (src + 4 == end
		&& (src[0] == '+'
		&& ((ch = src[1]) == 'i' || ch == 'I')
		&& ((ch = src[2]) == 'n' || ch == 'N')
		&& ((ch = src[3]) == 'f' || ch == 'F'))) {
		*result = Real64_Inf;
		return True;
	}
	else if (src + 4 == end
		&& (src[0] == '+'
		&& ((ch = src[1]) == 'n' || ch == 'N')
		&& ((ch = src[2]) == 'a' || ch == 'A')
		&& ((ch = src[3]) == 'n' || ch == 'N'))) {
		*result = Real64_NaN;
		return True;
	}
	else if (src + 4 == end
		&& (src[0] == '-'
		&& ((ch = src[1]) == 'i' || ch == 'I')
		&& ((ch = src[2]) == 'n' || ch == 'N')
		&& ((ch = src[3]) == 'f' || ch == 'F'))) {
		*result = Real64_NegInf;
		return True;
	}
	else if (src + 4 == end
		&& (src[0] == '-'
		&& ((ch = src[1]) == 'n' || ch == 'N')
		&& ((ch = src[2]) == 'a' || ch == 'A')
		&& ((ch = src[3]) == 'n' || ch == 'N'))) {
		*result = Real64_NegNaN;
		return True;
	}

	start = src;

	// Copy an optional initial '+' or '-' as a sign.
	if ((ch = *src) == '+' || ch == '-') {
		src++;
	}

	// Make sure this doesn't start with a ' or " or _ character, since those separators are illegal starting chars.
	if ((ch = *src) == '\'' || ch == '\"' || ch == '_') {
		*result = Real64_Zero;
		return False;
	}

	// Copy digit chunks and radix and exponent characters, discarding embedded ' and " and _ characters.
	// We don't need to validate this part, because the underlying parser will do so.
	while (src < end) {
		switch (ch = *src) {

		case '\'':
		case '\"':
		case '_':
			// Separator character.
			if (src > start) {
				StringBuilder_Append(cleanString, start, 0, src - start);
			}
			else {
				// Two separator chars in a row is illegal.
				*result = Real64_Zero;
				return False;
			}
			start = ++src;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case 'e':
		case 'E':
		case '+':
		case '-':
		case '.':
			// Legal numeric character of some kind.
			src++;
			break;

		default:
			// Unknown character is an error.
			*result = Real64_Zero;
			return False;
			break;
		}
	}

	if (src > start) {
		StringBuilder_Append(cleanString, start, 0, src - start);
	}
	else {
		// Ending with a separator character is illegal.
		*result = Real64_Zero;
		return False;
	}

	// Make sure this results in a C-style string.
	StringBuilder_AppendByte(cleanString, '\0');

	// The StringBuilder now contains the desired string, at it's at least *somewhat*
	// legitimately structured.  The rest of the parsing (and validation) can be done
	// by the underlying raw parser, which will return a NaN if the string isn't valid.
	// We read the content right out of the StringBuilder:  If the content is short
	// enough, all of the data will be on the stack, so we can avoid ever allocating
	// anything at all on the heap, which is great for performance.
	*result = Real64_FromRawCString((const char *)StringBuilder_GetBytes(cleanString));
	return !Real64_IsNaN(*result);
}

String Real64_ToFixedString(Real64 real64, Int minIntDigits, Int minFracDigits, Bool forceSign)
{
	Byte buffer[64];
	Int32 buflen, exp, kind;

	buflen = Real64_Decompose(buffer, &exp, &kind, real64);

	return Real_ToFixedString(buffer, buflen, exp, kind, (Int32)minIntDigits, (Int32)minFracDigits, forceSign);
}

String Real64_ToExpString(Real64 real64, Int minFracDigits, Bool forceSign)
{
	Byte buffer[64];
	Int32 buflen, exp, kind;

	buflen = Real64_Decompose(buffer, &exp, &kind, real64);

	return Real_ToExpString(buffer, buflen, exp, kind, (Int32)minFracDigits, forceSign);
}

String Real64_ToStringEx(Real64 real64, Int minIntDigits, Int minFracDigits, Bool forceSign)
{
	Byte buffer[64];
	Int32 buflen, exp, kind;

	buflen = Real64_Decompose(buffer, &exp, &kind, real64);

	if (exp + buflen - 1 > 9 || exp + buflen - 1 < -6) {
		// Very large (1'000'000'000 or larger), or very small (smaller than 0.00001), so
		// print in exponential notation.
		return Real_ToExpString(buffer, buflen, exp, kind, (Int32)minFracDigits, forceSign);
	}
	else {
		// Moderate range:  In (1'000'000'000, 0.00001], so print it as a traditional decimal string.
		return Real_ToFixedString(buffer, buflen, exp, kind, (Int32)minIntDigits, (Int32)minFracDigits, forceSign);
	}
}

SMILE_API_FUNC Real64 Real64_Mod(Real64 a, Real64 b)
{
	// Compute the remainder (whose sign will match a, the dividend).
	Real64 mod = Real64_Rem(a, b);

	// Correct the sign of the modulus to match that of b (the divisor).
	if ((b.value ^ mod.value) & 0x8000000000000000ULL)
		mod.value ^= 0x8000000000000000ULL;

	return mod;
}

// Possible fast classifications for the sort-comparison code below.
#define POS 0
#define NEG 1
#define PINF 2
#define NINF 3
#define PNAN 4
#define NNAN 5

static SByte _comparisonTable[6][6] = {
	/*+X  -X  +I  -I  +N  -N */
	{ 0, +1, -1, +1, -1, +1 },	// +Num
	{ -1, 0, -1, +1, -1, +1 },	// -Num
	{ +1, +1, 2, +1, -1, +1 },	// +Inf
	{ -1, -1, -1, 3, -1, +1 },	// -Inf
	{ +1, +1, +1, +1, 2, +1 },	// +NaN
	{ -1, -1, -1, -1, -1, 3 },	// -NaN
};

static Byte _classifyTable[64] = {
	POS, POS, POS, POS, POS, POS, POS, POS,
	POS, POS, POS, POS, POS, POS, POS, POS,
	POS, POS, POS, POS, POS, POS, POS, POS,
	POS, POS, POS, POS, POS, POS, PINF, PNAN,
	NEG, NEG, NEG, NEG, NEG, NEG, NEG, NEG,
	NEG, NEG, NEG, NEG, NEG, NEG, NEG, NEG,
	NEG, NEG, NEG, NEG, NEG, NEG, NEG, NEG,
	NEG, NEG, NEG, NEG, NEG, NEG, NINF, NNAN,
};

#define CLASSIFY(__number__) \
	(_classifyTable[(Int)(((__number__).value & 0xFC00000000000000ULL) >> (64 - 6))])

/// <summary>
/// Compare 'a' and 'b' for sorting purposes.  This returns 0 if 'a' and 'b' are equal,
/// and returns -1 if 'a' is less than 'b', and +1 if 'a' is greater than 'b'.  For sorting
/// purposes, NaNs are considered to be orderable values beyond Infinity, and NaNs or
/// Infinities of the same class with the same sign and coeffients are considered equal.
/// (SNaNs are also considered to be beyond QNaNs.)  Note also that for sorting purposes,
/// -0 equals +0.
/// </summary>
/// <param name="a">The first value to compare.</param>
/// <param name="b">The second value to compare.</param>
/// <returns>+1 if a > b; -1 if a < b; and 0 if a == b.</returns>
SMILE_API_FUNC Int Real64_Compare(Real64 a, Real64 b)
{
	Byte aClass = CLASSIFY(a);
	Byte bClass = CLASSIFY(b);

	switch (_comparisonTable[aClass][bClass]) {

	case +1:
		// A is definitely a larger class than B.
		return +1;

	case -1:
		// A is definitely a smaller class than B.
		return -1;

	case 2:
		// Same non-finite positive type, so just compare coefficient bits.
		if (a.value != b.value)
			return a.value > b.value ? +1 : -1;
		return 0;

	case 3:
		// Same non-finite negative type, so just compare coefficient bits.
		if (a.value != b.value)
			return a.value > b.value ? +1 : -1;
		return 0;

	case 0:
		// Both are finite values, so compare for real.
		if (Real64_Eq(a, b)) return 0;		// Equality is a slightly faster test, so it goes first.
		if (Real64_Lt(a, b)) return -1;
		return +1;

	default:
		// Shouldn't ever get here.
		return 0;
	}
}
