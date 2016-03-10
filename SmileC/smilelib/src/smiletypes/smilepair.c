//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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
#include <smile/smiletypes/text/smilestring.h>

SMILE_EASY_OBJECT_VTABLE(SmilePair);

SMILE_EASY_OBJECT_NO_SECURITY(SmilePair);
SMILE_EASY_OBJECT_NO_REALS(SmilePair);

SmilePair SmilePair_Create(SmileObject left, SmileObject right)
{
	SmilePair smilePair = GC_MALLOC_STRUCT(struct SmilePairInt);
	if (smilePair == NULL) Smile_Abort_OutOfMemory();
	smilePair->base = Smile_KnownObjects.Object;
	smilePair->kind = SMILE_KIND_PAIR | SMILE_SECURITY_WRITABLE;
	smilePair->vtable = SmilePair_VTable;
	smilePair->left = left;
	smilePair->right = right;
	return smilePair;
}

SmilePair SmilePair_CreateSource(SmileObject left, SmileObject right, LexerPosition position)
{
	struct SmilePairWithSourceInt *smilePair = GC_MALLOC_STRUCT(struct SmilePairWithSourceInt);
	if (smilePair == NULL) Smile_Abort_OutOfMemory();
	smilePair->base = Smile_KnownObjects.Object;
	smilePair->kind = SMILE_KIND_PAIR | SMILE_SECURITY_WRITABLE | SMILE_FLAG_WITHSOURCE;
	smilePair->vtable = SmilePair_VTable;
	smilePair->left = left;
	smilePair->right = right;
	smilePair->position = position;
	return (SmilePair)smilePair;
}

static Bool SmilePair_CompareEqual(SmilePair self, SmileObject other)
{
	SmilePair otherPair;

	if (SMILE_KIND(other) != SMILE_KIND_PAIR)
		return False;

	otherPair = (SmilePair)other;

	if (!SMILE_VCALL1(self->left, compareEqual, otherPair->left))
		return False;

	if (!SMILE_VCALL1(self->right, compareEqual, otherPair->right))
		return False;

	return True;
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
	DECLARE_LIST_BUILDER(listBuilder);

	UNUSED(self);

	LIST_BUILDER_APPEND(listBuilder, SmileSymbol_Create(Smile_KnownSymbols.left));
	LIST_BUILDER_APPEND(listBuilder, SmileSymbol_Create(Smile_KnownSymbols.right));

	return LIST_BUILDER_HEAD(listBuilder);
}

static Bool SmilePair_ToBool(SmilePair self)
{
	UNUSED(self);
	return True;
}

static Int32 SmilePair_ToInteger32(SmilePair self)
{
	UNUSED(self);
	return 1;
}

static String SmilePair_ToString(SmilePair self)
{
	if (self->right->kind == SMILE_KIND_SYMBOL
		&& self->left->kind <= SMILE_KIND_FUNCTION) {
		return String_Format("%S.%S", SMILE_VCALL(self->left, toString), SMILE_VCALL(self->right, toString));
	}
	else {
		return String_Format("(%S . %S)", SMILE_VCALL(self->left, toString), SMILE_VCALL(self->right, toString));
	}
}
