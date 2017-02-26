
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER64_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER64_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger64Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int64 value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger64_VTable;

SMILE_API_FUNC SmileInteger64 SmileInteger64_CreateInternal(Int64 value);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger64 SmileInteger64_Create(Int64 value)
{
	if (value >= -100 && value <= 100)
		return Smile_KnownObjects.SmallInt64s[value + 100];
	else
		return SmileInteger64_CreateInternal((Int)value);
}

#endif
