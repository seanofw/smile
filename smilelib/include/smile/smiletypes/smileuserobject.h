
#ifndef __SMILE_SMILETYPES_USEROBJECT_H__
#define __SMILE_SMILETYPES_USEROBJECT_H__

#ifndef __SMILE_SMILETYPES_OBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_DICT_INT32DICT_H__
#include <smile/dict/int32dict.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEFUNCTION_H__
#include <smile/smiletypes/smilefunction.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Type declarations

struct SmileUserObjectInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	SmileObject securityKey;
	Symbol name;
	struct Int32DictInt dict;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API_FUNC SmileUserObject SmileUserObject_CreateWithSize(SmileObject base, Symbol name, Int initialSize);
SMILE_API_FUNC SmileUserObject SmileUserObject_CreateFromArgPairs(SmileObject base, Symbol name, SmileArg *argPairs, Int numArgPairs);
SMILE_API_FUNC void SmileUserObject_InitWithSize(SmileUserObject userObject, SmileObject base, Symbol name, Int initialSize);
SMILE_API_FUNC void SmileUserObject_SetC(SmileUserObject self, const char *name, SmileObject value);
SMILE_API_FUNC void SmileUserObject_SetupFunction(SmileUserObject base, ExternalFunction function, void *param,
	const char *name, const char *argNames, Int argCheckFlags, Int minArgs, Int maxArgs, Int numArgsToTypeCheck, const Byte *argTypeChecks);
SMILE_API_FUNC void SmileUserObject_SetupSynonym(SmileUserObject base, const char *oldName, const char *newName);

#define SmileUserObject_Set(__obj__, __symbol__, __value__) \
	(SMILE_VCALL2((__obj__), setProperty, (__symbol__), (SmileObject)(__value__)))
#define SmileUserObject_Get(__obj__, __symbol__) \
	(SMILE_VCALL1((__obj__), getProperty, (__symbol__)))

Inline SmileUserObject SmileUserObject_Create(SmileObject base, Symbol name)
{
	return SmileUserObject_CreateWithSize(base, name, 8);
}

Inline void SmileUserObject_Init(SmileUserObject userObject, SmileObject base, Symbol name)
{
	SmileUserObject_InitWithSize(userObject, base, name, 8);
}

#endif
