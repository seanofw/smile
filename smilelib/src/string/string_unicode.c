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
#include <smile/internal/unicode.h>

static Byte GetCombiningClass(UInt32 ch);
static void SortCombiningCharacters(UInt32 *buffer, UInt32 *temp, Int start, Int length);

/// <summary>
/// This identity table is used by ConvertCase() when a character is not found in any of the other code-point
/// tables; these zeros result in a delta of zero, or the same code point:  i.e., an identity transform.
/// </summary>
static const Int32 _identityTable[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/// <summary>
/// Read from the string at the given index, extracting a complete Unicode character (code point)
/// that has been encoded in UTF-8 form.
/// </summary>
/// <param name="str">The string from which you would like to extract the next Unicode character.</param>
/// <param name="index">A pointer to the byte index from the start of the string at which the UTF-8-encoded
/// Unicode character (code point) can be found.  After the character has been read, this will be updated
/// to point to the start of the next Unicode character (code point) in the string.</param>
/// <returns>The value of the extracted Unicode character (code point).</returns>
Int32 String_ExtractUnicodeCharacter(const String str, Int *index)
{
	Byte c1, c2, c3, c4;
	const Byte *text = String_GetBytes(str);

	c1 = text[(*index)++];
	if (c1 < 0x80)
	{
		// Classic ASCII.
		return (int)c1;
	}
	else if (c1 < 0xC2)
	{
		// Illegal overencodings for ASCII.
		return -1;
	}
	else if (c1 < 0xE0)
	{
		// Low values in the Basic Multilingual Plane, like Latin and Greek and Hebrew.
		if ((*index) >= String_Length(str)) return -1;		// Out of input.
		c2 = text[(*index)++];
		if ((c2 & 0xC0) != 0x80) return -1;			// Illegal subsequent byte.
		return (c1 << 6) + c2 - 0x3080;
	}
	else if (c1 < 0xF0)
	{
		// High values in the Basic Multilingual Plane, like Chinese and Japanese and Arabic.
		if ((*index) >= String_Length(str) - 1)					// Out of input.
		{
			(*index) = String_Length(str);
			return 0xFFFD;
		}
		c2 = text[(*index)++];
		c3 = text[(*index)++];
		if ((c2 & 0xC0) != 0x80) return -1;			// Illegal subsequent byte.
		if ((c3 & 0xC0) != 0x80) return -1;			// Illegal subsequent byte.
		if (c1 == 0xE0 && c2 < 0xA0) return -1;		// Illegal overlong encoding.
		return (c1 << 12) + (c2 << 6) + c3 - 0xE2080;
	}
	else if (c1 < 0xF5)
	{
		// Four bytes for characters above the BMP (i.e., characters requiring more than 16 bits).
		if ((*index) >= String_Length(str) - 2)
		{
			(*index) = String_Length(str);
			return -1;
		}
		c2 = text[(*index)++];
		c3 = text[(*index)++];
		c4 = text[(*index)++];
		if ((c2 & 0xC0) != 0x80) return -1;			// Illegal subsequent byte.
		if ((c3 & 0xC0) != 0x80) return -1;			// Illegal subsequent byte.
		if ((c4 & 0xC0) != 0x80) return -1;			// Illegal subsequent byte.
		if (c1 == 0xF0 && c2 < 0x90) return -1;		// Illegal overlong encoding.
		if (c1 == 0xF4 && c2 >= 0x90) return -1;	// Illegal high value.
		return (c1 << 18) + (c2 << 12) + (c3 << 6) + c4 - 0x3C82080;
	}
	else
	{
		// Illegal high values.
		return -1;
	}
}

/// <summary>
/// Read from the string at the given start pointer, extracting a complete Unicode character (code point)
/// that has been encoded in UTF-8 form.
/// </summary>
/// <param name="start">A pointer to a pointer to the start of the the next Unicode character.  After the
/// character has been read, this will be updated to point to the start of the next Unicode character
/// (code point) in the string.</param>
/// <param name="end">An exclusive pointer to the end of the string (i.e., this points to the first address
/// <em>after</em> the characters in the string, or to the '\0' in a C-style string).</param>
/// <returns>The value of the extracted Unicode character (code point).</returns>
Int32 String_ExtractUnicodeCharacterInternal(const Byte **start, const Byte *end)
{
	Byte c1, c2, c3, c4;
	const Byte *text = *start;

	c1 = *text++;
	if (c1 < 0x80)
	{
		// Classic ASCII.
		return (*start = text), (int)c1;
	}
	else if (c1 < 0xC2)
	{
		// Illegal overencodings for ASCII.
		return (*start = text), -1;
	}
	else if (c1 < 0xE0)
	{
		// Low values in the Basic Multilingual Plane, like Latin and Greek and Hebrew.
		if (text >= end) return (*start = text), -1;					// Out of input.
		c2 = *text++;
		if ((c2 & 0xC0) != 0x80) return (*start = text), -1;			// Illegal subsequent byte.
		return (*start = text), (c1 << 6) + c2 - 0x3080;
	}
	else if (c1 < 0xF0)
	{
		// High values in the Basic Multilingual Plane, like Chinese and Japanese and Arabic.
		if (text >= end - 1)												// Out of input.
		{
			text = end;
			return (*start = text), -1;
		}
		c2 = *text++;
		c3 = *text++;
		if ((c2 & 0xC0) != 0x80) return (*start = text), -1;			// Illegal subsequent byte.
		if ((c3 & 0xC0) != 0x80) return (*start = text), -1;			// Illegal subsequent byte.
		if (c1 == 0xE0 && c2 < 0xA0) return (*start = text), -1;		// Illegal overlong encoding.
		return (*start = text), (c1 << 12) + (c2 << 6) + c3 - 0xE2080;
	}
	else if (c1 < 0xF5)
	{
		// Four bytes for characters above the BMP (i.e., characters requiring more than 16 bits).
		if (text >= end - 2)
		{
			text = end;
			return (*start = text), -1;
		}
		c2 = *text++;
		c3 = *text++;
		c4 = *text++;
		if ((c2 & 0xC0) != 0x80) return (*start = text), -1;			// Illegal subsequent byte.
		if ((c3 & 0xC0) != 0x80) return (*start = text), -1;			// Illegal subsequent byte.
		if ((c4 & 0xC0) != 0x80) return (*start = text), -1;			// Illegal subsequent byte.
		if (c1 == 0xF0 && c2 < 0x90) return (*start = text), -1;		// Illegal overlong encoding.
		if (c1 == 0xF4 && c2 >= 0x90) return (*start = text), -1;		// Illegal high value.
		return (*start = text), (c1 << 18) + (c2 << 12) + (c3 << 6) + c4 - 0x3C82080;
	}
	else
	{
		// Illegal high values.
		return (*start = text), -1;
	}
}

/// <summary>
/// Compare substrings in two strings, case-insensitive, to lexically order those substrings,
/// without extracting the substrings to new string instances if possible.
/// </summary>
/// <param name="a">The first string to compare.</param>
/// <param name="astart">The start of the substring to compare within the first string.</param>
/// <param name="alength">The number of characters to compare within the first string.</param>
/// <param name="b">The second string to compare.</param>
/// <param name="bstart">The start of the substring to compare within the second string.</param>
/// <param name="blength">The number of characters to compare within the second string.</param>
/// <param name="usedSlowConversion">If this comparison required that the substrings be extracted
/// and case-folded the full way, such as a single code point case-folded to multiple code points,
/// this will be set to True.  If the text could be compared in place much more quickly, this will
/// be set to False.</param>
/// <returns>0 if the strings were equal; -1 if a's substring comes before b's substring; or +1
/// if b's substring comes before a's substring.</returns>
Int String_CompareRangeI(const String a, Int astart, Int alength, const String b, Int bstart, Int blength, Bool *usedSlowConversion)
{
	const Byte *aptr, *bptr, *aend, *bend, *oldaptr, *oldbptr;
	Byte abyte, bbyte;
	Int32 ach, bch, newCodeValueA, newCodeValueB;
	Int codePageIndexA, codePageIndexB;
	const Int32 *codePageA, *codePageB;
	String foldedA, foldedB;

	if (astart < 0)
	{
		alength += astart;
		astart = 0;
	}
	if (bstart < 0)
	{
		blength += bstart;
		bstart = 0;
	}

	if (String_IsNullOrEmpty(a) || astart >= String_Length(a) || alength <= 0)
	{
		*usedSlowConversion = False;
		return String_IsNullOrEmpty(b) || bstart >= String_Length(b) || blength <= 0 ? 0 : -1;
	}
	if (String_IsNullOrEmpty(b) || bstart >= String_Length(b) || blength <= 0)
	{
		*usedSlowConversion = False;
		return -1;
	}

	if (alength > String_Length(a) - astart)
	{
		alength = String_Length(a) - astart;
	}
	if (blength > String_Length(b) - bstart)
	{
		blength = String_Length(b) - bstart;
	}

	aptr = String_GetBytes(a) + astart;
	bptr = String_GetBytes(b) + bstart;
	aend = aptr + alength;
	bend = bptr + blength;
	while (aptr < aend && bptr < bend)
	{
		// Read one complete Unicode code point from string A.
		abyte = *(oldaptr = aptr);
		if (abyte < 128)
		{
			ach = abyte;
			aptr++;
		}
		else
		{
			ach = String_ExtractUnicodeCharacterInternal(&aptr, aend);
			if (ach < 0) ach = 0xFFFD;
		}

		// Read one complete Unicode code point from string B.
		bbyte = *(oldbptr = bptr);
		if (bbyte < 128)
		{
			bch = bbyte;
			bptr++;
		}
		else
		{
			bch = String_ExtractUnicodeCharacterInternal(&bptr, bend);
			if (bch < 0) bch = 0xFFFD;
		}

		// Use the case-folding tables to fold both characters, hopefully resulting in the
		// same number of characters we started with.
		codePageIndexA = (UInt)(UInt32)ach >> 8;
		codePageIndexB = (UInt)(UInt32)bch >> 8;
		codePageA = codePageIndexA < UnicodeTables_CaseFoldingTableCount ? UnicodeTables_CaseFoldingTable[codePageIndexA] : _identityTable;
		codePageB = codePageIndexB < UnicodeTables_CaseFoldingTableCount ? UnicodeTables_CaseFoldingTable[codePageIndexB] : _identityTable;
		newCodeValueA = ach + codePageA[ach & 0xFF];
		newCodeValueB = bch + codePageB[bch & 0xFF];

		if (newCodeValueA <= 0 || newCodeValueB <= 0)
		{
			// We did as much of the easy character-for-character comparison as we could,
			// but one of these characters case-folded to multiple resulting characters.  So
			// we have to resort a different, slower technique:  Convert the rest of the strings
			// en masse to case-folded form, and then compare them directly.
			foldedA = String_CaseFoldRange(a, oldaptr - String_GetBytes(a), aend - oldaptr);
			foldedB = String_CaseFoldRange(b, oldbptr - String_GetBytes(b), bend - oldbptr);
			*usedSlowConversion = True;
			return String_Compare(foldedA, foldedB);
		}

		// Compare the case-folded versions of each character.
		if (newCodeValueA != newCodeValueB)
		{
			*usedSlowConversion = False;
			return newCodeValueA < newCodeValueB ? -1 : +1;
		}
	}

	*usedSlowConversion = False;

	// If we ran out of characters in either string, then sort accordingly.
	if (aptr != aend) return +1;
	if (bptr != bend) return -1;

	// The strings are equivalent, case-insensitive.
	return 0;
}

/// <summary>
/// Shared case-conversion function, using a common lookup-table structure shared by all of the case-conversion functions.
/// </summary>
/// <param name="str">The string whose substring you would like to convert.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <param name="caseTable">A pointer to the caseTable, which is itself pointers to 256-character tables containing code-point deltas.
/// A delta which results in a code point of zero will be looked up against the extended table, as it converts to multiple characters.</param>
/// <param name="caseTableExtended">A pointer to the caseTableExtended, which is similar to the caseTable, but which contains sequences
/// of code points for each converted code point instead of individual code points.  Only used if the code point delta in the caseTable results
/// in a zero.</param>
/// <param name="caseTableCount">The number of pointers to 256-code-point tables found within the caseTable/castTableExtended pointer tables.</param>
/// <returns>The case-converted string.</returns>
static String ConvertCase(const String str, Int start, Int length, const Int32 **caseTable, const Int32 ***caseTableExtended, Int32 caseTableCount)
{
	struct StringBuilderInt sb;
	StringBuilder stringBuilder;
	Int32 code, codePageIndex, newCodeValue, numCodeValues;
	const Int32 *zeroCodePage, *codePage, *codeValues;
	const Int32 **extendedCodePage;
	Int i;
	Byte ch;
	const Byte *text;

	if (String_IsNullOrEmpty(str)) return (String)str;

	if (start < 0)
	{
		length += start;
		start = 0;
	}

	if (length <= 0 || start >= String_Length(str))
		return String_Empty;

	if (length > String_Length(str) - start)
	{
		length = String_Length(str) - start;
	}

	stringBuilder = (StringBuilder)&sb;
	StringBuilder_InitWithSize(stringBuilder, String_Length(str) * 5 / 4);

	zeroCodePage = caseTable[0];

	text = String_GetBytes(str);
	for (i = start; i < start + length; )
	{
		ch = text[i];
		if (ch < 128)
		{
			i++;
			StringBuilder_AppendByte(stringBuilder, (Byte)(ch + zeroCodePage[ch]));
		}
		else
		{
			code = String_ExtractUnicodeCharacter(str, &i);
			if (code < 0) code = 0xFFFD;
			codePageIndex = code >> 8;
			codePage = (codePageIndex < caseTableCount ? caseTable[codePageIndex] : _identityTable);
			newCodeValue = code + codePage[code & 0xFF];
			if (newCodeValue != 0)
			{
				StringBuilder_AppendUnicode(stringBuilder, newCodeValue);
			}
			else
			{
				extendedCodePage = caseTableExtended[code >> 8];
				codeValues = extendedCodePage[code & 0xFF];
				numCodeValues = *codeValues++;
				while (numCodeValues-- > 0)
				{
					StringBuilder_AppendUnicode(stringBuilder, *codeValues++);
				}
			}
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Extract a substring from the given string, and then convert that substring to
/// lowercase where it is not already lowercase.
/// </summary>
/// <param name="str">The string whose substring you would like to convert.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <returns>The whole string, with the given substring converted to lowercase.</returns>
String String_ToLowerRange(const String str, Int start, Int length)
{
	return ConvertCase(str, start, length,
		UnicodeTables_LowercaseTable,
		UnicodeTables_LowercaseTableExtended,
		UnicodeTables_LowercaseTableCount);
}

/// <summary>
/// Extract a substring from the given string, and then convert that substring to
/// titlecase where it is not already titlecase.
/// </summary>
/// <param name="str">The string whose substring you would like to convert.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <returns>The whole string, with the given substring converted to titlecase.</returns>
String String_ToTitleRange(const String str, Int start, Int length)
{
	return ConvertCase(str, start, length,
		UnicodeTables_TitlecaseTable,
		UnicodeTables_TitlecaseTableExtended,
		UnicodeTables_TitlecaseTableCount);
}

/// <summary>
/// Extract a substring from the given string, and then convert that substring to
/// uppercase where it is not already titlecase.
/// </summary>
/// <param name="str">The string whose substring you would like to convert.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <returns>The whole string, with the given substring converted to uppercase.</returns>
String String_ToUpperRange(const String str, Int start, Int length)
{
	return ConvertCase(str, start, length,
		UnicodeTables_UppercaseTable,
		UnicodeTables_UppercaseTableExtended,
		UnicodeTables_UppercaseTableCount);
}

/// <summary>
/// Extract a substring from the given string, and then case-fold that substring, so that
/// its characters are suitable for case-insensitive comparison.
/// </summary>
/// <param name="str">The string whose substring you would like to convert.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <returns>The whole string, with the given substring case-folded.</returns>
String String_CaseFoldRange(const String str, Int start, Int length)
{
	return ConvertCase(str, start, length,
		UnicodeTables_CaseFoldingTable,
		UnicodeTables_CaseFoldingTableExtended,
		UnicodeTables_CaseFoldingTableCount);
}

/// <summary>
/// Extract a substring from the given string, and then decompose any composed Unicode
/// characters within it into their constituent combining forms.
/// </summary>
/// <param name="str">The string whose substring you would like to convert.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <returns>The whole string, with the given substring case-folded.</returns>
String String_DecomposeRange(const String str, Int start, Int length)
{
	return ConvertCase(str, start, length,
		UnicodeTables_DecompositionTable,
		UnicodeTables_DecompositionTableExtended,
		UnicodeTables_DecompositionTableCount);
}

/// <summary>
/// Extract a substring from the given string, and then compose any decomposed characters in
/// it so that all combining diacritics and compound characters are joined to result in as few
/// Unicode code points as possible.
/// </summary>
/// <param name="str">The string whose substring you would like to compose.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <returns>The whole string, with the given substring composed.</returns>
String String_ComposeRange(const String str, Int start, Int length)
{
	Int i;
	Int32 a, b, c, d;
	Int strLength;
	const Byte *text;
	StringBuilder stringBuilder;
	Byte ch;
	Int32 composed;

	if (String_IsNullOrEmpty(str)) return str;

	if (start < 0) {
		length += start;
		start = 0;
	}

	strLength = String_Length(str);
	text = String_GetBytes(str);

	if (length <= 0 || start >= strLength)
		return String_Empty;

	if (length > strLength - start)
		length = strLength - start;

	stringBuilder = StringBuilder_CreateWithSize(length * 5 / 4);

	// These four variables will act like a shift register.
	a = 0, b = -1, c = -1, d = -1;
	i = start;

	// Loop until the shift register becomes empty.  We prepopulate it with a zero value
	// that it will lose on the first shift; after that, it will be populated by incoming codes.
	while ((a & b & c & d) != -1) {
		// Shift down.
		a = b, b = c, c = d;

		// Push the next Unicode character onto the end of the shift register (d), if there are
		// characters remaining.  If the source string is empty, push a -1.
		if (i < start + length) {
			ch = text[i];
			if (ch < 128)
				i++, d = ch;
			else {
				d = String_ExtractUnicodeCharacter(str, &i);
				if (d < 0) d = 0xFFFD;
			}
		}
		else d = -1;

		// See if the register's left end can be composed.  If so, compose it, pushing
		// the new composed code up the register to the right.
		composed = Unicode_Compose(a, b, c, d);
		if (composed >= 0) {
			switch (composed >> 24) {
				case 1:
					a = composed & 0xFFFFFF;
					break;
				case 2:
					b = composed & 0xFFFFFF;
					a = -1;
					break;
				case 3:
					c = composed & 0xFFFFFF;
					b = -1;
					a = -1;
					break;
				case 4:
					d = composed & 0xFFFFFF;
					c = -1;
					b = -1;
					a = -1;
					break;
			}
		}

		// If we have something we can append at the left end of the register, append it.
		if (a != -1)
			StringBuilder_AppendUnicode(stringBuilder, (UInt32)a);
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Normalize a substring within a string, so that its combining characters are in canonical order.
/// </summary>
/// <param name="str">The string whose substring you would like to normalize.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <returns>The whole string, with the given substring normalized.</returns>
String String_NormalizeRange(const String str, Int start, Int length)
{
#	define INITIAL_HOLDING_LENGTH 20

	StringBuilder stringBuilder;
	Int i, j, lasti, strLength, codePageIndex, hdest;
	const Byte *text;
	Int32 ch;
	Byte b;
	Byte canonicalCombiningClass;
	UInt32 *holding, *newHolding;
	Int holdingLength, newHoldingLength;
	UInt32 initialHolding[INITIAL_HOLDING_LENGTH];
	UInt32 *temp;
	UInt32 initialTemp[INITIAL_HOLDING_LENGTH];

	if (String_IsNullOrEmpty(str)) return str;

	if (start < 0) {
		length += start;
		start = 0;
	}

	strLength = String_Length(str);
	text = String_GetBytes(str);

	if (length <= 0 || start >= strLength)
		return String_Empty;

	if (length > strLength - start)
		length = strLength - start;

	// This will hold the entire string, after we've normalized it.
	stringBuilder = StringBuilder_CreateWithSize(length * 5 / 4);

	// This smallish buffer will hold each completely-composed character as we find it (and its friends) in the string.
	// If we run out of space in the buffer, we'll allocate a bigger one from the heap, but nearly everything should be
	// able to fit in INITIAL_HOLDING_LENGTH on the stack.
	holding = initialHolding;
	holdingLength = INITIAL_HOLDING_LENGTH;

	i = 0;
	while (i < length) {
		// Read the next (full) character into 'ch'.
		b = text[i];
		if (b < 128) {
			// Plain ASCII is copied to the output verbatim.
			StringBuilder_AppendByte(stringBuilder, b);
			i++;
			continue;
		}
		else {
			ch = String_ExtractUnicodeCharacter(str, &i);
			if (ch < 0) ch = 0xFFFD;
		}

		// Find out its code page.  If it's *not* a combining character, just copy it to the output.
		codePageIndex = ch >> 8;
		canonicalCombiningClass = codePageIndex < UnicodeTables_CanonicalCombiningClassTableCount
			? UnicodeTables_CanonicalCombiningClassTable[codePageIndex][ch & 0xFF] : 0;

		if (canonicalCombiningClass == 0) {
			StringBuilder_AppendUnicode(stringBuilder, ch);
			continue;
		}

		// It's a combining diacritic, which means we need to sort-and-shuffle it relative to it
		// and any of its subsequent combining-diacritic friends.
		{
			// Collect all relevant code points for this fully-composed character into the 'holding' buffer,
			// stopping when we either run out of characters or when we reach another start character.
			hdest = 0;
			holding[hdest++] = ch;
			lasti = i;
			while (i < strLength) {
				lasti = i;
				b = text[i];
				if (b < 128) {
					ch = b;
					i++;
				}
				else {
					ch = String_ExtractUnicodeCharacter(str, &i);
					if (ch < 0) ch = 0xFFFD;
				}

				if (hdest >= holdingLength) {
					newHoldingLength = holdingLength * 2;
					newHolding = GC_MALLOC_RAW_ARRAY(UInt32, newHoldingLength);
					MemCpy(newHolding, holding, sizeof(UInt32) * holdingLength);
					holding = newHolding;
					holdingLength = newHoldingLength;
				}

				codePageIndex = ch >> 8;
				canonicalCombiningClass = codePageIndex < UnicodeTables_CanonicalCombiningClassTableCount
					? UnicodeTables_CanonicalCombiningClassTable[codePageIndex][ch & 0xFF] : 0;

				if (canonicalCombiningClass == 0) break;

				holding[hdest++] = ch;
			}
			i = lasti;

			// Only bother with sorting if we found more than one of these things.
			if (hdest > 1) {
				// Now that we've found the scope of the combining characters, we need to sort them.
				temp = hdest >= 16 ? GC_MALLOC_RAW_ARRAY(UInt32, holdingLength) : initialTemp;
				SortCombiningCharacters(holding, temp, 0, hdest);

				// Finally, the holding buffer's diacritics need to be copied to the output, now
				// that they are in sorted order.
				for (j = 0; j < hdest; j++) {
					StringBuilder_AppendUnicode(stringBuilder, holding[j]);
				}
			}
			else {
				// Only one diacritic, so it doesn't need sorting because it has no friends.
				StringBuilder_AppendUnicode(stringBuilder, holding[0]);
			}
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Look up the Unicode canonical combining class for the given code point.
/// </summary>
/// <param name="ch">The code point to look up.</param>
/// <returns>The canonical combining class for that code point, from the combining-class enum.</returns>
static Byte GetCombiningClass(UInt32 ch)
{
	UInt32 codePageIndex;
	const Byte *codePage;
	Byte canonicalCombiningClass;

	codePageIndex = ch >> 8;
	if (codePageIndex >= (UInt32)UnicodeTables_CanonicalCombiningClassTableCount) return 0;
	codePage = UnicodeTables_CanonicalCombiningClassTable[codePageIndex];
	canonicalCombiningClass = codePage[ch & 0xFF];
	return canonicalCombiningClass;
}

/// <summary>
/// Sort the characters in the given buffer by their canonical combining classes, using the given temporary
/// buffer as necessary.  This implements a stable sort.
/// </summary>
/// <param name="buffer">The buffer in which the sorting is to take place.</param>
/// <param name="temp">A temporary buffer, which must be the same size as 'buffer', which is used as
/// scratch space during the sort.</param>
/// <param name="start">The starting index at which the sorting is to begin in the buffer.</param>
/// <param name="length">The number of characters after that starting index to be sorted.</param>
static void SortCombiningCharacters(UInt32 *buffer, UInt32 *temp, Int start, Int length)
{
	UInt32 a, b, c;
	Byte ac, bc, cc, c1, c2;
	Int midpt, src1, src2, dest, count1, count2;

	// Optimized case for 0 and 1 combining character.
	if (length <= 1) return;

	// Optimized case for 2 combining characters.
	if (length == 2) {
		a = buffer[start];
		b = buffer[start + 1];
		ac = GetCombiningClass(a);
		bc = GetCombiningClass(b);
		if (bc < ac) {
			buffer[start] = b;
			buffer[start + 1] = a;
		}
		return;
	}

	// Optimized case for 3 combining characters.
	if (length == 3) {
		a = buffer[start];
		b = buffer[start + 1];
		c = buffer[start + 2];
		ac = GetCombiningClass(a);
		bc = GetCombiningClass(b);
		cc = GetCombiningClass(c);

		if (cc < ac && cc < bc) {
			buffer[start] = c;
			if (bc < ac) {
				buffer[start + 1] = b;
				buffer[start + 2] = a;
			}
			else {
				buffer[start + 1] = a;
				buffer[start + 2] = b;
			}
		}
		else if (bc < ac) {
			buffer[start] = b;
			if (cc < ac) {
				buffer[start + 1] = c;
				buffer[start + 2] = a;
			}
			else {
				buffer[start + 1] = a;
				buffer[start + 2] = c;
			}
		}
		else {
			buffer[start] = a;
			if (cc < bc) {
				buffer[start + 1] = c;
				buffer[start + 2] = b;
			}
			else {
				buffer[start + 1] = b;
				buffer[start + 2] = c;
			}
		}
		return;
	}

	// 4 or more combining characters:  It's time to divide and conquer using recursive mergesort.
	// It's important that we use a stable sort here.

	// Sort the subarrays.
	midpt = length / 2;
	SortCombiningCharacters(buffer, temp, start, midpt);
	SortCombiningCharacters(buffer, temp, start + midpt, length - midpt);

	// Merge the subarrays into a single combined array in temp.
	src1 = start;
	src2 = start + midpt;
	dest = start;
	count1 = midpt;
	count2 = length - midpt;
	while (count1 > 0 && count2 > 0) {
		c1 = GetCombiningClass(buffer[src1]);
		c2 = GetCombiningClass(buffer[src2]);
		if (c1 <= c2) {
			temp[dest++] = buffer[src1++];
			count1--;
		}
		else {
			temp[dest++] = buffer[src2++];
			count2--;
		}
	}
	while (count1 > 0) {
		temp[dest++] = buffer[src1++];
		count1--;
	}
	while (count2 > 0) {
		temp[dest++] = buffer[src2++];
		count2--;
	}

	// Copy from temp back to the original buffer.
	MemCpy((UInt32 *)buffer + start, (UInt32 *)temp + start, sizeof(UInt32) * length);
}

/// <summary>
/// Convert a substring within a string from UTF-8 encoding to that described by the given legacy code-page tables.
/// </summary>
/// <param name="str">The string whose substring you would like to convert to a code page.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <param name="utf8ToCodePageTables">A pointer to a table of pointers to 256-byte tables that describe how to
/// convert each Unicode code point.</param>
/// <param name="numTables">The number of 256-byte tables described.</param>
/// <returns>The whole string, with the given substring converted to that code page.</returns>
String String_ConvertUtf8ToCodePageRange(const String str, Int start, Int length, const Byte **utf8ToCodePageTables, Int numTables)
{
	Int strLength, src, dest;
	Int32 value, codePage;
	const Byte *text;
	Byte ch;
	String resultStr;
	Byte *newText;

	if (str == NULL || (strLength = String_Length(str)) <= 0) return str;
	text = String_GetBytes(str);

	if (start < 0) {
		length += start;
		start = 0;
	}

	if (length <= 0 || start >= strLength)
		return String_Empty;

	if (length > strLength - start)
	{
		length = strLength - start;
	}

	if (length <= 0) return String_Empty;

	resultStr = String_CreateInternal(length);
	newText = resultStr->_opaque.text;

	src = start;
	dest = 0;

	while (src < length)
	{
		ch = text[src];
		if (ch <= 127) {
			src++;
			ch = utf8ToCodePageTables[0][ch & 0xFF];
		}
		else {
			value = String_ExtractUnicodeCharacter(str, &src);
			if (value < 0) value = 0xFFFD;
			codePage = value >> 8;
			ch = codePage < numTables ? utf8ToCodePageTables[codePage][value & 0xFF] : '?';
		}
		newText[dest++] = ch;
	}

	newText[dest] = '\0';

	resultStr->_opaque.length = dest;		// TODO: Find a better way to do this that's just as fast but wastes less memory.

	return resultStr;
}

/// <summary>
/// Convert a substring within a string from a specific legacy code-page encoding to standard UTF-8.
/// </summary>
/// <param name="str">The string whose substring you would like to convert to UTF-8.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <param name="codePageToUtf8Table">A 256-entry table describing the resulting Unicode code point for each byte in the string.</param>
/// <returns>The whole string, with the given substring converted to UTF-8.</returns>
String String_ConvertCodePageToUtf8Range(const String str, Int start, Int length, const UInt16 *codePageToUtf8Table)
{
	StringBuilder stringBuilder;
	Int strLength, src;
	UInt16 value;
	const Byte *text;
	Byte ch;

	if (str == NULL || (strLength = String_Length(str)) <= 0) return str;
	text = String_GetBytes(str);

	if (start < 0) {
		length += start;
		start = 0;
	}

	if (length <= 0 || start >= strLength)
		return String_Empty;

	if (length > strLength - start)
	{
		length = strLength - start;
	}

	if (length <= 0) return String_Empty;

	stringBuilder = StringBuilder_CreateWithSize(length);
	src = 0;

	while (src < strLength)
	{
		ch = text[src++];
		value = codePageToUtf8Table[ch];
		StringBuilder_AppendUnicode(stringBuilder, (UInt32)value);
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Convert a substring within a string from UTF-8 encoding to that described by the given legacy code page.
/// </summary>
/// <param name="str">The string whose substring you would like to convert to a code page.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <param name="legacyCodePageID">The ID of the known legacy code-page.  If the code page is unknown, this will return the empty string.</param>
/// <returns>The whole string, with the given substring converted to that code page.</returns>
String String_ConvertUtf8ToKnownCodePageRange(const String str, Int start, Int length, Int legacyCodePageID)
{
	switch (legacyCodePageID) {

		case LEGACY_CODE_PAGE_ISO_8859_1:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_1Table, UnicodeTables_UnicodeToIso_8859_1TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_2:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_2Table, UnicodeTables_UnicodeToIso_8859_2TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_3:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_3Table, UnicodeTables_UnicodeToIso_8859_3TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_4:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_4Table, UnicodeTables_UnicodeToIso_8859_4TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_5:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_5Table, UnicodeTables_UnicodeToIso_8859_5TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_6:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_6Table, UnicodeTables_UnicodeToIso_8859_6TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_7:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_7Table, UnicodeTables_UnicodeToIso_8859_7TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_8:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_8Table, UnicodeTables_UnicodeToIso_8859_8TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_9:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_9Table, UnicodeTables_UnicodeToIso_8859_9TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_10:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_10Table, UnicodeTables_UnicodeToIso_8859_10TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_11:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_11Table, UnicodeTables_UnicodeToIso_8859_11TableCount);
		// There is no such thing as ISO 8859-12.
		case LEGACY_CODE_PAGE_ISO_8859_13:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_13Table, UnicodeTables_UnicodeToIso_8859_13TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_14:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_14Table, UnicodeTables_UnicodeToIso_8859_14TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_15:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_15Table, UnicodeTables_UnicodeToIso_8859_15TableCount);
		case LEGACY_CODE_PAGE_ISO_8859_16:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToIso_8859_16Table, UnicodeTables_UnicodeToIso_8859_16TableCount);

		case LEGACY_CODE_PAGE_CP437:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToCp437Table, UnicodeTables_UnicodeToCp437TableCount);

		case LEGACY_CODE_PAGE_WIN1250:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1250Table, UnicodeTables_UnicodeToWindows1250TableCount);
		case LEGACY_CODE_PAGE_WIN1251:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1251Table, UnicodeTables_UnicodeToWindows1251TableCount);
		case LEGACY_CODE_PAGE_WIN1252:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1252Table, UnicodeTables_UnicodeToWindows1252TableCount);
		case LEGACY_CODE_PAGE_WIN1253:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1253Table, UnicodeTables_UnicodeToWindows1253TableCount);
		case LEGACY_CODE_PAGE_WIN1254:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1254Table, UnicodeTables_UnicodeToWindows1254TableCount);
		case LEGACY_CODE_PAGE_WIN1255:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1255Table, UnicodeTables_UnicodeToWindows1255TableCount);
		case LEGACY_CODE_PAGE_WIN1256:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1256Table, UnicodeTables_UnicodeToWindows1256TableCount);
		case LEGACY_CODE_PAGE_WIN1257:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1257Table, UnicodeTables_UnicodeToWindows1257TableCount);
		case LEGACY_CODE_PAGE_WIN1258:
			return String_ConvertUtf8ToCodePageRange(str, start, length, UnicodeTables_UnicodeToWindows1258Table, UnicodeTables_UnicodeToWindows1258TableCount);

		default:
			return String_Empty;
	}
}

/// <summary>
/// Convert a substring within a string from a specific legacy code-page encoding to standard UTF-8.
/// </summary>
/// <param name="str">The string whose substring you would like to convert to UTF-8.</param>
/// <param name="start">The start of the substring within that string.</param>
/// <param name="length">The length of the substring within that string.</param>
/// <param name="legacyCodePageID">The ID of the known legacy code-page.  If the code page is unknown, this will return the empty string.</param>
/// <returns>The whole string, with the given substring converted to UTF-8.</returns>
String String_ConvertKnownCodePageToUtf8Range(const String str, Int start, Int length, Int legacyCodePageID)
{
	switch (legacyCodePageID) {

		case LEGACY_CODE_PAGE_ISO_8859_1:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_1ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_2:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_2ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_3:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_3ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_4:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_4ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_5:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_5ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_6:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_6ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_7:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_7ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_8:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_8ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_9:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_9ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_10:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_10ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_11:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_11ToUnicodeTable);
		// There is no such thing as ISO 8859-12.
		case LEGACY_CODE_PAGE_ISO_8859_13:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_13ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_14:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_14ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_15:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_15ToUnicodeTable);
		case LEGACY_CODE_PAGE_ISO_8859_16:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Iso_8859_16ToUnicodeTable);

		case LEGACY_CODE_PAGE_CP437:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Cp437ToUnicodeTable);

		case LEGACY_CODE_PAGE_WIN1250:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1250ToUnicodeTable);
		case LEGACY_CODE_PAGE_WIN1251:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1251ToUnicodeTable);
		case LEGACY_CODE_PAGE_WIN1252:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1252ToUnicodeTable);
		case LEGACY_CODE_PAGE_WIN1253:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1253ToUnicodeTable);
		case LEGACY_CODE_PAGE_WIN1254:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1254ToUnicodeTable);
		case LEGACY_CODE_PAGE_WIN1255:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1255ToUnicodeTable);
		case LEGACY_CODE_PAGE_WIN1256:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1256ToUnicodeTable);
		case LEGACY_CODE_PAGE_WIN1257:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1257ToUnicodeTable);
		case LEGACY_CODE_PAGE_WIN1258:
			return String_ConvertCodePageToUtf8Range(str, start, length, UnicodeTables_Windows1258ToUnicodeTable);

		default:
			return String_Empty;
	}
}

/// <summary>
/// Search through a given string for the first index of a given substring, case-insensitive.
/// </summary>
/// <param name="str">The string you would like to search through.</param>
/// <param name="pattern">The substring you would like to search for.</param>
/// <param name="start">The start character index within the string where the search should begin (usually zero).</param>
/// <returns>The zero-based index of the first match, or -1 if no match is found.</returns>
Int String_IndexOfI(const String str, const String pattern, Int start)
{
	Int end, slength, plength;
	Bool usedSlowConversion;

	slength = String_Length(str);
	plength = String_Length(pattern);

	if (start < 0) start = 0;

	for (end = slength - plength; start <= end; start++)
	{
		if (String_CompareRangeI(pattern, 0, plength, str, start, plength, &usedSlowConversion) == 0)
			return start;
	}
	return -1;
}

/// <summary>
/// Search backward through a given string for the last index of a given substring, case-insensitive.
/// </summary>
/// <param name="str">The string you would like to search through.</param>
/// <param name="pattern">The substring you would like to search for.</param>
/// <param name="start">The start character index within the string where the search should begin (usually the length of the string).
/// If passed 'n', the first substring comparison will be against the characters at (n - pattern.length) through (n - 1), inclusive.</param>
/// <returns>The last (rightmost) index within the string that matches the pattern, if any; if no part of the
/// string matches, this returns -1.</returns>
Int String_LastIndexOfI(const String str, const String pattern, Int start)
{
	Bool usedSlowConversion;
	Int slength, plength;

	slength = String_Length(str);
	plength = String_Length(pattern);

	if (plength > slength || start < plength) return -1;

	start -= plength;
	if (start >= slength - plength)
	{
		start = slength - plength;
	}
	for (; start >= 0; start--)
	{
		if (String_CompareRangeI(pattern, 0, plength, str, start, plength, &usedSlowConversion) == 0)
			return start;
	}
	return -1;
}

/// <summary>
/// Count the number of instances of the pattern found in the given string.
/// </summary>
/// <param name="str">The string to search in.</param>
/// <param name="pattern">The pattern to search for in the string.</param>
/// <param name="start">The start character index within the string where the search should begin (usually zero).</param>
/// <returns>The number of times the pattern can be found within the string,
/// using a non-overlapping forward linear search.  If the pattern is null
/// or the empty string, this will return zero.</returns>
Int String_CountOfI(const String str, const String pattern, Int start)
{
	Int index, patternLength;
	Int count;

	if (String_IsNullOrEmpty(pattern) || String_IsNullOrEmpty(str))
		return 0;

	patternLength = String_Length(pattern);
	count = 0;
	index = start;

	while ((index = String_IndexOfI(str, pattern, index)) >= 0) {
		count++;
		index += patternLength;
	}

	return count;
}

/// <summary>
/// Search through the given string looking for the given pattern, case-insensitive.
/// </summary>
/// <param name="str">The string to search through.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <returns>True if any part of the string matches the pattern, case-insensitive; False if no part of the string matches the pattern.</returns>
Bool String_ContainsI(const String str, const String pattern)
{
	return String_Length(pattern) <= String_Length(str) && String_IndexOfI(str, pattern, 0) >= 0;
}

/// <summary>
/// Test the starting bytes of the string against the given pattern, case-insensitive.
/// </summary>
/// <param name="str">The string to compare.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <returns>True if the start of the string matches the pattern, case-insensitive; False if the start of the string does not match the pattern.</returns>
Bool String_StartsWithI(const String str, const String pattern)
{
	Bool usedSlowConversion;

	return String_Length(pattern) <= String_Length(str)
		&& String_CompareRangeI(pattern, 0, String_Length(pattern), str, 0, String_Length(pattern), &usedSlowConversion) == 0;
}

/// <summary>
/// Test the ending bytes of the string against the given pattern.
/// </summary>
/// <param name="str">The string to compare.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <returns>True if the end of the string matches the pattern, case-insensitive; False if the end of the string does not match the pattern.</returns>
Bool String_EndsWithI(const String str, const String pattern)
{
	Bool usedSlowConversion;

	return String_Length(pattern) <= String_Length(str)
		&& String_CompareRangeI(pattern, 0, String_Length(pattern), str, String_Length(str) - String_Length(pattern), String_Length(pattern), &usedSlowConversion) == 0;
}

/// <summary>
/// Replace all instances of a pattern in a string with a replacement string (scanning from left to right),
/// case-insensitive.
/// </summary>
/// <param name="str">The string to search through (if NULL, an empty string is returned).</param>
/// <param name="pattern">The pattern to search for (if NULL, 'str' is returned).</param>
/// <param name="replacement">Replacement text for each instance of the pattern (if NULL, will be treated as empty).</param>
/// <returns>A new string where all instances of the pattern have been replaced by the given replacement string.</returns>
String String_ReplaceI(const String str, const String pattern, const String replacement)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	String r;
	const Byte *text, *patText, *repText;
	Int lastEnd, index, patLength, repLength;

	if (String_IsNullOrEmpty(str)) return String_Empty;
	if (String_IsNullOrEmpty(pattern)) return str;

	text = String_GetBytes(str);
	patText = String_GetBytes(pattern);
	patLength = String_Length(pattern);
	r = (replacement != NULL ? replacement : String_Empty);
	repText = String_GetBytes(r);
	repLength = String_Length(r);

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	lastEnd = 0;
	index = 0;
	while ((index = String_IndexOfI(str, pattern, index)) >= 0) {
		if (index > lastEnd) {
			StringBuilder_Append(stringBuilder, text, lastEnd, index - lastEnd);
		}
		StringBuilder_Append(stringBuilder, repText, 0, repLength);
		lastEnd = (index += String_Length(pattern));
	}

	if (lastEnd < String_Length(str)) {
		StringBuilder_Append(stringBuilder, text, lastEnd, String_Length(str) - lastEnd);
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Replace at most 'limit' instances of a pattern in a string with a replacement string (scanning from left to right).
/// </summary>
/// <param name="str">The string to search through (if NULL, an empty string is returned).</param>
/// <param name="pattern">The pattern to search for (if NULL, 'str' is returned).</param>
/// <param name="replacement">Replacement text for each instance of the pattern (if NULL, will be treated as empty).</param>
/// <param name="limit">The maximum number of replacements to perform (after these, all characters of
/// the string will be left unchanged.</param>
/// <returns>A new string where at most 'limit' instances of the pattern have been replaced by the given replacement string.</returns>
String String_ReplaceWithLimitI(const String str, const String pattern, const String replacement, Int limit)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	String r;
	const Byte *text, *patText, *repText;
	Int lastEnd, index, patLength, repLength;

	if (String_IsNullOrEmpty(str)) return String_Empty;
	if (String_IsNullOrEmpty(pattern)) return str;

	text = String_GetBytes(str);
	patText = String_GetBytes(pattern);
	patLength = String_Length(pattern);
	r = (replacement != NULL ? replacement : String_Empty);
	repText = String_GetBytes(r);
	repLength = String_Length(r);

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	lastEnd = 0;
	index = 0;
	while (limit > 0 && (index = String_IndexOfI(str, pattern, index)) >= 0) {
		if (index > lastEnd) {
			StringBuilder_Append(stringBuilder, text, lastEnd, index - lastEnd);
		}
		StringBuilder_Append(stringBuilder, repText, 0, repLength);
		lastEnd = (index += String_Length(pattern));
		limit--;
	}

	if (lastEnd < String_Length(str)) {
		StringBuilder_Append(stringBuilder, text, lastEnd, String_Length(str) - lastEnd);
	}

	return StringBuilder_ToString(stringBuilder);
}
