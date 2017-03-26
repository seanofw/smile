
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
//  Public interface

SMILE_API_DATA SmileVTable SmileString_VTable;

struct SmileStringInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	struct {
		Int length;
		Byte text[1024];
	} _opaque;
};

Inline SmileString SmileString_Create(String str)
{
	return (SmileString)str;
}

Inline String SmileString_GetString(SmileString str)
{
	return (String)str;
}

Inline SmileString SmileString_CreateC(const char *text)
{
	return (SmileString)String_FromC(text);
}

Inline const char *SmileString_ToC(SmileString str)
{
	return String_ToC((String)str);
}

Inline Int SmileString_Length(SmileString str)
{
	return String_Length((String)str);
}

#endif
