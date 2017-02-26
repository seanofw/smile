
#ifndef __SMILE_SMILETYPES_SMILEBOOL_H__
#define __SMILE_SMILETYPES_SMILEBOOL_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileBoolInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Bool value;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileBool_VTable;

SMILE_API_FUNC SmileBool SmileBool_Create(Bool value);

#define SmileBool_FromBool(__value__) \
	(Smile_KnownObjects.BooleanObjs[(__value__)])

#endif
