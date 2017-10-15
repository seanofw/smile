#ifndef __SMILE_SMILETYPES_TEXT_SMILEUNI_H__
#define __SMILE_SMILETYPES_TEXT_SMILEUNI_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_ENV_KNOWNOBJECTS_H__
#include <smile/env/knownobjects.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileUniInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	UInt32 code;
};

struct SmileUnboxedUniInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileUni_VTable;
SMILE_API_FUNC SmileUni SmileUni_CreateInternal(UInt32 code);
SMILE_API_FUNC SmileUni SmileUni_Init(SmileUni uni, UInt32 code);

SMILE_API_DATA SmileVTable SmileUnboxedUni_VTable;
SMILE_API_DATA SmileUnboxedUni SmileUnboxedUni_Instance;

Inline SmileUni SmileUni_Create(UInt32 code)
{
	if (code < 1024)
		return Smile_KnownObjects.Unis[code];
	else
		return SmileUni_CreateInternal(code);
}

Inline SmileArg SmileUnboxedUni_FromSafeInt64(Int64 code)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedUni_Instance;
	arg.unboxed.uni = (code < 0 || code >= 0x110000) ? 0xFFFD : (UInt32)code;
	return arg;
}

Inline SmileArg SmileUnboxedUni_FromSafeInt32(Int32 code)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedUni_Instance;
	arg.unboxed.uni = (code < 0 || code >= 0x110000) ? 0xFFFD : (UInt32)code;
	return arg;
}

Inline SmileArg SmileUnboxedUni_FromSafeInt(Int code)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedUni_Instance;
	arg.unboxed.uni = (code < 0 || code >= 0x110000) ? 0xFFFD : (UInt32)code;
	return arg;
}

Inline SmileArg SmileUnboxedUni_From(UInt32 code)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedUni_Instance;
	arg.unboxed.uni = (UInt32)code;
	return arg;
}

#endif