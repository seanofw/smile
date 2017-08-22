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

#include <smile/numeric/real.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/raw/smilebytearray.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/stringbuilder.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

extern SmileVTable SmileByteArray_VTable_ReadWrite;
extern SmileVTable SmileByteArray_VTable_ReadOnly;

SMILE_EASY_OBJECT_NO_CALL(SmileByteArray, "A ByteArray")
SMILE_EASY_OBJECT_NO_SOURCE(SmileByteArray)
SMILE_EASY_OBJECT_NO_UNBOX(SmileByteArray)

/// <summary>
/// Create a string from a ByteArray (or from a subset of a ByteArray).  This makes a copy
/// of the data (always), and so it requires O(n) time and space.
/// </summary>
String String_CreateFromPartialByteArray(const SmileByteArray byteArray, Int start, Int length)
{
	String string;

	// First, clip the start/length to the ByteArray.
	// This is a proper intersection computation, not an error-checking routine.
	if (start < 0)
		length += start, start = 0;
	if (start >= byteArray->length)
		return String_Empty;
	if (length > byteArray->length - start)
		length = byteArray->length - start;
	if (length <= 0)
		return String_Empty;

	// There's a nonempty array of bytes, so create a string from it.
	string = String_Create(byteArray->data + start, length);
	return string;
}

/// <summary>
/// Convert part of a string to a read-only ByteArray.  This is a constant-time operation (it
/// includes an allocation of the ByteArray object, but it's otherwise a constant-time
/// operation relative to the length of the string).  This performs proper intersection
/// computations for the provided start/length.
/// </summary>
SmileByteArray String_ToPartialByteArray(const String str, Int start, Int length)
{
	Int strLen = String_Length(str);

	// First, clip the start/length to the ByteArray.
	// This is a proper intersection computation, not an error-checking routine.
	if (start < 0)
		length += start, start = 0;
	if (start >= strLen)
		return SmileByteArray_CreateInternal((SmileObject)Smile_KnownBases.ByteArray, NULL, 0, False);
	if (length > strLen - start)
		length = strLen - start;
	if (length <= 0)
		return SmileByteArray_CreateInternal((SmileObject)Smile_KnownBases.ByteArray, NULL, 0, False);

	// There's a nonempty array of bytes, so create a string from it.
	return SmileByteArray_CreateInternal((SmileObject)Smile_KnownBases.ByteArray,
		(Byte *)String_GetBytes(str) + start, length, False);
}

SMILE_API_FUNC SmileByteArray StringBuilder_ToByteArray(StringBuilder stringBuilder)
{
	struct StringBuilderInt *sb = (struct StringBuilderInt *)stringBuilder;
	SmileByteArray byteArray = SmileByteArray_Create((SmileObject)Smile_KnownBases.ByteArray, sb->length, True);
	MemCpy(byteArray->data, sb->text, sb->length);
	return byteArray;
}

/// <summary>
/// Create a new, empty ByteArray, with all zero bytes.
/// </summary>
/// <param name="base">The base type this ByteArray inherits from.</param>
/// <param name="length">The number of bytes in the ByteArray, which must be nonnegative.</param>
/// <param name="writable">Whether Smile programs will see this ByteArray as writable or as read-only.</param>
SmileByteArray SmileByteArray_Create(SmileObject base, Int length, Bool writable)
{
	Byte *buffer;
	STATIC_STRING(PrivateKey, "");

	if (length < 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot create a ByteArray of negative size."));
	}

	buffer = length > 0 ? GC_MALLOC_BYTES(length) : NULL;

	return SmileByteArray_CreateInternal(base, buffer, length, writable);
}

/// <summary>
/// Create a new, empty ByteArray, based on the given data buffer.
/// </summary>
/// <param name="base">The base type this ByteArray inherits from.</param>
/// <param name="buffer">The source buffer this ByteArray comes from.</param>
/// <param name="length">The number of bytes in the ByteArray, which must be nonnegative.</param>
/// <param name="writable">Whether Smile programs will see this ByteArray as writable or as read-only.</param>
SmileByteArray SmileByteArray_CreateInternal(SmileObject base, Byte *buffer, Int length, Bool writable)
{
	SmileByteArray byteArray;
	STATIC_STRING(PrivateKey, "");

	byteArray = GC_MALLOC_STRUCT(struct SmileByteArrayInt);
	if (byteArray == NULL) Smile_Abort_OutOfMemory();

	byteArray->base = base;
	byteArray->kind = writable ? (SMILE_KIND_BYTEARRAY | SMILE_SECURITY_WRITABLE) : (SMILE_KIND_BYTEARRAY | SMILE_SECURITY_READONLY);
	byteArray->vtable = writable ? SmileByteArray_VTable_ReadWrite : SmileByteArray_VTable_ReadOnly;
	byteArray->securityKey = writable ? NullObject : (SmileObject)PrivateKey;
	byteArray->data = buffer;
	byteArray->length = length;

	return byteArray;
}

/// <summary>
/// Resize this ByteArray to a new length, adding zero bytes as needed.  This will not work on a read-only ByteArray.
/// </summary>
/// <param name="byteArray">The ByteArray to resize.</param>
/// <param name="length">The new number of bytes in the ByteArray, which must be nonnegative.</param>
void SmileByteArray_Resize(SmileByteArray byteArray, Int length)
{
	void *newBuffer;

	if (byteArray->length == length) return;

	if (!(byteArray->kind & SMILE_SECURITY_WRITABLE)) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot resize a read-only ByteArray."));
	}
	if (length < 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot resize a ByteArray to negative size."));
	}

	if (length == 0) {
		byteArray->data = NULL;
		return;
	}

	// Don't resize to something larger than size_t can represent.
	if ((UInt)length >= PtrIntMax)
		Smile_Abort_OutOfMemory();

	// Do the actual resizing (probably by allocating new memory).
	newBuffer = GC_REALLOC(byteArray->data, (size_t)length);
	if (newBuffer == NULL)
		Smile_Abort_OutOfMemory();

	// Update the result.
	byteArray->data = (Byte *)newBuffer;
	byteArray->length = length;
}

void SmileByteArray_SetSecurityKey(SmileByteArray self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	Bool isValidSecurityKey = self->securityKey->vtable->compareEqual(self->securityKey, (SmileUnboxedData) { 0 }, oldSecurityKey, (SmileUnboxedData) { 0 });
	if (!isValidSecurityKey)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, String_InvalidSecurityKey);

	self->securityKey = newSecurityKey;
}

void SmileByteArray_SetSecurity(SmileByteArray self, Int security, SmileObject securityKey)
{
	Bool isValidSecurityKey = self->securityKey->vtable->compareEqual(self->securityKey, (SmileUnboxedData) { 0 }, securityKey, (SmileUnboxedData) { 0 });
	if (!isValidSecurityKey)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, String_InvalidSecurityKey);

	self->kind = (self->kind & ~SMILE_SECURITY_READWRITEAPPEND) | (security & SMILE_SECURITY_READWRITEAPPEND);

	switch (security & SMILE_SECURITY_READWRITEAPPEND) {
	case SMILE_SECURITY_READONLY:
		self->vtable = SmileByteArray_VTable_ReadOnly;
		break;
	case SMILE_SECURITY_WRITABLE:
		self->vtable = SmileByteArray_VTable_ReadWrite;
		break;
	case SMILE_SECURITY_APPENDABLE:
	case SMILE_SECURITY_READWRITEAPPEND:
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, String_FromC("Cannot set a ByteArray to be appendable."));
		break;
	}
}

Int SmileByteArray_GetSecurity(SmileByteArray self)
{
	return self->kind & SMILE_SECURITY_READWRITEAPPEND;
}


UInt32 SmileByteArray_Hash(SmileByteArray self)
{
	return ((PtrInt)self & 0xFFFFFFFF) ^ Smile_HashOracle;
}

Bool SmileByteArray_CompareEqual(SmileByteArray self, SmileUnboxedData selfUnboxed, SmileObject other, SmileUnboxedData otherUnboxed)
{
	return ((SmileObject)self == other);
}

Bool SmileByteArray_DeepEqual(SmileByteArray self, SmileUnboxedData selfUnboxed, SmileObject other, SmileUnboxedData otherUnboxed, PointerSet visitedPointers)
{
	SmileByteArray otherByteArray;
	UNUSED(visitedPointers);

	if (SMILE_KIND(other) != SMILE_KIND_BYTEARRAY) return False;
	otherByteArray = (SmileByteArray)other;

	if (self->length != otherByteArray->length) return False;

	return !MemCmp(self->data, otherByteArray->data, self->length);
}

SmileObject SmileByteArray_GetProperty(SmileByteArray self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.length) {
		return (SmileObject)SmileInteger64_Create(self->length);
	}
	return self->base->vtable->getProperty(self->base, propertyName);
}

void SmileByteArray_SetProperty_ReadWrite(SmileByteArray self, Symbol propertyName, SmileObject value)
{
	Int newLength;

	if (propertyName != Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.object_security_error,
			String_Format("Cannot set property \"%S\" on a ByteArray.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
		return;
	}

	if (SMILE_KIND(value) == SMILE_KIND_INTEGER64) {
		newLength = (Int)((SmileInteger64)value)->value;
		SmileByteArray_Resize(self, newLength);
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.object_security_error,
			String_Format("Cannot set property \"%S\" on a ByteArray from a value that is not an Integer64.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

void SmileByteArray_SetProperty_ReadOnly(SmileByteArray self, Symbol propertyName, SmileObject value)
{
	if (propertyName != Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.object_security_error,
			String_Format("Cannot set property \"%S\" on a ByteArray.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
		return;
	}

	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a read-only ByteArray.",
			SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileByteArray_HasProperty(SmileByteArray self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.length);
}

SmileList SmileByteArray_GetPropertyNames(SmileByteArray self)
{
	SmileList head, tail;

	LIST_INIT(head, tail);

	UNUSED(self);

	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.length));

	return head;
}

Bool SmileByteArray_ToBool(SmileByteArray self, SmileUnboxedData unboxedData)
{
	return True;
}

Int32 SmileByteArray_ToInteger32(SmileByteArray self, SmileUnboxedData unboxedData)
{
	return (Int32)self->length;
}

Float64 SmileByteArray_ToFloat64(SmileByteArray self, SmileUnboxedData unboxedData)
{
	return (Float64)self->length;
}

Real64 SmileByteArray_ToReal64(SmileByteArray self, SmileUnboxedData unboxedData)
{
	return Real64_FromInt64(self->length);
}

String SmileByteArray_ToString(SmileByteArray self, SmileUnboxedData unboxedData)
{
	return String_Create(self->data, self->length);
}

SMILE_VTABLE(SmileByteArray_VTable_ReadWrite, SmileByteArray)
{
	SmileByteArray_CompareEqual,
	SmileByteArray_DeepEqual,
	SmileByteArray_Hash,

	SmileByteArray_SetSecurityKey,
	SmileByteArray_SetSecurity,
	SmileByteArray_GetSecurity,

	SmileByteArray_GetProperty,
	SmileByteArray_SetProperty_ReadWrite,
	SmileByteArray_HasProperty,
	SmileByteArray_GetPropertyNames,

	SmileByteArray_ToBool,
	SmileByteArray_ToString,

	SmileByteArray_Call,
	SmileByteArray_GetSourceLocation,
};

SMILE_VTABLE(SmileByteArray_VTable_ReadOnly, SmileByteArray)
{
	SmileByteArray_CompareEqual,
	SmileByteArray_DeepEqual,
	SmileByteArray_Hash,

	SmileByteArray_SetSecurityKey,
	SmileByteArray_SetSecurity,
	SmileByteArray_GetSecurity,

	SmileByteArray_GetProperty,
	SmileByteArray_SetProperty_ReadOnly,
	SmileByteArray_HasProperty,
	SmileByteArray_GetPropertyNames,

	SmileByteArray_ToBool,
	SmileByteArray_ToString,

	SmileByteArray_Call,
	SmileByteArray_GetSourceLocation,
};
