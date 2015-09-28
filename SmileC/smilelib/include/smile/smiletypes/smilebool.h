
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

SMILE_API SmileVTable SmileBool_VTable;

SMILE_API SmileBool SmileBool_Create(Bool value);

SMILE_API Bool SmileBool_CompareEqual(SmileBool self, SmileObject other);
SMILE_API UInt32 SmileBool_Hash(SmileBool self);
SMILE_API void SmileBool_SetSecurity(SmileBool self, Int security);
SMILE_API Int SmileBool_GetSecurity(SmileBool self);
SMILE_API SmileObject SmileBool_GetProperty(SmileBool self, Symbol propertyName);
SMILE_API void SmileBool_SetProperty(SmileBool self, Symbol propertyName, SmileObject value);
SMILE_API Bool SmileBool_HasProperty(SmileBool self, Symbol propertyName);
SMILE_API SmileList SmileBool_GetPropertyNames(SmileBool self);
SMILE_API Bool SmileBool_ToBool(SmileBool self);
SMILE_API Int32 SmileBool_ToInteger32(SmileBool self);
SMILE_API Real64 SmileBool_ToReal64(SmileBool self);
SMILE_API String SmileBool_ToString(SmileBool self);

#endif
