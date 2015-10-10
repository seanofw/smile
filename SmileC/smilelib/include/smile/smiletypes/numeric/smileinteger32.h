
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER32_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER32_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger32Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int32 value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API SmileVTable SmileInteger32_VTable;

SMILE_API SmileInteger32 SmileInteger32_CreateInternal(Int32 value);

SMILE_API Bool SmileInteger32_CompareEqual(SmileInteger32 self, SmileObject other);
SMILE_API UInt32 SmileInteger32_Hash(SmileInteger32 self);
SMILE_API void SmileInteger32_SetSecurity(SmileInteger32 self, Int security, SmileObject securityKey);
SMILE_API Int SmileInteger32_GetSecurity(SmileInteger32 self);
SMILE_API SmileObject SmileInteger32_GetProperty(SmileInteger32 self, Symbol propertyName);
SMILE_API void SmileInteger32_SetProperty(SmileInteger32 self, Symbol propertyName, SmileObject value);
SMILE_API Bool SmileInteger32_HasProperty(SmileInteger32 self, Symbol propertyName);
SMILE_API SmileList SmileInteger32_GetPropertyNames(SmileInteger32 self);
SMILE_API Bool SmileInteger32_ToBool(SmileInteger32 self);
SMILE_API Int32 SmileInteger32_ToInteger32(SmileInteger32 self);
SMILE_API Real64 SmileInteger32_ToReal64(SmileInteger32 self);
SMILE_API String SmileInteger32_ToString(SmileInteger32 self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger32 SmileInteger32_Create(Int32 value)
{
	if (value >= -100 && value <= 100)
		return Smile_KnownObjects.SmallInt32s[value + 100];
	else
		return SmileInteger32_CreateInternal(value);
}

#endif
