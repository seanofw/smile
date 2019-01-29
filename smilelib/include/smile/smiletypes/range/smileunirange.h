#ifndef __SMILE_SMILETYPES_RANGE_SMILEUNIRANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEUNIRANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileUniRangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	UInt32 start;
	UInt32 end;
	UInt32 stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileUniRange_VTable;

SMILE_API_FUNC SmileUniRange SmileUniRange_Create(UInt32 start, UInt32 end, UInt32 stepping);

#endif