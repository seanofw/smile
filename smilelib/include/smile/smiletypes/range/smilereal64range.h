#ifndef __SMILE_SMILETYPES_RANGE_SMILEREAL64RANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEREAL64RANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileReal64RangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Real64 start;
	Real64 end;
	Real64 stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileReal64Range_VTable;

SMILE_API_FUNC SmileReal64Range SmileReal64Range_Create(Real64 start, Real64 end, Real64 stepping);

#endif