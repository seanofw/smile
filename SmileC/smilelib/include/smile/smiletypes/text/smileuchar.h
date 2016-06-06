
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEUCHAR_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEUCHAR_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileUCharInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	UInt32 value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileUChar_VTable;

SMILE_API_FUNC SmileUChar SmileUChar_CreateInternal(UInt32 value);

SMILE_API_FUNC Bool SmileUChar_CompareEqual(SmileUChar self, SmileObject other);
SMILE_API_FUNC UInt32 SmileUChar_Hash(SmileUChar self);
SMILE_API_FUNC void SmileUChar_SetSecurity(SmileUChar self, Int security, SmileObject securityKey);
SMILE_API_FUNC Int SmileUChar_GetSecurity(SmileUChar self);
SMILE_API_FUNC SmileObject SmileUChar_GetProperty(SmileUChar self, Symbol propertyName);
SMILE_API_FUNC void SmileUChar_SetProperty(SmileUChar self, Symbol propertyName, SmileObject value);
SMILE_API_FUNC Bool SmileUChar_HasProperty(SmileUChar self, Symbol propertyName);
SMILE_API_FUNC SmileList SmileUChar_GetPropertyNames(SmileUChar self);
SMILE_API_FUNC Bool SmileUChar_ToBool(SmileUChar self);
SMILE_API_FUNC Int32 SmileUChar_ToInteger32(SmileUChar self);
SMILE_API_FUNC Real64 SmileUChar_ToReal64(SmileUChar self);
SMILE_API_FUNC String SmileUChar_ToString(SmileUChar self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileUChar SmileUChar_Create(UInt32 value)
{
	if (value <= 255)
		return Smile_KnownObjects.UChars[value];
	else
		return SmileUChar_CreateInternal(value);
}

#endif
