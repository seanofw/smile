
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER128_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER128_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger128Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int128 value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger128_VTable;

SMILE_API_FUNC SmileInteger128 SmileInteger128_CreateInternal(Int128 value);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger128 SmileInteger128_Create(Int128 value)
{
	if (value.hi == 0 && value.lo <= 100)
		return Smile_KnownObjects.SmallInt128s[(Int)(value.lo + 100)];
	else if (value.hi == -1 && (Int64)value.lo >= -100)
		return Smile_KnownObjects.SmallInt128s[(Int)((Int64)value.lo + 100)];
	else
		return SmileInteger128_CreateInternal(value);
}

#endif
