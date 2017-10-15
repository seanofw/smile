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
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileChar);

SmileChar SmileChar_Init(SmileChar smileChar, Byte ch)
{
	smileChar->base = (SmileObject)Smile_KnownBases.Char;
	smileChar->kind = SMILE_KIND_CHAR;
	smileChar->vtable = SmileChar_VTable;
	smileChar->ch = ch;
	return smileChar;
}

static UInt32 SmileChar_Hash(SmileChar obj)
{
	return (UInt32)obj->ch;
}

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileChar)
SMILE_EASY_OBJECT_NO_CALL(SmileChar, "A Char object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileChar)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileChar)

SMILE_EASY_OBJECT_TOBOOL(SmileChar, obj->ch != 0)
SMILE_EASY_OBJECT_TOSTRING(SmileChar, String_CreateRepeat(obj->ch, 1))

static Bool SmileChar_CompareEqual(SmileChar a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_CHAR) {
		return ((SmileChar)a)->ch == bData.ch;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_CHAR) {
		return ((SmileChar)a)->ch == ((SmileChar)b)->ch;
	}
	else return False;
}

static Bool SmileChar_DeepEqual(SmileChar a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_CHAR) {
		return ((SmileChar)a)->ch == bData.ch;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_CHAR) {
		return ((SmileChar)a)->ch == ((SmileChar)b)->ch;
	}
	else return False;
}

SmileObject SmileChar_Box(SmileArg src)
{
	return src.obj;
}

SmileArg SmileChar_Unbox(SmileChar smileChar)
{
	return SmileUnboxedChar_From(smileChar->ch);
}

//-------------------------------------------------------------------------------------------------

SMILE_EASY_OBJECT_VTABLE(SmileUnboxedChar);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUnboxedChar)
SMILE_EASY_OBJECT_NO_CALL(SmileUnboxedChar, "A Char")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUnboxedChar)
SMILE_EASY_OBJECT_NO_PROPERTIES(SmileUnboxedChar)

SMILE_EASY_OBJECT_HASH(SmileUnboxedChar, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileUnboxedChar, True)
SMILE_EASY_OBJECT_TOSTRING(SmileUnboxedChar, String_CreateRepeat(unboxedData.ch, 1))

static Bool SmileUnboxedChar_CompareEqual(SmileUnboxedChar a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_CHAR) {
		return aData.ch == bData.ch;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_CHAR) {
		return aData.ch == ((SmileChar)b)->ch;
	}
	else return False;
}

static Bool SmileUnboxedChar_DeepEqual(SmileUnboxedChar a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNBOXED_CHAR) {
		return aData.ch == bData.ch;
	}
	else if (SMILE_KIND(b) == SMILE_KIND_CHAR) {
		return aData.ch == ((SmileChar)b)->ch;
	}
	else return False;
}

static SmileObject SmileUnboxedChar_Box(SmileArg src)
{
	return (SmileObject)SmileChar_Create(src.unboxed.ch);
}

static SmileArg SmileUnboxedChar_Unbox(SmileUnboxedChar smileUnboxedInt)
{
	Smile_Abort_FatalError("Cannot re-unbox a unboxed object.");
	return (SmileArg) { 0 };
}

static struct SmileUnboxedCharInt SmileUnboxedChar_Instance_Struct = {
	SMILE_KIND_UNBOXED_CHAR,
	(SmileVTable)&SmileUnboxedChar_VTableData,
};

extern SmileUnboxedChar SmileUnboxedChar_Instance;
SmileUnboxedChar SmileUnboxedChar_Instance = &SmileUnboxedChar_Instance_Struct;
