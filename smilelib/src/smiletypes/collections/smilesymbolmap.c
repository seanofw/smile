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
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/collections/smilesymbolmap.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileSymbolMap);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileSymbolMap)
SMILE_EASY_OBJECT_NO_CALL(SmileSymbolMap, "A SymbolMap object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileSymbolMap)
SMILE_EASY_OBJECT_NO_UNBOX(SmileSymbolMap)

SMILE_EASY_OBJECT_TOBOOL(SmileSymbolMap, Int32Dict_Count(&obj->dict) > 0)
SMILE_EASY_OBJECT_TOSTRING(SmileSymbolMap, String_Format("SymbolMap of %d", Int32Dict_Count(&obj->dict)))
SMILE_EASY_OBJECT_HASH(SmileSymbolMap, (PtrInt)obj)

SmileSymbolMap SmileSymbolMap_CreateWithSize(Int32 newSize)
{
	SmileSymbolMap smileMap = GC_MALLOC_STRUCT(struct SmileSymbolMapInt);
	if (smileMap == NULL) Smile_Abort_OutOfMemory();
	smileMap->base = (SmileObject)Smile_KnownBases.SymbolMap;
	smileMap->kind = SMILE_KIND_SYMBOLMAP;
	smileMap->vtable = SmileSymbolMap_VTable;
	Int32Dict_ClearWithSize(&smileMap->dict, newSize);
	return smileMap;
}

static SmileObject SmileSymbolMap_GetProperty(SmileSymbolMap self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileInteger64_Create(Int32Dict_Count(&self->dict));
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileSymbolMap_SetProperty(SmileSymbolMap self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	if (propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on SymbolMap.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on SymbolMap: This property does not exist, and maps are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileSymbolMap_HasProperty(SmileSymbolMap self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileSymbolMap_GetPropertyNames(SmileSymbolMap self)
{
	SmileList head, tail;

	UNUSED(self);

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.length));

	return head;
}

static Bool SmileSymbolMap_CompareEqual(SmileSymbolMap a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	return (SmileObject)a == b;
}

static Bool SmileSymbolMap_DeepEqual(SmileSymbolMap a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_SYMBOLMAP) {
		// If they're the same object, early-out.
		if ((SmileObject)a == b)
			return True;

		if (!PointerSet_Add(visitedPointers, a)) {
			// We've already been here before, so it's whatever the last answer was.
			return True;
		}

		// First, make sure they're both of the same cardinality.
		if (Int32Dict_Count(&a->dict) != Int32Dict_Count(&((SmileSymbolMap)b)->dict))
			return False;

		// For each key/value pair in 'b'...
		INT32DICT_WALK(&((SmileSymbolMap)b)->dict,
			{
				// See if this node in 'b' has a matching node in 'a'.
				void *avalue;
				if (!Int32Dict_TryGetValue(&a->dict, node->key, &avalue))
					return False;

				// If they're not the same pointer, recursively deep-compare their data.
				if (avalue != (void *)node->value) {
					if (!SMILE_VCALL4((SmileObject)avalue, deepEqual, (SmileUnboxedData) { 0 }, (SmileObject)node->value, (SmileUnboxedData) { 0 }, visitedPointers))
						return False;
				}
			})

		// Everything matches up, so we're done.
		return True;
	}
	else return False;
}
