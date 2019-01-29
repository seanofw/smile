#ifndef __SMILE_SMILETYPES_RANGE_SMILEFLOAT64RANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEFLOAT64RANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileFloat64RangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Float64 start;
	Float64 end;
	Float64 stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileFloat64Range_VTable;

SMILE_API_FUNC SmileFloat64Range SmileFloat64Range_Create(Float64 start, Float64 end, Float64 stepping);

#endif