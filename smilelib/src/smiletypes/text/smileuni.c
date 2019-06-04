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
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileUni);

SmileUni SmileUni_CreateInternal(UInt32 code)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileUni smileUni = (SmileUni)GC_MALLOC_ATOMIC(sizeof(struct SmileUniInt));
	if (smileUni == NULL) Smile_Abort_OutOfMemory();
	smileUni->base = (SmileObject)Smile_KnownBases.Uni;
	smileUni->kind = SMILE_KIND_UNI;
	smileUni->vtable = SmileUni_VTable;
	smileUni->code = code;
	return smileUni;
}

SmileUni SmileUni_Init(SmileUni smileUni, UInt32 code)
{
	smileUni->base = (SmileObject)Smile_KnownBases.Uni;
	smileUni->kind = SMILE_KIND_UNI;
	smileUni->vtable = SmileUni_VTable;
	smileUni->code = code;
	return smileUni;
}

static UInt32 SmileUni_Hash(SmileUni obj)
{
	return (UInt32)obj->code;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUni)
SMILE_EASY_OBJECT_NO_CALL(SmileUni, "A Unicode code-point object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUni)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUni)

SMILE_EASY_OBJECT_TOBOOL(SmileUni, obj->code != 0)
SMILE_EASY_OBJECT_TOSTRING(SmileUni, String_CreateFromUnicode(obj->code))

static Bool SmileUni_CompareEqual(SmileUni a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_UNI) {
		return ((SmileUni)a)->code == bData.uni;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_UNI) {
		return ((SmileUni)a)->code == ((SmileUni)b)->code;
	}
	else return False;
}

static Bool SmileUni_DeepEqual(SmileUni a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_UNI) {
		return ((SmileUni)a)->code == bData.uni;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_UNI) {
		return ((SmileUni)a)->code == ((SmileUni)b)->code;
	}
	else return False;
}

SmileObject SmileUni_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileUni_Unbox(SmileUni smileUni)
{
	return SmileUnboxedUni_From(smileUni->code);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedUni);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedUni)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedUni, "A Unicode code-point")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedUni)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedUni)

SMILE_EASY_OBJECT_HASH(SmileUnboxedUni, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedUni, True)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedUni, String_CreateFromUnicode(unboxedData.uni))

static Bool SmileUnboxedUni_CompareEqual(SmileUnboxedUni a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_UNI) {
		return aData.uni == bData.uni;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_UNI) {
		return aData.uni == ((SmileUni)b)->code;
	}
	else return False;
}

static Bool SmileUnboxedUni_DeepEqual(SmileUnboxedUni a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_UNI) {
		return aData.uni == bData.uni;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_UNI) {
		return aData.uni == ((SmileUni)b)->code;
	}
	else return False;
}

static SmileObject SmileUnboxedUni_Box(SmileArg src)
{
	return (SmileObject)SmileUni_Create(src.unboxed.uni);
}

static SmileArg SmileUnboxedUni_Unbox(SmileUnboxedUni smileUnboxedInt)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg) { 0 };
}

static struct SmileUnboxedUniInt SmileUnboxedUni_Instance_Struct = {
	SMILE_KIND_UNBOXED_UNI,
	(SmileVTable)&SmileUnboxedUni_VTableData,
};

extern SmileUnboxedUni SmileUnboxedUni_Instance;
SmileUnboxedUni SmileUnboxedUni_Instance = &SmileUnboxedUni_Instance_Struct;
