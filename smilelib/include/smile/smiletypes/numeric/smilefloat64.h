
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEFLOAT64_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEFLOAT64_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileFloat64Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Float64 value;
};

struct SmileUnboxedFloat64Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileFloat64_VTable;
SMILE_API_DATA SmileVTable SmileUnboxedFloat64_VTable;

SMILE_API_FUNC SmileFloat64 SmileFloat64_Create(Float64 value);
SMILE_API_FUNC SmileFloat64 SmileFloat64_Init(SmileFloat64 smileFloat, Float64 value);
SMILE_API_DATA SmileUnboxedFloat64 SmileUnboxedFloat64_Instance;

Inline SmileArg SmileUnboxedFloat64_From(Float64 value)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedFloat64_Instance;
	arg.unboxed.f64 = value;
	return arg;
}

#endif
