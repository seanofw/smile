//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Command-Line Runner)
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

#include "stdafx.h"

#include <smile/platform/windows/ansi-console.h>

#include "style.h"

/// <summary>
/// Implementation of vscprinf(), since not all platforms have an equivalent.
/// </summary>
static int vscprintf(const char *format, va_list pargs)
{
	int retval;
	va_list argcopy;

	va_copy(argcopy, pargs);
	retval = vsnprintf(NULL, 0, format, argcopy);
	va_end(argcopy);

	return retval;
}

/// <summary>
/// Write a string that contains ANSI escape code stylings to the output, but strip
/// the escape codes if this isn't a TTY.  This will effectively colorize the output
/// on a real console, and leave all the ANSI codes out if the output is being piped
/// somewhere (like a file).
/// </summary>
/// <param name="fp">The file to write the ANSI escape code to.</param>
/// <param name="string">The string containing ANSI escape code stylings to write.</param>
/// <param name="size">The size of each item to write.</param>
/// <param name="count">The count of items to write.</param>
size_t fwrite_styled(const char *string, size_t size, size_t count, FILE *fp)
{
	const char *ptr, *end;
	size_t result;

	// If this is a TTY, write the output as given.
	if (_isatty(fileno(fp))) {
#		if _WIN32
			return fwrite_ansi_win32(string, size, count, fp);
#		else
			return fwrite(string, size, count, fp);
#		endif
	}

	// Find the extent of the string.
	ptr = string;
	end = string + (size * count);

	// Begin counting characters.
	result = 0;

	// Not a TTY, so strip escape codes from the output.
	while (ptr < end) {
		if (*ptr == '\033') {
			// Got an escape.  Swallow characters until we reach an alphabetic letter.
			char ch;
			while (ptr < end
				&& !(((ch = *ptr) >= 'a' && ch <= 'z')
					|| (ch >= 'A' && ch <= 'Z')))
				ptr++;
		}
		else {
			// Find the extent of this sequence of characters that *isn't* escape codes.
			const char *start = ptr;
			while (ptr < end && *ptr != '\033') ptr++;

			// Write the whole sequence.
			if (ptr > start) {
				size_t count;
				
#				if _WIN32
					count = fwrite_ansi_win32(start, 1, ptr - start, fp);
#				else
					count = fwrite(start, 1, ptr - start, fp);
#				endif
				if (count < 0) return count;

				result += count;
			}
		}
	}

	return result;
}

/// <summary>
/// Printf a string that contains ANSI escape code stylings to the output, but strip
/// the escape codes if this isn't a TTY.  This will effectively colorize the output
/// on a real console, and leave all the ANSI codes out if the output is being piped
/// somewhere (like a file).
/// </summary>
/// <param name="fp">The file to print the ANSI escape code to.</param>
/// <param name="format">The format string.</param>
size_t fprintf_styled(FILE *fp, const char *format, ...)
{
	va_list v;
	size_t result;

	va_start(v, format);
	result = vfprintf_styled(fp, format, v);
	va_end(v);

	return result;
}

/// <summary>
/// Printf a string that contains ANSI escape code stylings to the output, but strip
/// the escape codes if this isn't a TTY.  This will effectively colorize the output
/// on a real console, and leave all the ANSI codes out if the output is being piped
/// somewhere (like a file).
/// </summary>
/// <param name="fp">The file to print the ANSI escape code to.</param>
/// <param name="format">The format string.</param>
/// <param name="v">The arguments to format.</param>
size_t vfprintf_styled(FILE *fp, const char *format, va_list v)
{
	char localBuf[256];
	char *buf;
	int length;
	size_t result;

	// Find out how long the string will be.
	length = vscprintf(format, v);

	// Allocate enough memory to format it properly.
	if (length > 255) {
		buf = malloc(length + 1);
		if (buf == NULL)
			Smile_Abort_OutOfMemory();
	}
	else buf = localBuf;

	// Format it into the buffer.
	vsprintf(buf, format, v);

	// Write it for real.
	result = fwrite_styled(buf, 1, length, fp);

	// Free any allocated memory, if we needed any.
	if (buf != localBuf)
		free(buf);

	return result;
}

/// <summary>
/// Printf a string that contains ANSI escape code stylings to the output, but strip
/// the escape codes if this isn't a TTY.  This will effectively colorize the output
/// on a real console, and leave all the ANSI codes out if the output is being piped
/// somewhere (like a file).
/// </summary>
/// <param name="format">The format string.</param>
size_t printf_styled(const char *format, ...)
{
	va_list v;
	size_t result;

	va_start(v, format);
	result = vfprintf_styled(stdout, format, v);
	va_end(v);

	return result;
}

/// <summary>
/// Printf a string that contains ANSI escape code stylings to the output, but strip
/// the escape codes if this isn't a TTY.  This will effectively colorize the output
/// on a real console, and leave all the ANSI codes out if the output is being piped
/// somewhere (like a file).
/// </summary>
/// <param name="format">The format string.</param>
/// <param name="v">The arguments to format.</param>
size_t vprintf_styled(const char *format, va_list v)
{
	return vfprintf_styled(stdout, format, v);
}
