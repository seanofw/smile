//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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

#include <math.h>

#include <smile/gc.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>

Inline Int ParseFloatIntegerPart(const Byte *text, Int length, Int *index, Int numericBase, Float64 *result)
{
	Int digits, i;
	Int digitValue;
	Float64 value;
	
	i = *index;
	digits = 0;
	value = 0;

	while (i < length) {
		switch (text[i]) {
			case '\'': case '\"': case '_': i++; continue;
			case '0': digitValue = 0; break;
			case '1': digitValue = 1; break;
			case '2': digitValue = 2; break;
			case '3': digitValue = 3; break;
			case '4': digitValue = 4; break;
			case '5': digitValue = 5; break;
			case '6': digitValue = 6; break;
			case '7': digitValue = 7; break;
			case '8': digitValue = 8; break;
			case '9': digitValue = 9; break;
			case 'A': case 'a': digitValue = 10; break;
			case 'B': case 'b': digitValue = 11; break;
			case 'C': case 'c': digitValue = 12; break;
			case 'D': case 'd': digitValue = 13; break;
			case 'E': case 'e': digitValue = 14; break;
			case 'F': case 'f': digitValue = 15; break;
			case 'G': case 'g': digitValue = 16; break;
			case 'H': case 'h': digitValue = 17; break;
			case 'I': case 'i': digitValue = 18; break;
			case 'J': case 'j': digitValue = 19; break;
			case 'K': case 'k': digitValue = 20; break;
			case 'L': case 'l': digitValue = 21; break;
			case 'M': case 'm': digitValue = 22; break;
			case 'N': case 'n': digitValue = 23; break;
			case 'O': case 'o': digitValue = 24; break;
			case 'P': case 'p': digitValue = 25; break;
			case 'Q': case 'q': digitValue = 26; break;
			case 'R': case 'r': digitValue = 27; break;
			case 'S': case 's': digitValue = 28; break;
			case 'T': case 't': digitValue = 29; break;
			case 'U': case 'u': digitValue = 30; break;
			case 'V': case 'v': digitValue = 31; break;
			case 'W': case 'w': digitValue = 32; break;
			case 'X': case 'x': digitValue = 33; break;
			case 'Y': case 'y': digitValue = 34; break;
			case 'Z': case 'z': digitValue = 35; break;
			default: digitValue = 99; break;
		}
		if (digitValue >= numericBase) break;
		value *= numericBase;
		value += digitValue;
		digits++;
		i++;
	}

	*index = i;
	*result = value;
	return digits;
}

static const Float64 *_floatMultipliersByBase[37];

Inline Int ParseFloatFractionalPart(const Byte *text, Int length, Int *index, Int numericBase, Float64 *result)
{
	Float64 value;
	Int i, digits, significantDigits, digitValue;
	UInt64 intDigits;
	Bool isInitial;

	i = *index;
	intDigits = 0;
	digits = 0;
	significantDigits = 0;
	isInitial = True;

	while (i < length) {
		switch (text[i]) {
			case '\'': case '\"': case '_':
				i++;
				continue;
			case '0':
				if (isInitial) {
					digits++;
					i++;
					continue;
				}
				digitValue = 0;
				break;
			case '1': digitValue = 1; break;
			case '2': digitValue = 2; break;
			case '3': digitValue = 3; break;
			case '4': digitValue = 4; break;
			case '5': digitValue = 5; break;
			case '6': digitValue = 6; break;
			case '7': digitValue = 7; break;
			case '8': digitValue = 8; break;
			case '9': digitValue = 9; break;
			case 'A': case 'a': digitValue = 10; break;
			case 'B': case 'b': digitValue = 11; break;
			case 'C': case 'c': digitValue = 12; break;
			case 'D': case 'd': digitValue = 13; break;
			case 'E': case 'e': digitValue = 14; break;
			case 'F': case 'f': digitValue = 15; break;
			case 'G': case 'g': digitValue = 16; break;
			case 'H': case 'h': digitValue = 17; break;
			case 'I': case 'i': digitValue = 18; break;
			case 'J': case 'j': digitValue = 19; break;
			case 'K': case 'k': digitValue = 20; break;
			case 'L': case 'l': digitValue = 21; break;
			case 'M': case 'm': digitValue = 22; break;
			case 'N': case 'n': digitValue = 23; break;
			case 'O': case 'o': digitValue = 24; break;
			case 'P': case 'p': digitValue = 25; break;
			case 'Q': case 'q': digitValue = 26; break;
			case 'R': case 'r': digitValue = 27; break;
			case 'S': case 's': digitValue = 28; break;
			case 'T': case 't': digitValue = 29; break;
			case 'U': case 'u': digitValue = 30; break;
			case 'V': case 'v': digitValue = 31; break;
			case 'W': case 'w': digitValue = 32; break;
			case 'X': case 'x': digitValue = 33; break;
			case 'Y': case 'y': digitValue = 34; break;
			case 'Z': case 'z': digitValue = 35; break;
			default: digitValue = 99; break;
		}
		if (digitValue >= numericBase) break;

		// We cut off at 58 bits.  58 is the largest number greater than the 52 bits of a Float64's
		// mantissa, but still with enough room to handle up to base 36 (6 bits).  This means we're
		// able to be a full 6 bits more precise than the Float64's mantissa, which is exactly enough
		// for one extra digit of precision in every base.
		if (intDigits < (((UInt64)1) << (64 - 6))) {
			intDigits *= (UInt)numericBase;
			intDigits += digitValue;
			digits++;
			significantDigits++;
		}

		i++;
		isInitial = False;
	}

	if (digits > 0) {
		// We have the mantissa's correct value, but it's represented as a large integer, not
		// as fractional bits.  So now we do a little math to shuffle those integer bits down to
		// the correct position to be floating-point fractional bits.
		value = (Float64)intDigits;

		// Shuffle the bits down, quickly, using precomputed multipliers to avoid loss of
		// precision where possible.
		if (digits == significantDigits) {
			value /= _floatMultipliersByBase[numericBase][significantDigits - 1];
		}
		else {
			while (digits >= 64) {
				value /= _floatMultipliersByBase[numericBase][63];
			}
			if (digits > 0) {
				value /= _floatMultipliersByBase[numericBase][digits - 1];
			}
		}
	}
	else {
		// No digits, or we hit an invalid character.
		value = 0;
	}

	*index = i;
	*result = value;
	return digits;
}

Inline Float64 ApplyExponent(Float64 value, Int numericBase, Int exponent)
{
	Float64 multiplier;

	if (exponent < 0) {
		exponent = -exponent;
		while (exponent >= 64) {
			multiplier = _floatMultipliersByBase[numericBase][63];
			value /= multiplier;
			exponent -= 64;
		}
		if (exponent > 0) {
			multiplier = _floatMultipliersByBase[numericBase][exponent - 1];
			value /= multiplier;
		}
	}
	else if (exponent > 0) {
		while (exponent >= 64) {
			multiplier = _floatMultipliersByBase[numericBase][63];
			value *= multiplier;
			exponent -= 64;
		}
		if (exponent > 0) {
			multiplier = _floatMultipliersByBase[numericBase][exponent - 1];
			value *= multiplier;
		}
	}

	return value;
}

Inline Int ParseIntegerInternal(const Byte *text, Int length, Int *index, Int numericBase, UInt64 *result)
{
	Int i, digits;
	Int digitValue;
	UInt64 value;

	i = *index;

	value = 0;
	digits = 0;

	while (i < length) {
		switch (text[i]) {
		case '\'': case '\"': case '_': i++; continue;
		case '0': digitValue = 0; i++; break;
		case '1': digitValue = 1; i++; break;
		case '2': digitValue = 2; i++; break;
		case '3': digitValue = 3; i++; break;
		case '4': digitValue = 4; i++; break;
		case '5': digitValue = 5; i++; break;
		case '6': digitValue = 6; i++; break;
		case '7': digitValue = 7; i++; break;
		case '8': digitValue = 8; i++; break;
		case '9': digitValue = 9; i++; break;
		case 'A': case 'a': digitValue = 10; i++; break;
		case 'B': case 'b': digitValue = 11; i++; break;
		case 'C': case 'c': digitValue = 12; i++; break;
		case 'D': case 'd': digitValue = 13; i++; break;
		case 'E': case 'e': digitValue = 14; i++; break;
		case 'F': case 'f': digitValue = 15; i++; break;
		case 'G': case 'g': digitValue = 16; i++; break;
		case 'H': case 'h': digitValue = 17; i++; break;
		case 'I': case 'i': digitValue = 18; i++; break;
		case 'J': case 'j': digitValue = 19; i++; break;
		case 'K': case 'k': digitValue = 20; i++; break;
		case 'L': case 'l': digitValue = 21; i++; break;
		case 'M': case 'm': digitValue = 22; i++; break;
		case 'N': case 'n': digitValue = 23; i++; break;
		case 'O': case 'o': digitValue = 24; i++; break;
		case 'P': case 'p': digitValue = 25; i++; break;
		case 'Q': case 'q': digitValue = 26; i++; break;
		case 'R': case 'r': digitValue = 27; i++; break;
		case 'S': case 's': digitValue = 28; i++; break;
		case 'T': case 't': digitValue = 29; i++; break;
		case 'U': case 'u': digitValue = 30; i++; break;
		case 'V': case 'v': digitValue = 31; i++; break;
		case 'W': case 'w': digitValue = 32; i++; break;
		case 'X': case 'x': digitValue = 33; i++; break;
		case 'Y': case 'y': digitValue = 34; i++; break;
		case 'Z': case 'z': digitValue = 35; i++; break;
		default: digitValue = 99; i++; break;
		}

		if (digitValue >= numericBase) break;

		value *= numericBase;
		value += digitValue;
		digits++;
	}

	*index = i;
	*result = value;
	return digits;
}

Inline Bool ParseFloatExponent(const Byte *text, Int length, Int *index, Int numericBase, Float64 *result)
{
	Bool exponentSign;
	Int i, digits;
	UInt64 exponent;

	i = *index;
	
	exponentSign = False;
	if (i < length && text[i] == '+')
		i++;
	else if (i < length && text[i] == '-')
		i++, exponentSign = True;

	digits = ParseIntegerInternal(text, length, &i, numericBase, &exponent);

	if (digits == 0) {
		*result = 0;
		*index = i;
		return False;
	}

	*result = ApplyExponent(*result, numericBase, exponentSign ? -(Int)exponent : (Int)exponent);

	*index = i;
	return True;
}

/// <summary>
/// Parse a string as a 64-bit real value.  This will skip any initial and
/// trailing whitespace.  It will recognize an initial optional '+' or '-' character
/// to control the sign.  Apostrophe ('), quote ("), and underscore (_) will be
/// ignored if found between digits.  Digits are recognized in the set of [0-9a-zA-Z],
/// depending on the provided numeric base, and a single '.' is used to separate
/// the integer part from the fractional part.  An optional exponent may be provided
/// in the form of /[eE][+-][0-9a-zA-Z]+/, where the integer that follows the exponent
/// is in the same numeric base as the other digits.
/// </summary>
/// <param name="str">The string to parse as a 64-bit real value.</param>
/// <param name="numericBase">The base to use when parsing the value, which must be
/// in the range of 2 to 36.</param>
/// <param name="result">This will be set to the result of the parse (zero for an invalid value).</param>
/// <returns>True if the string contained a valid, parseable real value in the given base,
/// false if the string did not.  This will always return false for an illegal base.</returns>
Bool String_ParseFloat(const String str, Int numericBase, Float64 *result)
{
	Int index, length, digits;
	const Byte *text;
	Bool neg;
	Float64 integerPart, realPart, sum;

	if (numericBase < 2 || numericBase > 36 || String_IsNullOrEmpty(str)) {
		*result = 0;
		return False;
	}

	index = 0;
	length = String_Length(str);
	text = String_GetBytes(str);

	while (index < length && text[index] <= 32) index++;

	neg = False;
	if (index < length && text[index] == '+') {
		neg = False;
		index++;
	}
	else if (index < length && text[index] == '-') {
		neg = True;
		index++;
	}

	integerPart = 0;
	realPart = 0;

	digits = ParseFloatIntegerPart(text, length, &index, numericBase, &integerPart);

	if (index < length && text[index] == '.') {
		index++;
		digits += ParseFloatFractionalPart(text, length, &index, numericBase, &realPart);
	}

	if (digits == 0) {
		*result = 0;
		return False;
	}

	sum = integerPart + realPart;

	if (index < length && (text[index] == 'e' || text[index] == 'E')) {
		index++;
		if (!ParseFloatExponent(text, length, &index, numericBase, &sum))
			return False;
	}

	while (index < length && text[index] <= 32) index++;

	if (index != length) {
		*result = 0;
		return False;
	}

	*result = neg ? -sum : sum;
	return True;
}

#include "string_parsefloat.generated.inc"
