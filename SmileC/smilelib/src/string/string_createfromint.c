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

#define BUFFER_SIZE 128

static const Byte _digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static Byte *String_CreateFromIntegerInternal(Byte *buffer, UInt64 value, UInt32 numericBase)
{
	Byte *dest = buffer + BUFFER_SIZE;
	UInt64 digit;

	if (value == 0) {
		*--dest = '0';
		return dest;
	}

	while (value) {
		digit = value % numericBase;
		value /= numericBase;
		*--dest = _digits[(UInt)digit];
	}

	return dest;
}

String String_CreateFromInteger(Int64 value, Int numericBase, Bool includeSign)
{
	Byte buffer[BUFFER_SIZE];
	Byte *dest;
	Bool negative;

	if (numericBase < 2 || numericBase > 36)
		return NULL;

	if (value < 0) {
		negative = True;
		value = -value;
	}
	else {
		negative = False;
	}

	dest = String_CreateFromIntegerInternal(buffer, (UInt64)value, (UInt32)numericBase);

	if (negative) {
		*--dest = '-';
	}
	else if (includeSign) {
		*--dest = '+';
	}

	return String_Create(dest, (buffer + BUFFER_SIZE) - dest);
}

String String_CreateFromUnsignedInteger(UInt64 value, Int numericBase)
{
	Byte buffer[BUFFER_SIZE];
	Byte *dest;

	if (numericBase < 2 || numericBase > 36)
		return NULL;

	dest = String_CreateFromIntegerInternal(buffer, value, (UInt32)numericBase);

	return String_Create(dest, (buffer + BUFFER_SIZE) - dest);
}
