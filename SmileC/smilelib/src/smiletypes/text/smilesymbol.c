//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>

SmileSymbol SmileSymbol_Create(SmileEnv env, Symbol symbol)
{
	SmileSymbol smileInt = GC_MALLOC_STRUCT(struct SmileSymbolInt);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = env->knownObjects.Object;
	smileInt->env = env;
	smileInt->kind = SMILE_KIND_SYMBOL;
	smileInt->vtable = SmileSymbol_VTable;
	smileInt->symbol = symbol;
	return smileInt;
}

Bool SmileSymbol_CompareEqual(SmileSymbol self, SmileObject other)
{
	SmileSymbol otherInt;

	if (other->kind != SMILE_KIND_SYMBOL) return False;
	otherInt = (SmileSymbol)other;

	return self->symbol == otherInt->symbol;
}

UInt32 SmileSymbol_Hash(SmileSymbol self)
{
	return (UInt32)self->symbol;
}

void SmileSymbol_SetSecurity(SmileSymbol self, Int security)
{
	UNUSED(self);
	UNUSED(security);
	SmileEnv_ThrowException(self->env, self->env->knownSymbols.object_security_error,
		String_Format("Cannot alter security on symbols, which are read-only."));
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
	UNUSED(value);
	SmileEnv_ThrowException(self->env, self->env->knownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a symbol, which is read-only.",
		SymbolTable_GetName(self->env->symbolTable, propertyName)));
}

Bool SmileSymbol_HasProperty(SmileSymbol self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileSymbol_GetPropertyNames(SmileSymbol self)
{
	return self->env->knownObjects.Null;
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

Real64 SmileSymbol_ToReal64(SmileSymbol self)
{
	UNUSED(self);
	return 0.0;
}

String SmileSymbol_ToString(SmileSymbol self)
{
	return SymbolTable_GetName(self->env->symbolTable, self->symbol);
}

SMILE_VTABLE(SmileSymbol_VTable, SmileSymbol)
{
	SmileSymbol_CompareEqual,
	SmileSymbol_Hash,
	SmileSymbol_SetSecurity,
	SmileSymbol_GetSecurity,

	SmileSymbol_GetProperty,
	SmileSymbol_SetProperty,
	SmileSymbol_HasProperty,
	SmileSymbol_GetPropertyNames,

	SmileSymbol_ToBool,
	SmileSymbol_ToInteger32,
	SmileSymbol_ToReal64,
	SmileSymbol_ToString,
};
