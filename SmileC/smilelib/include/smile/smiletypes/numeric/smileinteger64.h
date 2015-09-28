
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER64_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER64_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger64Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int64 value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API SmileVTable SmileInteger64_VTable;

SMILE_API SmileInteger64 SmileInteger64_CreateInternal(Int64 value);

SMILE_API Bool SmileInteger64_CompareEqual(SmileInteger64 self, SmileObject other);
SMILE_API UInt32 SmileInteger64_Hash(SmileInteger64 self);
SMILE_API void SmileInteger64_SetSecurity(SmileInteger64 self, Int security);
SMILE_API Int SmileInteger64_GetSecurity(SmileInteger64 self);
SMILE_API SmileObject SmileInteger64_GetProperty(SmileInteger64 self, Symbol propertyName);
SMILE_API void SmileInteger64_SetProperty(SmileInteger64 self, Symbol propertyName, SmileObject value);
SMILE_API Bool SmileInteger64_HasProperty(SmileInteger64 self, Symbol propertyName);
SMILE_API SmileList SmileInteger64_GetPropertyNames(SmileInteger64 self);
SMILE_API Bool SmileInteger64_ToBool(SmileInteger64 self);
SMILE_API Int32 SmileInteger64_ToInteger64(SmileInteger64 self);
SMILE_API Real64 SmileInteger64_ToReal64(SmileInteger64 self);
SMILE_API String SmileInteger64_ToString(SmileInteger64 self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger64 SmileInteger64_Create(Int64 value)
{
	if (value >= -100 && value <= 100)
		return Smile_KnownObjects.SmallInt64s[value + 100];
	else
		return SmileInteger64_CreateInternal(value);
}

#endif
