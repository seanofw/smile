//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <smile/numeric/real64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>

SmileSymbol SmileSymbol_Create(Symbol symbol)
{
	SmileSymbol smileInt = GC_MALLOC_STRUCT(struct SmileSymbolInt);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Symbol;
	smileInt->kind = SMILE_KIND_SYMBOL;
	smileInt->vtable = SmileSymbol_VTable;
	smileInt->symbol = symbol;
	return smileInt;
}

Bool SmileSymbol_CompareEqual(SmileSymbol self, SmileObject other)
{
	SmileSymbol otherInt;

	if (SMILE_KIND(other) != SMILE_KIND_SYMBOL) return False;
	otherInt = (SmileSymbol)other;

	return self->symbol == otherInt->symbol;
}

UInt32 SmileSymbol_Hash(SmileSymbol self)
{
	return (UInt32)self->symbol;
}

void SmileSymbol_SetSecurityKey(SmileSymbol self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileSymbol_SetSecurity(SmileSymbol self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileSymbol_GetSecurity(SmileSymbol self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileSymbol_GetProperty(SmileSymbol self, Symbol propertyName)
{
	return self->base->vtable->getProperty((SmileObject)self, propertyName);
}

void SmileSymbol_SetProperty(SmileSymbol self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a symbol, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileSymbol_HasProperty(SmileSymbol self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileSymbol_GetPropertyNames(SmileSymbol self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileSymbol_ToBool(SmileSymbol self)
{
	UNUSED(self);
	return True;
}

Int32 SmileSymbol_ToInteger32(SmileSymbol self)
{
	UNUSED(self);
	return 0;
}

Float64 SmileSymbol_ToFloat64(SmileSymbol self)
{
	UNUSED(self);
	return 0.0;
}

Real64 SmileSymbol_ToReal64(SmileSymbol self)
{
	UNUSED(self);
	return Real64_Zero;
}

String SmileSymbol_ToString(SmileSymbol self)
{
	return SymbolTable_GetName(Smile_SymbolTable, self->symbol);
}

Bool SmileSymbol_Call(SmileSymbol self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError);

	return True;
}

SMILE_VTABLE(SmileSymbol_VTable, SmileSymbol)
{
	SmileSymbol_CompareEqual,
	SmileSymbol_Hash,

	SmileSymbol_SetSecurityKey,
	SmileSymbol_SetSecurity,
	SmileSymbol_GetSecurity,

	SmileSymbol_GetProperty,
	SmileSymbol_SetProperty,
	SmileSymbol_HasProperty,
	SmileSymbol_GetPropertyNames,

	SmileSymbol_ToBool,
	SmileSymbol_ToInteger32,
	SmileSymbol_ToFloat64,
	SmileSymbol_ToReal64,
	SmileSymbol_ToString,

	SmileSymbol_Call,
};
