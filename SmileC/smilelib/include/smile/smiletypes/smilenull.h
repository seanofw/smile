
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

SMILE_API SmileVTable SmileNull_VTable;

extern SmileNull SmileNull_Create(void);

extern Bool SmileNull_CompareEqual(SmileNull self, SmileObject other);
extern UInt32 SmileNull_Hash(SmileNull self);
extern void SmileNull_SetSecurity(SmileNull self, Int security);
extern Int SmileNull_GetSecurity(SmileNull self);
extern SmileObject SmileNull_GetProperty(SmileNull self, Symbol propertyName);
extern void SmileNull_SetProperty(SmileNull self, Symbol propertyName, SmileObject value);
extern Bool SmileNull_HasProperty(SmileNull self, Symbol propertyName);
extern SmileList SmileNull_GetPropertyNames(SmileNull self);
extern Bool SmileNull_ToBool(SmileNull self);
extern Int32 SmileNull_ToInteger32(SmileNull self);
extern Real64 SmileNull_ToReal64(SmileNull self);
extern String SmileNull_ToString(SmileNull self);

#endif
