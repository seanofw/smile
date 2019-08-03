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
#include <smile/mem.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/internal/types.h>

// How big the text[] array is in the String's _opaque struct.
#define STRING_TEXT_PADDING 65536

/// <summary>
/// Construct a new String instance containing the given substring of text,
/// starting at the given start index, going for the given length.
/// </summary>
/// <param name="text">The source bytes to use to construct the new string.</param>
/// <param name="start">The offset within the source bytes to start copying from.</param>
/// <param name="length">The length of bytes to copy into the new string.</param>
/// <returns>The new String instance.</returns>
String String_Create(const Byte *text, Int length)
{
	String str;
	Byte *newText;

	if (length <= 0) return String_Empty;

	// IMPORTANT NOTE:  GC_MALLOC_ATOMIC() here is technically the wrong thing to do, since
	//   the memory it allocates will actually have pointers inside it.  However, those pointers
	//   are exclusively pointing at things in the static data segment:  They never point to
	//   any data on the heap.  Therefore, the things that those pointers point to will always
	//   be part of the GC's root set, and that means those pointers can be excluded from the
	//   GC's consideration since it will have to trace them anyway.  The upshot is that while
	//   GC_MALLOC_ATOMIC() is technically wrong, it's not *really* wrong because it's safe for
	//   the GC to completely ignore all of the String's data and treat it as just a meaningless,
	//   featureless blob.

	str = (String)GC_MALLOC_ATOMIC(sizeof(struct StringStruct) - STRING_TEXT_PADDING + length + 1);
	if (str == NULL) Smile_Abort_OutOfMemory();

	str->kind = SMILE_KIND_STRING;
	str->vtable = (SmileVTable)&String_VTableData;
	str->base = (SmileObject)&String_BaseObjectStruct;
	str->_opaque.length = length;

	newText = str->_opaque.text;
	MemCpy(newText, text, length);
	newText[length] = '\0';

	return (String)str;
}

/// <summary>
/// Construct a new String instance of the given length, but whose bytes are as yet uninitialized (garbage).
/// </summary>
/// <param name="length">The length of bytes for the new string (not including the terminating nul character).</param>
/// <returns>The new String instance.</returns>
String String_CreateInternal(Int length)
{
	String str;

	if (length <= 0) return String_Empty;

	// See above note about GC_MALLOC_ATOMIC() before changing this code!

	str = (String)GC_MALLOC_ATOMIC(sizeof(struct StringStruct) - STRING_TEXT_PADDING + length + 1);
	if (str == NULL) Smile_Abort_OutOfMemory();

	str->kind = SMILE_KIND_STRING;
	str->vtable = (SmileVTable)&String_VTableData;
	str->base = (SmileObject)&String_BaseObjectStruct;
	str->_opaque.length = length;
	str->_opaque.text[length] = '\0';

	return (String)str;
}

/// <summary>
/// Construct a new String instance containing the given character repeated for the given count.
/// </summary>
/// <param name="b">The source character to use to construct the new string.</param>
/// <param name="repeatCount">The number of times to repeat that character when constructing the new string.</param>
/// <returns>The new String instance.</returns>
String String_CreateRepeat(Byte b, Int repeatCount)
{
	String str = String_CreateInternal(repeatCount);
	MemSet(str->_opaque.text, b, repeatCount);
	return str;
}

/// <summary>
/// Construct a new String instance containing the given Unicode code-point, converted to UTF-8.
/// </summary>
/// <param name="code">The source Unicode code-point to use to construct the new string.</param>
/// <returns>The new String instance.</returns>
String String_CreateFromUnicode(UInt32 code)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 16);
	INIT_INLINE_STRINGBUILDER(stringBuilder);

	StringBuilder_AppendUnicode(stringBuilder, code);

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Construct a new String instance by concatenating many other strings together.
/// </summary>
/// <param name="strs">The array of string instances to join to create the new string.</param>
/// <param name="numStrs">The number of string instances in the array.</param>
/// <returns>The new String instance.</returns>
String String_ConcatMany(const String *strs, Int numStrs)
{
	Int length, dest, i;
	String str, temp;
	Byte *newText;

	if (numStrs <= 0) return String_Empty;

	length = 0;
	for (i = 0; i < numStrs; i++)
		length += String_Length(strs[i]);

	if (length <= 0) return String_Empty;

	str = String_CreateInternal(length);
	newText = str->_opaque.text;

	dest = 0;
	for (i = 0; i < numStrs; i++)
	{
		temp = strs[i];
		MemCpy(newText + dest, String_GetBytes(temp), String_Length(temp));
		dest += String_Length(temp);
	}

	newText[length] = '\0';

	return (String)str;
}

/// <summary>
/// Construct a new String instance by concatenating many other strings together, with a glue string between them.
/// </summary>
/// <param name="glue">The glue string to insert between successive string instances.  This will not be
/// added to the start and end of the resulting string:  Only between strings.</param>
/// <param name="strs">The array of string instances to join to create the new string.</param>
/// <param name="numStrs">The number of string instances in the array.</param>
/// <returns>The new String instance.</returns>
String String_Join(const String glue, const String *strs, Int numStrs)
{
	Int length, dest, i;
	String str;
	Byte *newText;
	const Byte *glueText;
	Int glueLength;

	if (numStrs <= 0) return String_Empty;

	length = 0;
	for (i = 0; i < numStrs; i++)
	{
		length += String_Length(strs[i]);
	}
	length += (numStrs - 1) * String_Length(glue);

	if (length <= 0) return String_Empty;

	str = String_CreateInternal(length);
	newText = str->_opaque.text;

	glueText = String_GetBytes(glue);
	glueLength = String_Length(glue);

	MemCpy(newText, String_GetBytes(strs[0]), String_Length(strs[0]));
	dest = String_Length(strs[0]);

	for (i = 1; i < numStrs; i++)
	{
		MemCpy(newText + dest, glueText, glueLength);
		dest += glueLength;
		MemCpy(newText + dest, String_GetBytes(strs[i]), String_Length(strs[i]));
		dest += String_Length(strs[i]);
	}

	newText[length] = '\0';

	return (String)str;
}

/// <summary>
/// Answer whether two strings are equal, i.e., of the same length and containing the same bytes.
/// </summary>
/// <param name="a">The first string to compare.</param>
/// <param name="b">The second string to compare.</param>
/// <returns>True if they are equal, False if they are not of the same length or contain different bytes.</returns>
Bool String_Equals(const String a, const String b)
{
	const Byte *aText, *bText;

	if (a == b) return True;

	if (a == NULL) return b != NULL;
	if (b == NULL) return False;

	if (String_Length(a) != String_Length(b)) return False;

	aText = String_GetBytes(a);
	bText = String_GetBytes(b);

	switch (String_Length(a))
	{
		case 0: return True;
		case 1: return aText[0] == bText[0];
		case 2: return aText[0] == bText[0] && aText[1] == bText[1];
		case 3: return aText[0] == bText[0] && aText[1] == bText[1] && aText[2] == bText[2];
		case 4: return aText[0] == bText[0] && aText[1] == bText[1] && aText[2] == bText[2] && aText[3] == bText[3];

		default:
			return MemCmp(aText, bText, String_Length(a)) == 0;
	}
}

/// <summary>
/// Answer whether two strings are equal, i.e., of the same length and containing the same bytes.
/// </summary>
/// <param name="str">The Smile string to compare.</param>
/// <param name="other">The C string to compare.</param>
/// <returns>True if they are equal, False if they are not of the same length or contain different bytes.</returns>
Bool String_EqualsC(const String str, const char *other)
{
	Int length = StrLen(other);
	return String_EqualsInternal(str, (const Byte *)other, length);
}

/// <summary>
/// Answer whether a string equals a given chunk of bytes.
/// </summary>
/// <param name="a">The first string to compare (which can be NULL).</param>
/// <param name="bText">The text of the second string to compare.  This must not be NULL.</param>
/// <param name="bLength">The length of the second string to compare.</param>
/// <returns>True if they are equal, False if they are not of the same length or contain different bytes.</returns>
SMILE_API_FUNC Bool String_EqualsInternal(const String a, const Byte *bText, Int bLength)
{
	const Byte *aText;

	if (a == NULL) return False;

	aText = String_GetBytes(a);
	if (aText == bText) return True;

	if (String_Length(a) != bLength) return False;

	switch (String_Length(a))
	{
		case 0: return True;
		case 1: return aText[0] == bText[0];
		case 2: return aText[0] == bText[0] && aText[1] == bText[1];
		case 3: return aText[0] == bText[0] && aText[1] == bText[1] && aText[2] == bText[2];
		case 4: return aText[0] == bText[0] && aText[1] == bText[1] && aText[2] == bText[2] && aText[3] == bText[3];

		default:
			return MemCmp(aText, bText, String_Length(a)) == 0;
	}
}

/// <summary>
/// Answer whether two strings are equal, i.e., of the same length and containing the same bytes,
/// or if they are not equal, which one lexically precedes the other.
/// </summary>
/// <param name="a">The first string to compare.</param>
/// <param name="b">The second string to compare.</param>
/// <returns>0 if they are equal, -1 if str precedes other, +1 if str follows other.</returns>
Int String_Compare(const String a, const String b)
{
	const Byte *aText, *bText;
	Int alen, blen;
	Int i;

	if (a == b) return 0;

	if (a == NULL || String_Length(a) <= 0) return b == NULL || String_Length(b) <= 0 ? 0 : -1;
	if (b == NULL || String_Length(b) <= 0) return +1;

	aText = String_GetBytes(a);
	bText = String_GetBytes(b);
	alen = String_Length(a);
	blen = String_Length(b);

	for (i = alen < blen ? alen : blen; i--; aText++, bText++) {
		if (*aText != *bText)
			return *aText < *bText ? -1 : +1;
	}

	return alen == blen ? 0 : alen < blen ? -1 : +1;
}

/// <summary>
/// Answer whether substrings of two strings are equal, i.e., of the same length and containing the same bytes,
/// or if they are not equal, which one lexically precedes the other.
/// </summary>
/// <param name="a">The first string to compare.</param>
/// <param name="astart">The starting offset within the first string to compare.  If outside the string, it will be clipped to the string.</param>
/// <param name="alength">The length of bytes within the first string to compare.  If outside the string, it will be clipped to the string.</param>
/// <param name="b">The second string to compare.</param>
/// <param name="bstart">The starting offset within the second string to compare.  If outside the string, it will be clipped to the string.</param>
/// <param name="blength">The length of bytes within the second string to compare.  If outside the string, it will be clipped to the string.</param>
/// <returns>0 if they are equal, -1 if str precedes other, +1 if str follows other.</returns>
Int String_CompareRange(const String a, Int astart, Int alength, const String b, Int bstart, Int blength)
{
	const Byte *aptr, *bptr, *aend, *bend;
	Byte abyte, bbyte;

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
		return String_IsNullOrEmpty(b) || bstart >= String_Length(b) || blength <= 0 ? 0 : -1;
	}
	if (String_IsNullOrEmpty(b) || bstart >= String_Length(b) || blength <= 0)
	{
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
		if ((abyte = *aptr++) != (bbyte = *bptr++))
			return abyte < bbyte ? -1 : +1;
	}

	// If we ran out of characters in either string, then sort accordingly.
	if (aptr != aend) return +1;
	if (bptr != bend) return -1;

	// The strings are equivalent.
	return 0;
}

/// <summary>
/// Extract a substring from the given string that starts at the given index and continues
/// to the end of the string.
/// </summary>
/// <param name="str">The string from which a substring will be extracted.</param>
/// <param name="start">The starting offset within the string.  If this lies outside the string, it will be clipped to the string.</param>
/// <returns>The extracted (copied) substring.</returns>
String String_SubstringAt(const String str, Int start)
{
	if (start < 0) {
		start = 0;
	}
	if (start >= String_Length(str))
		return String_Empty;

	return String_Create(String_GetBytes(str) + start, String_Length(str) - start);
}

/// <summary>
/// Extract a substring from the given string that starts at the given index and continues
/// for 'length' bytes.
/// </summary>
/// <param name="str">The string from which a substring will be extracted.</param>
/// <param name="start">The starting offset within the string.  If this lies outside the string, it will be clipped to the string.</param>
/// <param name="length">The number of bytes to copy from the string.  If this lies outside the string, it will be clipped to the string.</param>
/// <returns>The extracted (copied) substring.</returns>
String String_Substring(const String str, Int start, Int length)
{
	if (start < 0) {
		length += start;
		start = 0;
	}
	if (start >= String_Length(str) || length <= 0)
		return String_Empty;
	if (length > String_Length(str) - start) {
		length = String_Length(str) - start;
	}

	return String_Create(String_GetBytes(str) + start, length);
}

/// <summary>
/// Extract a substring from the given string that starts at the given start position and continues
/// up to and including the given end position, stepping by the given step value.
/// </summary>
/// <param name="str">The string from which a substring will be extracted.</param>
/// <param name="start">The starting offset within the string.  If this lies outside the string, it will be clipped to the string.</param>
/// <param name="end">The ending offset within the string.  If this lies outside the string, it will be clipped to the string.</param>
/// <param name="step">How far to step between characters when extracting the substring.</param>
/// <returns>The extracted (copied) substring.</returns>
String String_SubstringByRange(const String str, Int64 start, Int64 end, Int64 step)
{
	Int64 length = String_Length(str);
	Int64 substringLength, resultLength;
	Int64 leftover;
	String result;
	Byte *resultBytes, *dest;
	const Byte *strBytes, *src, *srcEnd;

	// Handle the special (but common) case of a strict forward substring, not skipping characters.
	if (step == 1 && start <= end)
		return String_Substring(str, (Int)start, (Int)(end - start + 1));

	if (start <= end) {
		// Handle scenarios where the step is nonsensical or the range is entirely outside the string.
		if (step <= 0 || start >= length)
			return String_Empty;

		// Calculate a length that's actually on a 'step', one 'step' past the end marker.
		substringLength = (end + 1 - start);
		leftover = substringLength % step;
		substringLength += step - leftover;

		// If start is negative, clip it to the lowest step greater than or equal to zero.
		if (start < 0) {
			Int64 steps = (-start / step);
			start += steps * step, substringLength -= steps * step;
			if (start < 0) start += step, substringLength -= step;
			if (start >= length || substringLength < step) return String_Empty;
		}

		// If the end of the substring is past the end of the string, clip it to the string.
		if (start + substringLength > length + (step - 1)) {
			substringLength = ((length + (step - 1) - start) / step) * step;
			if (substringLength < step) return String_Empty;
		}

		// Calculate the resulting length of the extracted substring.  This should be at least 1.
		resultLength = substringLength / step;

		// Make some space for the new substring.
		result = String_CreateInternal((Int)resultLength);

		// Get pointers to the actual buffers.
		resultBytes = (Byte *)String_GetBytes(result);
		strBytes = (const Byte *)String_GetBytes(str);
		
		// Copy bytes.
		for (srcEnd = (src = strBytes + start) + substringLength, dest = resultBytes; src < srcEnd; src += step) {
			*dest++ = *src;
		}
		*dest = '\0';

		return result;
	}
	else {
		// Handle scenarios where the step is nonsensical or the range is entirely outside the string.
		if (step >= 0 || start < 0)
			return String_Empty;

		// Calculate a length that's actually on a 'step', one 'step' past the end marker.
		substringLength = (start + 1 - end);
		leftover = substringLength % -step;
		substringLength += -step - leftover;

		// If start is past the end of the string, clip it to the highest step less than or equal
		// to the last character of the string.
		if (start >= length) {
			Int64 steps = ((start - length) / -step);
			start -= steps * -step, substringLength -= steps * -step;
			if (start >= length) start += step, substringLength += step;
			if (start < 0 || substringLength < -step) return String_Empty;
		}

		// If the end of the substring is past the start of the string, clip it to the string.
		if (start - substringLength < -(-step - 1)) {
			substringLength = ((length + (-step - 1) - (length - start - 1)) / -step) * -step;
			if (substringLength < -step) return String_Empty;
		}

		// Calculate the resulting length of the extracted substring.  This should be at least 1.
		resultLength = substringLength / -step;

		// Make some space for the new substring.
		result = String_CreateInternal((Int)resultLength);

		// Get pointers to the actual buffers.
		resultBytes = (Byte *)String_GetBytes(result);
		strBytes = (const Byte *)String_GetBytes(str);

		// Copy bytes backwards from the source.
		for (srcEnd = (src = strBytes + start) - substringLength, dest = resultBytes; src > srcEnd; src += step) {
			*dest++ = *src;
		}
		*dest = '\0';

		return result;
	}
}

/// <summary>
/// Construct a new String instance by concatenating exactly two strings together.
/// </summary>
/// <param name="s1">The first string to concatenate.</param>
/// <param name="s2">The second string to concatenate.</param>
/// <returns>The new String instance.</returns>
String String_Concat(const String s1, const String s2)
{
	String result;
	Byte *newText;
	Int length;

	if (s1 == NULL || String_Length(s1) == 0) return s2 != NULL ? (String)s2 : String_Empty;
	if (s2 == NULL || String_Length(s2) == 0) return (String)s1;

	length = String_Length(s1) + String_Length(s2);

	result = String_CreateInternal(length);
	newText = result->_opaque.text;

	MemCpy(newText, String_GetBytes(s1), String_Length(s1));
	MemCpy(newText + String_Length(s1), String_GetBytes(s2), String_Length(s2));
	newText[length] = '\0';

	return (String)result;
}

/// <summary>
/// Construct a new String instance by concatenating a byte onto the end of a string.
/// </summary>
/// <param name="str">The string to concatenate.</param>
/// <param name="ch">The byte to append to it.</param>
/// <returns>The new String instance.</returns>
String String_ConcatByte(const String str, Byte ch)
{
	String result;
	Byte *newText;
	Int length;

	if (str == NULL || String_Length(str) == 0) {
		result = String_CreateInternal(1);
		result->_opaque.text[0] = ch;
		return result;
	}

	length = String_Length(str) + 1;

	result = String_CreateInternal(length);
	newText = result->_opaque.text;

	MemCpy(newText, String_GetBytes(str), String_Length(str));
	newText[length-1] = ch;
	newText[length] = '\0';

	return result;
}

/// <summary>
/// Determine whether the text at 'start' exactly matches the given pattern of the given length.
/// </summary>
/// <param name="text">The text to test.</param>
/// <param name="pattern">The pattern to test the text against.</param>
/// <param name="start">The offset within the text to start comparing.</param>
/// <param name="length">The number of bytes to compare.</param>
/// <returns>True if the text exactly matches the bytes of the pattern, False if they do not match.</returns>
static Bool IsMatch(const Byte *text, const Byte *pattern, Int start, Int length)
{
	Int i;

	for (i = 0; i < length; i++)
	{
		if (pattern[i] != text[start + i])
			return False;
	}

	return True;
}

/// <summary>
/// Search forward through the given string looking for the given pattern.
/// </summary>
/// <param name="str">The string to search through.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <param name="start">The offset within the text to start comparing (usually zero).</param>
/// <returns>The first (leftmost) index within the string that matches the pattern, if any; if no part of the
/// string matches, this returns -1.</returns>
Int String_IndexOf(const String str, const String pattern, Int start)
{
	Int end;

	if (String_Length(pattern) > String_Length(str)) return -1;

	if (start < 0) start = 0;

	for (end = String_Length(str) - String_Length(pattern); start <= end; start++) {
		if (IsMatch(String_GetBytes(str), String_GetBytes(pattern), start, String_Length(pattern)))
			return start;
	}
	return -1;
}

/// <summary>
/// Search backward through the given string looking for the given pattern.
/// </summary>
/// <param name="str">The string to search through.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <param name="start">The start character index within the string where the search should begin (usually the length of the string).
/// If passed 'n', the first substring comparison will be against the characters at (n - pattern.length) through (n - 1), inclusive.</param>
/// <returns>The last (rightmost) index within the string that matches the pattern, if any; if no part of the
/// string matches, this returns -1.</returns>
Int String_LastIndexOf(const String str, const String pattern, Int start)
{
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
		if (IsMatch(String_GetBytes(str), String_GetBytes(pattern), start, plength))
			return start;
	}
	return -1;
}

/// <summary>
/// Test the starting bytes of the string against the given pattern.
/// </summary>
/// <param name="str">The Smile string to compare.</param>
/// <param name="pattern">The C pattern to test the string against.</param>
/// <returns>True if the start of the string exactly matches the pattern; False if the start of the string does not exactly match the pattern.</returns>
Bool String_StartsWithC(const String str, const char *pattern)
{
	Int length = StrLen(pattern);
	return length <= String_Length(str)
		&& IsMatch(String_GetBytes(str), (const Byte *)pattern, 0, length);
}

/// <summary>
/// Test the starting bytes of the string against the given pattern.
/// </summary>
/// <param name="str">The string to compare.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <returns>True if the start of the string exactly matches the pattern; False if the start of the string does not exactly match the pattern.</returns>
Bool String_StartsWith(const String str, const String pattern)
{
	return String_Length(pattern) <= String_Length(str)
		&& IsMatch(String_GetBytes(str), String_GetBytes(pattern), 0, String_Length(pattern));
}

/// <summary>
/// Test the ending bytes of the string against the given pattern.
/// </summary>
/// <param name="str">The Smile string to compare.</param>
/// <param name="pattern">The C pattern to test the string against.</param>
/// <returns>True if the end of the string exactly matches the pattern; False if the end of the string does not exactly match the pattern.</returns>
Bool String_EndsWithC(const String str, const char *pattern)
{
	Int length = StrLen(pattern);
	return length <= String_Length(str)
		&& IsMatch(String_GetBytes(str), (const Byte *)pattern, String_Length(str) - length, length);
}

/// <summary>
/// Test the ending bytes of the string against the given pattern.
/// </summary>
/// <param name="str">The string to compare.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <returns>True if the end of the string exactly matches the pattern; False if the end of the string does not exactly match the pattern.</returns>
Bool String_EndsWith(const String str, const String pattern)
{
	return String_Length(pattern) <= String_Length(str)
		&& IsMatch(String_GetBytes(str), String_GetBytes(pattern), String_Length(str) - String_Length(pattern), String_Length(pattern));
}

/// <summary>
/// Search forward through the given string looking for the given pattern character.
/// </summary>
/// <param name="str">The string to search through.</param>
/// <param name="ch">The character to test the string against.</param>
/// <param name="start">The offset within the text to start comparing (usually zero).</param>
/// <returns>The first (leftmost) index within the string that matches the character, if any; if no part of the
/// string matches, this returns -1.</returns>
Int String_IndexOfChar(const String str, Byte ch, Int start)
{
	const Byte *text;
	Int end;

	text = String_GetBytes(str);

	for (end = String_Length(str); start < end; start++)
	{
		if (text[start] == ch)
			return start;
	}

	return -1;
}

/// <summary>
/// Search backward through the given string looking for the given pattern character.
/// </summary>
/// <param name="str">The string to search through.</param>
/// <param name="ch">The character to test the string against.</param>
/// <param name="start">The offset within the text to start comparing (usually equal to (str.Length - 1)).</param>
/// <returns>The last (rightmost) index within the string that matches the character, if any; if no part of the
/// string matches, this returns -1.</returns>
Int String_LastIndexOfChar(const String str, Byte ch, Int start)
{
	const Byte *text = String_GetBytes(str);

	if (start >= String_Length(str) - 1)
		start = String_Length(str) - 1;

	for (; start >= 0; start--)
	{
		if (text[start] == ch)
			return start;
	}

	return -1;
}

/// <summary>
/// Search forward through the given string looking for the any of the given pattern characters.
/// </summary>
/// <param name="str">The string to search through.</param>
/// <param name="chars">The characters to test each character in the string against.</param>
/// <param name="numChars">The number of characters to test each character in the string against.</param>
/// <param name="start">The offset within the text to start comparing (usually zero).</param>
/// <returns>The first (leftmost) index within the string that matches any of the characters, if any; if no part of the
/// string matches any of the characters, this returns -1.</returns>
Int String_IndexOfAnyChar(const String str, const Byte *chars, Int numChars, Int start)
{
	const Byte *text;
	Byte ch;
	Int end, i;

	if (start < 0) start = 0;

	text = String_GetBytes(str);

	for (end = String_Length(str); start < end; start++) {
		ch = text[start];
		for (i = 0; i < numChars; i++) {
			if (ch == chars[i])
				return start;
		}
	}

	return -1;
}

/// <summary>
/// Replace all instances of a pattern in a string with a replacement string (scanning from left to right).
/// </summary>
/// <param name="str">The string to search through (if NULL, an empty string is returned).</param>
/// <param name="pattern">The pattern to search for (if NULL, 'str' is returned).</param>
/// <param name="replacement">Replacement text for each instance of the pattern (if NULL, will be treated as empty).</param>
/// <returns>A new string where all instances of the pattern have been replaced by the given replacement string.</returns>
String String_Replace(const String str, const String pattern, const String replacement)
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
	while ((index = String_IndexOf(str, pattern, index)) >= 0) {
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
String String_ReplaceWithLimit(const String str, const String pattern, const String replacement, Int limit)
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
	while (limit > 0 && (index = String_IndexOf(str, pattern, index)) >= 0) {
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

/// <summary>
/// Replace all instances of a character in a string with a replacement character.
/// </summary>
/// <param name="str">The string to search through (if NULL, an empty string is returned).</param>
/// <param name="pattern">The pattern to search for.</param>
/// <param name="replacement">A replacement character for each instance of the pattern.</param>
/// <returns>A new string where all instances of the pattern character have been replaced by
/// the given replacement character.  If the pattern character is not found at all, this may
/// be the same instance as the originally-provided string.</returns>
String String_ReplaceChar(const String str, Byte pattern, Byte replacement)
{
	String newString;
	const Byte *src;
	Byte *newText, *dest;
	Int length;
	Byte ch;
	Int i, j;

	if (String_IsNullOrEmpty(str)) return String_Empty;

	length = String_Length(str);
	src = String_GetBytes(str);

	// Search forward for a match for the pattern.
	for (i = 0; i < length; i++) {
		if (*src == pattern) {
			goto replace;
		}
	}

	// No match, so return the original string, avoiding an allocation.
	return str;

replace:
	// Got a match for the pattern, so allocate a new string.
	newString = String_CreateInternal(length);
	dest = newText = newString->_opaque.text;

	// Copy every character up to the first match verbatim into the new string,
	// since we know they don't match the pattern.  This avoids repeating
	// the comparisons we already performed in the loop above.
	for (j = 0; j < i; j++) {
		*dest++ = *src++;
	}

	// Copy the rest, testing each character to see if it matches the pattern.
	for ( ; j < length; j++) {
		*dest++ = ((ch = *src++) == pattern ? replacement : ch);
	}

	*dest = '\0';

	return newString;
}

/// <summary>
/// Replace at most 'limit' instances of a character in a string with a replacement character.
/// </summary>
/// <param name="str">The string to search through (if NULL, an empty string is returned).</param>
/// <param name="pattern">The pattern to search for.</param>
/// <param name="replacement">A replacement character for each instance of the pattern.</param>
/// <param name="limit">The maximum number of replacements to perform (after these, all characters of
/// the string will be left unchanged.</param>
/// <returns>A new string where at most 'limit' instances of the pattern character have been replaced by the given replacement character.</returns>
String String_ReplaceCharWithLimit(const String str, Byte pattern, Byte replacement, Int limit)
{
	String newString;
	const Byte *src;
	Byte *newText, *dest;
	Int length;
	Byte ch;
	Int i;

	if (String_IsNullOrEmpty(str)) return String_Empty;

	length = String_Length(str);

	newString = String_CreateInternal(length);

	src = String_GetBytes(str);
	dest = newText = newString->_opaque.text;

	for (i = 0; i < length; i++) {
		if ((ch = *src++) == pattern && limit > 0) {
			*dest++ = replacement;
			limit--;
		}
		else *dest++ = ch;
	}
	*dest = '\0';

	return newString;
}

/// <summary>
/// Perform sprintf()-like formatting, using the given format string and arguments.
/// </summary>
/// <remarks>
/// This does not support all sprintf() arguments, only the most common ones:
///
///   %s, %S, %d, %u, %x, %X, %c
///
/// This supports the 'h' and 'l' modifiers for %d, %u, %x, and %X, and for those
/// same format modes, it supports '0' padding and digits for justification (up to
/// a maximum width of 100).
/// </remarks>
/// <param name="format">The format string, C-style, followed by any arguments.</param>
/// <returns>The formatted string.</returns>
String String_Format(const char *format, ...)
{
	va_list v;
	StringBuilder stringBuilder;
	struct StringBuilderInt sb;

	va_start(v, format);
	stringBuilder = (StringBuilder)&sb;
	StringBuilder_Init(stringBuilder);
	StringBuilder_AppendFormatv(stringBuilder, format, v);
	va_end(v);

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Perform vsprintf()-like formatting, using the given format string and arguments.
/// </summary>
/// <remarks>
/// This does not support all sprintf() arguments, only the most common ones:
///
///   %s, %S, %d, %u, %x, %X, %c
///
/// This supports the 'h' and 'l' modifiers for %d, %u, %x, and %X, and for those
/// same format modes, it supports '0' padding and digits for justification (up to
/// a maximum width of 100).
/// </remarks>
/// <param name="format">The format string, C-style.</param>
/// <param name="v">The format arguments, as a va_list.</param>
/// <returns>The formatted string.</returns>
String String_FormatV(const char *format, va_list v)
{
	StringBuilder stringBuilder;
	struct StringBuilderInt sb;
	
	stringBuilder = (StringBuilder)&sb;
	StringBuilder_Init(stringBuilder);
	StringBuilder_AppendFormatv(stringBuilder, format, v);
	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Perform sprintf()-like formatting, using the given format string instance and arguments.
/// </summary>
/// <remarks>
/// This does not support all sprintf() arguments, only the most common ones:
///
///   %s, %S, %d, %u, %x, %X, %c
///
/// This supports the 'h' and 'l' modifiers for %d, %u, %x, and %X, and for those
/// same format modes, it supports '0' padding and digits for justification (up to
/// a maximum width of 100).
/// </remarks>
/// <param name="format">The format string instance, followed by any arguments.</param>
/// <returns>The formatted string.</returns>
String String_FormatString(const String format, ...)
{
	va_list v;
	StringBuilder stringBuilder;
	struct StringBuilderInt sb;
	
	va_start(v, format);
	stringBuilder = (StringBuilder)&sb;
	StringBuilder_Init(stringBuilder);
	StringBuilder_AppendFormatStringv(stringBuilder, format, v);
	va_end(v);

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Perform vsprintf()-like formatting, using the given format string instance and arguments.
/// </summary>
/// <remarks>
/// This does not support all sprintf() arguments, only the most common ones:
///
///   %s, %S, %d, %u, %x, %X, %c
///
/// This supports the 'h' and 'l' modifiers for %d, %u, %x, and %X, and for those
/// same format modes, it supports '0' padding and digits for justification (up to
/// a maximum width of 100).
/// </remarks>
/// <param name="format">The format string instance.</param>
/// <param name="v">The format arguments, as a va_list.</param>
/// <returns>The formatted string.</returns>
String String_FormatStringV(const String format, va_list v)
{
	StringBuilder stringBuilder;
	struct StringBuilderInt sb;

	stringBuilder = (StringBuilder)&sb;
	StringBuilder_Init(stringBuilder);
	StringBuilder_AppendFormatStringv(stringBuilder, format, v);
	return StringBuilder_ToString(stringBuilder);
}
