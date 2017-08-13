#ifndef __SMILE_SMILETYPES_RANGE_SMILEINTEGER32RANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEINTEGER32RANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileInteger32RangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Int32 start;
	Int32 end;
	Int32 stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileInteger32Range_VTable;

SMILE_API_FUNC SmileInteger32Range SmileInteger32Range_Create(Int32 start, Int32 end, Int32 stepping);

#endif