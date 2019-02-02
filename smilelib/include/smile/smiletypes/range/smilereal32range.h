#ifndef __SMILE_SMILETYPES_RANGE_SMILEREAL32RANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEREAL32RANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileReal32RangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Real32 start;
	Real32 end;
	Real32 stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileReal32Range_VTable;

SMILE_API_FUNC SmileReal32Range SmileReal32Range_Create(Real32 start, Real32 end, Real32 stepping);

#endif