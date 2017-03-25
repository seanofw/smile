
#ifndef __SMILE_SMILETYPES_SMILENULL_H__
#define __SMILE_SMILETYPES_SMILENULL_H__

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILELIST_H__
#include <smile/smiletypes/smilelist.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Public interface (Null is just a special List)

SMILE_API_DATA SmileVTable SmileNull_VTable;

extern SmileNull SmileNull_Create(void);

#endif
