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

#include <math.h>

#include <smile/gc.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/numeric/real32.h>
#include <smile/numeric/real64.h>
#include <smile/numeric/real128.h>
#include <smile/numeric/realshared.h>

extern Real128 Real128_FromRawCString(const char *str);

static Bool ParseExactInBase10(const String str, Real128 *result);

SMILE_API_FUNC Bool String_ParseReal(const String str, Int numericBase, Real128 *result)
{
	Float64 floatResult;

	// If they want base-10, parse the number exactly in base-10.
	if (numericBase == 10)
		return ParseExactInBase10(str, result);

	// We don't have a good implementation for parsing in the unusual bases, so fall back on
	// String_ParseFloat() and converting types, which isn't perfect, and will probably lose
	// some ULPs, but it's better than nothing.
	if (!String_ParseFloat(str, numericBase, &floatResult))
	{
		*result = Real128_Zero;
		return False;
	}
	*result = Real128_FromFloat64(floatResult);
	return True;
}

static Bool ParseExactInBase10(const String str, Real128 *result)
{
	DECLARE_INLINE_STRINGBUILDER(cleanString, 256);
	const Byte *end, *start;
	Byte ch;
	Int length;
	const Byte *src;

	src = String_GetBytes(str);
	length = String_Length(str);

	end = src + length;

	INIT_INLINE_STRINGBUILDER(cleanString);

	// We need to clean the Smile-isms out of the string so that it's just raw digits,
	// decimal points, and possibly 'E' and signs.  Then we can pass it to the native
	// parsing function.

	// Skip initial whitespace.
	while (src < end && *src <= '\x20') src++;

	// If there's no content, this is a fail.
	if (src >= end) {
		*result = Real128_Zero;
		return False;
	}

	// Trim off trailing whitespace.
	while (end > src && end[-1] <= '\x20') end--;

	start = src;

	// Copy an optional initial '+' or '-' as a sign.
	if ((ch = *src) == '+' || ch == '-') {
		src++;
	}

	// Make sure this doesn't start with a ' or " or _ character, since those separators are illegal starting chars.
	if ((ch = *src) == '\'' || ch == '\"' || ch == '_') {
		*result = Real128_Zero;
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
				*result = Real128_Zero;
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
			*result = Real128_Zero;
			return False;
			break;
		}
	}

	if (src > start) {
		StringBuilder_Append(cleanString, start, 0, src - start);
	}
	else {
		// Ending with a separator character is illegal.
		*result = Real128_Zero;
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
	*result = Real128_FromRawCString((const char *)StringBuilder_GetBytes(cleanString));
	return !Real128_IsNaN(*result);
}
