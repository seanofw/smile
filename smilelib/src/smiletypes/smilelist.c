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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/stringbuilder.h>

SMILE_EASY_OBJECT_VTABLE(SmileList);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileList);
SMILE_EASY_OBJECT_NO_REALS(SmileList);
SMILE_EASY_OBJECT_NO_CALL(SmileList);
SMILE_EASY_OBJECT_NO_UNBOX(SmileList)

SmileList SmileList_Cons(SmileObject a, SmileObject d)
{
	SmileList smileList = GC_MALLOC_STRUCT(struct SmileListInt);
	if (smileList == NULL) Smile_Abort_OutOfMemory();
	smileList->base = (SmileObject)Smile_KnownBases.List;
	smileList->kind = SMILE_KIND_LIST | SMILE_SECURITY_WRITABLE;
	smileList->vtable = SmileList_VTable;
	smileList->a = a;
	smileList->d = d;
	return smileList;
}

SmileList SmileList_ConsWithSource(SmileObject a, SmileObject d, LexerPosition position)
{
	struct SmileListWithSourceInt *smileList = GC_MALLOC_STRUCT(struct SmileListWithSourceInt);
	if (smileList == NULL) Smile_Abort_OutOfMemory();
	smileList->base = (SmileObject)Smile_KnownBases.List;
	smileList->kind = SMILE_KIND_LIST | SMILE_FLAG_WITHSOURCE | SMILE_SECURITY_WRITABLE;
	smileList->vtable = SmileList_VTable;
	smileList->a = a;
	smileList->d = d;
	smileList->position = position;
	return (SmileList)smileList;
}

SmileList SmileList_CreateListFromArray(SmileObject *objects, Int numObjects)
{
	SmileList head, tail;

	LIST_INIT(head, tail);

	while (numObjects-- > 0) {
		LIST_APPEND(head, tail, *objects++);
	}

	return head;
}

SmileList SmileList_CreateList(SmileObject firstObject, ...)
{
	SmileList result;
	va_list v;

	va_start(v, firstObject);
	result = SmileList_CreateListv(firstObject, v);
	va_end(v);

	return result;
}

SmileList SmileList_CreateListv(SmileObject firstObject, va_list v)
{
	SmileList head, tail;
	SmileObject obj;

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, firstObject);

	while ((obj = va_arg(v, SmileObject)) != NULL) {
		LIST_APPEND(head, tail, obj);
	}

	return head;
}

/// <summary>
/// Determine the length of this list, and whether it is well-formed (i.e., whether it consists of
/// a sequence of SmileList objects, chained by their ->d pointers, followed by SmileNull for the
/// last ->d pointer).
/// </summary>
/// <param name="list">The list to analyze.</param>
/// <returns>The length of the list, if it is well-formed; or -1 if it is not well-formed.</returns>
Int SmileList_Length(SmileList list)
{
	Int length = 0;

	for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = (SmileList)list->d) {
		length++;
	}

	return SMILE_KIND(list) == SMILE_KIND_NULL ? length : -1;
}

static Bool SmileList_CompareEqual(SmileList self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	UNUSED(selfData);
	UNUSED(otherData);

	return (SmileObject)self == other;
}

static Bool SmileList_DeepEqual(SmileList self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	UNUSED(selfData);
	UNUSED(otherData);

	for (;;) {
		if (SMILE_KIND(other) != SMILE_KIND(self)) return False;
		if (SMILE_KIND(self) == SMILE_KIND_NULL) return True;
	
		if (PointerSet_Add(visitedPointers, self->a)) {
			if (!SMILE_VCALL4(self->a, deepEqual, (SmileUnboxedData){ 0 }, ((SmileList)other)->a, (SmileUnboxedData){ 0 }, visitedPointers))
				return False;
		}
	
		if (PointerSet_Add(visitedPointers, self->d)) {
			if ((SMILE_KIND(self->d) & ~SMILE_KIND_LIST_BIT) != SMILE_KIND_NULL) {
				if (!SMILE_VCALL4(self->d, deepEqual, (SmileUnboxedData){ 0 }, ((SmileList)other)->d, (SmileUnboxedData){ 0 }, visitedPointers))
					return False;
				return True;
			}
		}
	
		self = (SmileList)self->d;
		other = ((SmileList)other)->d;
	}
}

static UInt32 SmileList_Hash(SmileList self)
{
	return (UInt32)(PtrInt)self;
}

static SmileObject SmileList_GetProperty(SmileList self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.a)
		return self->a;
	else if (propertyName == Smile_KnownSymbols.d)
		return self->d;
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileList_SetProperty(SmileList self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.a)
		self->a = value;
	else if (propertyName == Smile_KnownSymbols.d)
		self->d = value;
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on list cell: This property does not exist, and list cells are not appendable objects.",
			SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileList_HasProperty(SmileList self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.a || propertyName == Smile_KnownSymbols.d);
}

static SmileList SmileList_GetPropertyNames(SmileList self)
{
	SmileList head, tail;

	UNUSED(self);

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.a));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.d));

	return head;
}

static Bool SmileList_ToBool(SmileList self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return True;
}

static Int32 SmileList_ToInteger32(SmileList self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return 1;
}

static Bool IsNormallyStructuredList(SmileList list)
{
	while (SMILE_KIND(list) == SMILE_KIND_LIST) {
		list = (SmileList)list->d;
	}

	return SMILE_KIND(list) == SMILE_KIND_NULL;
}

Int SmileList_SafeLength(SmileList list)
{
	Int length = 0;
	SmileList tortoise = list, hare;

	if (SMILE_KIND(tortoise) != SMILE_KIND_LIST)
		return 0;
	hare = tortoise = (SmileList)tortoise->d;
	length++;

	hare = LIST_REST(hare);

	while (tortoise != hare && SMILE_KIND(tortoise) == SMILE_KIND_LIST) {
		tortoise = (SmileList)tortoise->d;
		hare = LIST_REST(LIST_REST(hare));
		length++;
	}

	return (SMILE_KIND(tortoise) != SMILE_KIND_LIST ? length : -1);
}

// If this list has no cycles and ends in a Null, then it is well-formed.  If it contains a cycle, or
// it has some ->d pointer that points at anything other than a List or Null object, then it is not well-formed.
Bool SmileList_IsWellFormed(SmileObject probableList)
{
	SmileList tortoise = (SmileList)probableList, hare;

	if (SMILE_KIND(tortoise) != SMILE_KIND_LIST)
		return SMILE_KIND(tortoise) == SMILE_KIND_NULL;
	hare = tortoise = (SmileList)tortoise->d;

	hare = LIST_REST(hare);

	while (tortoise != hare && SMILE_KIND(tortoise) == SMILE_KIND_LIST) {
		tortoise = (SmileList)tortoise->d;
		hare = LIST_REST(LIST_REST(hare));
	}

	return SMILE_KIND(tortoise) == SMILE_KIND_NULL;
}

// Determine if this list contains a cycle, using the straightforward Floyd tortoise-and-hare algorithm.
Bool SmileList_HasCycle(SmileObject probableList)
{
	SmileList tortoise = (SmileList)probableList, hare;

	if (SMILE_KIND(tortoise) != SMILE_KIND_LIST)
		return False;
	hare = tortoise = (SmileList)tortoise->d;

	hare = LIST_REST(hare);

	while (tortoise != hare && SMILE_KIND(tortoise) == SMILE_KIND_LIST) {
		tortoise = (SmileList)tortoise->d;
		hare = LIST_REST(LIST_REST(hare));
	}

	return SMILE_KIND(tortoise) == SMILE_KIND_LIST && tortoise == hare;
}

String SmileList_Join(SmileList list, String glue)
{
	SmileList tortoise = list, hare;
	String piece;
	Bool hasGlue = String_Length(glue) > 0;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);

	if (SMILE_KIND(tortoise) != SMILE_KIND_LIST)
		return String_Empty;

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	if (SMILE_KIND(tortoise->a) == SMILE_KIND_STRING)
		piece = (String)tortoise->a;
	else
		piece = SMILE_VCALL1(tortoise->a, toString, (SmileUnboxedData) { 0 });
	StringBuilder_AppendString(stringBuilder, piece);

	hare = tortoise = (SmileList)tortoise->d;
	hare = LIST_REST(hare);

	while (tortoise != hare && SMILE_KIND(tortoise) == SMILE_KIND_LIST) {
		if (hasGlue) {
			StringBuilder_AppendString(stringBuilder, glue);
		}

		if (SMILE_KIND(tortoise->a) == SMILE_KIND_STRING)
			piece = (String)tortoise->a;
		else
			piece = SMILE_VCALL1(tortoise->a, toString, (SmileUnboxedData) { 0 });

		StringBuilder_AppendString(stringBuilder, piece);

		tortoise = (SmileList)tortoise->d;
		hare = LIST_REST(LIST_REST(hare));
	}

	return SMILE_KIND(tortoise) == SMILE_KIND_NULL ? StringBuilder_ToString(stringBuilder) : NULL;
}

static int _indent = 0;

static String SmileList_ToString(SmileList self, SmileUnboxedData unboxedData)
{
	Bool useIndents;
	Bool isFirst;
	SmileList list;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);

	UNUSED(unboxedData);

	if (!IsNormallyStructuredList(self))
		return String_Format("(%S ## %S)",
			SMILE_VCALL1(self->a, toString, (SmileUnboxedData){ 0 }),
			SMILE_VCALL1(self->d, toString, (SmileUnboxedData){ 0 }));

	if (SMILE_KIND(self->a) == SMILE_KIND_SYMBOL
		&& ((SmileSymbol)self->a)->symbol == SMILE_SPECIAL_SYMBOL__QUOTE
		&& SMILE_KIND(LIST_REST(LIST_REST(self))) == SMILE_KIND_NULL)
	{
		SmileObject quotedItem = LIST_FIRST(LIST_REST(self));
		if (SMILE_KIND(quotedItem) == SMILE_KIND_LIST || SMILE_KIND(quotedItem) == SMILE_KIND_SYMBOL) {
			return String_Concat(String_Backtick, SMILE_VCALL1(quotedItem, toString, (SmileUnboxedData){ 0 }));
		}
	}

	INIT_INLINE_STRINGBUILDER(stringBuilder);
	StringBuilder_AppendByte(stringBuilder, '[');

	useIndents = (SMILE_KIND(self->a) == SMILE_KIND_SYMBOL &&
		(((SmileSymbol)self->a)->symbol == SMILE_SPECIAL_SYMBOL__SCOPE
		|| ((SmileSymbol)self->a)->symbol == SMILE_SPECIAL_SYMBOL__PROGN));

	if (useIndents) _indent++;

	isFirst = True;
	for (list = self; SMILE_KIND(list) != SMILE_KIND_NULL; list = (SmileList)list->d) {
		if (useIndents && !isFirst) {
			StringBuilder_AppendRepeat(stringBuilder, '\t', _indent);
		}
		StringBuilder_AppendString(stringBuilder, SMILE_VCALL1(list->a, toString, (SmileUnboxedData){ 0 }));
		if (!useIndents) {
			if (SMILE_KIND(list->d) != SMILE_KIND_NULL) {
				StringBuilder_AppendByte(stringBuilder, ' ');
			}
		}
		else {
			StringBuilder_Append(stringBuilder, "\r\n", 0, 2);
		}
		isFirst = False;
	}

	if (useIndents) {
		_indent--;
		StringBuilder_AppendRepeat(stringBuilder, '\t', _indent);
	}

	StringBuilder_AppendByte(stringBuilder, ']');

	return StringBuilder_ToString(stringBuilder);
}

static LexerPosition SmileList_GetSourceLocation(SmileList list)
{
	if (list->kind & SMILE_FLAG_WITHSOURCE) {
		struct SmileListWithSourceInt *listWithSource = (struct SmileListWithSourceInt *)list;
		return listWithSource->position;
	}
	else return NULL;
}
