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
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileInteger64);

SmileInteger64 SmileInteger64_CreateInternal(Int64 value)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileInteger64 smileInt = (SmileInteger64)GC_MALLOC_ATOMIC(sizeof(struct SmileInteger64Int));
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Integer64;
	smileInt->kind = SMILE_KIND_INTEGER64;
	smileInt->vtable = SmileInteger64_VTable;
	smileInt->value = value;
	return smileInt;
}

static UInt32 SmileInteger64_Hash(SmileInteger64 self)
{
	UInt64 value = (UInt64)self->value;
	return (UInt32)(value ^ (value >> 32));
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileInteger64)
SMILE_EASY_OBJECT_NO_CALL(SmileInteger64)
SMILE_EASY_OBJECT_NO_SOURCE(SmileInteger64)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileInteger64)

SMILE_EASY_OBJECT_TOBOOL(SmileInteger64, obj->value != 0)
SMILE_EASY_OBJECT_TOINT(SmileInteger64, (Int32)obj->value)
SMILE_EASY_OBJECT_TOREAL(SmileInteger64, Real64_FromInt64(obj->value))
SMILE_EASY_OBJECT_TOFLOAT(SmileInteger64, (Float64)obj->value)
SMILE_EASY_OBJECT_TOSTRING(SmileInteger64, String_Format("%ld", obj->value))

static Bool SmileInteger64_CompareEqual(SmileInteger64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER64) {
		return ((SmileInteger64)a)->value == bData.i64;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER64) {
		return ((SmileInteger64)a)->value == ((SmileInteger64)b)->value;
	}
	else return False;
}

static Bool SmileInteger64_DeepEqual(SmileInteger64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER64) {
		return ((SmileInteger64)a)->value == bData.i64;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER64) {
		return ((SmileInteger64)a)->value == ((SmileInteger64)b)->value;
	}
	else return False;
}

SmileObject SmileInteger64_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileInteger64_Unbox(SmileInteger64 smileInt)
{
	return SmileUnboxedInteger64_From(smileInt->value);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedInteger64);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedInteger64)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedInteger64)
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedInteger64)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedInteger64)

SMILE_EASY_OBJECT_HASH(SmileUnboxedInteger64, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedInteger64, (Bool)!!unboxedData.i64)
SMILE_EASY_OBJECT_TOINT(SmileUnboxedInteger64, (Int32)unboxedData.i64)
SMILE_EASY_OBJECT_TOREAL(SmileUnboxedInteger64, Real64_FromInt64(unboxedData.i64))
SMILE_EASY_OBJECT_TOFLOAT(SmileUnboxedInteger64, (Float64)unboxedData.i64)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedInteger64, String_Format("%ld", unboxedData.i64))

static Bool SmileUnboxedInteger64_CompareEqual(SmileUnboxedInteger64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER64) {
		return aData.i64 == bData.i64;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER64) {
		return aData.i64 == ((SmileInteger64)b)->value;
	}
	else return False;
}

static Bool SmileUnboxedInteger64_DeepEqual(SmileUnboxedInteger64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER64) {
		return aData.i64 == bData.i64;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER64) {
		return aData.i64 == ((SmileInteger64)b)->value;
	}
	else return False;
}

static SmileObject SmileUnboxedInteger64_Box(SmileArg src)
{
	return (SmileObject)SmileInteger64_Create(src.unboxed.i64);
}

static SmileArg SmileUnboxedInteger64_Unbox(SmileUnboxedInteger64 smileUnboxedInt)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg){ 0 };
}

static struct SmileUnboxedInteger64Int SmileUnboxedInteger64_Instance_Struct = {
	SMILE_KIND_UNBOXED_INTEGER64,
	(SmileVTable)&SmileUnboxedInteger64_VTableData,
};

extern SmileUnboxedInteger64 SmileUnboxedInteger64_Instance = &SmileUnboxedInteger64_Instance_Struct;
