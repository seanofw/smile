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
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileInteger16);

SmileInteger16 SmileInteger16_Init(SmileInteger16 smileInt, Int16 value)
{
	smileInt->base = (SmileObject)Smile_KnownBases.Integer16;
	smileInt->kind = SMILE_KIND_INTEGER16;
	smileInt->vtable = SmileInteger16_VTable;
	smileInt->value = value;
	return smileInt;
}

SmileInteger16 SmileInteger16_CreateInternal(Int16 value)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileInteger16 smileInt = (SmileInteger16)GC_MALLOC_ATOMIC(sizeof(struct SmileInteger16Int));
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Integer16;
	smileInt->kind = SMILE_KIND_INTEGER16;
	smileInt->vtable = SmileInteger16_VTable;
	smileInt->value = value;
	return smileInt;
}

static UInt32 SmileInteger16_Hash(SmileInteger16 obj)
{
	return Smile_ApplyHashOracle(obj->value);
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileInteger16)
SMILE_EASY_OBJECT_NO_CALL(SmileInteger16, "An Integer16 object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileInteger16)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileInteger16)

SMILE_EASY_OBJECT_TOBOOL(SmileInteger16, obj->value != 0)
SMILE_EASY_OBJECT_TOSTRING(SmileInteger16, String_Format("%ld", obj->value))

static Bool SmileInteger16_CompareEqual(SmileInteger16 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER16) {
		return ((SmileInteger16)a)->value == bData.i16;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER16) {
		return ((SmileInteger16)a)->value == ((SmileInteger16)b)->value;
	}
	else return False;
}

static Bool SmileInteger16_DeepEqual(SmileInteger16 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER16) {
		return ((SmileInteger16)a)->value == bData.i16;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER16) {
		return ((SmileInteger16)a)->value == ((SmileInteger16)b)->value;
	}
	else return False;
}

SmileObject SmileInteger16_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileInteger16_Unbox(SmileInteger16 smileInt)
{
	return SmileUnboxedInteger16_From(smileInt->value);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedInteger16);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedInteger16)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedInteger16, "An Integer16")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedInteger16)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedInteger16)

SMILE_EASY_OBJECT_HASH(SmileUnboxedInteger16, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedInteger16, (Bool)!!unboxedData.i16)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedInteger16, String_CreateFromInteger(unboxedData.i16, 10, False))

static Bool SmileUnboxedInteger16_CompareEqual(SmileUnboxedInteger16 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER16) {
		return aData.i16 == bData.i16;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER16) {
		return aData.i16 == ((SmileInteger16)b)->value;
	}
	else return False;
}

static Bool SmileUnboxedInteger16_DeepEqual(SmileUnboxedInteger16 a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_INTEGER16) {
		return aData.i16 == bData.i16;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_INTEGER16) {
		return aData.i16 == ((SmileInteger16)b)->value;
	}
	else return False;
}

static SmileObject SmileUnboxedInteger16_Box(SmileArg src)
{
	return (SmileObject)SmileInteger16_Create(src.unboxed.i16);
}

static SmileArg SmileUnboxedInteger16_Unbox(SmileUnboxedInteger16 smileUnboxedInt)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg){ 0 };
}

static struct SmileUnboxedInteger16Int SmileUnboxedInteger16_Instance_Struct = {
	SMILE_KIND_UNBOXED_INTEGER16,
	(SmileVTable)&SmileUnboxedInteger16_VTableData,
};

extern SmileUnboxedInteger16 SmileUnboxedInteger16_Instance;
SmileUnboxedInteger16 SmileUnboxedInteger16_Instance = &SmileUnboxedInteger16_Instance_Struct;
