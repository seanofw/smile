#ifndef __SMILE_SMILETYPES_RANGE_SMILESYMBOLMAP_H__
#define __SMILE_SMILETYPES_RANGE_SMILESYMBOLMAP_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileSymbolMapInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	struct Int32DictStruct dict;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileSymbolMap_VTable;

SMILE_API_FUNC SmileSymbolMap SmileSymbolMap_CreateWithSize(Int32 newSize);

Inline SmileSymbolMap SmileSymbolMap_Create(void)
{
	return SmileSymbolMap_CreateWithSize(16);
}

#endif