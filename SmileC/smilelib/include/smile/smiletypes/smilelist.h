
#ifndef __SMILE_SMILETYPES_SMILELIST_H__
#define __SMILE_SMILETYPES_SMILELIST_H__

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

struct SmileListInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileObject a;
	SmileObject d;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API SmileVTable SmileList_VTable;

SMILE_API SmileList SmileList_Cons(SmileEnv env, SmileObject a, SmileObject d);
SMILE_API SmileList SmileList_CreateList(SmileEnv env, SmileObject *objects, Int numObjects);

SMILE_API Bool SmileList_CompareEqual(SmileList self, SmileObject other);
SMILE_API UInt32 SmileList_Hash(SmileList self);
SMILE_API void SmileList_SetSecurity(SmileList self, Int security);
SMILE_API Int SmileList_GetSecurity(SmileList self);
SMILE_API SmileObject SmileList_GetProperty(SmileList self, Symbol propertyName);
SMILE_API void SmileList_SetProperty(SmileList self, Symbol propertyName, SmileObject value);
SMILE_API Bool SmileList_HasProperty(SmileList self, Symbol propertyName);
SMILE_API SmileList SmileList_GetPropertyNames(SmileList self);
SMILE_API Bool SmileList_ToBool(SmileList self);
SMILE_API Int32 SmileList_ToInteger32(SmileList self);
SMILE_API Real64 SmileList_ToReal64(SmileList self);
SMILE_API String SmileList_ToString(SmileList self);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileList SmileList_Rest(SmileList list)
{
	SmileObject d = list->d;
	return ((d->kind & ~SMILE_KIND_LIST) == SMILE_KIND_NULL) ? (SmileList)d : (SmileList)list->env->knownObjects.Null;
}

Inline SmileObject SmileList_First(SmileList list)
{
	return list->a;
}

#define LIST_REST(__list__) (SmileList_Rest(__list__))

#define LIST_FIRST(__list__) ((__list__)->a)
#define LIST_SECOND(__list__) LIST_FIRST(LIST_REST(__list__))
#define LIST_THIRD(__list__) LIST_FIRST(LIST_REST(LIST_REST(__list__)))
#define LIST_FOURTH(__list__) LIST_FIRST(LIST_REST(LIST_REST(LIST_REST(__list__))))
#define LIST_FIFTH(__list__) LIST_FIRST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(__list__)))))

//-------------------------------------------------------------------------------------------------
//  List builders

#define DECLARE_LIST_BUILDER(__env__, __name__) \
	SmileList __name__##Null = (__env__)->knownObjects.Null, \
		__name__##Head = __name__##Null, \
		*__name__##Tail = &(__name__##Head), \
		__name__##Temp
#define LIST_BUILDER_APPEND(__env__, __name__, __item__) \
	( \
		__name__##Temp = SmileList_Cons((__env__), (SmileObject)__item__, (SmileObject)__name__##Null), \
		*__name__##Tail = (__name__##Temp), \
		__name__##Tail = (SmileList *)&((__name__##Temp)->d) \
	)
#define LIST_BUILDER_INSERT(__env__, __name__, __item__) \
	( \
		__name__##Head = SmileList_Cons((__env__), (SmileObject)__item__, (SmileObject)__name__##Head), \
		__name__##Tail = (__name__##Tail == &(__name__##Head) ? &((SmileList *)(__name__##Head)->d) : (__name__##Tail)) \
	)
#define LIST_BUILDER_HEAD(__name__) \
	(__name__##Head)

#endif
