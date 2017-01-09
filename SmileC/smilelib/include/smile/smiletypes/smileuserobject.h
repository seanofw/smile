
#ifndef __SMILE_SMILETYPES_USEROBJECT_H__
#define __SMILE_SMILETYPES_USEROBJECT_H__

#ifndef __SMILE_SMILETYPES_OBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileUserObjectInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileObject securityKey;
	struct Int32DictInt dict;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_FUNC SmileUserObject SmileUserObject_CreateWithSize(SmileObject base, Int initialSize);

#define SmileUserObject_Set(__obj__, __symbol__, __value__) \
	(SMILE_VCALL2((__obj__), setProperty, (__symbol__), (SmileObject)(__value__)))
#define SmileUserObject_Get(__obj__, __symbol__) \
	(SMILE_VCALL1((__obj__), getProperty, (__symbol__)))

Inline SmileUserObject SmileUserObject_Create(SmileObject base)
{
	return SmileUserObject_CreateWithSize(base, 8);
}

Inline void SmileUserObject_QuickSet(SmileUserObject self, const char *name, SmileObject value)
{
	Symbol symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, name);
	Int32Dict_SetValue((Int32Dict)&self->dict, symbol, value);
}

#endif
