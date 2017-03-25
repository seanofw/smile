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
#include <smile/smiletypes/numeric/smileinteger128.h>
#include <smile/smiletypes/easyobject.h>

SMILE_EASY_OBJECT_VTABLE(SmileInteger128);

SmileInteger128 SmileInteger128_CreateInternal(Int128 value)
{
	SmileInteger128 smileInt = GC_MALLOC_STRUCT(struct SmileInteger128Int);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Integer128;
	smileInt->kind = SMILE_KIND_INTEGER128;
	smileInt->vtable = SmileInteger128_VTable;
	smileInt->value = value;
	return smileInt;
}

static UInt32 SmileInteger128_Hash(SmileInteger128 self)
{
	UInt64 hi = (UInt64)self->value.hi;
	UInt64 lo = (UInt64)self->value.lo;
	return (UInt32)(hi ^ (hi >> 32)) ^ (UInt32)(lo ^ (lo >> 32));
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

static Float64 SmileInteger128_ToFloat64(SmileInteger128 self, SmileUnboxedData unboxedData)
{
	UInt64 uhi, ulo;
	Int bits;

	UNUSED(unboxedData);

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

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileInteger128)
SMILE_EASY_OBJECT_NO_CALL(SmileInteger128)
SMILE_EASY_OBJECT_NO_SOURCE(SmileInteger128)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileInteger128)
SMILE_EASY_OBJECT_NO_UNBOX(SmileInteger128)

SMILE_EASY_OBJECT_COMPARE(SmileInteger128, SMILE_KIND_INTEGER128, a->value.hi == b->value.hi && a->value.lo == b->value.lo)
SMILE_EASY_OBJECT_DEEP_COMPARE(SmileInteger128, SMILE_KIND_INTEGER128, a->value.hi == b->value.hi && a->value.lo == b->value.lo)
SMILE_EASY_OBJECT_TOBOOL(SmileInteger128, (obj->value.hi | obj->value.lo) != 0)
SMILE_EASY_OBJECT_TOINT(SmileInteger128, (Int32)obj->value.lo)
SMILE_EASY_OBJECT_TOREAL(SmileInteger128, Real64_FromFloat64(SmileInteger128_ToFloat64(obj, unboxedData)))
SMILE_EASY_OBJECT_TOSTRING(SmileInteger128, String_Format("%ldL", obj->value.lo))
