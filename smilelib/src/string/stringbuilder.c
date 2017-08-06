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
#include <smile/stringbuilder.h>
#include <smile/bittwiddling.h>

/// <summary>
/// Initialize a StringBuilder structure, allocating enough initial room for 'initialSize' characters.
/// </summary>
/// <param name="stringBuilder">The stringBuilder to initialize.</param>
/// <param name="initialSize">The initial number of characters to allocate (plus one more for a possible trailing '\0').</param>
void StringBuilder_InitWithSize(StringBuilder stringBuilder, Int initialSize)
{
	struct StringBuilderInt *sb;
	Byte *newText;

	if (initialSize < 256) initialSize = 256;
	initialSize = NextPowerOfTwo(initialSize);

	sb = (struct StringBuilderInt *)stringBuilder;
	newText = GC_MALLOC_TEXT(initialSize);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	sb->text = newText;
	sb->length = 0;
	sb->max = initialSize;

	newText[0] = '\0';
}

/// <summary>
/// Create a new StringBuilder, allocating enough initial room for the given string.
/// </summary>
/// <param name="text">The text to initially copy into the StringBuilder.</param>
/// <param name="start">The start of the substring to copy from that text.</param>
/// <param name="length">The length of the substring to copy from that text.</param>
/// <returns>A new StringBuilder containing the given substring as its initial text.</returns>
StringBuilder StringBuilder_CreateFromBytes(const Byte *text, Int start, Int length)
{
	struct StringBuilderInt *sb;
	Byte *newText;
	Int initialSize;

	if (length < 0) length = 0;

	initialSize = length + 1;
	if (initialSize < 256) initialSize = 256;
	initialSize = NextPowerOfTwo(initialSize);

	sb = GC_MALLOC_STRUCT(struct StringBuilderInt);
	if (sb == NULL) Smile_Abort_OutOfMemory();
	newText = GC_MALLOC_TEXT(initialSize);
	if (newText == NULL) Smile_Abort_OutOfMemory();

	sb->text = newText;
	sb->length = length;
	sb->max = initialSize;

	MemCpy(newText, text + start, length);
	newText[length] = '\0';

	return (StringBuilder)sb;
}

/// <summary>
/// Grow a StringBuilder's internal array to accommodate at least 'length' additional characters
/// beyond its current storage.
/// </summary>
/// <param name="sb">The StringBuilder to grow.</param>
/// <param name="length">The minimum number of additional characters that are needed within the
/// StringBuilder.</param>
Inline void StringBuilder_Grow(struct StringBuilderInt *sb, Int length)
{
	Byte *newText;
	Int newMax;

	if (sb->length + length < sb->max) return;

	newMax = NextPowerOfTwo(sb->length + length);
	newText = GC_MALLOC_TEXT(newMax);
	if (newText == NULL) Smile_Abort_OutOfMemory();
	MemCpy(newText, sb->text, sb->length + 1);
	sb->text = newText;
	sb->max = newMax;
}

/// <summary>
/// Append the given substring of text to the end of the given StringBuilder.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to append to.</param>
/// <param name="text">The text to append.</param>
/// <param name="start">The start of the substring to copy from that text.</param>
/// <param name="length">The length of the substring to copy from that text.</param>
void StringBuilder_Append(StringBuilder stringBuilder, const Byte *text, Int start, Int length)
{
	struct StringBuilderInt *sb = (struct StringBuilderInt *)stringBuilder;

	if (length <= 0) return;

	StringBuilder_Grow(sb, length);
	MemCpy((Byte *)sb->text + sb->length, text + start, length);
	sb->text[sb->length += length] = '\0';
}

/// <summary>
/// Append the given single character to the end of the given StringBuilder.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to append to.</param>
/// <param name="ch">The character to append.</param>
void StringBuilder_AppendByte(StringBuilder stringBuilder, Byte ch)
{
	struct StringBuilderInt *sb = (struct StringBuilderInt *)stringBuilder;

	StringBuilder_Grow(sb, 1);
	sb->text[sb->length] = ch;
	sb->text[++sb->length] = '\0';
}

/// <summary>
/// Append the given single character, repeated 'length' times, to the end of the given StringBuilder.
/// This is much more efficient than calling StringBuilder_AppendByte() 'length' times.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to append to.</param>
/// <param name="ch">The character to append.</param>
/// <param name="length">The number of times to append that character.</param>
void StringBuilder_AppendRepeat(StringBuilder stringBuilder, Byte ch, Int length)
{
	struct StringBuilderInt *sb = (struct StringBuilderInt *)stringBuilder;

	if (length <= 0) return;

	StringBuilder_Grow(sb, length);
	MemSet((Byte *)sb->text + sb->length, ch, length);
	sb->text[sb->length += length] = '\0';
}

/// <summary>
/// Append the given single Unicode code point, encoded as UTF-8, to the end of the given StringBuilder.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to append to.</param>
/// <param name="value">The Unicode code point to append.  Values of 127 or less will be represented
/// as standard ASCII bytes, while values of 128 or larger will be encoded as UTF-8.  Illegal values
/// beyond the defined 0x110000 endpoint of Unicode will be encoded as though they were 0xFFFD, or the
/// "illegal character" code point.</param>
void StringBuilder_AppendUnicode(StringBuilder stringBuilder, UInt32 value)
{
	struct StringBuilderInt *sb = (struct StringBuilderInt *)stringBuilder;

	if (value < 0x80) {
		// Classic ASCII.
		if (sb->length + 1 >= sb->max)
			StringBuilder_Grow(sb, 1);
		sb->text[sb->length++] = (Byte)value;
	}
	else if (value < 0x800) {
		// Low values in the Basic Multilingual Plane, like Latin and Greek and Hebrew.
		if (sb->length + 2 >= sb->max)
			StringBuilder_Grow(sb, 2);
		sb->text[sb->length++] = (Byte)((value >> 6) | 0xC0);
		sb->text[sb->length++] = (Byte)((value & 0x3F) | 0x80);
	}
	else if (value < 0x10000)
	{
		// High values in the Basic Multilingual Plane, like Chinese and Japanese and Arabic.
		if (sb->length + 3 >= sb->max)
			StringBuilder_Grow(sb, 3);
		sb->text[sb->length++] = (Byte)((value >> 12) | 0xE0);
		sb->text[sb->length++] = (Byte)(((value >> 6) & 0x3F) | 0x80);
		sb->text[sb->length++] = (Byte)((value & 0x3F) | 0x80);
	}
	else if (value < 0x110000)
	{
		// Four bytes for characters above the BMP (i.e., characters requiring more than 16 bits).
		if (sb->length + 4 >= sb->max)
			StringBuilder_Grow(sb, 4);
		sb->text[sb->length++] = (Byte)((value >> 18) | 0xF0);
		sb->text[sb->length++] = (Byte)(((value >> 12) & 0x3F) | 0x80);
		sb->text[sb->length++] = (Byte)(((value >> 6) & 0x3F) | 0x80);
		sb->text[sb->length++] = (Byte)((value & 0x3F) | 0x80);
	}
	else
	{
		// Not legal, so encode the "unknown replacement character" character.
		value = 0xFFFD;
		if (sb->length + 3 >= sb->max)
			StringBuilder_Grow(sb, 3);
		sb->text[sb->length++] = (Byte)((value >> 12) | 0xE0);
		sb->text[sb->length++] = (Byte)(((value >> 6) & 0x3F) | 0x80);
		sb->text[sb->length++] = (Byte)((value & 0x3F) | 0x80);
	}
}
