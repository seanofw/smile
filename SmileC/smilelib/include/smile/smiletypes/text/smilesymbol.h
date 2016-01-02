
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

SMILE_API_DATA SmileVTable SmileSymbol_VTable;

SMILE_API_FUNC SmileSymbol SmileSymbol_Create(Symbol symbol);

SMILE_API_FUNC Bool SmileSymbol_CompareEqual(SmileSymbol self, SmileObject other);
SMILE_API_FUNC UInt32 SmileSymbol_Hash(SmileSymbol self);
SMILE_API_FUNC void SmileSymbol_SetSecurity(SmileSymbol self, Int security, SmileObject securityKey);
SMILE_API_FUNC Int SmileSymbol_GetSecurity(SmileSymbol self);
SMILE_API_FUNC SmileObject SmileSymbol_GetProperty(SmileSymbol self, Symbol propertyName);
SMILE_API_FUNC void SmileSymbol_SetProperty(SmileSymbol self, Symbol propertyName, SmileObject value);
SMILE_API_FUNC Bool SmileSymbol_HasProperty(SmileSymbol self, Symbol propertyName);
SMILE_API_FUNC SmileList SmileSymbol_GetPropertyNames(SmileSymbol self);
SMILE_API_FUNC Bool SmileSymbol_ToBool(SmileSymbol self);
SMILE_API_FUNC Int32 SmileSymbol_ToInteger32(SmileSymbol self);
SMILE_API_FUNC Real64 SmileSymbol_ToReal64(SmileSymbol self);
SMILE_API_FUNC String SmileSymbol_ToString(SmileSymbol self);

#endif
