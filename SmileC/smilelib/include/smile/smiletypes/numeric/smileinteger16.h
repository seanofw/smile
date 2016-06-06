
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER16_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER16_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger16Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int16 value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger16_VTable;

SMILE_API_FUNC SmileInteger16 SmileInteger16_CreateInternal(Int16 value);

SMILE_API_FUNC Bool SmileInteger16_CompareEqual(SmileInteger16 self, SmileObject other);
SMILE_API_FUNC UInt32 SmileInteger16_Hash(SmileInteger16 self);
SMILE_API_FUNC void SmileInteger16_SetSecurity(SmileInteger16 self, Int security, SmileObject securityKey);
SMILE_API_FUNC Int SmileInteger16_GetSecurity(SmileInteger16 self);
SMILE_API_FUNC SmileObject SmileInteger16_GetProperty(SmileInteger16 self, Symbol propertyName);
SMILE_API_FUNC void SmileInteger16_SetProperty(SmileInteger16 self, Symbol propertyName, SmileObject value);
SMILE_API_FUNC Bool SmileInteger16_HasProperty(SmileInteger16 self, Symbol propertyName);
SMILE_API_FUNC SmileList SmileInteger16_GetPropertyNames(SmileInteger16 self);
SMILE_API_FUNC Bool SmileInteger16_ToBool(SmileInteger16 self);
SMILE_API_FUNC Int32 SmileInteger16_ToInteger32(SmileInteger16 self);
SMILE_API_FUNC Real64 SmileInteger16_ToReal64(SmileInteger16 self);
SMILE_API_FUNC String SmileInteger16_ToString(SmileInteger16 self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger16 SmileInteger16_Create(Int16 value)
{
	if (value >= -100 && value <= 100)
		return Smile_KnownObjects.SmallInt16s[value + 100];
	else
		return SmileInteger16_CreateInternal(value);
}

#endif
