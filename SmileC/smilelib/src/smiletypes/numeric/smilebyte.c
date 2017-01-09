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
#include <smile/smiletypes/numeric/smilebyte.h>

SmileByte SmileByte_CreateInternal(Byte value)
{
	SmileByte smileByte = GC_MALLOC_STRUCT(struct SmileByteInt);
	if (smileByte == NULL) Smile_Abort_OutOfMemory();
	smileByte->base = (SmileObject)Smile_KnownBases.Byte;
	smileByte->kind = SMILE_KIND_BYTE;
	smileByte->vtable = SmileByte_VTable;
	smileByte->value = value;
	return smileByte;
}

Bool SmileByte_CompareEqual(SmileByte self, SmileObject other)
{
	SmileByte otherInt;

	if (SMILE_KIND(other) != SMILE_KIND_BYTE) return False;
	otherInt = (SmileByte)other;

	return self->value == otherInt->value;
}

UInt32 SmileByte_Hash(SmileByte self)
{
	return self->value;
}

void SmileByte_SetSecurityKey(SmileByte self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileByte_SetSecurity(SmileByte self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileByte_GetSecurity(SmileByte self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileByte_GetProperty(SmileByte self, Symbol propertyName)
{
	return self->base->vtable->getProperty(self->base, propertyName);
}

void SmileByte_SetProperty(SmileByte self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a byte, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileByte_HasProperty(SmileByte self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileByte_GetPropertyNames(SmileByte self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileByte_ToBool(SmileByte self)
{
	return self->value != 0;
}

Int32 SmileByte_ToInteger32(SmileByte self)
{
	return self->value;
}

Float64 SmileByte_ToFloat64(SmileByte self)
{
	return (Float64)self->value;
}

Real64 SmileByte_ToReal64(SmileByte self)
{
	return Real64_FromInt32(self->value);
}

String SmileByte_ToString(SmileByte self)
{
	return String_Format("%ux", (UInt32)self->value);
}

Bool SmileByte_Call(SmileByte self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError);

	return True;
}

SMILE_VTABLE(SmileByte_VTable, SmileByte)
{
	SmileByte_CompareEqual,
	SmileByte_Hash,

	SmileByte_SetSecurityKey,
	SmileByte_SetSecurity,
	SmileByte_GetSecurity,

	SmileByte_GetProperty,
	SmileByte_SetProperty,
	SmileByte_HasProperty,
	SmileByte_GetPropertyNames,

	SmileByte_ToBool,
	SmileByte_ToInteger32,
	SmileByte_ToFloat64,
	SmileByte_ToReal64,
	SmileByte_ToString,

	SmileByte_Call,
};
