
#ifndef __SMILE_SMILETYPES_RAW_SMILEBYTEARRAY_H__
#define __SMILE_SMILETYPES_RAW_SMILEBYTEARRAY_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileByteArrayInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileObject securityKey;
	Int length;
	Byte *data;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileByteArray_VTable_ReadOnly;
SMILE_API_DATA SmileVTable SmileByteArray_VTable_ReadWrite;

SMILE_API_FUNC SmileByteArray SmileByteArray_Create(SmileObject base, Int length, Bool writable);
SMILE_API_FUNC SmileByteArray SmileByteArray_CreateInternal(SmileObject base, Byte *buffer, Int length, Bool writable);
SMILE_API_FUNC void SmileByteArray_Resize(SmileByteArray byteArray, Int length);

SMILE_API_FUNC String String_CreateFromPartialByteArray(const SmileByteArray byteArray, Int start, Int length);
SMILE_API_FUNC SmileByteArray String_ToPartialByteArray(const String str, Int start, Int length);

/// <summary>
/// Create a string from a full byte array.  This requires O(n) time and space to copy the
/// bytes.
/// </summary>
Inline String String_CreateFromByteArray(const SmileByteArray byteArray)
{
	return String_Create(byteArray->data, byteArray->length);
}

/// <summary>
/// Convert a string to a read-only ByteArray.  This is a constant-time operation (it
/// includes an allocation of the ByteArray object, but it's otherwise a constant-time
/// operation relative to the length of the string).
/// </summary>
Inline SmileByteArray String_ToByteArray(const String str)
{
	return SmileByteArray_CreateInternal((SmileObject)Smile_KnownBases.ByteArray,
		(Byte *)String_GetBytes(str), String_Length(str), False);
}

#endif
