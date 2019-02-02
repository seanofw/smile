#ifndef __SMILE_SMILETYPES_RANGE_SMILEFLOAT32RANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEFLOAT32RANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileFloat32RangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Float32 start;
	Float32 end;
	Float32 stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileFloat32Range_VTable;

SMILE_API_FUNC SmileFloat32Range SmileFloat32Range_Create(Float32 start, Float32 end, Float32 stepping);

#endif