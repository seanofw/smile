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
#include <smile/string.h>
#include <smile/stringbuilder.h>

STATIC_STRING(Space, " ");

STATIC_STRING(CommaSpace, ", ");

/// <summary>
/// Split a string by a given pattern string.
/// </summary>
/// <param name="str">The string to split.</param>
/// <param name="pattern">The substring on which to split that string.  If this is the empty string, no splitting will be performed.</param>
/// <param name="limit">The maximum number of split pieces to emit as output.</param>
/// <param name="options">Optional flags to control the split; if 0, normal splitting will occur;
/// if StringSplitOptions_RemoveEmptyEntries is set, any empty strings in the output will be discarded.</param>
/// <param name="pieces">If non-NULL, this will be set to an array containing the resulting split strings.</param>
/// <returns>The number of strings resulting from the split (i.e., the number of strings returned in the 'pieces' array).</returns>
Int String_SplitWithOptions(const String str, const String pattern, Int limit, Int options, String **pieces)
{
	struct ArrayInt a;
	Array array = &a;
	Int startIndex, splitIndex, len;

	if (limit == 0) limit = IntMax;

	Array_Init(array, sizeof(String), 64, False);

	startIndex = 0;

	if (!String_IsNullOrEmpty(pattern)) {
		while (limit > 0 && (splitIndex = String_IndexOf(str, pattern, startIndex)) >= 0) {
			if (splitIndex == startIndex) {
				if (!(options & StringSplitOptions_RemoveEmptyEntries)) {
					*((String *)Array_Push(array)) = String_Empty;
					limit--;
				}
			}
			else {
				*((String *)Array_Push(array)) = String_Substring(str, startIndex, splitIndex - startIndex);
				limit--;
			}
			startIndex = splitIndex + String_Length(pattern);
		}
	}
	else {
		len = String_Length(str);
		while (limit > 0 && startIndex < len) {
			*((String *)Array_Push(array)) = String_Substring(str, startIndex, 1);
			limit--;
			startIndex++;
		}
	}

	if (limit > 0) {
		if (!String_IsNullOrEmpty(str)) {
			if (startIndex < String_Length(str)) {
				*((String *)Array_Push(array)) = startIndex > 0 ? String_Substring(str, startIndex, String_Length(str) - startIndex) : str;
			}
		}
		else {
			*((String *)Array_Push(array)) = String_Empty;
		}
	}

	if (pieces != NULL)
		*pieces = (String *)a.data;

	return a.length;
}

/// <summary>
/// Reverse the raw bytes of the given string, ignoring whether they may be compound UTF-8 values forming a single Unicode code point.
/// </summary>
/// <param name="str">The string to reverse.</param>
/// <returns>The reversed string.</returns>
String String_RawReverse(const String str)
{
	String result;
	Byte *dest;
	const Byte *src;
	Int i, length;

	if (String_IsNullOrEmpty(str)) return str;

	length = String_Length(str);
	result = String_CreateInternal(length);
	dest = (Byte *)String_GetBytes(result) + length;
	src = (Byte *)String_GetBytes(str);

	for (i = 0; i < length; i++) {
		*--dest = *src++;
	}

	return result;
}

/// <summary>
/// Reverse the characters in the given string, properly keeping multi-byte UTF-8 values as the same byte sequence
/// so that the resulting string represents the same Unicode code points.<br />
/// <br />
/// This function is exactly equivalent to decoding all of the UTF-8-encoded bytes in the string into a sequence of
/// Unicode code points, reversing those code points, and then converting the result back to UTF-8.<br />
/// <br />
/// Note that this does *NOT* attempt to maintain the order of complex structures such as combining diacritics; those
/// will be reversed as well, which means they will likely end up on the wrong character as a result.  This function
/// merely ensures that the UTF-8-encoded code points are undamaged after reversing them, which String_RawReverse()
/// does not do.
/// </summary>
/// <param name="str">The string to reverse.</param>
/// <returns>The reversed string.</returns>
String String_Reverse(const String str)
{
	String result;
	Byte *dest;
	const Byte *src;
	Byte ch;
	Int i, length;

	if (String_IsNullOrEmpty(str)) return str;

	length = String_Length(str);
	result = String_CreateInternal(length);
	dest = (Byte *)String_GetBytes(result);
	src = (Byte *)String_GetBytes(str);

	for (i = 0; i < length; ) {
		ch = src[i++];
		if (ch < 0x80) {
			dest[length - i - 0] = ch;
		}
		else if (ch < 0xE0) {
			dest[length - i - 1] = ch;
			dest[length - i - 0] = (i + 0 < length) ? src[i + 0] : 0;
			i++;
		}
		else if (ch < 0xF0) {
			dest[length - i - 2] = ch;
			dest[length - i - 1] = (i + 0 < length) ? src[i + 0] : 0;
			dest[length - i - 0] = (i + 1 < length) ? src[i + 1] : 0;
			i += 2;
		}
		else if (ch < 0xF8) {
			dest[length - i - 3] = ch;
			dest[length - i - 2] = (i + 0 < length) ? src[i + 0] : 0;
			dest[length - i - 1] = (i + 1 < length) ? src[i + 1] : 0;
			dest[length - i - 0] = (i + 2 < length) ? src[i + 2] : 0;
			i += 3;
		}
		else if (ch < 0xFC) {
			dest[length - i - 4] = ch;
			dest[length - i - 3] = (i + 0 < length) ? src[i + 0] : 0;
			dest[length - i - 2] = (i + 1 < length) ? src[i + 1] : 0;
			dest[length - i - 1] = (i + 2 < length) ? src[i + 2] : 0;
			dest[length - i - 0] = (i + 3 < length) ? src[i + 3] : 0;
			i += 4;
		}
		else {
			dest[length - i - 5] = ch;
			dest[length - i - 4] = (i + 0 < length) ? src[i + 0] : 0;
			dest[length - i - 3] = (i + 1 < length) ? src[i + 1] : 0;
			dest[length - i - 2] = (i + 2 < length) ? src[i + 2] : 0;
			dest[length - i - 1] = (i + 3 < length) ? src[i + 3] : 0;
			dest[length - i - 0] = (i + 4 < length) ? src[i + 4] : 0;
			i += 5;
		}
	}

	return result;
}

/// <summary>
/// Create a new string that is the result of repeating the given string 'count' times.
/// </summary>
/// <param name="str">The string to repeat.</param>
/// <param name="count">The number of times to repeat that string.  Zero or negative
/// values will result in the empty string.  One will result in the same exact string.
/// All other values will repeat the string that many times, as expected.</param>
/// <returns>The string, repeated 'count' times.</returns>
String String_Repeat(const String str, Int count)
{
	Int i;

	switch (count) {
		case 0:
			return String_Empty;
		case 1:
			return str;
		case 2:
			return String_Concat(str, str);
		case 3:
			{
				String strs[3];
				strs[0] = str;
				strs[1] = str;
				strs[2] = str;
				return String_ConcatMany(strs, 3);
			}
		case 4:
			{
				String strs[4];
				strs[0] = str;
				strs[1] = str;
				strs[2] = str;
				strs[3] = str;
				return String_ConcatMany(strs, 4);
			}
		default:
			{
				StringBuilder stringBuilder = StringBuilder_Create();
				for (i = 0; i < count; i++)
				{
					StringBuilder_AppendString(stringBuilder, str);
				}
				return StringBuilder_ToString(stringBuilder);
			}
	}
}

/// <summary>
/// Pad the given string on its start end, if necessary, so that it is at least 'minLength' bytes long.
/// </summary>
/// <param name="str">The string to pad.</param>
/// <param name="minLength">The minimum length of the resulting string.  If the given string is shorter
/// than this, 'padChar' will be injected at the start of the resulting string to make it at least this long.
/// If the given string is longer than or equal to this, it will be left unchanged.</param>
/// <param name="padChar">The character to use for padding the string, if necessary.</param>
/// <returns>The padded string, or, if it was already long enough, the original string.</returns>
String String_PadStart(const String str, Int minLength, Byte padChar)
{
	Int length = String_Length(str);
	String result;
	Byte *dest;
	Int padLength;

	if (length >= minLength) return str;
	padLength = minLength - length;

	result = String_CreateInternal(minLength);
	dest = (Byte *)String_GetBytes(result);

	MemSet(dest, padChar, padLength);
	MemCpy(dest + padLength, String_GetBytes(str), length);

	return result;
}

/// <summary>
/// Pad the given string on its end, if necessary, so that it is at least 'minLength' bytes long.
/// </summary>
/// <param name="str">The string to pad.</param>
/// <param name="minLength">The minimum length of the resulting string.  If the given string is shorter
/// than this, 'padChar' will be injected at the end of the resulting string to make it at least this long.
/// If the given string is longer than or equal to this, it will be left unchanged.</param>
/// <param name="padChar">The character to use for padding the string, if necessary.</param>
/// <returns>The padded string, or, if it was already long enough, the original string.</returns>
String String_PadEnd(const String str, Int minLength, Byte padChar)
{
	Int length = String_Length(str);
	String result;
	Byte *dest;
	Int padLength;

	if (length >= minLength) return str;
	padLength = minLength - length;

	result = String_CreateInternal(minLength);
	dest = (Byte *)String_GetBytes(result);

	MemCpy(dest, String_GetBytes(str), length);
	MemSet(dest + length, padChar, padLength);

	return result;
}

/// <summary>
/// Slash-append the given strings.  This function is useful for constructing sane paths to files.<br />
/// <br />
/// The given strings will be concatenated together with forward slashes in between; however, any preexisting
/// forward slashes or backslashes on the start or end of the provided strings will be removed first, with
/// the exception of any initial slashes on the first string and any trailing slashes on the last string.<br />
/// <br />
/// <strong>Examples:</strong><br />
/// "foo" + "\\bar/" + "/baz"    --&gt;   "foo/bar/baz"<br />
/// "//foo/" + "//bar" + "baz"   --&gt;   "//foo/bar/baz"
/// </summary>
/// <param name="strs">An array of the strings to concatenate.</param>
/// <param name="numStrs">The number of strings in the array.</param>
/// <returns>The slash-concatenated string.</returns>
String String_SlashAppend(const String *strs, Int numStrs)
{
	String str;
	StringBuilder stringBuilder;
	Bool isFirst;
	Int i, skipChars, length, newLength;
	Byte ch;
	const Byte *src;
	const Byte *sbText;

	if (numStrs == 0) return String_Empty;
	else if (numStrs == 1) return strs[0];

	stringBuilder = StringBuilder_Create();
	isFirst = True;

	for (i = 0; i < numStrs; i++)
	{
		str = strs[i];

		// First string gets appended verbatim.  All the rest get cleanup as we progress
		// through the content.
		if (isFirst) {
			StringBuilder_AppendString(stringBuilder, str);
			isFirst = False;
		}
		else {
			length = String_Length(str);
			src = String_GetBytes(str);

			// Strip off initial slashes from the string we're about to append.
			skipChars = 0;
			while (skipChars < length && ((ch = src[skipChars]) == '/' || ch == '\\'))
			{
				skipChars++;
			}
			if (skipChars == length) continue;

			// Strip off trailing slashes on the StringBuilder, if it has any there.
			newLength = StringBuilder_GetLength(stringBuilder);
			sbText = StringBuilder_GetBytes(stringBuilder);
			while (newLength > 0 && ((ch = sbText[newLength - 1]) == '/' || ch == '\\'))
			{
				newLength--;
			}
			if (newLength != StringBuilder_GetLength(stringBuilder))
			{
				StringBuilder_SetLength(stringBuilder, newLength);
			}

			// Ensure we have at least one slash separating the new component from the previous content.
			StringBuilder_AppendByte(stringBuilder, '/');
			StringBuilder_Append(stringBuilder, src, skipChars, length - skipChars);
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Trim whitespace from the ends of the string.  Whitespace is defined for this function
/// as byte values in the range of 0 to 32, inclusive.
/// </summary>
/// <param name="str">The string to trim.</param>
/// <param name="trimStart">Whether to trim whitespace from the start of the string.</param>
/// <param name="trimEnd">Whether to trim whitespace from the end of the string.</param>
/// <returns>The trimmed string.</returns>
String String_TrimWhitespace(const String str, Bool trimStart, Bool trimEnd)
{
	const Byte *src = String_GetBytes(str);
	Int start, end, length = String_Length(str);
	Byte ch;

	start = 0;
	end = length;

	if (trimStart)
	{
		while (start < length && ((ch = src[start]) >= '\x00' && ch <= '\x20'))
		{
			start++;
		}
	}

	if (trimEnd)
	{
		while (end > start && ((ch = src[end - 1]) >= '\x00' && ch <= '\x20'))
		{
			end--;
		}
	}

	if (start == 0 && end == length)
		return str;

	return String_Substring(str, start, end - start);
}

/// <summary>
/// Compact whitespace within the given string:  Any sequences of one or more whitespace
/// characters will be replaced with a single space (' ') character.  Whitespace is defined
/// for this function as byte values in the range of 0 to 32, inclusive.
/// </summary>
/// <param name="str">The string to whitespace-compact.</param>
/// <returns>The whitespace-compacted string.</returns>
String String_CompactWhitespace(const String str)
{
	Int i, length;
	Byte ch, lastch;
	Byte *result, *dest;
	const Byte *src;

	length = String_Length(str);
	result = GC_MALLOC_TEXT(length);
	if (result == NULL) Smile_Abort_OutOfMemory();
	src = String_GetBytes(str);

	dest = result;
	lastch = '\x00';
	for (i = 0; i < length; i++) {
		ch = src[i];
		if (ch >= '\x00' && ch <= '\x20') {
			if (dest > result && !(lastch >= '\x00' && lastch <= '\x20')) {
				*dest++ = ' ';
			}
		}
		else {
			*dest++ = ch;
		}
		lastch = ch;
	}

	if (dest > result && lastch >= '\x00' && lastch <= '\x20') {
		dest--;
	}

	return String_Create(result, dest - result);
}

/// <summary>
/// Replace unsafe characters within the string with their C-style backslash escape sequences.<br />
/// <br />
/// Characters in the range of [7, 13] will be replaced with their C-style named escapes, like
/// '\t' and '\n'; the special characters apostrophe, quote, and backslash will all be preceded
/// with a backslash; and all other byte values less than 32 or greater than 126 will be replaced
/// with a hexadecimal escape sequence, like '\x1E'.  Note that the common (but not C-compatible)
/// escape code '\e' is <em>not</em> generated by this function; the output of this is strictly
/// compatible with C's encoding.
/// </summary>
/// <param name="str">The string to add C-style slashes to.</param>
/// <returns>The safely-escaped string.</returns>
String String_AddCSlashes(const String str)
{
	Int i, length;
	const Byte *src;
	Byte ch;
	StringBuilder stringBuilder;

	if (String_IsNullOrEmpty(str))
		return String_Empty;

	length = String_Length(str);
	src = String_GetBytes(str);

	stringBuilder = StringBuilder_CreateWithSize(length);

	for (i = 0; i < length; i++)
	{
		ch = src[i];
		if (ch < 32)
		{
			switch (ch)
			{
				case 7: StringBuilder_Append(stringBuilder, "\\a", 0, 2); break;
				case 8: StringBuilder_Append(stringBuilder, "\\b", 0, 2); break;
				case 9: StringBuilder_Append(stringBuilder, "\\t", 0, 2); break;
				case 10: StringBuilder_Append(stringBuilder, "\\n", 0, 2); break;
				case 11: StringBuilder_Append(stringBuilder, "\\v", 0, 2); break;
				case 12: StringBuilder_Append(stringBuilder, "\\f", 0, 2); break;
				case 13: StringBuilder_Append(stringBuilder, "\\r", 0, 2); break;
				default:
					StringBuilder_AppendFormat(stringBuilder, "\\x%02X", (UInt)ch);
					break;
			}
		}
		else if (ch >= 127)
		{
			StringBuilder_AppendFormat(stringBuilder, "\\x%02X", (UInt)ch);
		}
		else if (ch == '\'' || ch == '\"' || ch == '\\')
		{
			StringBuilder_AppendByte(stringBuilder, '\\');
			StringBuilder_AppendByte(stringBuilder, ch);
		}
		else
		{
			StringBuilder_AppendByte(stringBuilder, ch);
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Replace backslash-escaped characters within a string with their equivalent C-style
/// backslash-escape-sequence values.<br />
/// <br />
/// This recognizes all of the standard C-style named escape sequences, like '\t' and '\n',
/// as well as C-style numeric escape sequences, like '\13' and '\0' (which will be treated
/// as decimal values), and C-style hexadecimal escape sequences, like '\x1E'.  This also
/// recognizes the common (but nonstandard) '\e' escape to mean the value 27.
/// </summary>
/// <param name="str">The string to decode C-style slashes from.</param>
/// <returns>The unescaped string.</returns>
String String_StripCSlashes(const String str)
{
	Int i, length;
	const Byte *src;
	Byte *result, *dest;
	Byte ch;
	UInt value;

	if (String_IsNullOrEmpty(str))
		return String_Empty;

	length = String_Length(str);
	src = String_GetBytes(str);

	// Safe to allocate 'length' bytes for the result, since String_StripCSlashes() can only ever make the string shorter.
	result = GC_MALLOC_TEXT(length);
	if (result == NULL) Smile_Abort_OutOfMemory();
	dest = result;

	for (i = 0; i < length; )
	{
		if ((ch = src[i++]) != '\\')
		{
			*dest++ = ch;
			continue;
		}
		if (i >= length)
			continue;

		switch (ch = src[i])
		{
			case 'a': *dest++ = 7; i++; break;
			case 'b': *dest++ = 8; i++; break;
			case 't': *dest++ = 9; i++; break;
			case 'n': *dest++ = 10; i++; break;
			case 'v': *dest++ = 11; i++; break;
			case 'f': *dest++ = 12; i++; break;
			case 'r': *dest++ = 13; i++; break;
			case 'e': *dest++ = 27; i++; break;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				{
					value = ch - '0';
					i++;

					if (i < length && (ch = src[i]) >= '0' && ch <= '9')
					{
						value *= 10;
						value += ch - '0';
						i++;

						if (i < length && (ch = src[i]) >= '0' && ch <= '9')
						{
							value *= 10;
							value += ch - '0';
							i++;
						}
					}

					*dest++ = (Byte)(value & 0xFF);
					break;
				}

			case 'x':
				{
					i++;
					value = 0;
					while (i < length)
					{
						if ((ch = src[i]) >= '0' && ch <= '9')
						{
							i++;
							value = (value << 4) | (ch - '0');
						}
						else if (ch >= 'a' && ch <= 'f')
						{
							i++;
							value = (value << 4) | (ch - 'a' + 10);
						}
						else if (ch >= 'A' && ch <= 'F')
						{
							i++;
							value = (value << 4) | (ch - 'A' + 10);
						}
						else break;
					}
					*dest++ = (Byte)(value & 0xFF);
					break;
				}

			default:
				*dest++ = ch;
				i++;
				break;
		}
	}

	return String_Create(result, dest - result);
}

/// <summary>
/// OMG U gais like the be5t encryption evar!!!1!  This l337 funkshn replaces
/// letters in teh string with othr leters so nobody can reed it!!!1!!
/// </summary>
/// <param name="str">The string to "encrypt" or "decrypt."</param>
/// <returns>The "encrypted" or "decrypted" string.</param>
/// <remarks>Letters in [a-zA-Z] will be replaced with their equivalent, identically-
/// cased letter 13 positions away from them in the alphabet.  All other characters
/// will be left unchanged.</remarks>
String String_Rot13(const String str)
{
	String result;
	Byte *dest;
	const Byte *src;
	Byte ch;
	Int i, length;

	if (String_IsNullOrEmpty(str))
		return String_Empty;

	src = String_GetBytes(str);
	length = String_Length(str);

	result = String_CreateInternal(length);
	dest = (Byte *)String_GetBytes(result);

	for (i = 0; i < length; i++)
	{
		ch = src[i];

		if (ch >= 'a' && ch <= 'm')			*dest++ = ch + 13;
		else if (ch >= 'n' && ch <= 'z')	*dest++ = ch - 13;
		else if (ch >= 'A' && ch <= 'M')	*dest++ = ch + 13;
		else if (ch >= 'N' && ch <= 'Z')	*dest++ = ch - 13;
		else								*dest++ = ch;
	}

	return result;
}

/// <summary>
/// Escape punctuation characters in the given string, specific to the needs of regular expressions,
/// with backslash characters.  This encodes whitespace characters in the range of [0, 31] the same
/// way that AddCSlashes() does, and also adds a backslash before any of the following symbols:<br />
/// <br />
///      \  *  +  ?  |  {  }  [  ]  (  )  ^  $  .  #<br />
/// <br />
/// All other characters, including those above 127, are left unchanged.
/// </summary>
/// <param name="str">The string to encode for use in a regular expression.</param>
/// <return>The string, with all regex-unsafe characters escaped.</return>
String String_RegexEscape(const String str)
{
	Int i, length;
	const Byte *src;
	Byte ch;
	StringBuilder stringBuilder;

	if (String_IsNullOrEmpty(str))
		return String_Empty;

	length = String_Length(str);
	src = String_GetBytes(str);

	stringBuilder = StringBuilder_CreateWithSize(length);

	for (i = 0; i < length; i++)
	{
		switch (ch = src[i])
		{
			case 7: StringBuilder_Append(stringBuilder, "\\a", 0, 2); break;
			case 8: StringBuilder_Append(stringBuilder, "\\b", 0, 2); break;
			case 9: StringBuilder_Append(stringBuilder, "\\t", 0, 2); break;
			case 10: StringBuilder_Append(stringBuilder, "\\n", 0, 2); break;
			case 11: StringBuilder_Append(stringBuilder, "\\v", 0, 2); break;
			case 12: StringBuilder_Append(stringBuilder, "\\f", 0, 2); break;
			case 13: StringBuilder_Append(stringBuilder, "\\r", 0, 2); break;

			case 0: case 1: case 2: case 3: case 4: case 5: case 6:
			case 14: case 15: case 16: case 17: case 18: case 19: case 20:
			case 21: case 22: case 23: case 24: case 25: case 26: case 27:
			case 28: case 29: case 30: case 31:
				StringBuilder_AppendFormat(stringBuilder, "\\x%02X", (UInt)ch);
				break;

			case '\\': case '*': case '+': case '?': case '|':
			case '{': case '}': case '[': case ']': case '(': case ')':
			case '^': case '$': case '.': case '#':
				StringBuilder_AppendByte(stringBuilder, '\\');
				StringBuilder_AppendByte(stringBuilder, ch);
				break;
			default:
				StringBuilder_AppendByte(stringBuilder, ch);
				break;
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

static Bool String_WildcardMatchInternal(const Byte *pattern, const Byte *patternEnd, const Byte *text, const Byte *textEnd, Int options)
{
	Byte patternChar, textChar, nextPatternChar;

	while (pattern < patternEnd) {
		switch (patternChar = *pattern++) {
			case '?':
				// If we ran out of characters, or hit a '/' in filename mode, this is a fail.
				if (text == textEnd)
					return False;
				textChar = *text++;
				if ((options & StringWildcardOptions_FilenameMode) && textChar == '/')
					return False;
				break;

			case '*':
				// Consume trailing '*' and '?' characters, since they don't mean much (except '?',
				// which adds mandatory filler space).
				while (pattern < patternEnd && ((patternChar = *pattern) == '?' || patternChar == '*')) {
					if (((options & StringWildcardOptions_FilenameMode) && text < textEnd && *text == '/')
						|| (patternChar == '?' && text == textEnd))
						return False;
					pattern++;
					text++;
				}

				// If we ran out of characters in the pattern, then this is a successful match,
				// since this star can consume everything after it in the text.
				if (pattern == patternEnd)
					return True;

				// Determine the next character in the text that we're searching for.
				nextPatternChar = ((options & StringWildcardOptions_BackslashEscapes) && patternChar == '\\' && pattern + 1 < patternEnd ? *++pattern : patternChar);

				// Skim forward in the text looking for that next character, and then recursively
				// perform a pattern-match on the remainders of the pattern and text from there.
				// We use that next character to optimize the recursion, so that we don't recurse
				// if we know there won't be a match.
				while (text < textEnd) {
					textChar = *text;
					if ((options & StringWildcardOptions_FilenameMode) && textChar == '/') {
						// '/' can't match a '*' or '?' in filename mode.
						if (nextPatternChar != '/') return False;
						else return String_WildcardMatchInternal(pattern, patternEnd, text, textEnd, options);
					}
					if (textChar == nextPatternChar && String_WildcardMatchInternal(pattern, patternEnd, text, textEnd, options))
						return True;
					text++;
				}

				// None of the recursive searches matched, so this is a fail.
				return False;

			case '\\':
				if ((options & StringWildcardOptions_BackslashEscapes)) {
					// Consume the next character as the real pattern character.
					if (pattern >= patternEnd)
						return False;	// Bad pattern.
					patternChar = *pattern++;
				}
				// Fallthrough...

			default:
				if (text == textEnd)
					return False;	// Ran out of characters.
				if (patternChar != *text++)
					return False;	// No match.
				break;
		}
	}

	return text == textEnd;
}

/// <summary>
/// Perform simple wildcard pattern-matching against a string.  This recognizes the common
/// wildcard symbols '*' as matching zero or more characters and '?' as matching exactly one character.
/// </summary>
/// <param name="pattern">The pattern to match, which may contain '*' and '?' wildcards.</param>
/// <param name="text">The text to try to match against.</param>
/// <param name="options">FilenameMode: If set, this treats '/' in the text specially, and will not allow a
/// '/' to match either '*' or '?'.  If clear, '/' will be treated like any other character.<br />
/// <br />
/// BackslashEscapes:  If set, this causes a backslash ('\') in the pattern to escape the
/// character following it, so that a pattern may contain a literal '*' or '?' character.  If clear,
/// a backslash will be treated as just another pattern character.<br />
/// <br />
/// CaseInsensitive:  If set, this causes the comparison to be performed case-insensitive, using
/// Unicode case-folding rules (note: you may want to apply normalization first before using this flag
/// on non-English inputs).  If clear, each byte of the input will be compared verbatim.</param>
/// <returns>True if the text exactly and completely matches the pattern, False if the text did not
/// match the pattern or if only part of the text or part of the pattern matched.</returns>
Bool String_WildcardMatch(const String pattern, const String text, Int options)
{
	String realPattern, realText;
	const Byte *patternText, *textText;

	// TODO: Handle case-insensitivity more efficiently (difficult for full Unicode support).
	if (options & StringWildcardOptions_CaseInsensitive) {
		realPattern = String_CaseFold(pattern);
		realText = String_CaseFold(text);
	}
	else {
		realPattern = pattern;
		realText = text;
	}

	patternText = String_GetBytes(realPattern);
	textText = String_GetBytes(realText);

	return String_WildcardMatchInternal(
		patternText, patternText + String_Length(realPattern),
		textText, textText + String_Length(realText),
		options
	);
}

/// <summary>
/// Join a sequence of strings together, separating them with the Oxford comma, and prefixing the
/// last item using the given conjunction string.  So this takes an array of strings like
/// { "Alice", "Bill", "Charlie", "Dave" } and returns a string like "Alice, Bill, Charlie, and Dave".
/// </summary>
/// <param name="items">The array of items to join.</param>
/// <param name="numItems">The number of items in the array.</param>
/// <param name="conjunction">A conjunction word to include before the last item.  This will
/// have a space automatically precede it in the final string, if it is used.</param>
/// <returns>The strings joined as a simple English list.</returns>
String String_JoinEnglishNames(const String *items, Int numItems, const String conjunction)
{
	StringBuilder stringBuilder;
	Int i;

	switch (numItems) {
		case 0:
			return String_Empty;
		case 1:
			return items[0];
		case 2:
			{
				String pieces[5];
				pieces[0] = items[0];
				pieces[1] = Space;
				pieces[2] = conjunction;
				pieces[3] = Space;
				pieces[4] = items[1];
				return String_ConcatMany(pieces, 5);
			}
		case 3:
			{
				String pieces[7];
				pieces[0] = items[0];
				pieces[1] = CommaSpace;
				pieces[2] = items[1];
				pieces[3] = CommaSpace;
				pieces[4] = conjunction;
				pieces[5] = Space;
				pieces[6] = items[2];
				return String_ConcatMany(pieces, 7);
			}
	}

	stringBuilder = StringBuilder_Create();

	for (i = 0; i < numItems; i++) {
		if (i != 0)
			StringBuilder_AppendString(stringBuilder, CommaSpace);
		if (i == numItems - 1) {
			StringBuilder_AppendString(stringBuilder, conjunction);
			StringBuilder_AppendByte(stringBuilder, ' ');
		}
		StringBuilder_AppendString(stringBuilder, items[i]);
	}

	return StringBuilder_ToString(stringBuilder);
}
