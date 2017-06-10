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
#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/internal/staticstring.h>

SMILE_EASY_OBJECT_VTABLE(SmileNonterminal);

SMILE_EASY_OBJECT_NO_SOURCE(SmileNonterminal);
SMILE_EASY_OBJECT_READONLY_SECURITY(SmileNonterminal);
SMILE_EASY_OBJECT_NO_REALS(SmileNonterminal);
SMILE_EASY_OBJECT_NO_CALL(SmileNonterminal);
SMILE_EASY_OBJECT_NO_UNBOX(SmileNonterminal)

SmileNonterminal SmileNonterminal_Create(Symbol nonterminal, Symbol name, Symbol repeat, Symbol separator, Int numWithSymbols, Symbol *withSymbols)
{
	SmileNonterminal smileSyntax = GC_MALLOC_STRUCT(struct SmileNonterminalInt);
	if (smileSyntax == NULL) Smile_Abort_OutOfMemory();
	smileSyntax->base = Smile_KnownBases.Primitive;
	smileSyntax->kind = SMILE_KIND_NONTERMINAL | SMILE_SECURITY_READONLY;
	smileSyntax->vtable = SmileNonterminal_VTable;

	smileSyntax->nonterminal = nonterminal;
	smileSyntax->name = name;
	smileSyntax->repeat = repeat;
	smileSyntax->separator = separator;
	smileSyntax->numWithSymbols = (Int32)numWithSymbols;
	smileSyntax->withSymbols = withSymbols;

	return smileSyntax;
}

static Bool SmileNonterminal_CompareEqual(SmileNonterminal self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	SmileNonterminal otherNonterminal;

	UNUSED(selfData);
	UNUSED(otherData);

	if (SMILE_KIND(other) != SMILE_KIND_NONTERMINAL)
		return False;

	otherNonterminal = (SmileNonterminal)other;

	if (self->nonterminal != otherNonterminal->nonterminal
		|| self->name != otherNonterminal->name
		|| self->repeat != otherNonterminal->repeat
		|| self->separator != otherNonterminal->separator)
		return False;

	return True;
}

static Bool SmileNonterminal_DeepEqual(SmileNonterminal self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	return SmileNonterminal_CompareEqual(self, selfData, other, otherData);
}

static UInt32 SmileNonterminal_Hash(SmileNonterminal self)
{
	return self->nonterminal + self->name;
}

static SmileObject SmileNonterminal_GetProperty(SmileNonterminal self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.nonterminal)
		return (SmileObject)SmileSymbol_Create(self->nonterminal);
	else if (propertyName == Smile_KnownSymbols.name)
		return (SmileObject)SmileSymbol_Create(self->name);
	else if (propertyName == Smile_KnownSymbols.repeat) {
		if (self->repeat == 0) return NullObject;
		return (SmileObject)SmileSymbol_Create(self->repeat);
	}
	else if (propertyName == Smile_KnownSymbols.separator) {
		if (self->separator == 0) return NullObject;
		return (SmileObject)SmileSymbol_Create(self->separator);
	}
	else
		return NullObject;
}

static void SmileNonterminal_SetProperty(SmileNonterminal self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);

	Smile_ThrowException(Smile_KnownSymbols.property_error,
		String_Format("Cannot set property \"%S\" on syntax nonterminal: Syntax nonterminals are read-only objects.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

static Bool SmileNonterminal_HasProperty(SmileNonterminal self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.nonterminal
		|| propertyName == Smile_KnownSymbols.name
		|| propertyName == Smile_KnownSymbols.repeat
		|| propertyName == Smile_KnownSymbols.separator);
}

static SmileList SmileNonterminal_GetPropertyNames(SmileNonterminal self)
{
	SmileList head, tail;

	UNUSED(self);

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.nonterminal));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.name));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.repeat));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.separator));

	return head;
}

static Bool SmileNonterminal_ToBool(SmileNonterminal self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return True;
}

static Int32 SmileNonterminal_ToInteger32(SmileNonterminal self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return 1;
}

static String SmileNonterminal_WithsToString(SmileNonterminal self)
{
	Int i;
	String withs;
	DECLARE_INLINE_STRINGBUILDER(withBuilder, 256);

	STATIC_STRING(withText, "with ");
	STATIC_STRING(commaText, ", ");
	STATIC_STRING(colonText, ": ");

	if (self->numWithSymbols <= 0)
		return String_Empty;

	INIT_INLINE_STRINGBUILDER(withBuilder);

	for (i = 0; i < self->numWithSymbols; i++) {
		StringBuilder_AppendString(withBuilder, i == 0 ? withText : commaText);
		StringBuilder_AppendString(withBuilder, SymbolTable_GetName(Smile_SymbolTable, self->withSymbols[i]));
	}

	StringBuilder_AppendString(withBuilder, colonText);

	withs = StringBuilder_ToString(withBuilder);
	return withs;
}

static String SmileNonterminal_ToString(SmileNonterminal self, SmileUnboxedData unboxedData)
{
	UNUSED(unboxedData);

	return String_Format("%S%S%S %S%s%S",
		SmileNonterminal_WithsToString(self),
		SymbolTable_GetName(Smile_SymbolTable, self->nonterminal),
		self->repeat != 0 ? SymbolTable_GetName(Smile_SymbolTable, self->repeat) : String_Empty,
		SymbolTable_GetName(Smile_SymbolTable, self->name),
		self->separator != 0 ? " " : "",
		self->separator != 0 ? SymbolTable_GetName(Smile_SymbolTable, self->separator) : String_Empty);
}
