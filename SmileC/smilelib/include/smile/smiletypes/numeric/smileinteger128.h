
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER128_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER128_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger128Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int128 value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger128_VTable;

SMILE_API_FUNC SmileInteger128 SmileInteger128_CreateInternal(Int128 value);

SMILE_API_FUNC Bool SmileInteger128_CompareEqual(SmileInteger128 self, SmileObject other);
SMILE_API_FUNC UInt32 SmileInteger128_Hash(SmileInteger128 self);
SMILE_API_FUNC void SmileInteger128_SetSecurity(SmileInteger128 self, Int security, SmileObject securityKey);
SMILE_API_FUNC Int SmileInteger128_GetSecurity(SmileInteger128 self);
SMILE_API_FUNC SmileObject SmileInteger128_GetProperty(SmileInteger128 self, Symbol propertyName);
SMILE_API_FUNC void SmileInteger128_SetProperty(SmileInteger128 self, Symbol propertyName, SmileObject value);
SMILE_API_FUNC Bool SmileInteger128_HasProperty(SmileInteger128 self, Symbol propertyName);
SMILE_API_FUNC SmileList SmileInteger128_GetPropertyNames(SmileInteger128 self);
SMILE_API_FUNC Bool SmileInteger128_ToBool(SmileInteger128 self);
SMILE_API_FUNC Int32 SmileInteger128_ToInteger32(SmileInteger128 self);
SMILE_API_FUNC Real64 SmileInteger128_ToReal64(SmileInteger128 self);
SMILE_API_FUNC String SmileInteger128_ToString(SmileInteger128 self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger128 SmileInteger128_Create(Int128 value)
{
	if (value.hi == 0 && value.lo <= 100)
		return Smile_KnownObjects.SmallInt128s[(Int)(value.lo + 100)];
	else if (value.hi == -1 && (Int64)value.lo >= -100)
		return Smile_KnownObjects.SmallInt128s[(Int)((Int64)value.lo + 100)];
	else
		return SmileInteger128_CreateInternal(value);
}

#endif
