
#ifndef __SMILE_SMILETYPES_SMILEBOOL_H__
#define __SMILE_SMILETYPES_SMILEBOOL_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileBoolInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Bool value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileBool_VTable;

SMILE_API_FUNC SmileBool SmileBool_Create(Bool value);

SMILE_API_FUNC Bool SmileBool_CompareEqual(SmileBool self, SmileObject other);
SMILE_API_FUNC UInt32 SmileBool_Hash(SmileBool self);
SMILE_API_FUNC void SmileBool_SetSecurity(SmileBool self, Int security, SmileObject securityKey);
SMILE_API_FUNC Int SmileBool_GetSecurity(SmileBool self);
SMILE_API_FUNC SmileObject SmileBool_GetProperty(SmileBool self, Symbol propertyName);
SMILE_API_FUNC void SmileBool_SetProperty(SmileBool self, Symbol propertyName, SmileObject value);
SMILE_API_FUNC Bool SmileBool_HasProperty(SmileBool self, Symbol propertyName);
SMILE_API_FUNC SmileList SmileBool_GetPropertyNames(SmileBool self);
SMILE_API_FUNC Bool SmileBool_ToBool(SmileBool self);
SMILE_API_FUNC Int32 SmileBool_ToInteger32(SmileBool self);
SMILE_API_FUNC Real64 SmileBool_ToReal64(SmileBool self);
SMILE_API_FUNC String SmileBool_ToString(SmileBool self);

#endif
