
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

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileByte_VTable;

SMILE_API_FUNC SmileByte SmileByte_CreateInternal(Byte value);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileByte SmileByte_Create(Byte value)
{
	return Smile_KnownObjects.Bytes[value];
}

#endif
