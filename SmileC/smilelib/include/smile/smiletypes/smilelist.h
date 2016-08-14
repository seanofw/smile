
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

#ifndef __SMILE_PARSING_LEXER_H__
#include <smile/parsing/lexer.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileListInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileObject a;
	SmileObject d;
};

struct SmileListWithSourceInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileObject a;
	SmileObject d;
	LexerPosition position;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_DATA SmileVTable SmileList_VTable;

SMILE_API_FUNC SmileList SmileList_Cons(SmileObject a, SmileObject d);
SMILE_API_FUNC SmileList SmileList_ConsWithSource(SmileObject a, SmileObject d, LexerPosition position);
SMILE_API_FUNC SmileList SmileList_CreateListFromArray(SmileObject *objects, Int numObjects);
SMILE_API_FUNC SmileList SmileList_CreateList(SmileObject firstObject, ...);
SMILE_API_FUNC SmileList SmileList_CreateListv(SmileObject firstObject, va_list v);

//-------------------------------------------------------------------------------------------------
//  Inline operations

Inline SmileList SmileList_Rest(SmileList list)
{
	SmileObject d = list->d;
	return SMILE_KIND(d) == SMILE_KIND_LIST ? (SmileList)d : NullList;
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
#define LIST_SIXTH(__list__) LIST_FIRST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(__list__))))))
#define LIST_SEVENTH(__list__) LIST_FIRST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(__list__)))))))
#define LIST_EIGHTH(__list__) LIST_FIRST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(__list__))))))))
#define LIST_NINTH(__list__) LIST_FIRST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(__list__)))))))))
#define LIST_TENTH(__list__) LIST_FIRST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(LIST_REST(__list__))))))))))

#define LIST_INIT(__head__, __tail__) \
	((__tail__) = (__head__) = NullList)

#define LIST_APPEND(__head__, __tail__, __newElement__) \
	((__tail__) = (SMILE_KIND(__head__) == SMILE_KIND_NULL) \
		? ((__head__) = SmileList_Cons((SmileObject)(__newElement__), NullObject)) \
		: (SmileList)(((__tail__)->d = (SmileObject)SmileList_Cons((SmileObject)(__newElement__), NullObject))))

#define LIST_APPEND_WITH_SOURCE(__head__, __tail__, __newElement__, __position__) \
	((__tail__) = (SMILE_KIND(__head__) == SMILE_KIND_NULL) \
		? ((__head__) = SmileList_ConsWithSource((SmileObject)(__newElement__), NullObject, (__position__))) \
		: (SmileList)(((__tail__)->d = (SmileObject)SmileList_ConsWithSource((SmileObject)(__newElement__), NullObject, (__position__)))))

#endif
