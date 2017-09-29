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

#include <smile/gc.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/internal/staticstring.h>
#include <smile/smiletypes/smilelist.h>

typedef enum {
	Char_Other = 0,
	Char_LowercaseLetter = 1,
	Char_UppercaseLetter = 2,
	Char_Number = 3,
} CharClass;

/// <summary>
/// Given a Unicode code point, determine approximately which class of characters it belongs
/// to, for purposes of correctly identifying word boundaries in identifiers.
/// </summary>
/// <param name="ch">The character (Unicode code point) to identify.</param>
/// <returns>The known character class of the given code point.</returns>
Inline CharClass DetectCharClass(UInt ch)
{
	if (ch < 0x80) {

		switch (ch) {

			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
			case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
				return Char_Other;
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			case 0x38: case 0x39:
				return Char_Number;
			                      case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:
			case 0x40:
				return Char_Other;
			           case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
			case 0x58: case 0x59: case 0x5A:
				return Char_UppercaseLetter;
			                                 case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
			case 0x60:
				return Char_Other;
			           case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
			case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			case 0x78: case 0x79: case 0x7A:
				return Char_LowercaseLetter;
			                                 case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			default:
				return Char_Other;
		}
	}
	else {
		return Char_Other;
	}
}

/// <summary>
/// Convert the given string, which is presumably *some* kind of programming-language
/// identifier, to CamelCase form.  This does *not* strip diacritics from the string,
/// but it does treat punctuation as separators to be discarded.  This correctly detects
/// word boundaries in most common identifier forms and generates a suitable CamelCase
/// equivalent.
/// </summary>
/// <param name="string">The string to convert to CamelCase.</param>
/// <param name="uppercaseInitialLetter">Whether to convert to CamelCase (True) or to camelCase (False).</param>
/// <param name="lowercaseAcronyms">Whether to convert any acronyms (like "VIN") to a lowercased form (like "Vin").</param>
/// <returns>The string converted to CamelCase.</returns>
String String_CamelCase(String string, Bool uppercaseInitialLetter, Bool lowercaseAcronyms)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	DECLARE_INLINE_STRINGBUILDER(wordBuilder, 256);
	const Byte *src, *end;
	Byte ch;
	CharClass lastClass, currentClass;
	Bool isFirstWord;

	if (String_IsNullOrEmpty(string))
		return string;

	INIT_INLINE_STRINGBUILDER(stringBuilder);
	INIT_INLINE_STRINGBUILDER(wordBuilder);

	src = String_GetBytes(string);
	end = src + String_Length(string);
	lastClass = Char_Other;

	isFirstWord = True;

#	define EMIT_RESULT \
		((StringBuilder_AppendStringBuilder(stringBuilder, wordBuilder)), \
		 (StringBuilder_SetLength(wordBuilder, 0)), \
		 (isFirstWord = False))

	// Spin over the input, splitting it into "words", where each "word" is converted
	// to uppercase initial letters with (hopefully) lowercase subsequent letters.
	while (src < end) {
		ch = *src++;
		currentClass = DetectCharClass(ch);

		switch (lastClass | currentClass) {
			case ((Char_Other << 4) | Char_Other):
				// Discard character:  _ --> _
				break;
			case ((Char_Other << 4) | Char_LowercaseLetter):
				// Start of an alphabetic word:  _ --> a
				StringBuilder_AppendByte(wordBuilder, ch - ('a' - 'A'));
				break;
			case ((Char_Other << 4) | Char_UppercaseLetter):
				// Start of an alphabetic word:  _ --> A
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
			case ((Char_Other << 4) | Char_Number):
				// Start of a numeric "word":  _ --> 1
				StringBuilder_AppendByte(wordBuilder, ch);
				break;

			case ((Char_LowercaseLetter << 4) | Char_Other):
				// End of an alphabetic word:  a --> _
				EMIT_RESULT;
				break;
			case ((Char_LowercaseLetter << 4) | Char_LowercaseLetter):
				// Middle of an alphabetic word:  a --> a
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
			case ((Char_LowercaseLetter << 4) | Char_UppercaseLetter):
				// End of the previous word, start of the next:  a --> A
				EMIT_RESULT;
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
			case ((Char_LowercaseLetter << 4) | Char_Number):
				// This word has a numeric "tail":  a --> 1   (i.e., "foo2")
				StringBuilder_AppendByte(wordBuilder, ch);
				break;

			case ((Char_UppercaseLetter << 4) | Char_Other):
				// End of an acronym or initialism:  A --> _
				EMIT_RESULT;
				break;
			case ((Char_UppercaseLetter << 4) | Char_LowercaseLetter):
				// Likely the beginning of a word:  A --> a
				// May be the end of an initialism:  "VIND --> ecoding" should be split as "VIN" and "Decoding"
				// However, this might also be a transition out of something that is almost but not
				// quite an initialism:  "CS --> tring".  In this case, we split as "CString", not as "C" and "String".
				if (StringBuilder_GetLength(wordBuilder) > 2) {
					Int wordLength = StringBuilder_GetLength(wordBuilder);
					Byte last1 = StringBuilder_At(wordBuilder, wordLength - 1);
					Byte last2 = StringBuilder_At(wordBuilder, wordLength - 2);
					Bool needLowercase = lowercaseAcronyms || (isFirstWord && !uppercaseInitialLetter);

					StringBuilder_AppendStringBuilderSubstring(stringBuilder, wordBuilder, 0, wordLength - 2);
					StringBuilder_SetLength(wordBuilder, 0);
					isFirstWord = False;

					StringBuilder_AppendByte(wordBuilder, needLowercase ? last2 - ('a' - 'A') : last2);
					StringBuilder_AppendByte(wordBuilder, needLowercase ? last1 - ('a' - 'A') : last1);
				}
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
			case ((Char_UppercaseLetter << 4) | Char_UppercaseLetter):
				{
					// Continuing an acronym or initialism:  A --> A
					Bool needLowercase = lowercaseAcronyms || (isFirstWord && !uppercaseInitialLetter);
					StringBuilder_AppendByte(wordBuilder, needLowercase ? ch + ('a' - 'A') : ch);
				}
				break;
			case ((Char_UppercaseLetter << 4) | Char_Number):
				// This acronym or initialism has a numeric "tail":  A --> 1   (i.e., "FOO2")
				StringBuilder_AppendByte(wordBuilder, ch);
				break;

			case ((Char_Number << 4) | Char_Other):
				// End of a numeric word:  1 --> _
				EMIT_RESULT;
				break;
			case ((Char_Number << 4) | Char_LowercaseLetter):
				// Likely the beginning of a word:  1 --> a   ("foo2bar" --> "Foo2" "Bar")
				EMIT_RESULT;
				break;
			case ((Char_Number << 4) | Char_UppercaseLetter):
				// Likely the beginning of a word:  1 --> A   ("FOO2BAR" --> "FOO2" "BAR")
				EMIT_RESULT;
				break;
			case ((Char_Number << 4) | Char_Number):
				// Still in the middle of a number:  1 --> 2
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
		}

		lastClass = (currentClass << 4);
	}

	// If there's any word still not yet fully emitted, finish it off.
	if (StringBuilder_GetLength(wordBuilder) > 0) {
		StringBuilder_AppendStringBuilder(stringBuilder, wordBuilder);
	}

	// If they didn't want an uppercase initial letter (which is what we probably emitted),
	// turn it into a lowercase letter.
	if (!uppercaseInitialLetter && StringBuilder_GetLength(stringBuilder) > 0) {
		ch = StringBuilder_At(stringBuilder, 0);
		if (ch >= 'A' && ch <= 'Z')
			ch += ('a' - 'A');
		StringBuilder_SetAt(stringBuilder, 0, ch);
	}

#	undef EMIT_RESULT

	// And we're done.
	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Convert the given string, which is presumably *some* kind of programming-language
/// identifier, to hyphenized form (i.e., FooBarBaz becomes foo-bar-baz).  This does
/// *not* strip diacritics from the string, but punctuation will be treated as a word
/// boundary.  Identifiers and initialisms are detected and outputted as separate words.
/// All outputted letters will be lowercase.
/// </summary>
/// <param name="string">The string to convert to hyphenized form.</param>
/// <param name="separator">The separator character to use (usually '-' or '_').</param>
/// <returns>The string converted to hyphenized form.</returns>
String String_Hyphenize(String string, Byte separator)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	DECLARE_INLINE_STRINGBUILDER(wordBuilder, 256);
	const Byte *src, *end;
	Byte ch;
	CharClass lastClass, currentClass;
	Bool isFirstWord;

	if (String_IsNullOrEmpty(string))
		return string;

	INIT_INLINE_STRINGBUILDER(stringBuilder);
	INIT_INLINE_STRINGBUILDER(wordBuilder);

	src = String_GetBytes(string);
	end = src + String_Length(string);
	lastClass = Char_Other;

	isFirstWord = True;

#	define EMIT_RESULT \
		((!isFirstWord ? StringBuilder_AppendByte(stringBuilder, separator) : (void)0), \
		 (StringBuilder_AppendStringBuilder(stringBuilder, wordBuilder)), \
		 (StringBuilder_SetLength(wordBuilder, 0)), \
		 (isFirstWord = False))

	// Spin over the input, splitting it into "words", where each "word" is converted
	// to uppercase initial letters with (hopefully) lowercase subsequent letters.
	while (src < end) {
		ch = *src++;
		currentClass = DetectCharClass(ch);

		switch (lastClass | currentClass) {
			case ((Char_Other << 4) | Char_Other):
				// Discard character:  _ --> _
				break;
			case ((Char_Other << 4) | Char_LowercaseLetter):
				// Start of an alphabetic word:  _ --> a
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
			case ((Char_Other << 4) | Char_UppercaseLetter):
				// Start of an alphabetic word:  _ --> A
				StringBuilder_AppendByte(wordBuilder, ch + ('a' - 'A'));
				break;
			case ((Char_Other << 4) | Char_Number):
				// Start of a numeric "word":  _ --> 1
				StringBuilder_AppendByte(wordBuilder, ch);
				break;

			case ((Char_LowercaseLetter << 4) | Char_Other):
				// End of an alphabetic word:  a --> _
				EMIT_RESULT;
				break;
			case ((Char_LowercaseLetter << 4) | Char_LowercaseLetter):
				// Middle of an alphabetic word:  a --> a
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
			case ((Char_LowercaseLetter << 4) | Char_UppercaseLetter):
				// End of the previous word, start of the next:  a --> A
				EMIT_RESULT;
				StringBuilder_AppendByte(wordBuilder, ch + ('a' - 'A'));
				break;
			case ((Char_LowercaseLetter << 4) | Char_Number):
				// This word has a numeric "tail":  a --> 1   (i.e., "foo2")
				StringBuilder_AppendByte(wordBuilder, ch);
				break;

			case ((Char_UppercaseLetter << 4) | Char_Other):
				// End of an acronym or initialism:  A --> _
				EMIT_RESULT;
				break;
			case ((Char_UppercaseLetter << 4) | Char_LowercaseLetter):
				// Likely the beginning of a word:  A --> a
				// May be the end of an initialism:  "VIND --> ecoding" should be split as "VIN" and "Decoding"
				// However, this might also be a transition out of something that is almost but not
				// quite an initialism:  "CS --> tring".  In this case, we split as "CString", not as "C" and "String".
				if (StringBuilder_GetLength(wordBuilder) > 2) {
					Int wordLength = StringBuilder_GetLength(wordBuilder);
					Byte last1 = StringBuilder_At(wordBuilder, wordLength - 1);
					Byte last2 = StringBuilder_At(wordBuilder, wordLength - 2);

					if (!isFirstWord)
						StringBuilder_AppendByte(stringBuilder, separator);
					StringBuilder_AppendStringBuilderSubstring(stringBuilder, wordBuilder, 0, wordLength - 2);
					StringBuilder_SetLength(wordBuilder, 0);
					isFirstWord = False;

					StringBuilder_AppendByte(wordBuilder, last2);
					StringBuilder_AppendByte(wordBuilder, last1);
				}
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
			case ((Char_UppercaseLetter << 4) | Char_UppercaseLetter):
				// Continuing an acronym or initialism:  A --> A
				StringBuilder_AppendByte(wordBuilder, ch + ('a' - 'A'));
				break;
			case ((Char_UppercaseLetter << 4) | Char_Number):
				// This acronym or initialism has a numeric "tail":  A --> 1   (i.e., "FOO2")
				StringBuilder_AppendByte(wordBuilder, ch);
				break;

			case ((Char_Number << 4) | Char_Other):
				// End of a numeric word:  1 --> _
				EMIT_RESULT;
				break;
			case ((Char_Number << 4) | Char_LowercaseLetter):
				// Likely the beginning of a word:  1 --> a   ("foo2bar" --> "Foo2" "Bar")
				EMIT_RESULT;
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
			case ((Char_Number << 4) | Char_UppercaseLetter):
				// Likely the beginning of a word:  1 --> A   ("FOO2BAR" --> "FOO2" "BAR")
				EMIT_RESULT;
				StringBuilder_AppendByte(wordBuilder, ch + ('a' - 'A'));
				break;
			case ((Char_Number << 4) | Char_Number):
				// Still in the middle of a number:  1 --> 2
				StringBuilder_AppendByte(wordBuilder, ch);
				break;
		}

		lastClass = (currentClass << 4);
	}

	// If there's any word still not yet fully emitted, finish it off.
	if (StringBuilder_GetLength(wordBuilder) > 0) {
		if (!isFirstWord)
			StringBuilder_AppendByte(stringBuilder, separator);
		StringBuilder_AppendStringBuilder(stringBuilder, wordBuilder);
	}

#	undef EMIT_RESULT

	// And we're done.
	return StringBuilder_ToString(stringBuilder);
}
