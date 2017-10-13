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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileReal32);

SmileReal32 SmileReal32_Create(Real32 value)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileReal32 smileReal = (SmileReal32)GC_MALLOC_ATOMIC(sizeof(struct SmileReal32Int));
	if (smileReal == NULL) Smile_Abort_OutOfMemory();
	smileReal->base = (SmileObject)Smile_KnownBases.Real32;
	smileReal->kind = SMILE_KIND_REAL32;
	smileReal->vtable = SmileReal32_VTable;
	smileReal->value = value;
	return smileReal;
}

static UInt32 SmileReal32_Hash(SmileReal32 obj)
{
	return (UInt32)(*(UInt32 *)&obj->value);
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileReal32)
SMILE_EASY_OBJECT_NO_CALL(SmileReal32, "An Real32 object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileReal32)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileReal32)

SMILE_EASY_OBJECT_TOBOOL(SmileReal32, !Real32_IsZero(unboxedData.r32))
SMILE_EASY_OBJECT_TOSTRING(SmileReal32, Real32_ToStringEx(unboxedData.r32, 0, 0, False))

static Bool SmileReal32_CompareEqual(SmileReal32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_REAL32) {
		return a->value.value == bData.r32.value;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_REAL32) {
		return a->value.value == ((SmileReal32)b)->value.value;
	}
	else return False;
}

static Bool SmileReal32_DeepEqual(SmileReal32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_REAL32) {
		return a->value.value == bData.r32.value;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_REAL32) {
		return a->value.value == ((SmileReal32)b)->value.value;
	}
	else return False;
}

SmileObject SmileReal32_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileReal32_Unbox(SmileReal32 smileReal)
{
	return SmileUnboxedReal32_From(smileReal->value);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedReal32);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedReal32)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedReal32, "A Real32")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedReal32)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedReal32)

SMILE_EASY_OBJECT_HASH(SmileUnboxedReal32, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedReal32, !Real32_IsZero(unboxedData.r32))
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedReal32, Real32_ToStringEx(unboxedData.r32, 0, 0, False))

static Bool SmileUnboxedReal32_CompareEqual(SmileUnboxedReal32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_REAL32) {
		return aData.r32.value == bData.r32.value;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_REAL32) {
		return aData.r32.value == ((SmileReal32)b)->value.value;
	}
	else return False;
}

static Bool SmileUnboxedReal32_DeepEqual(SmileUnboxedReal32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_REAL32) {
		return aData.r32.value == bData.r32.value;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_REAL32) {
		return aData.r32.value == ((SmileReal32)b)->value.value;
	}
	else return False;
}

static SmileObject SmileUnboxedReal32_Box(SmileArg src)
{
	return (SmileObject)SmileReal32_Create(src.unboxed.r32);
}

static SmileArg SmileUnboxedReal32_Unbox(SmileUnboxedReal32 smileUnboxedReal)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg){ 0 };
}

static struct SmileUnboxedReal32Int SmileUnboxedReal32_Instance_Struct = {
	SMILE_KIND_UNBOXED_REAL32,
	(SmileVTable)&SmileUnboxedReal32_VTableData,
};

extern SmileUnboxedReal32 SmileUnboxedReal32_Instance;
SmileUnboxedReal32 SmileUnboxedReal32_Instance = &SmileUnboxedReal32_Instance_Struct;
