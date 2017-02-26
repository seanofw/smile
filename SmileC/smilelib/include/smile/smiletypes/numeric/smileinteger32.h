
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

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger32_VTable;

SMILE_API_FUNC SmileInteger32 SmileInteger32_CreateInternal(Int32 value);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger32 SmileInteger32_Create(Int32 value)
{
	if (value >= -100 && value <= 100)
		return Smile_KnownObjects.SmallInt32s[value + 100];
	else
		return SmileInteger32_CreateInternal(value);
}

#endif
