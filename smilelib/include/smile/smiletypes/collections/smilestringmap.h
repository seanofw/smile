#ifndef __SMILE_SMILETYPES_COLLECTIONS_SMILESTRINGMAP_H__
#define __SMILE_SMILETYPES_COLLECTIONS_SMILESTRINGMAP_H__

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_DICT_STRINGDICT_H__
#include <smile/dict/stringdict.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileStringMapInt {
	DECLARE_BASE_OBJECT_PROPERTIES;

	struct StringDictStruct dict;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileStringMap_VTable;

SMILE_API_FUNC SmileStringMap SmileStringMap_CreateWithSize(Int newSize);

Inline SmileStringMap SmileStringMap_Create(void)
{
	return SmileStringMap_CreateWithSize(16);
}

#endif