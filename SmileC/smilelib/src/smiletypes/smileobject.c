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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/easyobject.h>

SMILE_EASY_OBJECT_VTABLE(SmileObject);

SmileObject SmileObject_Create(void)
{
	SmileObject obj = GC_MALLOC_STRUCT(struct SmileObjectInt);
	obj->kind = SMILE_KIND_PRIMITIVE;
	obj->base = NULL;
	obj->vtable = SmileObject_VTable;
	return obj;
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

STATIC_STRING(PrimitiveString, "Primitive");

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileObject)
SMILE_EASY_OBJECT_NO_CALL(SmileObject)
SMILE_EASY_OBJECT_NO_SOURCE(SmileObject)
SMILE_EASY_OBJECT_NO_UNBOX(SmileObject)

SMILE_EASY_OBJECT_HASH(SmileObject, 0)
SMILE_EASY_OBJECT_TOBOOL(SmileObject, 0)
SMILE_EASY_OBJECT_TOINT(SmileObject, 0)
SMILE_EASY_OBJECT_TOREAL(SmileObject, Real64_Zero)
SMILE_EASY_OBJECT_TOFLOAT(SmileObject, 0.0)
SMILE_EASY_OBJECT_TOSTRING(SmileObject, PrimitiveString)
