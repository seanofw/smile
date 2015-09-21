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
#include <smile/smiletypes/numeric/smileinteger32.h>

SmileInteger32 SmileInteger32_CreateInternal(SmileEnv env, Int32 value)
{
	SmileInteger32 smileInt = GC_MALLOC_STRUCT(struct SmileInteger32Int);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = env->knownObjects.Object;
	smileInt->env = env;
	smileInt->kind = SMILE_KIND_INTEGER32;
	smileInt->vtable = SmileInteger32_VTable;
	smileInt->value = value;
	return smileInt;
}

Bool SmileInteger32_CompareEqual(SmileInteger32 self, SmileObject other)
{
	SmileInteger32 otherInt;

	if (other->kind != SMILE_KIND_INTEGER32) return False;
	otherInt = (SmileInteger32)other;

	return self->value == otherInt->value;
}

UInt32 SmileInteger32_Hash(SmileInteger32 self)
{
	return self->value;
}

void SmileInteger32_SetSecurity(SmileInteger32 self, Int security)
{
	UNUSED(self);
	UNUSED(security);
	SmileEnv_ThrowException(self->env, self->env->knownSymbols.object_security_error,
		String_Format("Cannot alter security on integers, which are read-only."));
}

Int SmileInteger32_GetSecurity(SmileInteger32 self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileInteger32_GetProperty(SmileInteger32 self, Symbol propertyName)
{
	return self->base->vtable->getProperty((SmileObject)self, propertyName);
}

void SmileInteger32_SetProperty(SmileInteger32 self, Symbol propertyName, SmileObject value)
{
	UNUSED(value);
	SmileEnv_ThrowException(self->env, self->env->knownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on an integer, which is read-only.",
		SymbolTable_GetName(self->env->symbolTable, propertyName)));
}

Bool SmileInteger32_HasProperty(SmileInteger32 self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileInteger32_GetPropertyNames(SmileInteger32 self)
{
	return self->env->knownObjects.Null;
}

Bool SmileInteger32_ToBool(SmileInteger32 self)
{
	return self->value != 0;
}

Int32 SmileInteger32_ToInteger32(SmileInteger32 self)
{
	return self->value;
}

Real64 SmileInteger32_ToReal64(SmileInteger32 self)
{
	return (Real64)self->value;
}

String SmileInteger32_ToString(SmileInteger32 self)
{
	return String_Format("%d", self->value);
}

SMILE_VTABLE(SmileInteger32_VTable, SmileInteger32)
{
	SmileInteger32_CompareEqual,
	SmileInteger32_Hash,
	SmileInteger32_SetSecurity,
	SmileInteger32_GetSecurity,

	SmileInteger32_GetProperty,
	SmileInteger32_SetProperty,
	SmileInteger32_HasProperty,
	SmileInteger32_GetPropertyNames,

	SmileInteger32_ToBool,
	SmileInteger32_ToInteger32,
	SmileInteger32_ToReal64,
	SmileInteger32_ToString,
};
