
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEREAL32_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEREAL32_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_NUMERIC_REAL32_H__
#include <smile/numeric/real32.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileReal32Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Real32 value;
};

struct SmileUnboxedReal32Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileReal32_VTable;
SMILE_API_DATA SmileVTable SmileUnboxedReal32_VTable;

SMILE_API_FUNC SmileReal32 SmileReal32_Create(Real32 value);
SMILE_API_DATA SmileUnboxedReal32 SmileUnboxedReal32_Instance;

Inline SmileArg SmileUnboxedReal32_From(Real32 value)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedReal32_Instance;
	arg.unboxed.r32 = value;
	return arg;
}

#endif
