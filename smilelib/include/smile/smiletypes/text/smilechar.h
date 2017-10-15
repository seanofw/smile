
#ifndef __SMILE_SMILETYPES_TEXT_SMILECHAR_H__
#define __SMILE_SMILETYPES_TEXT_SMILECHAR_H__

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

struct SmileCharInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Byte ch;
};

struct SmileUnboxedCharInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileChar_VTable;
SMILE_API_FUNC SmileChar SmileChar_Init(SmileChar smileChar, Byte ch);

SMILE_API_DATA SmileVTable SmileUnboxedChar_VTable;
SMILE_API_DATA SmileUnboxedChar SmileUnboxedChar_Instance;

Inline SmileChar SmileChar_Create(Byte ch)
{
	return Smile_KnownObjects.Chars[ch];
}

Inline SmileArg SmileUnboxedChar_From(Byte ch)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedChar_Instance;
	arg.unboxed.ch = ch;
	return arg;
}

#endif
