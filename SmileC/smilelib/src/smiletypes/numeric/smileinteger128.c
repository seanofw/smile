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
#include <smile/smiletypes/numeric/smileinteger128.h>

SmileInteger128 SmileInteger128_CreateInternal(Int128 value)
{
	SmileInteger128 smileInt = GC_MALLOC_STRUCT(struct SmileInteger128Int);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = Smile_KnownObjects.Object;
	smileInt->kind = SMILE_KIND_INTEGER128;
	smileInt->vtable = SmileInteger128_VTable;
	smileInt->value = value;
	return smileInt;
}

Bool SmileInteger128_CompareEqual(SmileInteger128 self, SmileObject other)
{
	SmileInteger128 otherInt;

	if (SMILE_KIND(other) != SMILE_KIND_INTEGER64) return False;
	otherInt = (SmileInteger128)other;

	return self->value.hi == otherInt->value.hi
		&& self->value.lo == otherInt->value.lo;
}

UInt32 SmileInteger128_Hash(SmileInteger128 self)
{
	UInt64 hi = (UInt64)self->value.hi;
	UInt64 lo = (UInt64)self->value.lo;
	return (UInt32)(hi ^ (hi >> 32)) ^ (UInt32)(lo ^ (lo >> 32));
}

void SmileInteger128_SetSecurityKey(SmileInteger128 self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileInteger128_SetSecurity(SmileInteger128 self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileInteger128_GetSecurity(SmileInteger128 self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
}

SmileObject SmileInteger128_GetProperty(SmileInteger128 self, Symbol propertyName)
{
	return self->base->vtable->getProperty((SmileObject)self, propertyName);
}

void SmileInteger128_SetProperty(SmileInteger128 self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on an integer, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileInteger128_HasProperty(SmileInteger128 self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileInteger128_GetPropertyNames(SmileInteger128 self)
{
	UNUSED(self);
	return NullList;
}

Bool SmileInteger128_ToBool(SmileInteger128 self)
{
	return (self->value.hi | self->value.lo) != 0;
}

Int32 SmileInteger128_ToInteger32(SmileInteger128 self)
{
	return (Int32)self->value.lo;
}

Inline Int UIntLg(UInt64 value)
{
	Int lg = 0;
	while (value) {
		value >>= 1;
		lg++;
	}
	return lg;
}

Float64 SmileInteger128_ToFloat64(SmileInteger128 self)
{
	UInt64 uhi, ulo;
	Int bits;

	if (self->value.hi < 0) {
		if (self->value.hi == -1)
			return (Float64)self->value.lo;
		uhi = (UInt64)-self->value.hi;
		ulo = (UInt64)self->value.lo;
		bits = UIntLg(uhi);
		ulo = (ulo >> bits) | (uhi << (64 - bits));
		return -ldexp((Float64)(~ulo + 1), bits);
	}
	else {
		if (self->value.hi == 0)
			return (Float64)self->value.lo;
		uhi = (UInt64)self->value.hi;
		ulo = (UInt64)self->value.lo;
		bits = UIntLg(uhi);
		ulo = (ulo >> bits) | (uhi << (64 - bits));
		return ldexp((Float64)ulo, bits);
	}
}

Real64 SmileInteger128_ToReal64(SmileInteger128 self)
{
	return Real64_FromFloat64(SmileInteger128_ToFloat64(self));
}

String SmileInteger128_ToString(SmileInteger128 self)
{
	return String_Format("%ldL", self->value.lo);
}

Bool SmileInteger128_Call(SmileInteger128 self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError);

	return True;
}

SMILE_VTABLE(SmileInteger128_VTable, SmileInteger128)
{
	SmileInteger128_CompareEqual,
	SmileInteger128_Hash,

	SmileInteger128_SetSecurityKey,
	SmileInteger128_SetSecurity,
	SmileInteger128_GetSecurity,

	SmileInteger128_GetProperty,
	SmileInteger128_SetProperty,
	SmileInteger128_HasProperty,
	SmileInteger128_GetPropertyNames,

	SmileInteger128_ToBool,
	SmileInteger128_ToInteger32,
	SmileInteger128_ToFloat64,
	SmileInteger128_ToReal64,
	SmileInteger128_ToString,

	SmileInteger128_Call,
};
