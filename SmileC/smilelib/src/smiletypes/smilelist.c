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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>

SmileList SmileList_Cons(SmileEnv env, SmileObject a, SmileObject d)
{
	SmileList smileList = GC_MALLOC_STRUCT(struct SmileListInt);
	if (smileList == NULL) Smile_Abort_OutOfMemory();
	smileList->base = env->knownObjects.Object;
	smileList->env = env;
	smileList->kind = SMILE_KIND_LIST;
	smileList->vtable = SmileList_VTable;
	smileList->a = a;
	smileList->d = d;
	return smileList;
}

Bool SmileList_CompareEqual(SmileList self, SmileObject other)
{
	SmileList otherList;

	if (other->kind != SMILE_KIND_LIST) return False;
	otherList = (SmileList)other;

	return self->a == otherList->a && self->d == otherList->d;
}

UInt32 SmileList_Hash(SmileList self)
{
	return (UInt32)(PtrInt)self;
}

void SmileList_SetSecurity(SmileList self, Int security)
{
	Int currentSecurity = self->kind & SMILE_SECURITY_READWRITEAPPEND;

	if ((currentSecurity | security) != currentSecurity) {
		// Attempting to make this less secure, so throw an exception, since we do not yet support keyed security.
		SmileEnv_ThrowException(self->env, self->env->knownSymbols.object_security_error,
			String_Format("Cannot alter security of an object to be less than its current security level."));
	}

	self->kind = (currentSecurity & ~SMILE_SECURITY_READWRITEAPPEND) | security;
}

Int SmileList_GetSecurity(SmileList self)
{
	return self->kind & SMILE_SECURITY_READWRITEAPPEND;
}

SmileObject SmileList_GetProperty(SmileList self, Symbol propertyName)
{
	if (propertyName == self->env->knownSymbols.a)
		return self->a;
	else if (propertyName == self->env->knownSymbols.d)
		return self->d;
	else
		return self->base->vtable->getProperty((SmileObject)self, propertyName);
}

void SmileList_SetProperty(SmileList self, Symbol propertyName, SmileObject value)
{
	if (!self->kind & SMILE_SECURITY_WRITABLE) {
		SmileEnv_ThrowException(self->env, self->env->knownSymbols.object_security_error,
			String_Format("Cannot set property \"%S\" on list cell: This list cell's properties are not writable.",
			SymbolTable_GetName(self->env->symbolTable, propertyName)));
	}
	else if (propertyName == self->env->knownSymbols.a)
		self->a = value;
	else if (propertyName == self->env->knownSymbols.d)
		self->d = value;
	else {
		SmileEnv_ThrowException(self->env, self->env->knownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on list cell: This property does not exist, and list cells are not appendable objects.",
			SymbolTable_GetName(self->env->symbolTable, propertyName)));
	}
}

Bool SmileList_HasProperty(SmileList self, Symbol propertyName)
{
	return (propertyName == self->env->knownSymbols.a || propertyName == self->env->knownSymbols.d);
}

SmileList SmileList_GetPropertyNames(SmileList self)
{
	SmileEnv env = self->env;

	DECLARE_LIST_BUILDER(env, listBuilder);
	LIST_BUILDER_APPEND(env, listBuilder, SmileSymbol_Create(env, env->knownSymbols.a));
	LIST_BUILDER_APPEND(env, listBuilder, SmileSymbol_Create(env, env->knownSymbols.d));
	return LIST_BUILDER_HEAD(listBuilder);
}

Bool SmileList_ToBool(SmileList self)
{
	UNUSED(self);
	return True;
}

Int32 SmileList_ToInteger32(SmileList self)
{
	UNUSED(self);
	return 1;
}

Real64 SmileList_ToReal64(SmileList self)
{
	UNUSED(self);
	return 1;
}

String SmileList_ToString(SmileList self)
{
	UNUSED(self);
	return String_Format("List");
}

SMILE_VTABLE(SmileList_VTable, SmileList)
{
	SmileList_CompareEqual,
	SmileList_Hash,
	SmileList_SetSecurity,
	SmileList_GetSecurity,

	SmileList_GetProperty,
	SmileList_SetProperty,
	SmileList_HasProperty,
	SmileList_GetPropertyNames,

	SmileList_ToBool,
	SmileList_ToInteger32,
	SmileList_ToReal64,
	SmileList_ToString,
};
