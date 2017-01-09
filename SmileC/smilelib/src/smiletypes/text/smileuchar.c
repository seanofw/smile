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
#include <smile/smiletypes/text/smileuchar.h>

SmileUChar SmileUChar_CreateInternal(UInt32 value)
{
	SmileUChar smileUChar = GC_MALLOC_STRUCT(struct SmileUCharInt);
	if (smileUChar == NULL) Smile_Abort_OutOfMemory();
	smileUChar->base = (SmileObject)Smile_KnownBases.UChar;
	smileUChar->kind = SMILE_KIND_BYTE;
	smileUChar->vtable = SmileUChar_VTable;
	smileUChar->value = value;
	return smileUChar;
}

Bool SmileUChar_CompareEqual(SmileUChar self, SmileObject other)
{
	SmileUChar otherInt;

	if (SMILE_KIND(other) != SMILE_KIND_UCHAR) return False;
	otherInt = (SmileUChar)other;

	return self->value == otherInt->value;
}

UInt32 SmileUChar_Hash(SmileUChar self)
{
	return self->value;
}

void SmileUChar_SetSecurityKey(SmileUChar self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileUChar_SetSecurity(SmileUChar self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileUChar_GetSecurity(SmileUChar self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileUChar_GetProperty(SmileUChar self, Symbol propertyName)
{
	return self->base->vtable->getProperty(self->base, propertyName);
}

void SmileUChar_SetProperty(SmileUChar self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a uchar, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileUChar_HasProperty(SmileUChar self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileUChar_GetPropertyNames(SmileUChar self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileUChar_ToBool(SmileUChar self)
{
	return self->value != 0;
}

Int32 SmileUChar_ToInteger32(SmileUChar self)
{
	return self->value;
}

Float64 SmileUChar_ToFloat64(SmileUChar self)
{
	return (Float64)self->value;
}

Real64 SmileUChar_ToReal64(SmileUChar self)
{
	return Real64_FromInt32(self->value);
}

String SmileUChar_ToString(SmileUChar self)
{
	return String_Format("\\u%04X", (UInt32)self->value);
}

Bool SmileUChar_Call(SmileUChar self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError);

	return True;
}

SMILE_VTABLE(SmileUChar_VTable, SmileUChar)
{
	SmileUChar_CompareEqual,
	SmileUChar_Hash,

	SmileUChar_SetSecurityKey,
	SmileUChar_SetSecurity,
	SmileUChar_GetSecurity,

	SmileUChar_GetProperty,
	SmileUChar_SetProperty,
	SmileUChar_HasProperty,
	SmileUChar_GetPropertyNames,

	SmileUChar_ToBool,
	SmileUChar_ToInteger32,
	SmileUChar_ToFloat64,
	SmileUChar_ToReal64,
	SmileUChar_ToString,

	SmileUChar_Call,
};
