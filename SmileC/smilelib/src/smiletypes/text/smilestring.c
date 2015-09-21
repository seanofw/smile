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
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/smilelist.h>

SmileString SmileString_Create(SmileEnv env, String string)
{
	SmileString str = GC_MALLOC_STRUCT(struct SmileStringInt);
	if (str == NULL) Smile_Abort_OutOfMemory();
	str->base = env->knownObjects.Object;
	str->env = env;
	str->kind = SMILE_KIND_STRING;
	str->vtable = SmileString_VTable;
	str->string.text = ((struct StringInt *)string)->text;
	str->string.length = ((struct StringInt *)string)->length;
	return str;
}

Bool SmileString_CompareEqual(SmileString self, SmileObject other)
{
	SmileString otherString;
	Int length;

	if (other->kind != SMILE_KIND_STRING) return False;
	otherString = (SmileString)other;

	if (otherString == self) return True;

	length = self->string.length;
	if (length != otherString->string.length) return False;

	return !MemCmp(self->string.text, otherString->string.text, length);
}

UInt32 SmileString_Hash(SmileString self)
{
	String str = SmileString_ToString(self);
	return String_Hash(str);
}

void SmileString_SetSecurity(SmileString self, Int security)
{
	UNUSED(self);
	UNUSED(security);
	SmileEnv_ThrowException(self->env, self->env->knownSymbols.object_security_error,
		String_Format("Cannot alter security on strings, which are read-only."));
}

Int SmileString_GetSecurity(SmileString self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileString_GetProperty(SmileString self, Symbol propertyName)
{
	if (propertyName == self->env->knownSymbols.length) {
		return (SmileObject)SmileInteger32_Create(self->env, self->string.length);
	}
	return self->base->vtable->getProperty((SmileObject)self, propertyName);
}

void SmileString_SetProperty(SmileString self, Symbol propertyName, SmileObject value)
{
	UNUSED(value);
	SmileEnv_ThrowException(self->env, self->env->knownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a string, which is read-only.",
		SymbolTable_GetName(self->env->symbolTable, propertyName)));
}

Bool SmileString_HasProperty(SmileString self, Symbol propertyName)
{
	return (propertyName == self->env->knownSymbols.length);
}

SmileList SmileString_GetPropertyNames(SmileString self)
{
	SmileEnv env = self->env;

	DECLARE_LIST_BUILDER(env, listBuilder);
	LIST_BUILDER_APPEND(env, listBuilder, SmileSymbol_Create(env, env->knownSymbols.length));
	return LIST_BUILDER_HEAD(listBuilder);
}

Bool SmileString_ToBool(SmileString self)
{
	Bool result;
	return String_ParseBool(SmileString_ToString(self), &result) ? result : False;
}

Int32 SmileString_ToInteger32(SmileString self)
{
	Int64 result;
	return String_ParseInteger(SmileString_ToString(self), 10, &result) ? (Int32)result : 0;
}

Real64 SmileString_ToReal64(SmileString self)
{
	Real64 result;
	return String_ParseReal(SmileString_ToString(self), 10, &result) ? result : 0.0;
}

SMILE_VTABLE(SmileString_VTable, SmileString)
{
	SmileString_CompareEqual,
	SmileString_Hash,
	SmileString_SetSecurity,
	SmileString_GetSecurity,

	SmileString_GetProperty,
	SmileString_SetProperty,
	SmileString_HasProperty,
	SmileString_GetPropertyNames,

	SmileString_ToBool,
	SmileString_ToInteger32,
	SmileString_ToReal64,
	SmileString_ToString,
};
