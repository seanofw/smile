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
#include <smile/smiletypes/smilesyntax.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>

static String SmileSyntax_AsString(SmileSyntax self);

String SmileSyntax_ToString(SmileSyntax self)
{
	return SmileSyntax_AsString(self);
}

#define SmileSyntax_ToString SmileSyntax_AsString

SMILE_EASY_OBJECT_VTABLE(SmileSyntax);

SMILE_EASY_OBJECT_NO_SECURITY(SmileSyntax);
SMILE_EASY_OBJECT_NO_REALS(SmileSyntax);
SMILE_EASY_OBJECT_NO_CALL(SmileSyntax);

SmileSyntax SmileSyntax_Create(Symbol nonterminal, SmileList pattern, SmileObject replacement, LexerPosition position)
{
	SmileSyntax smileSyntax = GC_MALLOC_STRUCT(struct SmileSyntaxInt);
	if (smileSyntax == NULL) Smile_Abort_OutOfMemory();
	smileSyntax->base = Smile_KnownBases.Primitive;
	smileSyntax->kind = SMILE_KIND_SYNTAX | SMILE_SECURITY_READONLY;
	smileSyntax->vtable = SmileSyntax_VTable;

	smileSyntax->nonterminal = nonterminal;
	smileSyntax->pattern = pattern;
	smileSyntax->replacement = replacement;
	smileSyntax->position = position;

	return smileSyntax;
}

Bool SmileSyntax_Equals(SmileSyntax a, SmileSyntax b)
{
	return SmileSyntax_CompareEqual(a, (SmileObject)b);
}

static Bool SmileSyntax_CompareEqual(SmileSyntax self, SmileObject other)
{
	SmileSyntax otherSyntax;

	if (SMILE_KIND(other) != SMILE_KIND_SYNTAX)
		return False;

	otherSyntax = (SmileSyntax)other;

	if (self->nonterminal != otherSyntax->nonterminal)
		return False;

	if (!SMILE_VCALL1(self->pattern, compareEqual, (SmileObject)otherSyntax->pattern))
		return False;

	if (!SMILE_VCALL1(self->replacement, compareEqual, (SmileObject)otherSyntax->replacement))
		return False;

	return True;
}

static UInt32 SmileSyntax_Hash(SmileSyntax self)
{
	UInt32 hash = self->nonterminal;
	SmileList node;

	for (node = self->pattern; node != NullList; node = SmileList_Rest(node)) {
		hash++;
		switch (SMILE_KIND(node->a)) {
			case SMILE_KIND_SYMBOL:
				hash = (hash << 1) | (hash >> 31);
				hash += ((SmileSymbol)(node->a))->symbol;
				break;
			case SMILE_KIND_NONTERMINAL:
				hash = (hash << 1) | (hash >> 31);
				hash += ((SmileNonterminal)(node->a))->nonterminal;
				break;
		}
	}

	return hash;
}

static SmileObject SmileSyntax_GetProperty(SmileSyntax self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.nonterminal)
		return (SmileObject)SmileSymbol_Create(self->nonterminal);
	else if (propertyName == Smile_KnownSymbols.pattern)
		return (SmileObject)self->pattern;
	else if (propertyName == Smile_KnownSymbols.replacement)
		return (SmileObject)self->replacement;
	else
		return NullObject;
}

static void SmileSyntax_SetProperty(SmileSyntax self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);

	Smile_ThrowException(Smile_KnownSymbols.property_error,
		String_Format("Cannot set property \"%S\" on syntax production: Syntax productions are read-only objects.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

static Bool SmileSyntax_HasProperty(SmileSyntax self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.nonterminal
		|| propertyName == Smile_KnownSymbols.pattern
		|| propertyName == Smile_KnownSymbols.replacement);
}

static SmileList SmileSyntax_GetPropertyNames(SmileSyntax self)
{
	SmileList head, tail;

	UNUSED(self);

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.nonterminal));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.pattern));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.replacement));

	return head;
}

static Bool SmileSyntax_ToBool(SmileSyntax self)
{
	UNUSED(self);
	return True;
}

static Int32 SmileSyntax_ToInteger32(SmileSyntax self)
{
	SmileList node;
	Int32 count = 0;

	UNUSED(self);

	for (node = self->pattern; node != NullList; node = SmileList_Rest(node)) {
		count++;
	}

	return count;
}

static String SmileSyntax_ToString(SmileSyntax self)
{
	return String_Format("%S: %S => %S",
		SymbolTable_GetName(Smile_SymbolTable, self->nonterminal),
		SMILE_VCALL(self->pattern, toString), SMILE_VCALL(self->replacement, toString));
}
