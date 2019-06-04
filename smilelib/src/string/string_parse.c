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

#include <smile/gc.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>

/// <summary>
/// Parse a string as a Boolean (truthy) value.  This will ignore any initial and
/// trailing whitespace, and will recognize any of the following six strings (case
/// insensitive):
///     0, 1, t, f, true, false
/// </summary>
/// <param name="str">The string to parse as a Boolean value.</param>
/// <param name="result">This will be set to the result of the parse.  If the string
/// is unknown, this will always be false.</param>
/// <returns>True if this string matches one of the six known Boolean strings, false
/// if this string is unknown.</returns>
Bool String_ParseBool(const String str, Bool *result)
{
	Int index, length;
	const Byte *text;

	length = String_Length(str);
	text = String_GetBytes(str);

	index = 0;

	while (index < length && text[index] <= 32) index++;
	if (index == length) {
		*result = False;
		return False;
	}

	// We recognize any of the following six strings (case insensitive):  0, 1, t, f, true, false
	switch (text[index]) {
		case '0':
			index++;
			*result = False;
			break;
		case '1':
			index++;
			*result = True;
			break;
		case 't':
		case 'T':
			if (index + 1 < length && (text[index + 1] == 'r' || text[index + 1] == 'R')
				&& index + 2 < length && (text[index + 2] == 'u' || text[index + 2] == 'U')
				&& index + 3 < length && (text[index + 3] == 'e' || text[index + 3] == 'E'))
				index += 4;
			else
				index++;
			*result = True;
			break;
		case 'f':
		case 'F':
			if (index + 1 < length && (text[index + 1] == 'a' || text[index + 1] == 'A')
				&& index + 2 < length && (text[index + 2] == 'l' || text[index + 2] == 'L')
				&& index + 3 < length && (text[index + 3] == 's' || text[index + 3] == 'S')
				&& index + 4 < length && (text[index + 4] == 'e' || text[index + 4] == 'E'))
				index += 5;
			else
				index++;
			*result = False;
			break;
		default:
			*result = False;
			break;
	}

	while (index < length && text[index] <= 32) index++;

	if (index == length) return True;

	*result = False;
	return False;
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

/// <summary>
/// Parse a string as a 64-bit signed integer.  This will skip any initial and
/// trailing whitespace.  It will recognize an initial optional '+' or '-' character
/// to control the sign.  Apostrophe ('), quote ("), and underscore (_) will be
/// ignored if found between digits.  Digits are recognized in the set of [0-9a-zA-Z],
/// depending on the provided numeric base.
/// </summary>
/// <param name="str">The string to parse as a 64-bit integer value.</param>
/// <param name="numericBase">The base to use when parsing the value, which must be
/// in the range of 2 to 36.</param>
/// <param name="result">This will be set to the result of the parse (zero for an invalid value).</param>
/// <returns>True if the string contained a valid, parseable integer in the given base,
/// false if the string did not.  This will always return false for an illegal base.</returns>
Bool String_ParseInteger(const String str, Int numericBase, Int64 *result)
{
	Int index, length;
	const Byte *text;
	Bool neg;
	UInt64 uvalue;
	Int digits;

	length = String_Length(str);
	text = String_GetBytes(str);

	if (numericBase < 2 || numericBase > 36 || String_IsNullOrEmpty(str)) {
		*result = 0;
		return False;
	}

	index = 0;

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

	uvalue = 0;
	digits = ParseIntegerInternal(text, length, &index, numericBase, &uvalue);

	while (index < length && text[index] <= 32) index++;

	if (digits == 0 || index != length) {
		*result = 0;
		return False;
	}

	*result = neg ? -(Int64)uvalue : (Int64)uvalue;
	return True;
}
