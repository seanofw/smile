// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

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

#include <smile/numeric/float64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileFloat64);

SmileFloat64 SmileFloat64_Init(SmileFloat64 smileFloat, Float64 value)
{
	smileFloat->base = (SmileObject)Smile_KnownBases.Float64;
	smileFloat->kind = SMILE_KIND_FLOAT64;
	smileFloat->vtable = SmileFloat64_VTable;
	smileFloat->value = value;
	return smileFloat;
}

SmileFloat64 SmileFloat64_Create(Float64 value)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileFloat64 smileFloat = (SmileFloat64)GC_MALLOC_ATOMIC(sizeof(struct SmileFloat64Int));
	if (smileFloat == NULL) Smile_Abort_OutOfMemory();
	smileFloat->base = (SmileObject)Smile_KnownBases.Float64;
	smileFloat->kind = SMILE_KIND_FLOAT64;
	smileFloat->vtable = SmileFloat64_VTable;
	smileFloat->value = value;
	return smileFloat;
}

static UInt32 SmileFloat64_Hash(SmileFloat64 obj)
{
	return (UInt32)(*(UInt64 *)&obj->value ^ (*(UInt64 *)&obj->value >> 32));
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileFloat64)
SMILE_EASY_OBJECT_NO_CALL(SmileFloat64, "An Float64 object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileFloat64)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileFloat64)

SMILE_EASY_OBJECT_TOBOOL(SmileFloat64, unboxedData.f64 != 0.0)
SMILE_EASY_OBJECT_TOSTRING(SmileFloat64, Float64_ToStringEx(unboxedData.f64, 0, 0, False))

static Bool SmileFloat64_CompareEqual(SmileFloat64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_FLOAT64) {
		return a->value == bData.f64;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_FLOAT64) {
		return a->value == ((SmileFloat64)b)->value;
	}
	else return False;
}

static Bool SmileFloat64_DeepEqual(SmileFloat64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_FLOAT64) {
		return a->value == bData.f64;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_FLOAT64) {
		return a->value == ((SmileFloat64)b)->value;
	}
	else return False;
}

SmileObject SmileFloat64_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileFloat64_Unbox(SmileFloat64 smileFloat)
{
	return SmileUnboxedFloat64_From(smileFloat->value);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedFloat64);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedFloat64)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedFloat64, "A Float64")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedFloat64)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedFloat64)

SMILE_EASY_OBJECT_HASH(SmileUnboxedFloat64, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedFloat64, unboxedData.f64 != 0.0)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedFloat64, Float64_ToStringEx(unboxedData.f64, 0, 0, False))

static Bool SmileUnboxedFloat64_CompareEqual(SmileUnboxedFloat64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_FLOAT64) {
		return aData.f64 == bData.f64;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_FLOAT64) {
		return aData.f64 == ((SmileFloat64)b)->value;
	}
	else return False;
}

static Bool SmileUnboxedFloat64_DeepEqual(SmileUnboxedFloat64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_FLOAT64) {
		return aData.f64 == bData.f64;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_FLOAT64) {
		return aData.f64 == ((SmileFloat64)b)->value;
	}
	else return False;
}

static SmileObject SmileUnboxedFloat64_Box(SmileArg src)
{
	return (SmileObject)SmileFloat64_Create(src.unboxed.f64);
}

static SmileArg SmileUnboxedFloat64_Unbox(SmileUnboxedFloat64 smileUnboxedFloat)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg){ 0 };
}

static struct SmileUnboxedFloat64Int SmileUnboxedFloat64_Instance_Struct = {
	SMILE_KIND_UNBOXED_FLOAT64,
	(SmileVTable)&SmileUnboxedFloat64_VTableData,
};

extern SmileUnboxedFloat64 SmileUnboxedFloat64_Instance;
SmileUnboxedFloat64 SmileUnboxedFloat64_Instance = &SmileUnboxedFloat64_Instance_Struct;
