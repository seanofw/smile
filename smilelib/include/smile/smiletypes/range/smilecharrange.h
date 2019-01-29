#ifndef __SMILE_SMILETYPES_RANGE_SMILECHARRANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILECHARRANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileCharRangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Byte start;
	Byte end;
	Byte stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileCharRange_VTable;

SMILE_API_FUNC SmileCharRange SmileCharRange_Create(Byte start, Byte end, Byte stepping);

#endif