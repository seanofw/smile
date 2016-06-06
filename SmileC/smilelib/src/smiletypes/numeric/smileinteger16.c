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
#include <smile/smiletypes/numeric/smileinteger16.h>

SmileInteger16 SmileInteger16_CreateInternal(Int16 value)
{
	SmileInteger16 smileInt = GC_MALLOC_STRUCT(struct SmileInteger16Int);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = Smile_KnownObjects.Object;
	smileInt->kind = SMILE_KIND_INTEGER16;
	smileInt->vtable = SmileInteger16_VTable;
	smileInt->value = value;
	return smileInt;
}

Bool SmileInteger16_CompareEqual(SmileInteger16 self, SmileObject other)
{
	SmileInteger16 otherInt;

	if (SMILE_KIND(other) != SMILE_KIND_INTEGER16) return False;
	otherInt = (SmileInteger16)other;

	return self->value == otherInt->value;
}

UInt32 SmileInteger16_Hash(SmileInteger16 self)
{
	return self->value;
}

void SmileInteger16_SetSecurityKey(SmileInteger16 self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileInteger16_SetSecurity(SmileInteger16 self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileInteger16_GetSecurity(SmileInteger16 self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileInteger16_GetProperty(SmileInteger16 self, Symbol propertyName)
{
	return self->base->vtable->getProperty((SmileObject)self, propertyName);
}

void SmileInteger16_SetProperty(SmileInteger16 self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on an integer, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileInteger16_HasProperty(SmileInteger16 self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileInteger16_GetPropertyNames(SmileInteger16 self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileInteger16_ToBool(SmileInteger16 self)
{
	return self->value != 0;
}

Int32 SmileInteger16_ToInteger32(SmileInteger16 self)
{
	return (Int32)self->value;
}

Float64 SmileInteger16_ToFloat64(SmileInteger16 self)
{
	return (Float64)self->value;
}

Real64 SmileInteger16_ToReal64(SmileInteger16 self)
{
	return Real64_FromInt32(self->value);
}

String SmileInteger16_ToString(SmileInteger16 self)
{
	return String_Format("%dh", (Int32)self->value);
}

SMILE_VTABLE(SmileInteger16_VTable, SmileInteger16)
{
	SmileInteger16_CompareEqual,
	SmileInteger16_Hash,

	SmileInteger16_SetSecurityKey,
	SmileInteger16_SetSecurity,
	SmileInteger16_GetSecurity,

	SmileInteger16_GetProperty,
	SmileInteger16_SetProperty,
	SmileInteger16_HasProperty,
	SmileInteger16_GetPropertyNames,

	SmileInteger16_ToBool,
	SmileInteger16_ToInteger32,
	SmileInteger16_ToFloat64,
	SmileInteger16_ToReal64,
	SmileInteger16_ToString,
};
