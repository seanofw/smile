#ifndef __SMILE_SMILETYPES_RANGE_SMILEINTEGER64RANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEINTEGER64RANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger64RangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int64 start;
	Int64 end;
	Int64 stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger64Range_VTable;

SMILE_API_FUNC SmileInteger64Range SmileInteger64Range_Create(Int64 start, Int64 end, Int64 stepping);

#endif