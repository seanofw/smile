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
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileInteger32);

SmileInteger32 SmileInteger32_Init(SmileInteger32 smileInt, Int32 value)
{
	smileInt->base = (SmileObject)Smile_KnownBases.Integer32;
	smileInt->kind = SMILE_KIND_INTEGER32;
	smileInt->vtable = SmileInteger32_VTable;
	smileInt->value = value;
	return smileInt;
}

SmileInteger32 SmileInteger32_CreateInternal(Int32 value)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileInteger32 smileInt = (SmileInteger32)GC_MALLOC_ATOMIC(sizeof(struct SmileInteger32Int));
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Integer32;
	smileInt->kind = SMILE_KIND_INTEGER32;
	smileInt->vtable = SmileInteger32_VTable;
	smileInt->value = value;
	return smileInt;
}

static UInt32 SmileInteger32_Hash(SmileInteger32 obj)
{
	return (UInt32)obj->value;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileInteger32)
SMILE_EASY_OBJECT_NO_CALL(SmileInteger32, "An Integer32 object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileInteger32)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileInteger32)

SMILE_EASY_OBJECT_TOBOOL(SmileInteger32, obj->value != 0)
SMILE_EASY_OBJECT_TOSTRING(SmileInteger32, String_Format("%ld", obj->value))

static Bool SmileInteger32_CompareEqual(SmileInteger32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER32) {
		return ((SmileInteger32)a)->value == bData.i32;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER32) {
		return ((SmileInteger32)a)->value == ((SmileInteger32)b)->value;
	}
	else return False;
}

static Bool SmileInteger32_DeepEqual(SmileInteger32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER32) {
		return ((SmileInteger32)a)->value == bData.i32;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER32) {
		return ((SmileInteger32)a)->value == ((SmileInteger32)b)->value;
	}
	else return False;
}

SmileObject SmileInteger32_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileInteger32_Unbox(SmileInteger32 smileInt)
{
	return SmileUnboxedInteger32_From(smileInt->value);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedInteger32);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedInteger32)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedInteger32, "An Integer32")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedInteger32)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedInteger32)

SMILE_EASY_OBJECT_HASH(SmileUnboxedInteger32, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedInteger32, (Bool)!!unboxedData.i32)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedInteger32, String_CreateFromInteger(unboxedData.i32, 10, False))

static Bool SmileUnboxedInteger32_CompareEqual(SmileUnboxedInteger32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER32) {
		return aData.i32 == bData.i32;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER32) {
		return aData.i32 == ((SmileInteger32)b)->value;
	}
	else return False;
}

static Bool SmileUnboxedInteger32_DeepEqual(SmileUnboxedInteger32 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER32) {
		return aData.i32 == bData.i32;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER32) {
		return aData.i32 == ((SmileInteger32)b)->value;
	}
	else return False;
}

static SmileObject SmileUnboxedInteger32_Box(SmileArg src)
{
	return (SmileObject)SmileInteger32_Create(src.unboxed.i32);
}

static SmileArg SmileUnboxedInteger32_Unbox(SmileUnboxedInteger32 smileUnboxedInt)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg){ 0 };
}

static struct SmileUnboxedInteger32Int SmileUnboxedInteger32_Instance_Struct = {
	SMILE_KIND_UNBOXED_INTEGER32,
	(SmileVTable)&SmileUnboxedInteger32_VTableData,
};

extern SmileUnboxedInteger32 SmileUnboxedInteger32_Instance;
SmileUnboxedInteger32 SmileUnboxedInteger32_Instance = &SmileUnboxedInteger32_Instance_Struct;
