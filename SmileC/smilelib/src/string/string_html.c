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
#include <smile/internal/html.h>

/// <summary>
/// Perform limited HTML-encoding on a string.  This will replace each of the four
/// most special HTML characters, &lt;, &gt;, &amp;, and &quot, and encode them as &amp;lt;,
/// &amp;gt;, &amp;amp;, and &amp;quot; respectively.
/// </summary>
/// <param name="str">The string to HTML-encode.</param>
/// <returns>The resulting HTML-safe string.</returns>
String String_HtmlEncode(const String str)
{
	StringBuilder stringBuilder;
	Int i, length;
	const Byte *text;
	Byte ch;
	
	length = String_Length(str);
	text = String_GetBytes(str);
	stringBuilder = StringBuilder_CreateWithSize(length + (length >> 2));

	for (i = 0; i < length; i++) {
		ch = text[i];
		switch (ch) {
			case '&':
				StringBuilder_Append(stringBuilder, "&amp;", 0, 5);
				break;
			case '<':
				StringBuilder_Append(stringBuilder, "&lt;", 0, 4);
				break;
			case '>':
				StringBuilder_Append(stringBuilder, "&gt;", 0, 4);
				break;
			case '\"':
				StringBuilder_Append(stringBuilder, "&quot;", 0, 6);
				break;
			default:
				StringBuilder_AppendByte(stringBuilder, ch);
				break;
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Perform extensive HTML-encoding on a string.  This will encode each of the four
/// unsafe HTML characters, &lt;, &gt;, &amp;, and &quot, as well as apostrophe ('),
/// as well as all values outside the ASCII range, treating them as UTF-8-encoded
/// Unicode code points and replacing them with either named or numeric escapes (the
/// named form if they are any of the 254 defined HTML5 entity names).
/// </summary>
/// <param name="str">The string to HTML-encode.</param>
/// <returns>The resulting HTML-safe string, which will only consist of ASCII code values
/// in the range of 0 to 127.</returns>
String String_HtmlEncodeToAscii(const String str)
{
	StringBuilder stringBuilder;
	Int i, length;
	const Byte *text;
	Byte ch;
	Int32 code;
	String htmlEntityName;
	
	length = String_Length(str);
	text = String_GetBytes(str);

	stringBuilder = StringBuilder_CreateWithSize(length + (length >> 2));

	for (i = 0; i < length; ) {
		ch = text[i];
		if (ch < 128) {
			switch (ch) {
			case '&':
				StringBuilder_Append(stringBuilder, "&amp;", 0, 5);
				break;
			case '<':
				StringBuilder_Append(stringBuilder, "&lt;", 0, 4);
				break;
			case '>':
				StringBuilder_Append(stringBuilder, "&gt;", 0, 4);
				break;
			case '\"':
				StringBuilder_Append(stringBuilder, "&quot;", 0, 6);
				break;
			default:
				StringBuilder_AppendByte(stringBuilder, ch);
				break;
			}
			i++;
			continue;
		}

		code = String_ExtractUnicodeCharacter(str, &i);

		if (code < 10000 && (htmlEntityName = HtmlEntityValueToName(code)) != NULL) {
			StringBuilder_AppendByte(stringBuilder, '&');
			StringBuilder_AppendString(stringBuilder, htmlEntityName);
			StringBuilder_AppendByte(stringBuilder, ';');
		}
		else {
			StringBuilder_AppendFormat(stringBuilder, "&#%u;", code);
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

Inline Int32 ParseHtmlNamedEntity(const Byte *text, Int length, Int *index)
{
	Int start, i;
	Byte ch;
	struct StringInt name;
	Int32 value;

	i = *index;
	
	// Capture alphanumeric characters.
	start = i;
	while (i < length) {
		ch = text[i];
		if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
			i++;
		else break;
	}

	// If we didn't have alphanumeric characters, it's malformed.
	if (i <= start) {
		*index = i;
		return -1;
	}
	else {
		// See if this is a name we've heard of by looking it up in the table (quickly).
		name.text = (Byte *)text + start;
		name.length = i - start;
		value = HtmlEntityNameToValue((String)&name);

		// Return the code for that known name.
		*index = i;
		return value;
	}
}

Inline Int32 ParseHtmlNumericForm(const Byte *text, Int length, Int *index)
{
	Byte ch;
	Int start, i;
	Int32 value;

	i = *index;
	value = 0;

	// See if this is a hex or decimal form...
	i++;
	if (i < length && ((ch = text[i]) == 'x' || ch == 'X')) {
		// This is a "&#x...;" hexadecimal form, so compute its value.
		start = ++i;
		while (i < length) {
			ch = text[i];
			if (ch >= '0' && ch <= '9') {
				value = (value << 4) | (ch - '0');
				i++;
			}
			else if (ch >= 'a' && ch <= 'f') {
				value = (value << 4) | (ch - 'a' + 10);
				i++;
			}
			else if (ch >= 'A' && ch <= 'F') {
				value = (value << 4) | (ch - 'A' + 10);
				i++;
			}
			else break;
		}
	}
	else {
		// This is a "&#...;" decimal form, so compute its value.
		start = i;
		while (i < length) {
			ch = text[i];
			if (ch >= '0' && ch <= '9') {
				value = (value * 10) + (ch - '0');
				i++;
			}
			else break;
		}
	}

	// If we consumed digits, this must be a valid numeric entity, so return its code.
	*index = i;
	return i > start ? value : -1;
}

Inline Int32 ParseHtmlEntity(const Byte *text, Int length, Int *index)
{
	Int i, ampersand;
	Int32 value;

	i = *index;
	ampersand = i++;

	if (i < length && text[i] == '#') {
		// This is a "&#...;" numeric form.
		value = ParseHtmlNumericForm(text, length, &i);
	}
	else {
		// This is a "&...;" named entity, or maybe just a floating ampersand.  So capture
		// all the alphanumeric characters that follow and see if they match up to a known
		// entity.
		value = ParseHtmlNamedEntity(text, length, &i);
	}

	if (value < 0 || i >= length || text[i++] != ';') {
		// Malformed, so rewind, copy the '&' to the output, and treat the rest as literal characters.
		i = ampersand + 1;
		*index = i;
		return '&';
	}
	else {
		// Got a valid (enough) entity, so append it to the output.
		*index = i;
		return value;
	}
}

/// <summary>
/// Perform HTML-decoding on a string.  This will decode each of the four
/// unsafe HTML characters, &lt;, &gt;, &amp;, and &quot, as well as apostrophe ('),
/// as well as all of the defined 254 HTML named entities, and any numeric entities,
/// replacing them with their UTF-8-encoded Unicode code points.
/// </summary>
/// <param name="str">The string to HTML-decode.</param>
/// <returns>The resulting decoded string.</returns>
String String_HtmlDecode(const String str)
{
	StringBuilder result;
	Int i, length, start;
	const Byte *text;
	Byte ch;
	Int32 uch;

	if (String_IsNullOrEmpty(str)) return str;

	result = NULL;
	length = String_Length(str);
	text = String_GetBytes(str);

	for (i = 0; i < length; ) {
		// Copy non-escaped characters.
		start = i;
		ch = '\0';
		while (i < length && (ch = text[i]) != '&') i++;

		// If there are any non-escaped characters, copy them to the output.
		if (i > start) {
			// If the non-escaped content is the whole string, then just return it verbatim.
			if (i == length && start == 0)
				return str;

			// Copy the non-escaped part to the StringBuilder.
			if (result == NULL)
				result = StringBuilder_CreateWithSize(length);
			StringBuilder_Append(result, text, start, i - start);
		}

		// If we reached a '&', process the character escape (or degenerate form).
		if (ch == '&') {
			uch = ParseHtmlEntity(text, length, &i);
			if (result == NULL)
				result = StringBuilder_CreateWithSize(length);
			StringBuilder_AppendUnicode(result, uch);
		}
	}

	return StringBuilder_ToString(result);
}

/// <summary>
/// Perform URL-encoding on a string.  The resulting string can safely be used
/// in any part of a URL, including path, query string, or hash.  This encodes
/// all characters outside the printable ASCII range, as well as space (as "%20"),
/// and all special characters in this set:
///        !  *  '  ;  :  @  &  =  +  $  ,  /  ?  #  %  (  )  [  ]
/// </summary>
/// <param name="str">The string to URL-encode.</param>
/// <returns>The resulting encoded string.</returns>
String String_UrlEncode(const String str)
{
	Byte ch;
	StringBuilder stringBuilder;
	const Byte *text;
	Int i, length;

	length = String_Length(str);
	text = String_GetBytes(str);
	stringBuilder = StringBuilder_CreateWithSize(length * 2);

	for (i = 0; i < length; i++) {
		switch (ch = text[i]) {
			default:
				if (ch <= 32 || ch >= 127)
					goto encode;
				StringBuilder_AppendByte(stringBuilder, ch);
				break;

			case '!': case '*': case '\'': case ';': case ':':
			case '@': case '&': case '=': case '+': case '$':
			case ',': case '/': case '?': case '#': case '%':
			case '(': case ')': case '[': case ']':
			encode:
				StringBuilder_AppendFormat(stringBuilder, "%%%02X", (UInt)ch);
				break;
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Perform URL-encoding on a string, encoding only those characters that would
/// typically cause problems in the query string, leaving the other characters
/// unchanged.  The resulting string can safely be used only in the query string,
/// but is usually more readable than the result of full URL-encoding.  This encodes
/// all characters outside the printable ASCII range, as well as space (as "%20"),
/// and the five special characters in this set:   &  =  ?  #  %
/// </summary>
/// <param name="str">The string to URL-query-encode.</param>
/// <returns>The resulting encoded string.</returns>
String String_UrlQueryEncode(const String str)
{
	Byte ch;
	StringBuilder stringBuilder;
	const Byte *text;
	Int i, length;

	length = String_Length(str);
	text = String_GetBytes(str);
	stringBuilder = StringBuilder_CreateWithSize(length * 2);

	for (i = 0; i < length; i++) {
		switch (ch = text[i]) {
			default:
				if (ch <= 32 || ch >= 127)
					goto encode;
				StringBuilder_AppendByte(stringBuilder, ch);
				break;

			case '&': case '=': case '?': case '#': case '%':
			encode :
				StringBuilder_AppendFormat(stringBuilder, "%%%02X", (UInt)ch);
				   break;
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

Inline Byte DecodeHex(Byte ch)
{
	switch (ch) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
		default: return 0;
	}
}

Inline Bool IsHex(Byte ch)
{
	switch (ch) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'a': case 'A':
		case 'b': case 'B':
		case 'c': case 'C':
		case 'd': case 'D':
		case 'e': case 'E':
		case 'f': case 'F':
			return True;
		default:
			return False;
	}
}

/// <summary>
/// Perform URL-decoding on a string, decoding any characters encoded using the "%XX"
/// form as replacement 8-bit character values.
/// </summary>
/// <param name="str">The string to URL-decode.</param>
/// <returns>The resulting decoded string.</returns>
String String_UrlDecode(const String str)
{
	Byte ch, c1, c2, value;
	StringBuilder stringBuilder;
	const Byte *text;
	Int i, length, start;

	length = String_Length(str);
	text = String_GetBytes(str);
	stringBuilder = StringBuilder_CreateWithSize(length);

	for (i = 0; i < length; i++) {
		start = i;
		while (i < length && (ch = text[i]) != '%') i++;
		if (i > start) {
			StringBuilder_Append(stringBuilder, text, start, i - start);
		}
		if (i + 2 < length && IsHex(c1 = text[i + 1]) && IsHex(c2 = text[i + 2])) {
			value = (DecodeHex(c1) << 4) | DecodeHex(c2);
			StringBuilder_AppendByte(stringBuilder, value);
			i += 2;
		}
		else if (i < length) {
			StringBuilder_AppendByte(stringBuilder, '%');
		}
	}

	return StringBuilder_ToString(stringBuilder);
}
