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
#include <smile/smiletypes/smilesyntax.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>

SMILE_EASY_OBJECT_VTABLE(SmileNonterminal);

SMILE_EASY_OBJECT_NO_SECURITY(SmileNonterminal);
SMILE_EASY_OBJECT_NO_REALS(SmileNonterminal);
SMILE_EASY_OBJECT_NO_CALL(SmileNonterminal);

SmileNonterminal SmileNonterminal_Create(Symbol nonterminal, Symbol name, Symbol repeat, Symbol separator)
{
	SmileNonterminal smileSyntax = GC_MALLOC_STRUCT(struct SmileNonterminalInt);
	if (smileSyntax == NULL) Smile_Abort_OutOfMemory();
	smileSyntax->base = Smile_KnownObjects.Object;
	smileSyntax->kind = SMILE_KIND_NONTERMINAL | SMILE_SECURITY_READONLY;
	smileSyntax->vtable = SmileNonterminal_VTable;

	smileSyntax->nonterminal = nonterminal;
	smileSyntax->name = name;
	smileSyntax->repeat = repeat;
	smileSyntax->separator = separator;

	return smileSyntax;
}

static Bool SmileNonterminal_CompareEqual(SmileNonterminal self, SmileObject other)
{
	SmileNonterminal otherNonterminal;

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

static UInt32 SmileNonterminal_Hash(SmileNonterminal self)
{
	return self->nonterminal + self->name;
}

static SmileObject SmileNonterminal_GetProperty(SmileNonterminal self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.nonterminal)
		return (SmileObject)self->nonterminal;
	else if (propertyName == Smile_KnownSymbols.name)
		return (SmileObject)self->name;
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

static Bool SmileNonterminal_ToBool(SmileNonterminal self)
{
	UNUSED(self);
	return True;
}

static Int32 SmileNonterminal_ToInteger32(SmileNonterminal self)
{
	UNUSED(self);
	return 1;
}

static String SmileNonterminal_ToString(SmileNonterminal self)
{
	return String_Format("%S%S %S%s%S",
		SymbolTable_GetName(Smile_SymbolTable, self->nonterminal),
		self->repeat != 0 ? SymbolTable_GetName(Smile_SymbolTable, self->repeat) : String_Empty,
		SymbolTable_GetName(Smile_SymbolTable, self->name),
		self->separator != 0 ? " " : "",
		self->separator != 0 ? SymbolTable_GetName(Smile_SymbolTable, self->separator) : String_Empty);
}
