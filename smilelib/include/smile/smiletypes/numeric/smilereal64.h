
#ifndef __SMILE_SMILETYPES_NUMERIC_SMILEREAL64_H__
#define __SMILE_SMILETYPES_NUMERIC_SMILEREAL64_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_NUMERIC_REAL64_H__
#include <smile/numeric/real64.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileReal64Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Real64 value;
};

struct SmileUnboxedReal64Int {
	DECLARE_BASE_OBJECT_PROPERTIES;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileReal64_VTable;
SMILE_API_DATA SmileVTable SmileUnboxedReal64_VTable;

SMILE_API_FUNC SmileReal64 SmileReal64_Create(Real64 value);
SMILE_API_DATA SmileUnboxedReal64 SmileUnboxedReal64_Instance;

Inline SmileArg SmileUnboxedReal64_From(Real64 value)
{
	SmileArg arg;
	arg.obj = (SmileObject)SmileUnboxedReal64_Instance;
	arg.unboxed.r64 = value;
	return arg;
}

#endif
