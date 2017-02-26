
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER16_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEINTEGER16_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger16Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int16 value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger16_VTable;

SMILE_API_FUNC SmileInteger16 SmileInteger16_CreateInternal(Int16 value);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileInteger16 SmileInteger16_Create(Int16 value)
{
	if (value >= -100 && value <= 100)
		return Smile_KnownObjects.SmallInt16s[value + 100];
	else
		return SmileInteger16_CreateInternal(value);
}

#endif
