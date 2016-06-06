
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

SMILE_API_FUNC Bool SmileByte_CompareEqual(SmileByte self, SmileObject other);
SMILE_API_FUNC UInt32 SmileByte_Hash(SmileByte self);
SMILE_API_FUNC void SmileByte_SetSecurity(SmileByte self, Int security, SmileObject securityKey);
SMILE_API_FUNC Int SmileByte_GetSecurity(SmileByte self);
SMILE_API_FUNC SmileObject SmileByte_GetProperty(SmileByte self, Symbol propertyName);
SMILE_API_FUNC void SmileByte_SetProperty(SmileByte self, Symbol propertyName, SmileObject value);
SMILE_API_FUNC Bool SmileByte_HasProperty(SmileByte self, Symbol propertyName);
SMILE_API_FUNC SmileList SmileByte_GetPropertyNames(SmileByte self);
SMILE_API_FUNC Bool SmileByte_ToBool(SmileByte self);
SMILE_API_FUNC Int32 SmileByte_ToInteger32(SmileByte self);
SMILE_API_FUNC Real64 SmileByte_ToReal64(SmileByte self);
SMILE_API_FUNC String SmileByte_ToString(SmileByte self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileByte SmileByte_Create(Byte value)
{
	return Smile_KnownObjects.Bytes[value];
}

#endif
