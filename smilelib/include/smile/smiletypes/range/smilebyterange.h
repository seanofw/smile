#ifndef __SMILE_SMILETYPES_RANGE_SMILEBYTERANGE_H__
#define __SMILE_SMILETYPES_RANGE_SMILEBYTERANGE_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileByteRangeInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Byte start;
	Byte end;
	SByte stepping;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileByteRange_VTable;

SMILE_API_FUNC SmileByteRange SmileByteRange_Create(Byte start, Byte end, SByte stepping);

#endif