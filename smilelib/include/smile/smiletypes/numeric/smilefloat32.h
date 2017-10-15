
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEFLOAT32_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEFLOAT32_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileFloat32Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Float32 value;
};

struct SmileUnboxedFloat32Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileFloat32_VTable;
SMILE_API_DATA SmileVTable SmileUnboxedFloat32_VTable;

SMILE_API_FUNC SmileFloat32 SmileFloat32_Create(Float32 value);
SMILE_API_FUNC SmileFloat32 SmileFloat32_Init(SmileFloat32 smileFloat, Float32 value);
SMILE_API_DATA SmileUnboxedFloat32 SmileUnboxedFloat32_Instance;

Inline SmileArg SmileUnboxedFloat32_From(Float32 value)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedFloat32_Instance;
	arg.unboxed.f32 = value;
	return arg;
}

#endif
