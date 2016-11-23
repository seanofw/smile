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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/stringbuilder.h>

SMILE_EASY_OBJECT_VTABLE(SmileList);

SMILE_EASY_OBJECT_NO_SECURITY(SmileList);
SMILE_EASY_OBJECT_NO_REALS(SmileList);

SmileList SmileList_Cons(SmileObject a, SmileObject d)
{
	SmileList smileList = GC_MALLOC_STRUCT(struct SmileListInt);
	if (smileList == NULL) Smile_Abort_OutOfMemory();
	smileList->base = Smile_KnownObjects.Object;
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
	smileList->base = Smile_KnownObjects.Object;
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

static Bool SmileList_CompareEqual(SmileList self, SmileObject other)
{
	SmileList otherList;

	do {
		if (!SmileObject_IsList(other)) return False;
		otherList = (SmileList)other;

		// Compare this list element.
		if (!SMILE_VCALL1(self->a, compareEqual, otherList->a))
			return False;

		self = (SmileList)self->d;
		other = otherList->d;

		// We use a loop to unroll the ->d recursion, where possible.
	} while (SMILE_KIND(self) == SMILE_KIND_LIST);

	// Compare the last ->d objects, whatever they are.
	return SMILE_VCALL1((SmileObject)self, compareEqual, other);
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
		return NullObject;
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

static Bool SmileList_ToBool(SmileList self)
{
	UNUSED(self);
	return True;
}

static Int32 SmileList_ToInteger32(SmileList self)
{
	UNUSED(self);
	return 1;
}

static Bool IsNormallyStructuredList(SmileList list)
{
	while (SMILE_KIND(list) == SMILE_KIND_LIST) {
		list = (SmileList)list->d;
	}

	return SMILE_KIND(list) == SMILE_KIND_NULL;
}

static int _indent = 0;

STATIC_STRING(BacktickString, "`");

static String SmileList_ToString(SmileList self)
{
	Bool useIndents;
	Bool isFirst;
	SmileList list;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);

	if (!IsNormallyStructuredList(self))
		return String_Format("(%S ## %S)", SMILE_VCALL(self->a, toString), SMILE_VCALL(self->d, toString));

	if (SMILE_KIND(self->a) == SMILE_KIND_SYMBOL
		&& ((SmileSymbol)self->a)->symbol == Smile_KnownSymbols.quote_
		&& SMILE_KIND(LIST_REST(LIST_REST(self))) == SMILE_KIND_NULL)
	{
		SmileObject quotedItem = LIST_FIRST(LIST_REST(self));
		if (SMILE_KIND(quotedItem) == SMILE_KIND_LIST || SMILE_KIND(quotedItem) == SMILE_KIND_SYMBOL) {
			return String_Concat(BacktickString, SMILE_VCALL(quotedItem, toString));
		}
	}

	INIT_INLINE_STRINGBUILDER(stringBuilder);
	StringBuilder_AppendByte(stringBuilder, '[');

	useIndents = (SMILE_KIND(self->a) == SMILE_KIND_SYMBOL &&
		(((SmileSymbol)self->a)->symbol == Smile_KnownSymbols.scope_
		|| ((SmileSymbol)self->a)->symbol == Smile_KnownSymbols.progn_));

	if (useIndents) _indent++;

	isFirst = True;
	for (list = self; SMILE_KIND(list) != SMILE_KIND_NULL; list = (SmileList)list->d) {
		if (useIndents && !isFirst) {
			StringBuilder_AppendRepeat(stringBuilder, '\t', _indent);
		}
		StringBuilder_AppendString(stringBuilder, SMILE_VCALL(list->a, toString));
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
