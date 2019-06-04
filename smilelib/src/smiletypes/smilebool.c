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

#include <smile/numeric/real64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/parsing/parser.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileBool);

SmileBool SmileBool_Create(Bool value)
{
	SmileBool smileInt = GC_MALLOC_STRUCT(struct SmileBoolInt);
	if (smileInt == NULL) Smile_Abort_OutOfMemory();
	smileInt->base = (SmileObject)Smile_KnownBases.Bool;
	smileInt->kind = SMILE_KIND_BOOL;
	smileInt->vtable = SmileBool_VTable;
	smileInt->value = value;
	return smileInt;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileBool)
SMILE_EASY_OBJECT_NO_CALL(SmileBool, "A Bool object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileBool)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileBool)

SMILE_EASY_OBJECT_HASH(SmileBool, obj->value)
SMILE_EASY_OBJECT_TOBOOL(SmileBool, obj->value)
SMILE_EASY_OBJECT_TOSTRING(SmileBool, obj->value ? String_True : String_False)

static Bool SmileBool_CompareEqual(SmileBool a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_BOOL) {
		return ((SmileBool)a)->value == bData.b;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_BOOL) {
		return ((SmileBool)a)->value == ((SmileBool)b)->value;
	}
	else return False;
}

static Bool SmileBool_DeepEqual(SmileBool a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_BOOL) {
		return ((SmileBool)a)->value == bData.b;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_BOOL) {
		return ((SmileBool)a)->value == ((SmileBool)b)->value;
	}
	else return False;
}

SmileObject SmileBool_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileBool_Unbox(SmileBool smileBool)
{
	return SmileUnboxedBool_From(smileBool->value);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedBool);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedBool)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedBool, "A Bool")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedBool)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedBool)

SMILE_EASY_OBJECT_HASH(SmileUnboxedBool, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedBool, unboxedData.b)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedBool, unboxedData.b ? String_True : String_False)

static Bool SmileUnboxedBool_CompareEqual(SmileUnboxedBool a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_BOOL) {
		return aData.b == bData.b;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_BOOL) {
		return aData.b == ((SmileBool)b)->value;
	}
	else return False;
}

static Bool SmileUnboxedBool_DeepEqual(SmileUnboxedBool a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_BOOL) {
		return aData.b == bData.b;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_BOOL) {
		return aData.b == ((SmileBool)b)->value;
	}
	else return False;
}

static SmileObject SmileUnboxedBool_Box(SmileArg src)
{
	return (SmileObject)Smile_KnownObjects.BooleanObjs[src.unboxed.b];
}

static SmileArg SmileUnboxedBool_Unbox(SmileUnboxedBool smileUnboxedBool)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg){ 0 };
}

static struct SmileUnboxedBoolInt SmileUnboxedBool_Instance_Struct = {
	SMILE_KIND_UNBOXED_BOOL,
	(SmileVTable)&SmileUnboxedBool_VTableData,
};

extern SmileUnboxedBool SmileUnboxedBool_Instance;
SmileUnboxedBool SmileUnboxedBool_Instance = &SmileUnboxedBool_Instance_Struct;
