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
#include <smile/smiletypes/numeric/smileinteger64.h>

SmileInteger64 SmileInteger64_CreateInternal(Int64 value)
{
	SmileInteger64 smileInt = GC_MALLOC_STRUCT(struct SmileInteger64Int);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Integer64;
	smileInt->kind = SMILE_KIND_INTEGER64;
	smileInt->vtable = SmileInteger64_VTable;
	smileInt->value = value;
	return smileInt;
}

Bool SmileInteger64_CompareEqual(SmileInteger64 self, SmileObject other)
{
	SmileInteger64 otherInt;

	if (SMILE_KIND(other) != SMILE_KIND_INTEGER64) return False;
	otherInt = (SmileInteger64)other;

	return self->value == otherInt->value;
}

UInt32 SmileInteger64_Hash(SmileInteger64 self)
{
	UInt64 value = (UInt64)self->value;
	return (UInt32)(value ^ (value >> 32));
}

void SmileInteger64_SetSecurityKey(SmileInteger64 self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileInteger64_SetSecurity(SmileInteger64 self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileInteger64_GetSecurity(SmileInteger64 self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileInteger64_GetProperty(SmileInteger64 self, Symbol propertyName)
{
	return self->base->vtable->getProperty(self->base, propertyName);
}

void SmileInteger64_SetProperty(SmileInteger64 self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on an integer, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileInteger64_HasProperty(SmileInteger64 self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileInteger64_GetPropertyNames(SmileInteger64 self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileInteger64_ToBool(SmileInteger64 self)
{
	return self->value != 0;
}

Int32 SmileInteger64_ToInteger32(SmileInteger64 self)
{
	return (Int32)self->value;
}

Float64 SmileInteger64_ToFloat64(SmileInteger64 self)
{
	return (Float64)self->value;
}

Real64 SmileInteger64_ToReal64(SmileInteger64 self)
{
	return Real64_FromInt64(self->value);
}

String SmileInteger64_ToString(SmileInteger64 self)
{
	return String_Format("%ld", self->value);
}

SMILE_VTABLE(SmileInteger64_VTable, SmileInteger64)
{
	SmileInteger64_CompareEqual,
	SmileInteger64_Hash,

	SmileInteger64_SetSecurityKey,
	SmileInteger64_SetSecurity,
	SmileInteger64_GetSecurity,

	SmileInteger64_GetProperty,
	SmileInteger64_SetProperty,
	SmileInteger64_HasProperty,
	SmileInteger64_GetPropertyNames,

	SmileInteger64_ToBool,
	SmileInteger64_ToInteger32,
	SmileInteger64_ToFloat64,
	SmileInteger64_ToReal64,
	SmileInteger64_ToString,
};
