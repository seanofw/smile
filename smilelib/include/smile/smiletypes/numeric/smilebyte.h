
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEBYTE_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEBYTE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileByteInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Byte value;
};

struct SmileUnboxedByteInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileByte_VTable;
SMILE_API_DATA SmileVTable SmileUnboxedByte_VTable;

SMILE_API_FUNC SmileByte SmileByte_InitInternal(SmileByte smileByte, Byte value);
SMILE_API_DATA SmileUnboxedByte SmileUnboxedByte_Instance;

Inline SmileArg SmileUnboxedByte_From(Byte value)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedByte_Instance;
	arg.unboxed.i8 = value;
	return arg;
}

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileByte SmileByte_Create(Byte value)
{
	return Smile_KnownObjects.Bytes[value];
}

#endif
