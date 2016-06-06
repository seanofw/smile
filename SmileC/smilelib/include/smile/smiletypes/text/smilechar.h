
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILECHAR_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILECHAR_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileCharInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Byte value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileChar_VTable;

SMILE_API_FUNC SmileChar SmileChar_CreateInternal(Byte value);

SMILE_API_FUNC Bool SmileChar_CompareEqual(SmileChar self, SmileObject other);
SMILE_API_FUNC UInt32 SmileChar_Hash(SmileChar self);
SMILE_API_FUNC void SmileChar_SetSecurity(SmileChar self, Int security, SmileObject securityKey);
SMILE_API_FUNC Int SmileChar_GetSecurity(SmileChar self);
SMILE_API_FUNC SmileObject SmileChar_GetProperty(SmileChar self, Symbol propertyName);
SMILE_API_FUNC void SmileChar_SetProperty(SmileChar self, Symbol propertyName, SmileObject value);
SMILE_API_FUNC Bool SmileChar_HasProperty(SmileChar self, Symbol propertyName);
SMILE_API_FUNC SmileList SmileChar_GetPropertyNames(SmileChar self);
SMILE_API_FUNC Bool SmileChar_ToBool(SmileChar self);
SMILE_API_FUNC Int32 SmileChar_ToInteger32(SmileChar self);
SMILE_API_FUNC Real64 SmileChar_ToReal64(SmileChar self);
SMILE_API_FUNC String SmileChar_ToString(SmileChar self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileChar SmileChar_Create(Byte value)
{
	return Smile_KnownObjects.Chars[value];
}

#endif
