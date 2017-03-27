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

#include <smile/types.h>
#include <smile/numeric/real64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>

SMILE_EASY_OBJECT_VTABLE(SmilePair);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmilePair);
SMILE_EASY_OBJECT_NO_REALS(SmilePair);
SMILE_EASY_OBJECT_NO_CALL(SmilePair);
SMILE_EASY_OBJECT_NO_UNBOX(SmilePair)

SmilePair SmilePair_Create(SmileObject left, SmileObject right)
{
	SmilePair smilePair = GC_MALLOC_STRUCT(struct SmilePairInt);
	if (smilePair == NULL) Smile_Abort_OutOfMemory();
	smilePair->base = (SmileObject)Smile_KnownBases.Pair;
	smilePair->kind = SMILE_KIND_PAIR | SMILE_SECURITY_WRITABLE;
	smilePair->vtable = SmilePair_VTable;
	smilePair->left = left;
	smilePair->right = right;
	return smilePair;
}

SmilePair SmilePair_CreateWithSource(SmileObject left, SmileObject right, LexerPosition position)
{
	struct SmilePairWithSourceInt *smilePair = GC_MALLOC_STRUCT(struct SmilePairWithSourceInt);
	if (smilePair == NULL) Smile_Abort_OutOfMemory();
	smilePair->base = (SmileObject)Smile_KnownBases.Pair;
	smilePair->kind = SMILE_KIND_PAIR | SMILE_SECURITY_WRITABLE | SMILE_FLAG_WITHSOURCE;
	smilePair->vtable = SmilePair_VTable;
	smilePair->left = left;
	smilePair->right = right;
	smilePair->position = position;
	return (SmilePair)smilePair;
}

static Bool SmilePair_CompareEqual(SmilePair self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	UNUSED(selfData);
	UNUSED(otherData);
	return (SmileObject)self == other;
}

static Bool SmilePair_DeepEqual(SmilePair self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	UNUSED(selfData);
	UNUSED(otherData);

	for (;;) {
		if (SMILE_KIND(other) != SMILE_KIND(self)) return False;

		if (PointerSet_Add(visitedPointers, self->left)) {
			if (!SMILE_VCALL4(self->left, deepEqual, (SmileUnboxedData){ 0 }, ((SmilePair)other)->left, (SmileUnboxedData){ 0 }, visitedPointers))
				return False;
		}

		if (PointerSet_Add(visitedPointers, self->right)) {
			if (SMILE_KIND(self->right) != SMILE_KIND_PAIR) {
				if (!SMILE_VCALL4(self->right, deepEqual, (SmileUnboxedData){ 0 }, ((SmilePair)other)->right, (SmileUnboxedData){ 0 }, visitedPointers))
					return False;
				return True;
			}
		}

		self = (SmilePair)self->right;
		other = ((SmilePair)other)->right;
	}
}

static UInt32 SmilePair_Hash(SmilePair self)
{
	return (UInt32)(PtrInt)self;
}

static SmileObject SmilePair_GetProperty(SmilePair self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.left)
		return self->left;
	else if (propertyName == Smile_KnownSymbols.right)
		return self->right;
	else
		return NullObject;
}

static void SmilePair_SetProperty(SmilePair self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.left)
		self->left = value;
	else if (propertyName == Smile_KnownSymbols.right)
		self->right = value;
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on pair: This property does not exist, and pairs are not appendable objects.",
			SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmilePair_HasProperty(SmilePair self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.left || propertyName == Smile_KnownSymbols.right);
}

static SmileList SmilePair_GetPropertyNames(SmilePair self)
{
	SmileList head, tail;

	UNUSED(self);

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.left));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.right));

	return head;
}

static Bool SmilePair_ToBool(SmilePair self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return True;
}

static Int32 SmilePair_ToInteger32(SmilePair self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return 1;
}

static String SmilePair_ToString(SmilePair self, SmileUnboxedData unboxedData)
{
	UNUSED(unboxedData);

	if (self->right->kind == SMILE_KIND_SYMBOL
		&& self->left->kind <= SMILE_KIND_FUNCTION) {
		return String_Format("%S.%S",
			SMILE_VCALL1(self->left, toString, (SmileUnboxedData){ 0 }),
			SMILE_VCALL1(self->right, toString, (SmileUnboxedData){ 0 }));
	}
	else {
		return String_Format("(%S . %S)",
			SMILE_VCALL1(self->left, toString, (SmileUnboxedData){ 0 }),
			SMILE_VCALL1(self->right, toString, (SmileUnboxedData){ 0 }));
	}
}

static LexerPosition SmilePair_GetSourceLocation(SmilePair pair)
{
	if (pair->kind & SMILE_FLAG_WITHSOURCE) {
		struct SmilePairWithSourceInt *pairWithSource = (struct SmilePairWithSourceInt *)pair;
		return pairWithSource->position;
	}
	else return NULL;
}
