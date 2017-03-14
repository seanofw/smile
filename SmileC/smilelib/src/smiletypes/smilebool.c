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

STATIC_STRING(trueString, "true");
STATIC_STRING(falseString, "false");

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileBool)
SMILE_EASY_OBJECT_NO_CALL(SmileBool)
SMILE_EASY_OBJECT_NO_SOURCE(SmileBool)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileBool)

SMILE_EASY_OBJECT_COMPARE(SmileBool, SMILE_KIND_BOOL, a->value == b->value)
SMILE_EASY_OBJECT_HASH(SmileBool, obj->value)
SMILE_EASY_OBJECT_TOBOOL(SmileBool, obj->value)
SMILE_EASY_OBJECT_TOINT(SmileBool, obj->value)
SMILE_EASY_OBJECT_TOREAL(SmileBool, obj->value ? Real64_One : Real64_Zero)
SMILE_EASY_OBJECT_TOFLOAT(SmileBool, (Float64)obj->value)
SMILE_EASY_OBJECT_TOSTRING(SmileBool, obj->value ? trueString : falseString)

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
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedBool)
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedBool)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedBool)

SMILE_EASY_OBJECT_COMPARE(SmileUnboxedBool, SMILE_KIND_UNBOXED_BOOL, False)
SMILE_EASY_OBJECT_HASH(SmileUnboxedBool, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedBool, False)
SMILE_EASY_OBJECT_TOINT(SmileUnboxedBool, 0)
SMILE_EASY_OBJECT_TOREAL(SmileUnboxedBool, Real64_Zero)
SMILE_EASY_OBJECT_TOFLOAT(SmileUnboxedBool, 0.0)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedBool, String_Empty)

static SmileObject SmileUnboxedBool_Box(SmileArg src)
{
	return (SmileObject)SmileBool_Create(src.unboxed.b);
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

extern SmileUnboxedBool SmileUnboxedBool_Instance = &SmileUnboxedBool_Instance_Struct;
