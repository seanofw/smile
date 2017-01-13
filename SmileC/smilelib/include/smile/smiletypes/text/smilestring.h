
#ifndef __SMILE_SMILETYPES_TEXT_SMILESTRING_H__
#define __SMILE_SMILETYPES_TEXT_SMILESTRING_H__

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

struct SmileStringInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	struct StringInt string;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileString_VTable;

SMILE_API_FUNC SmileString SmileString_Create(String str);

SMILE_API_FUNC Bool SmileString_CompareEqual(SmileString self, SmileObject other);
SMILE_API_FUNC UInt32 SmileString_Hash(SmileString self);
SMILE_API_FUNC void SmileString_SetSecurity(SmileString self, Int security, SmileObject securityKey);
SMILE_API_FUNC Int SmileString_GetSecurity(SmileString self);
SMILE_API_FUNC SmileObject SmileString_GetProperty(SmileString self, Symbol propertyName);
SMILE_API_FUNC void SmileString_SetProperty(SmileString self, Symbol propertyName, SmileObject value);
SMILE_API_FUNC Bool SmileString_HasProperty(SmileString self, Symbol propertyName);
SMILE_API_FUNC SmileList SmileString_GetPropertyNames(SmileString self);
SMILE_API_FUNC Bool SmileString_ToBool(SmileString self);
SMILE_API_FUNC Int32 SmileString_ToInteger32(SmileString self);
SMILE_API_FUNC Real64 SmileString_ToReal64(SmileString self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

#define SmileString_GetString(__str__) \
	((String)&((__str__)->string))

Inline SmileString SmileString_CreateC(const char *text)
{
	String str = String_FromC(text);
	return SmileString_Create(str);
}

Inline String SmileString_ToString(SmileString str)
{
	return String_Format("\"%S\"", String_AddCSlashes((String)&(str->string)));
}

Inline const char *SmileString_ToC(SmileString str)
{
	return (const char *)str->string.text;
}

Inline Int SmileString_Length(SmileString str)
{
	return str->string.length;
}

#endif
