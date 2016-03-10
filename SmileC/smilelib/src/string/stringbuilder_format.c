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
#include <smile/stringbuilder.h>

#include <math.h>

#if defined(_MSC_VER)
	#include <float.h>
	#ifndef isnan
		#define isnan(x) _isnan(x)
	#endif
	#ifndef isinf
		#define isinf(x) (!_finite(x))
	#endif
#endif

static void StringBuilder_AppendFormatInternal(StringBuilder stringBuilder, const Byte *text, Int textLength, va_list v);

/// <summary>
/// Perform simplified sprintf-style formatting, appending the result to the given StringBuilder.
/// This supports a limited set of sprintf-style percent codes.  See StringBuilder_AppendFormatInternal()
/// for details as to which percent codes are supported.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to which the formatted string will be appended.</param>
/// <param name="format">The format string, which is C-style nul-terminated.</param>
void StringBuilder_AppendFormat(StringBuilder stringBuilder, const char *format, ...)
{
	va_list v;

	if (format == NULL || format[0] == '\0') return;

	va_start(v, format);
	StringBuilder_AppendFormatInternal(stringBuilder, (const Byte *)format, StrLen(format), v);
	va_end(v);
}

/// <summary>
/// Perform simplified sprintf-style formatting, appending the result to the given StringBuilder.
/// This supports a limited set of sprintf-style percent codes.  See StringBuilder_AppendFormatInternal()
/// for details as to which percent codes are supported.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to which the formatted string will be appended.</param>
/// <param name="format">The format string, which is C-style nul-terminated.</param>
/// <param name="v">The va_list that describes the format arguments.</param>
void StringBuilder_AppendFormatv(StringBuilder stringBuilder, const char *format, va_list v)
{
	if (format == NULL || format[0] == '\0') return;

	StringBuilder_AppendFormatInternal(stringBuilder, (const Byte *)format, StrLen(format), v);
}

/// <summary>
/// Perform simplified sprintf-style formatting, appending the result to the given StringBuilder.
/// This supports a limited set of sprintf-style percent codes.  See StringBuilder_AppendFormatInternal()
/// for details as to which percent codes are supported.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to which the formatted string will be appended.</param>
/// <param name="format">The format string.</param>
void StringBuilder_AppendFormatString(StringBuilder stringBuilder, const String format, ...)
{
	const struct StringInt *s = (const struct StringInt *)format;
	va_list v;

	if (format == NULL || s->length <= 0) return;

	va_start(v, format);
	StringBuilder_AppendFormatInternal(stringBuilder, s->text, s->length, v);
	va_end(v);
}

/// <summary>
/// Perform simplified sprintf-style formatting, appending the result to the given StringBuilder.
/// This supports a limited set of sprintf-style percent codes.  See StringBuilder_AppendFormatInternal()
/// for details as to which percent codes are supported.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to which the formatted string will be appended.</param>
/// <param name="format">The format string.</param>
/// <param name="v">The va_list that describes the format arguments.</param>
void StringBuilder_AppendFormatStringv(StringBuilder stringBuilder, const String format, va_list v)
{
	const struct StringInt *s = (const struct StringInt *)format;

	if (format == NULL || s->length <= 0) return;

	StringBuilder_AppendFormatInternal(stringBuilder, s->text, s->length, v);
}

#define BUFFER_LIMIT 256

/// <summary>
/// Perform simplified sprintf-style formatting, appending the result to the given StringBuilder.
/// This supports a limited set of sprintf-style percent codes, as follows:
/// <ul>
/// <li>%% - An embedded percent symbol</li>
/// <li>%s - A C-style nul-terminated string argument.</li>
/// <li>%S - A String (object) argument.</li>
/// <li>%c - An 8-bit Byte argument, formatted as its equivalent character.</li>
/// <li>%d - A signed Int argument, formatted as a signed decimal number.</li>
/// <li>%u - An unsigned UInt argument, formatted as an unsigned decimal number.</li>
/// <li>%x - An unsigned UInt argument, formatted as an unsigned hexadecimal number in lowercase.</li>
/// <li>%X - An unsigned UInt argument, formatted as an unsigned hexadecimal number in uppercase.</li>
/// <li>%f - A 64-bit "double" floating-point argument, always formatted as "###.###", with as
///	         few digits after the decimal place as possible.  %e and %g are recognized as synonyms.</li>
/// <li>%p - A pointer argument.</li>
/// </ul>
/// In addition, the %d, %u, %x, and %X format arguments also accept the following modifiers:
/// <ul>
/// <li>h - The argument must be Int32 or UInt32, and only 32 bits of integer value will be formatted.</li>
/// <li>l - The argument must be Int64 or UInt64, and 64 bits of integer value will be formatted.</li>
/// <li>1-9 - These digits may be used to specify the minimum width of the output; ' ' character will be applied if necessary to pad it.
///           This width may be at most 255 characters; any larger width will be treated as 255.</li>
/// <li>0 - If this is specified with a width, the number will be padded with '0', not with ' ' characters.</li>
/// </ul>
/// There is no support for other common format codes, other modifiers (like '+' and '-'), or precision in addition to width.
/// </summary>
/// <param name="stringBuilder">The StringBuilder to which the formatted string will be appended.</param>
/// <param name="text">The format string itself.</param>
/// <param name="textLength">The length of the format string.</param>
/// <param name="v">The va_list that describes the format arguments.</param>
void StringBuilder_AppendFormatInternal(StringBuilder stringBuilder, const Byte *text, Int textLength, va_list v)
{
	Int start = 0, i = 0;
	Int mode;
	Int size;
	Byte padChar;
	Int width;
	UInt64 number;
	Bool negative;
	double d, intPart, fracPart, origFracPart;

	Byte buffer[BUFFER_LIMIT];

	while (i < textLength) {
		Byte ch = text[i];
		if (ch != '%') {
			i++;
			continue;
		}

		if (i > start) {
			StringBuilder_Append(stringBuilder, text, start, i - start);
		}

		i++;

		// Special case: if this is "%%", just emit a single percent character.
		if (i < textLength && text[i] == '%') {
			StringBuilder_AppendByte(stringBuilder, '%');
			i++;
			start = i;
			continue;
		}

		// Got a '%' start character, so get the mode character after it.
		// We only support "%s", "%S", "%d", "%u", "%x", "%X", and "%c", with no format parameters except 'h' and 'l'
		// and digits and zero.  Digits and zero only affect numeric formatting, and there is a max width of BUFFER_LIMIT characters.
		// "%d" implies a type of Int, while "%u", "%x", and "%X" implies a type of UInt;
		// The 'h' and 'l' modifiers imply explicit 32-bit and 64-bit, respectively.
		size = 0;
		padChar = ' ';
		width = 0;
	retry:
		mode = -1;
		if (i < textLength) {
			mode = text[i++];
		}

		switch (mode) {

			case 'h':
				size = -1;
				goto retry;

			case 'l':
				size = +1;
				goto retry;

			case '0':
				if (width != 0) width *= 10;
				else padChar = '0';
				goto retry;

			case '1': width = (width * 10) + 1; goto retry;
			case '2': width = (width * 10) + 2; goto retry;
			case '3': width = (width * 10) + 3; goto retry;
			case '4': width = (width * 10) + 4; goto retry;
			case '5': width = (width * 10) + 5; goto retry;
			case '6': width = (width * 10) + 6; goto retry;
			case '7': width = (width * 10) + 7; goto retry;
			case '8': width = (width * 10) + 8; goto retry;
			case '9': width = (width * 10) + 9; goto retry;

			case 'c':
				{
					// 8-bit character.  Because of the vagaries of C, this is 'int', not 'char'.
					char ch = (char)va_arg(v, int);
					StringBuilder_AppendByte(stringBuilder, ch);
				}
				break;

			case 's':
				{
					// C-style nul-terminated 8-bit string.
					const char *src = va_arg(v, const char *);
					if (src == NULL) continue;
					StringBuilder_AppendC(stringBuilder, src, 0, StrLen(src));
				}
				break;

			case 'S':
				{
					// String object.
					const String src = va_arg(v, const String);
					if (src == NULL) continue;
					StringBuilder_AppendString(stringBuilder, src);
				}
				break;

			case 'd':
				{
					// Signed integer.
					Int64 value;
					switch (size) {
						default:
						case 0:
							value = va_arg(v, Int);
							break;
						case +1:
							value = va_arg(v, Int64);
							break;
						case -1:
							value = va_arg(v, Int32);
							break;
					}
					if (value < 0) {
						number = (UInt64)-value;
						negative = True;
					}
					else {
						number = (UInt64)value;
						negative = False;
					}
					goto common_number_format;
				}
				break;

			case 'u':
				{
					// Unsigned integer.
					switch (size) {
						default:
						case 0:
							number = va_arg(v, UInt);
							break;
						case +1:
							number = va_arg(v, UInt64);
							break;
						case -1:
							number = va_arg(v, UInt32);
							break;
					}
					negative = False;
				}

			common_number_format:
				{
					Byte *dest = buffer + BUFFER_LIMIT;
					while (number != 0) {
						Byte digit = (Byte)(number % 10L);
						number /= 10L;
						*--dest = digit + '0';
					}
					if (dest == buffer + BUFFER_LIMIT)
						*--dest = '0';
					if (width > BUFFER_LIMIT - 1) width = BUFFER_LIMIT - 1;
					width -= (buffer + BUFFER_LIMIT) - dest;
					if (negative) {
						if (padChar != '0')
							*--dest = '-';
						width--;
					}
					if (width > 0) {
						while (width--) {
							*--dest = padChar;
						}
					}
					if (negative && padChar == '0')
						*--dest = '-';
					StringBuilder_Append(stringBuilder, dest, 0, (buffer + BUFFER_LIMIT) - dest);
				}
				break;

			case 'p':
			case 'P':
				{
					Byte *dest = buffer + BUFFER_LIMIT;
					Byte letterOffset = (mode == 'P' ? 'A' - 10 : 'a' - 10);

					number = (UInt64)(PtrInt)va_arg(v, void *);

					while (number != 0) {
						Byte digit = (Byte)(number & 0xFL);
						number >>= 4;
						*--dest = digit <= 9 ? digit + '0' : digit + letterOffset;
					}

					width = sizeof(PtrInt) * 2;
					width -= (buffer + BUFFER_LIMIT) - dest;

					if (width > 0) {
						while (width--) {
							*--dest = '0';
						}
					}
					*--dest = 'x';
					*--dest = '0';

					StringBuilder_Append(stringBuilder, dest, 0, (buffer + BUFFER_LIMIT) - dest);
				}
				break;

			case 'x':
			case 'X':
				{
					Byte *dest = buffer + BUFFER_LIMIT;
					Byte letterOffset = (mode == 'X' ? 'A' - 10 : 'a' - 10);

					// Unsigned integer, in hex.
					switch (size) {
						default:
						case 0:
							number = va_arg(v, UInt);
							break;
						case +1:
							number = va_arg(v, UInt64);
							break;
						case -1:
							number = va_arg(v, UInt32);
							break;
					}

					while (number != 0) {
						Byte digit = (Byte)(number & 0xFL);
						number >>= 4;
						*--dest = digit <= 9 ? digit + '0' : digit + letterOffset;
					}
					if (dest == buffer + BUFFER_LIMIT)
						*--dest = '0';
					if (width > BUFFER_LIMIT - 1) width = BUFFER_LIMIT - 1;
					width -= (buffer + BUFFER_LIMIT) - dest;
					if (width > 0) {
						while (width--) {
							*--dest = padChar;
						}
					}
					StringBuilder_Append(stringBuilder, dest, 0, (buffer + BUFFER_LIMIT) - dest);
				}
				break;

			case 'e':
			case 'f':
			case 'g':
				d = va_arg(v, double);

				if (isnan(d)) {
					StringBuilder_AppendC(stringBuilder, "NaN", 0, 3);
					break;
				}
				if (isinf(d)) {
					if (d < 0.0) StringBuilder_AppendByte(stringBuilder, '-');
					StringBuilder_AppendC(stringBuilder, "inf", 0, 3);
					break;
				}

				if (d < 0.0) {
					StringBuilder_AppendByte(stringBuilder, '-');
					d = -d;
				}

				{
					Byte *dest;
					
					intPart = floor(d);
					origFracPart = fracPart = d - intPart;

					dest = buffer + BUFFER_LIMIT;
					while (intPart >= 1.0 && dest > buffer) {
						Byte digit = (Byte)(fmod(intPart, 10.0));
						intPart /= 10.0;
						*--dest = digit + '0';
					}
					if (dest == buffer + BUFFER_LIMIT)
						*--dest = '0';

					StringBuilder_Append(stringBuilder, dest, 0, (buffer + BUFFER_LIMIT) - dest);

					dest = buffer;
					while (fracPart > 0.0 && dest < buffer + 6) {
						double realDigit;
						fracPart *= 10.0;
						realDigit = floor(fracPart);
						fracPart -= realDigit;
						*dest++ = (Byte)realDigit + '0';
					}

					if (origFracPart == 0.0 || fracPart == 0.0) {
						// Truncate unnecessary trailing zeros.
						while (dest > buffer && dest[-1] == '0') dest--;
					}

					if (dest > buffer) {
						StringBuilder_AppendByte(stringBuilder, '.');
						StringBuilder_Append(stringBuilder, buffer, 0, dest - buffer);
					}
				}
				break;

			default:
				// Unknown format mode.
				StringBuilder_AppendByte(stringBuilder, '%');
				if (mode >= 0)
					StringBuilder_AppendByte(stringBuilder, (Byte)mode);
				break;
		}

		start = i;
	}

	if (i > start)
	{
		// Add the rest of the characters to the output.
		StringBuilder_Append(stringBuilder, text, start, i - start);
	}
}
