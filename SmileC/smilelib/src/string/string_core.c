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

#include <smile/gc.h>
#include <smile/mem.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/internal/types.h>

/// <summary>
/// The shared empty string instance, which is readonly static (non-heap) data.
/// </summary>
EXTERN_STATIC_STRING(String_Empty, "");

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
	struct StringInt *str;
	Byte *newText;

	if (length <= 0) return String_Empty;

	str = GC_MALLOC_STRUCT(struct StringInt);
	if (str == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(length);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	str->length = length;
	str->text = newText;

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
	struct StringInt *str;
	Byte *newText;

	if (length <= 0) return String_Empty;

	str = GC_MALLOC_STRUCT(struct StringInt);
	if (str == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(length);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	str->length = length;
	str->text = newText;

	newText[length] = '\0';

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
	struct StringInt *str;
	Byte *newText;

	if (repeatCount <= 0) return String_Empty;

	str = GC_MALLOC_STRUCT(struct StringInt);
	if (str == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(repeatCount);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	str->length = repeatCount;
	str->text = newText;

	MemSet(newText, b, repeatCount);
	newText[repeatCount] = '\0';

	return (String)str;
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
	struct StringInt *str;
	const struct StringInt *temp;
	Byte *newText;

	if (numStrs <= 0) return String_Empty;

	length = 0;
	for (i = 0; i < numStrs; i++)
	{
		length += ((const struct StringInt *)(strs[i]))->length;
	}

	if (length <= 0) return String_Empty;

	str = GC_MALLOC_STRUCT(struct StringInt);
	if (str == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(length);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	str->length = length;
	str->text = newText;

	dest = 0;
	for (i = 0; i < numStrs; i++)
	{
		temp = ((const struct StringInt *)(strs[i]));
		MemCpy(newText + dest, temp->text, temp->length);
		dest += temp->length;
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
	struct StringInt *str;
	Byte *newText;
	const Byte *glueText;
	Int glueLength;

	if (numStrs <= 0) return String_Empty;

	length = 0;
	for (i = 0; i < numStrs; i++)
	{
		length += ((const struct StringInt *)(strs[i]))->length;
	}
	length += (numStrs - 1) * ((const struct StringInt *)glue)->length;

	if (length <= 0) return String_Empty;

	str = GC_MALLOC_STRUCT(struct StringInt);
	if (str == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(length);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	str->length = length;
	str->text = newText;

	glueText = ((const struct StringInt *)glue)->text;
	glueLength = ((const struct StringInt *)glue)->length;

	MemCpy(newText, ((const struct StringInt *)(strs[0]))->text, ((const struct StringInt *)(strs[0]))->length);
	dest = ((const struct StringInt *)(strs[0]))->length;

	for (i = 1; i < numStrs; i++)
	{
		MemCpy(newText + dest, glueText, glueLength);
		dest += glueLength;
		MemCpy(newText + dest, ((const struct StringInt *)(strs[i]))->text, ((const struct StringInt *)(strs[i]))->length);
		dest += ((const struct StringInt *)(strs[i]))->length;
	}

	newText[length] = '\0';

	return (String)str;
}

/// <summary>
/// Answer whether two strings are equal, i.e., of the same length and containing the same bytes.
/// </summary>
/// <param name="str">The first string to compare.</param>
/// <param name="other">The second string to compare.</param>
/// <returns>True if they are equal, False if they are not of the same length or contain different bytes.</returns>
Bool String_Equals(const String str, const String other)
{
	const struct StringInt *a = (const struct StringInt *)str;
	const struct StringInt *b = (const struct StringInt *)other;
	const Byte *aText, *bText;

	if (a == b) return True;

	if (a == NULL) return b != NULL;
	if (b == NULL) return False;

	if (a->length != b->length) return False;

	aText = a->text;
	bText = b->text;

	switch (a->length)
	{
		case 0: return True;
		case 1: return aText[0] == bText[0];
		case 2: return aText[0] == bText[0] && aText[1] == bText[1];
		case 3: return aText[0] == bText[0] && aText[1] == bText[1] && aText[2] == bText[2];
		case 4: return aText[0] == bText[0] && aText[1] == bText[1] && aText[2] == bText[2] && aText[3] == bText[3];

		default:
			return MemCmp(aText, bText, a->length) == 0;
	}
}

/// <summary>
/// Answer whether two strings are equal, i.e., of the same length and containing the same bytes,
/// or if they are not equal, which one lexically precedes the other.
/// </summary>
/// <param name="str">The first string to compare.</param>
/// <param name="other">The second string to compare.</param>
/// <returns>0 if they are equal, -1 if str precedes other, +1 if str follows other.</returns>
Int String_Compare(const String str, const String other)
{
	const struct StringInt *a = (const struct StringInt *)str;
	const struct StringInt *b = (const struct StringInt *)other;
	const Byte *aText, *bText;
	Int alen, blen;
	Int i;

	if (a == b) return 0;

	if (a == NULL || a->length <= 0) return b == NULL || b->length <= 0 ? 0 : -1;
	if (b == NULL || b->length <= 0) return +1;

	aText = a->text;
	bText = b->text;
	alen = a->length;
	blen = b->length;

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
	const struct StringInt *astr = (const struct StringInt *)a;
	const struct StringInt *bstr = (const struct StringInt *)b;
	const Byte *aptr, *bptr, *aend, *bend;
	Byte abyte, bbyte;

	if (astart < 0)
	{
		alength += astart;
		alength = 0;
	}
	if (bstart < 0)
	{
		blength += bstart;
		blength = 0;
	}

	if (String_IsNullOrEmpty(a) || astart >= astr->length || alength <= 0)
	{
		return String_IsNullOrEmpty(b) || bstart >= bstr->length || blength <= 0 ? 0 : -1;
	}
	if (String_IsNullOrEmpty(b) || bstart >= bstr->length || blength <= 0)
	{
		return -1;
	}

	if (alength > astr->length - astart)
	{
		alength = astr->length - astart;
	}
	if (blength > bstr->length - bstart)
	{
		blength = bstr->length - bstart;
	}

	aptr = astr->text + astart;
	bptr = bstr->text + bstart;
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
	struct StringInt *s = (struct StringInt *)str;

	if (start < 0) {
		start = 0;
	}
	if (start >= s->length)
		return String_Empty;

	return String_Create(s->text + start, s->length - start);
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
	struct StringInt *s = (struct StringInt *)str;

	if (start < 0) {
		length += start;
		start = 0;
	}
	if (start >= s->length || length <= 0)
		return String_Empty;
	if (length > s->length - start) {
		length = s->length - start;
	}

	return String_Create(s->text + start, length);
}

/// <summary>
/// Construct a new String instance by concatenating exactly two strings together.
/// </summary>
/// <param name="str">The first string to concatenate.</param>
/// <param name="other">The second string to concatenate.</param>
/// <returns>The new String instance.</returns>
String String_Concat(const String str, const String other)
{
	struct StringInt *result, *s1, *s2;
	Byte *newText;
	Int length;

	s1 = (struct StringInt *)str;
	s2 = (struct StringInt *)other;

	if (s1 == NULL || s1->length == 0) return s2 != NULL ? (String)s2 : String_Empty;
	if (s2 == NULL || s2->length == 0) return (String)s1;

	length = s1->length + s2->length;

	result = GC_MALLOC_STRUCT(struct StringInt);
	if (result == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(length);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	result->length = length;
	result->text = newText;

	MemCpy(newText, s1->text, s1->length);
	MemCpy(newText + s1->length, s2->text, s2->length);
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
	struct StringInt *result, *s1;
	Byte *newText;
	Int length;

	s1 = (struct StringInt *)str;

	if (s1 == NULL || s1->length == 0)
		return String_CreateRepeat(ch, 1);

	length = s1->length + 1;

	result = GC_MALLOC_STRUCT(struct StringInt);
	if (result == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(length);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	result->length = length;
	result->text = newText;

	MemCpy(newText, s1->text, s1->length);
	newText[length-1] = ch;
	newText[length] = '\0';

	return (String)result;
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
	const struct StringInt *s, *p;
	Int end;

	s = (const struct StringInt *)str;
	p = (const struct StringInt *)pattern;

	if (p->length > s->length) return -1;

	if (start < 0) start = 0;

	for (end = s->length - p->length; start <= end; start++) {
		if (IsMatch(s->text, p->text, start, p->length))
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
	const struct StringInt *s, *p;
	Int slength, plength;

	s = (const struct StringInt *)str;
	p = (const struct StringInt *)pattern;
	slength = s->length;
	plength = p->length;

	if (plength > slength || start < plength) return -1;

	start -= plength;
	if (start >= slength - plength)
	{
		start = slength - plength;
	}
	for (; start >= 0; start--)
	{
		if (IsMatch(s->text, p->text, start, plength))
			return start;
	}
	return -1;
}

/// <summary>
/// Test the starting bytes of the string against the given pattern.
/// </summary>
/// <param name="str">The string to compare.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <returns>True if the start of the string exactly matches the pattern; False if the start of the string does not exactly match the pattern.</returns>
Bool String_StartsWith(const String str, const String pattern)
{
	const struct StringInt *s, *p;

	s = (const struct StringInt *)str;
	p = (const struct StringInt *)pattern;

	return p->length <= s->length && IsMatch(s->text, p->text, 0, p->length);
}

/// <summary>
/// Test the ending bytes of the string against the given pattern.
/// </summary>
/// <param name="str">The string to compare.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <returns>True if the end of the string exactly matches the pattern; False if the end of the string does not exactly match the pattern.</returns>
Bool String_EndsWith(const String str, const String pattern)
{
	const struct StringInt *s, *p;

	s = (const struct StringInt *)str;
	p = (const struct StringInt *)pattern;

	return p->length <= s->length && IsMatch(s->text, p->text, s->length - p->length, p->length);
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
	const struct StringInt *s;
	Byte *text;
	Int end;

	s = (const struct StringInt *)str;
	text = s->text;

	for (end = s->length; start < end; start++)
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
	const struct StringInt *s;
	Byte *text;

	s = (const struct StringInt *)str;
	text = s->text;

	if (start >= s->length - 1)
		start = s->length - 1;

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
	const struct StringInt *s;
	Byte *text;
	Byte ch;
	Int end, i;

	s = (const struct StringInt *)str;
	text = s->text;

	for (end = s->length; start < end; start++)
	{
		ch = text[start];
		for (i = 0; i < numChars; i++)
		{
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
	const struct StringInt *s, *p, *r;
	StringBuilder stringBuilder;
	struct StringBuilderInt sb;
	Byte *text, *patText, *repText;
	Int lastEnd, index;

	if (String_IsNullOrEmpty(str)) return String_Empty;
	if (String_IsNullOrEmpty(pattern)) return str;

	s = (const struct StringInt *)str;
	text = s->text;
	p = (const struct StringInt *)pattern;
	patText = p->text;
	r = replacement != NULL ? (const struct StringInt *)replacement : (const struct StringInt *)String_Empty;
	repText = r->text;

	stringBuilder = (StringBuilder)&sb;
	StringBuilder_InitWithSize(stringBuilder, s->length);

	lastEnd = 0;
	index = 0;
	while ((index = String_IndexOf(str, pattern, index)) >= 0)
	{
		if (index > lastEnd)
		{
			StringBuilder_Append(stringBuilder, text, lastEnd, index - lastEnd);
		}
		StringBuilder_Append(stringBuilder, repText, 0, r->length);
		lastEnd = (index += p->length);
	}

	if (lastEnd < s->length)
	{
		StringBuilder_Append(stringBuilder, text, lastEnd, s->length - lastEnd);
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Replace all instances of a character in a string with a replacement character.
/// </summary>
/// <param name="str">The string to search through (if NULL, an empty string is returned).</param>
/// <param name="pattern">The pattern to search for.</param>
/// <param name="replacement">A replacement character for each instance of the pattern.</param>
/// <returns>A new string where all instances of the pattern character have been replaced by the given replacement character.</returns>
String String_ReplaceChar(const String str, Byte pattern, Byte replacement)
{
	const struct StringInt *s;
	struct StringInt *newString;
	Byte *text, *newText, *dest;
	const Byte *src;
	Int length;
	Byte ch;
	Int i;

	if (String_IsNullOrEmpty(str)) return String_Empty;

	s = (const struct StringInt *)str;
	text = s->text;
	length = s->length;

	newString = GC_MALLOC_STRUCT(struct StringInt);
	if (newString == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(length);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	newString->length = length;
	newString->text = newText;

	src = text;
	dest = newText;

	for (i = 0; i < length; i++)
	{
		*dest++ = ((ch = *src++) == pattern ? replacement : ch);
	}
	*dest = '\0';

	return (String)newString;
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
