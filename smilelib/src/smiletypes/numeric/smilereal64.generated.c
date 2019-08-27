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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileReal64);

SmileReal64 SmileReal64_Init(SmileReal64 smileReal, Real64 value)
{
	smileReal->base = (SmileObject)Smile_KnownBases.Real64;
	smileReal->kind = SMILE_KIND_REAL64;
	smileReal->vtable = SmileReal64_VTable;
	smileReal->value = value;
	return smileReal;
}

SmileReal64 SmileReal64_Create(Real64 value)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileReal64 smileReal = (SmileReal64)GC_MALLOC_ATOMIC(sizeof(struct SmileReal64Int));
	if (smileReal == NULL) Smile_Abort_OutOfMemory();
	smileReal->base = (SmileObject)Smile_KnownBases.Real64;
	smileReal->kind = SMILE_KIND_REAL64;
	smileReal->vtable = SmileReal64_VTable;
	smileReal->value = value;
	return smileReal;
}

static UInt32 SmileReal64_Hash(SmileReal64 obj)
{
	return Smile_ApplyHashOracle(*(UInt64 *)&obj->value);
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileReal64)
SMILE_EASY_OBJECT_NO_CALL(SmileReal64, "An Real64 object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileReal64)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileReal64)

SMILE_EASY_OBJECT_TOBOOL(SmileReal64, !Real64_IsZero(unboxedData.r64))
SMILE_EASY_OBJECT_TOSTRING(SmileReal64, Real64_ToStringEx(unboxedData.r64, 0, 0, False))

static Bool SmileReal64_CompareEqual(SmileReal64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_REAL64) {
		return a->value.value == bData.r64.value;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_REAL64) {
		return a->value.value == ((SmileReal64)b)->value.value;
	}
	else return False;
}

static Bool SmileReal64_DeepEqual(SmileReal64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_REAL64) {
		return a->value.value == bData.r64.value;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_REAL64) {
		return a->value.value == ((SmileReal64)b)->value.value;
	}
	else return False;
}

SmileObject SmileReal64_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileReal64_Unbox(SmileReal64 smileReal)
{
	return SmileUnboxedReal64_From(smileReal->value);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedReal64);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedReal64)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedReal64, "A Real64")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedReal64)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedReal64)

SMILE_EASY_OBJECT_HASH(SmileUnboxedReal64, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedReal64, !Real64_IsZero(unboxedData.r64))
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedReal64, Real64_ToStringEx(unboxedData.r64, 0, 0, False))

static Bool SmileUnboxedReal64_CompareEqual(SmileUnboxedReal64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_REAL64) {
		return aData.r64.value == bData.r64.value;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_REAL64) {
		return aData.r64.value == ((SmileReal64)b)->value.value;
	}
	else return False;
}

static Bool SmileUnboxedReal64_DeepEqual(SmileUnboxedReal64 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_REAL64) {
		return aData.r64.value == bData.r64.value;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_REAL64) {
		return aData.r64.value == ((SmileReal64)b)->value.value;
	}
	else return False;
}

static SmileObject SmileUnboxedReal64_Box(SmileArg src)
{
	return (SmileObject)SmileReal64_Create(src.unboxed.r64);
}

static SmileArg SmileUnboxedReal64_Unbox(SmileUnboxedReal64 smileUnboxedReal)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
}

static struct SmileUnboxedReal64Int SmileUnboxedReal64_Instance_Struct = {
	SMILE_KIND_UNBOXED_REAL64,
	(SmileVTable)&SmileUnboxedReal64_VTableData,
};

extern SmileUnboxedReal64 SmileUnboxedReal64_Instance;
SmileUnboxedReal64 SmileUnboxedReal64_Instance = &SmileUnboxedReal64_Instance_Struct;
