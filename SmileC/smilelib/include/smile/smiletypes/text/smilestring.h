
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
