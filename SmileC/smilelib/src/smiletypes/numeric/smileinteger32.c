//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2017 Sean Werkema
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
#include <smile/smiletypes/numeric/smileinteger32.h>

SmileInteger32 SmileInteger32_CreateInternal(Int32 value)
{
	SmileInteger32 smileInt = GC_MALLOC_STRUCT(struct SmileInteger32Int);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Integer32;
	smileInt->kind = SMILE_KIND_INTEGER32;
	smileInt->vtable = SmileInteger32_VTable;
	smileInt->value = value;
	return smileInt;
}

Bool SmileInteger32_CompareEqual(SmileInteger32 self, SmileObject other)
{
	SmileInteger32 otherInt;

	if (SMILE_KIND(other) != SMILE_KIND_INTEGER32) return False;
	otherInt = (SmileInteger32)other;

	return self->value == otherInt->value;
}

UInt32 SmileInteger32_Hash(SmileInteger32 self)
{
	return self->value;
}

void SmileInteger32_SetSecurityKey(SmileInteger32 self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileInteger32_SetSecurity(SmileInteger32 self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileInteger32_GetSecurity(SmileInteger32 self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileInteger32_GetProperty(SmileInteger32 self, Symbol propertyName)
{
	return self->base->vtable->getProperty(self->base, propertyName);
}

void SmileInteger32_SetProperty(SmileInteger32 self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on an integer, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileInteger32_HasProperty(SmileInteger32 self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileInteger32_GetPropertyNames(SmileInteger32 self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileInteger32_ToBool(SmileInteger32 self)
{
	return self->value != 0;
}

Int32 SmileInteger32_ToInteger32(SmileInteger32 self)
{
	return self->value;
}

Float64 SmileInteger32_ToFloat64(SmileInteger32 self)
{
	return (Float64)self->value;
}

Real64 SmileInteger32_ToReal64(SmileInteger32 self)
{
	return Real64_FromInt32(self->value);
}

String SmileInteger32_ToString(SmileInteger32 self)
{
	return String_Format("%d", self->value);
}

Bool SmileInteger32_Call(SmileInteger32 self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError);

	return True;
}

SMILE_VTABLE(SmileInteger32_VTable, SmileInteger32)
{
	SmileInteger32_CompareEqual,
	SmileInteger32_Hash,

	SmileInteger32_SetSecurityKey,
	SmileInteger32_SetSecurity,
	SmileInteger32_GetSecurity,

	SmileInteger32_GetProperty,
	SmileInteger32_SetProperty,
	SmileInteger32_HasProperty,
	SmileInteger32_GetPropertyNames,

	SmileInteger32_ToBool,
	SmileInteger32_ToInteger32,
	SmileInteger32_ToFloat64,
	SmileInteger32_ToReal64,
	SmileInteger32_ToString,

	SmileInteger32_Call,
};
