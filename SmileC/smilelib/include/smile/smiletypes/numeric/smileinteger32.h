
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

struct SmileUnboxedInteger32Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger32_VTable;
SMILE_API_DATA SmileVTable SmileUnboxedInteger32_VTable;

SMILE_API_FUNC SmileInteger32 SmileInteger32_CreateInternal(Int32 value);
SMILE_API_DATA SmileUnboxedInteger32 SmileUnboxedInteger32_Instance;

Inline SmileArg SmileUnboxedInteger32_From(Int32 value)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedInteger32_Instance;
	arg.unboxed.i32 = value;
	return arg;
}

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger32 SmileInteger32_Create(Int32 value)
{
	if ((UInt32)(value + 100) <= 200)
		return Smile_KnownObjects.SmallInt32s[value + 100];
	else
		return SmileInteger32_CreateInternal(value);
}

#endif
