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
#include <smile/smiletypes/smilebool.h>

SmileBool SmileBool_Create(Bool value)
{
	SmileBool smileInt = GC_MALLOC_STRUCT(struct SmileBoolInt);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = Smile_KnownObjects.Object;
	smileInt->kind = SMILE_KIND_BOOL;
	smileInt->vtable = SmileBool_VTable;
	smileInt->value = value;
	return smileInt;
}

Bool SmileBool_CompareEqual(SmileBool self, SmileObject other)
{
	SmileBool otherBool;

	if (SMILE_KIND(other) != SMILE_KIND_BOOL) return False;
	otherBool = (SmileBool)other;

	return self->value == otherBool->value;
}

UInt32 SmileBool_Hash(SmileBool self)
{
	return (UInt32)self->value;
}

void SmileBool_SetSecurityKey(SmileBool self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileBool_SetSecurity(SmileBool self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileBool_GetSecurity(SmileBool self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileBool_GetProperty(SmileBool self, Symbol propertyName)
{
	return self->base->vtable->getProperty((SmileObject)self, propertyName);
}

void SmileBool_SetProperty(SmileBool self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a Booleans, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileBool_HasProperty(SmileBool self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileBool_GetPropertyNames(SmileBool self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileBool_ToBool(SmileBool self)
{
	return self->value;
}

Int32 SmileBool_ToInteger32(SmileBool self)
{
	return (Int32)self->value;
}

Float64 SmileBool_ToFloat64(SmileBool self)
{
	return (Float64)self->value;
}

Real64 SmileBool_ToReal64(SmileBool self)
{
	return Real64_FromInt32(self->value);
}

STATIC_STRING(trueString, "true");
STATIC_STRING(falseString, "false");

String SmileBool_ToString(SmileBool self)
{
	return self->value ? trueString : falseString;
}

Bool SmileBool_Call(SmileBool self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError);

	return True;
}

SMILE_VTABLE(SmileBool_VTable, SmileBool)
{
	SmileBool_CompareEqual,
	SmileBool_Hash,

	SmileBool_SetSecurityKey,
	SmileBool_SetSecurity,
	SmileBool_GetSecurity,

	SmileBool_GetProperty,
	SmileBool_SetProperty,
	SmileBool_HasProperty,
	SmileBool_GetPropertyNames,

	SmileBool_ToBool,
	SmileBool_ToInteger32,
	SmileBool_ToFloat64,
	SmileBool_ToReal64,
	SmileBool_ToString,

	SmileBool_Call,
};
