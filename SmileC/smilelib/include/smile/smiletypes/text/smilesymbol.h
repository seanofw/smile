
#ifndef __SMILE_SMILETYPES_TEXT_SMILESYMBOL_H__
#define __SMILE_SMILETYPES_TEXT_SMILESYMBOL_H__

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

struct SmileSymbolInt {
	DECLARE_BASE_OBJECT_PROPERTIES;
	Symbol symbol;
};

//-------------------------------------------------------------------------------------------------
//  Public interface

SMILE_API SmileVTable SmileSymbol_VTable;

SMILE_API SmileSymbol SmileSymbol_Create(Symbol symbol);

SMILE_API Bool SmileSymbol_CompareEqual(SmileSymbol self, SmileObject other);
SMILE_API UInt32 SmileSymbol_Hash(SmileSymbol self);
SMILE_API void SmileSymbol_SetSecurity(SmileSymbol self, Int security);
SMILE_API Int SmileSymbol_GetSecurity(SmileSymbol self);
SMILE_API SmileObject SmileSymbol_GetProperty(SmileSymbol self, Symbol propertyName);
SMILE_API void SmileSymbol_SetProperty(SmileSymbol self, Symbol propertyName, SmileObject value);
SMILE_API Bool SmileSymbol_HasProperty(SmileSymbol self, Symbol propertyName);
SMILE_API SmileList SmileSymbol_GetPropertyNames(SmileSymbol self);
SMILE_API Bool SmileSymbol_ToBool(SmileSymbol self);
SMILE_API Int32 SmileSymbol_ToInteger32(SmileSymbol self);
SMILE_API Real64 SmileSymbol_ToReal64(SmileSymbol self);
SMILE_API String SmileSymbol_ToString(SmileSymbol self);

#endif
