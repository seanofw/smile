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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/internal/staticstring.h>

SMILE_EASY_OBJECT_VTABLE(SmileObject);

SmileObject SmileObject_Create(void)
{
	SmileObject obj = GC_MALLOC_STRUCT(struct SmileObjectInt);
	obj->kind = SMILE_KIND_PRIMITIVE;
	obj->base = NULL;
	obj->vtable = SmileObject_VTable;
	return obj;
}

Bool SmileObject_RecursiveEquals(SmileObject a, SmileObject b)
{
	if (a == b) return True;	// Easiest case.

	if (a == NULL || b == NULL) return False;		// Should never have C NULL.

	if (SMILE_KIND(a) != SMILE_KIND(b)) return False;	// Differing types can't be equal.

next:
	switch (SMILE_KIND(a)) {

		case SMILE_KIND_LIST:
			if (!SmileObject_RecursiveEquals(((SmileList)a)->a, ((SmileList)b)->a))
				return False;
			a = ((SmileList)a)->d;
			b = ((SmileList)b)->d;
			goto next;

		case SMILE_KIND_PRIMITIVE:
			return True;

		case SMILE_KIND_NULL:
			return True;

		case SMILE_KIND_BYTE:
			if (((SmileByte)a)->value != ((SmileByte)b)->value)
				return False;
			return True;

		case SMILE_KIND_INTEGER16:
			if (((SmileInteger16)a)->value != ((SmileInteger16)b)->value)
				return False;
			return True;

		case SMILE_KIND_INTEGER32:
			if (((SmileInteger32)a)->value != ((SmileInteger32)b)->value)
				return False;
			return True;

		case SMILE_KIND_INTEGER64:
			if (((SmileInteger64)a)->value != ((SmileInteger64)b)->value)
				return False;
			return True;

		case SMILE_KIND_BOOL:
			if (((SmileBool)a)->value != ((SmileBool)b)->value)
				return False;
			return True;

		case SMILE_KIND_FLOAT32:
			if (((SmileFloat32)a)->value != ((SmileFloat32)b)->value)
				return False;
			return True;

		case SMILE_KIND_FLOAT64:
			if (((SmileFloat64)a)->value != ((SmileFloat64)b)->value)
				return False;
			return True;

		case SMILE_KIND_SYMBOL:
			if (((SmileSymbol)a)->symbol != ((SmileSymbol)b)->symbol)
				return False;
			return True;

		case SMILE_KIND_REAL32:
			if (((SmileReal32)a)->value.value != ((SmileReal32)b)->value.value)
				return False;
			return True;

		case SMILE_KIND_REAL64:
			if (((SmileReal64)a)->value.value != ((SmileReal64)b)->value.value)
				return False;
			return True;

		case SMILE_KIND_CHAR:
			if (((SmileChar)a)->ch != ((SmileChar)b)->ch)
				return False;
			return True;

		case SMILE_KIND_UNI:
			if (((SmileUni)a)->code != ((SmileUni)b)->code)
				return False;
			return True;

		case SMILE_KIND_STRING:
			if (!String_Equals((String)a, (String)b))
				return False;
			return True;

		default:
			// All other types are reference types of some kind and are not strictly equal.
			return False;
	}
}

Bool SmileObject_DeepCompare(SmileObject self, SmileObject other)
{
	PointerSet visitedPointers = PointerSet_Create();
	Bool result;

	result = SMILE_VCALL4(self, deepEqual, (SmileUnboxedData){ 0 }, other, (SmileUnboxedData){ 0 }, visitedPointers);

	PointerSet_Clear(visitedPointers);	// Be nicer to the GC by getting rid of pointers we don't need to keep.

	return result;
}

Bool SmileArg_DeepCompare(SmileArg self, SmileArg other)
{
	PointerSet visitedPointers = PointerSet_Create();
	Bool result;

	result = SMILE_VCALL4(self.obj, deepEqual, self.unboxed, other.obj, other.unboxed, visitedPointers);

	PointerSet_Clear(visitedPointers);	// Be nicer to the GC by getting rid of pointers we don't need to keep.

	return result;
}

Bool SmileObject_CompareEqual(SmileObject self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	UNUSED(selfData);
	UNUSED(otherData);
	return self == other;
}

Bool SmileObject_DeepEqual(SmileObject self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	UNUSED(selfData);
	UNUSED(otherData);
	UNUSED(visitedPointers);
	return self == other;
}

SmileObject SmileObject_GetProperty(SmileObject self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return NullObject;
}

void SmileObject_SetProperty(SmileObject self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on base Primitive, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileObject_HasProperty(SmileObject self, Symbol propertyName)
{
	UNUSED(self);
	UNUSED(propertyName);
	return False;
}

SmileList SmileObject_GetPropertyNames(SmileObject self)
{
	UNUSED(self);
	return NullList;
}

SmileObject SmileArg_BoxByPointer(SmileArg *arg)
{
	return SmileArg_Box(*arg);
}

STATIC_STRING(PrimitiveString, "Primitive");

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileObject)
SMILE_EASY_OBJECT_NO_CALL(SmileObject, "The Primitive object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileObject)
SMILE_EASY_OBJECT_NO_UNBOX(SmileObject)

SMILE_EASY_OBJECT_HASH(SmileObject, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileObject, 0)
SMILE_EASY_OBJECT_TOSTRING(SmileObject, PrimitiveString)
