
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

SMILE_API SmileVTable SmileString_VTable;

SMILE_API SmileString SmileString_Create(String str);

SMILE_API Bool SmileString_CompareEqual(SmileString self, SmileObject other);
SMILE_API UInt32 SmileString_Hash(SmileString self);
SMILE_API void SmileString_SetSecurity(SmileString self, Int security);
SMILE_API Int SmileString_GetSecurity(SmileString self);
SMILE_API SmileObject SmileString_GetProperty(SmileString self, Symbol propertyName);
SMILE_API void SmileString_SetProperty(SmileString self, Symbol propertyName, SmileObject value);
SMILE_API Bool SmileString_HasProperty(SmileString self, Symbol propertyName);
SMILE_API SmileList SmileString_GetPropertyNames(SmileString self);
SMILE_API Bool SmileString_ToBool(SmileString self);
SMILE_API Int32 SmileString_ToInteger32(SmileString self);
SMILE_API Real64 SmileString_ToReal64(SmileString self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileString SmileString_CreateC(const char *text)
{
	String str = String_FromC(text);
	return SmileString_Create(str);
}

Inline String SmileString_ToString(SmileString str)
{
	return (String)&(str->string);
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
