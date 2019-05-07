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
#include <smile/smiletypes/smileloanword.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/regex.h>

static String SmileLoanword_AsString(SmileLoanword self, SmileUnboxedData unboxedData);

String SmileLoanword_ToString(SmileLoanword self)
{
	return SmileLoanword_AsString(self, (SmileUnboxedData) { 0 });
}

#define SmileLoanword_ToString SmileLoanword_AsString

SMILE_EASY_OBJECT_VTABLE(SmileLoanword);

SMILE_EASY_OBJECT_NO_SOURCE(SmileLoanword);
SMILE_EASY_OBJECT_READONLY_SECURITY(SmileLoanword);
SMILE_EASY_OBJECT_NO_CALL(SmileLoanword, "A Loanword object");
SMILE_EASY_OBJECT_NO_UNBOX(SmileLoanword)

SmileLoanword SmileLoanword_Create(Symbol name, Regex regex, SmileObject replacement, LexerPosition position)
{
	SmileLoanword smileLoanword = GC_MALLOC_STRUCT(struct SmileLoanwordInt);
	if (smileLoanword == NULL) Smile_Abort_OutOfMemory();
	smileLoanword->base = Smile_KnownBases.Primitive;
	smileLoanword->kind = SMILE_KIND_LOANWORD | SMILE_SECURITY_READONLY;
	smileLoanword->vtable = SmileLoanword_VTable;

	smileLoanword->name = name;
	smileLoanword->regex = regex;
	smileLoanword->replacement = replacement;
	smileLoanword->position = position;

	return smileLoanword;
}

Bool SmileLoanword_Equals(SmileLoanword a, SmileLoanword b)
{
	return SmileLoanword_CompareEqual(a, (SmileUnboxedData) { 0 }, (SmileObject)b, (SmileUnboxedData) { 0 });
}

static Bool SmileLoanword_CompareEqual(SmileLoanword self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	UNUSED(selfData);
	UNUSED(otherData);

	return (SmileObject)self == other;
}

static Bool SmileLoanword_DeepEqual(SmileLoanword self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	SmileLoanword otherLoanword;

	UNUSED(selfData);
	UNUSED(otherData);

	if (SMILE_KIND(other) != SMILE_KIND_LOANWORD)
		return False;

	otherLoanword = (SmileLoanword)other;

	if (self->name != otherLoanword->name)
		return False;

	if (!Regex_Equal(self->regex, otherLoanword->regex))
		return False;

	if (PointerSet_Add(visitedPointers, self->replacement)) {
		if (!SMILE_VCALL4(self->replacement, deepEqual, (SmileUnboxedData) { 0 }, (SmileObject)otherLoanword->replacement, (SmileUnboxedData) { 0 }, visitedPointers))
			return False;
	}

	return True;
}

static UInt32 SmileLoanword_Hash(SmileLoanword self)
{
	UInt32 hash = self->name;
	hash = (hash * 29) + Regex_Hash(self->regex);
	return (UInt32)Smile_ApplyHashOracle(hash);
}

static SmileObject SmileLoanword_GetProperty(SmileLoanword self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.name)
		return (SmileObject)SmileSymbol_Create(self->name);
	else if (propertyName == Smile_KnownSymbols.pattern)
		return (SmileObject)self->regex;
	else if (propertyName == Smile_KnownSymbols.replacement)
		return (SmileObject)self->replacement;
	else
		return NullObject;
}

static void SmileLoanword_SetProperty(SmileLoanword self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);

	Smile_ThrowException(Smile_KnownSymbols.property_error,
		String_Format("Cannot set property \"%S\" on loanword: Loanwords are read-only objects.",
			SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

static Bool SmileLoanword_HasProperty(SmileLoanword self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.name
		|| propertyName == Smile_KnownSymbols.pattern
		|| propertyName == Smile_KnownSymbols.replacement);
}

static SmileList SmileLoanword_GetPropertyNames(SmileLoanword self)
{
	SmileList head, tail;

	UNUSED(self);

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.name));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.pattern));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.replacement));

	return head;
}

static Bool SmileLoanword_ToBool(SmileLoanword self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return True;
}

static Int32 SmileLoanword_ToInteger32(SmileLoanword self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return 0;
}

static String SmileLoanword_ToString(SmileLoanword self, SmileUnboxedData unboxedData)
{
	UNUSED(unboxedData);

	return String_Format("%S: %S => %S",
		SymbolTable_GetName(Smile_SymbolTable, self->name),
		Regex_ToString(self->regex),
		SMILE_VCALL1(self->replacement, toString, (SmileUnboxedData) { 0 }));
}
