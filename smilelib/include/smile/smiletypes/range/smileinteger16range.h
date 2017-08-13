#ifndef __SMILE_SMILETYPES_RANGE_SMILEINTEGER16RANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEINTEGER16RANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger16RangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int16 start;
	Int16 end;
	Int16 stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger16Range_VTable;

SMILE_API_FUNC SmileInteger16Range SmileInteger16Range_Create(Int16 start, Int16 end, Int16 stepping);

#endif