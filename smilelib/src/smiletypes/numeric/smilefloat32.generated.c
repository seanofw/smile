// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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
#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileFloat32);

SmileFloat32 SmileFloat32_Init(SmileFloat32 smileFloat, Float32 value)
{
	smileFloat->base = (SmileObject)Smile_KnownBases.Float32;
	smileFloat->kind = SMILE_KIND_FLOAT32;
	smileFloat->vtable = SmileFloat32_VTable;
	smileFloat->value = value;
	return smileFloat;
}

SmileFloat32 SmileFloat32_Create(Float32 value)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileFloat32 smileFloat = (SmileFloat32)GC_MALLOC_ATOMIC(sizeof(struct SmileFloat32Int));
	if (smileFloat == NULL) Smile_Abort_OutOfMemory();
	smileFloat->base = (SmileObject)Smile_KnownBases.Float32;
	smileFloat->kind = SMILE_KIND_FLOAT32;
	smileFloat->vtable = SmileFloat32_VTable;
	smileFloat->value = value;
	return smileFloat;
}

static UInt32 SmileFloat32_Hash(SmileFloat32 obj)
{
	return Smile_ApplyHashOracle(*(UInt32 *)&obj->value);
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileFloat32)
SMILE_EASY_OBJECT_NO_CALL(SmileFloat32, "An Float32 object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileFloat32)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileFloat32)

SMILE_EASY_OBJECT_TOBOOL(SmileFloat32, unboxedData.f32 != 0.0f)
SMILE_EASY_OBJECT_TOSTRING(SmileFloat32, Float64_ToStringEx((Float64)unboxedData.f32, 0, 0, False))

static Bool SmileFloat32_CompareEqual(SmileFloat32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_FLOAT32) {
		return a->value == bData.f32;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_FLOAT32) {
		return a->value == ((SmileFloat32)b)->value;
	}
	else return False;
}

static Bool SmileFloat32_DeepEqual(SmileFloat32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_FLOAT32) {
		return a->value == bData.f32;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_FLOAT32) {
		return a->value == ((SmileFloat32)b)->value;
	}
	else return False;
}

SmileObject SmileFloat32_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileFloat32_Unbox(SmileFloat32 smileFloat)
{
	return SmileUnboxedFloat32_From(smileFloat->value);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedFloat32);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedFloat32)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedFloat32, "A Float32")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedFloat32)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedFloat32)

SMILE_EASY_OBJECT_HASH(SmileUnboxedFloat32, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedFloat32, unboxedData.f32 != 0.0f)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedFloat32, Float64_ToStringEx((Float64)unboxedData.f32, 0, 0, False))

static Bool SmileUnboxedFloat32_CompareEqual(SmileUnboxedFloat32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_FLOAT32) {
		return aData.f32 == bData.f32;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_FLOAT32) {
		return aData.f32 == ((SmileFloat32)b)->value;
	}
	else return False;
}

static Bool SmileUnboxedFloat32_DeepEqual(SmileUnboxedFloat32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_FLOAT32) {
		return aData.f32 == bData.f32;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_FLOAT32) {
		return aData.f32 == ((SmileFloat32)b)->value;
	}
	else return False;
}

static SmileObject SmileUnboxedFloat32_Box(SmileArg src)
{
	return (SmileObject)SmileFloat32_Create(src.unboxed.f32);
}

static SmileArg SmileUnboxedFloat32_Unbox(SmileUnboxedFloat32 smileUnboxedFloat)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg){ 0 };
}

static struct SmileUnboxedFloat32Int SmileUnboxedFloat32_Instance_Struct = {
	SMILE_KIND_UNBOXED_FLOAT32,
	(SmileVTable)&SmileUnboxedFloat32_VTableData,
};

extern SmileUnboxedFloat32 SmileUnboxedFloat32_Instance;
SmileUnboxedFloat32 SmileUnboxedFloat32_Instance = &SmileUnboxedFloat32_Instance_Struct;
