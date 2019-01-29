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
SMILE_EASY_OBJECT_NO_CALL(SmileList, "A List");
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

SmileList SmileList_SafeTail(SmileList list)
{
	SmileList tortoise = list, hare, last;

	if (SMILE_KIND(list) != SMILE_KIND_LIST)
		return list;

	last = tortoise;
	hare = tortoise = (SmileList)tortoise->d;

	hare = LIST_REST(hare);

	while (tortoise != hare && SMILE_KIND(tortoise) == SMILE_KIND_LIST) {
		last = tortoise;
		tortoise = (SmileList)tortoise->d;
		hare = LIST_REST(LIST_REST(hare));
	}

	return (SMILE_KIND(tortoise) != SMILE_KIND_LIST ? last : NULL);
}

SmileList SmileList_SafeClone(SmileList list, SmileList *newTail)
{
	SmileList tortoise = list, hare;
	SmileList destHead = NullList, destTail = NullList;

	if (SMILE_KIND(tortoise) != SMILE_KIND_LIST)
		return destHead;
	LIST_APPEND(destHead, destTail, tortoise->a);
	hare = tortoise = (SmileList)tortoise->d;

	hare = LIST_REST(hare);

	while (tortoise != hare && SMILE_KIND(tortoise) == SMILE_KIND_LIST) {
		LIST_APPEND(destHead, destTail, tortoise->a);
		tortoise = (SmileList)tortoise->d;
		hare = LIST_REST(LIST_REST(hare));
	}

	if (newTail != NULL)
		*newTail = destTail;

	return (SMILE_KIND(tortoise) != SMILE_KIND_LIST ? destHead : NULL);
}

SmileList SmileList_CellAt(SmileList list, Int index)
{
	Int length = 0;

	if (index < 0)
		return NULL;

	for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = (SmileList)list->d, length++)
	{
		if (index == length)
			return list;
	}

	return NULL;
}

SmileList SmileList_ApplyStepping(SmileList list, Int step)
{
	SmileList destHead = NullList, destTail = NullList;
	Int i;

	if (step <= 1) return list;

	while (SMILE_KIND(list) == SMILE_KIND_LIST)
	{
		// Append this cell.
		if (SMILE_KIND(destTail) == SMILE_KIND_NULL) {
			destTail = destHead = list;
		}
		else {
			destTail->d = (SmileObject)list;
			destTail = list;
		}

		// Skip the correct number of cells.
		for (i = 0; i < step; i++)
			list = LIST_REST(list);
	}

	destTail->d = NullObject;

	return destHead;
}

SmileList SmileList_CloneRange(SmileList list, Int start, Int length, SmileList *newTail)
{
	SmileList destHead = NullList, destTail = NullList;

	if (start < 0) {
		length += start;
		start = 0;
	}
	if (length <= 0)
		return NullList;

	list = SmileList_CellAt(list, start);
	if (list == NULL)
		return NullList;

	if (SMILE_KIND(list) != SMILE_KIND_LIST)
		return list;

	for (; SMILE_KIND(list) == SMILE_KIND_LIST && length; list = (SmileList)list->d, length--) {
		LIST_APPEND(destHead, destTail, list->a);
	}

	if (newTail != NULL)
		*newTail = destTail;

	destTail->d = (SMILE_KIND(list) == SMILE_KIND_LIST ? NullObject : (SmileObject)list);

	return destHead;
}

SmileList SmileList_Reverse(SmileList list, SmileList *newTail)
{
	SmileList destHead = NullList, destTail = NullList;
	SmileList next;

	for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = next) {
		next = (SmileList)list->d;

		if (SMILE_KIND(destHead) == SMILE_KIND_NULL) {
			list->d = NullObject;
			destHead = destTail = list;
		}
		else {
			list->d = (SmileObject)destHead;
			destHead = list;
		}
	}

	if (newTail != NULL)
		*newTail = destTail;

	return destHead;
}

SmileList SmileList_CloneReverse(SmileList list, SmileList *newTail)
{
	SmileList destHead = NullList, destTail = NullList;
	SmileList newCell;

	for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = (SmileList)list->d) {
		newCell = LIST_CONS(list->a, destHead);

		if (SMILE_KIND(destTail) == SMILE_KIND_NULL)
			destTail = newCell;
		destHead = newCell;
	}

	if (newTail != NULL)
		*newTail = destTail;

	return destHead;
}

// If this list has no cycles and ends in a Null, then it is well-formed.  If it contains a cycle, or
// it has some ->d pointer that points at anything other than a List or Null object, then it is not well-formed.
Bool SmileList_IsWellFormed(SmileObject probableList)
{
	SmileList tortoise = (SmileList)probableList, hare;

	if (SMILE_KIND(tortoise) != SMILE_KIND_LIST)
		return SMILE_KIND(tortoise) == SMILE_KIND_NULL;
	hare = tortoise = (SmileList)tortoise->d;

	hare = (SMILE_KIND(hare) == SMILE_KIND_LIST ? (SmileList)hare->d : NullList);

	while (tortoise != hare && SMILE_KIND(tortoise) == SMILE_KIND_LIST) {
		tortoise = (SmileList)tortoise->d;
		hare = (SMILE_KIND(hare) == SMILE_KIND_LIST ? (SmileList)hare->d : NullList);
		hare = (SMILE_KIND(hare) == SMILE_KIND_LIST ? (SmileList)hare->d : NullList);
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
	else {
		SmileArg arg = SmileArg_Unbox(tortoise->a);
		piece = SMILE_VCALL1(arg.obj, toString, arg.unboxed);
	}
	StringBuilder_AppendString(stringBuilder, piece);

	hare = tortoise = (SmileList)tortoise->d;
	hare = LIST_REST(hare);

	while (tortoise != hare && SMILE_KIND(tortoise) == SMILE_KIND_LIST) {
		if (hasGlue) {
			StringBuilder_AppendString(stringBuilder, glue);
		}

		if (SMILE_KIND(tortoise->a) == SMILE_KIND_STRING)
			piece = (String)tortoise->a;
		else if (SMILE_KIND(tortoise->a) == SMILE_KIND_NULL)
			piece = String_Empty;
		else
			piece = SMILE_VCALL1(tortoise->a, toString, (SmileUnboxedData) { 0 });

		StringBuilder_AppendString(stringBuilder, piece);

		tortoise = (SmileList)tortoise->d;
		hare = LIST_REST(LIST_REST(hare));
	}

	return SMILE_KIND(tortoise) == SMILE_KIND_NULL ? StringBuilder_ToString(stringBuilder) : NULL;
}

Bool SmileObject_IsCallToSymbol(Symbol symbol, SmileObject obj)
{
	SmileList list;

	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return False;

	list = (SmileList)obj;

	if (SMILE_KIND((SmileList)list->a) != SMILE_KIND_SYMBOL)
		return False;

	if (((SmileSymbol)(SmileList)list->a)->symbol != symbol)
		return False;

	return True;
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

/// <summary>
/// Sort the given list "in place" by mutating its pointers.  This uses
/// a derivative of Simon Tatham's linked-list mergesort, so it runs in
/// O(n lg n) time.  This is a stable sort.  The list to be sorted must
/// not be circular, and the last 'd' pointer's data will always be discarded
/// (so it *should* be Null, but if you do not provide a well-formed list,
/// it may not be).
/// </summary>
SmileList SmileList_Sort(SmileList list, Int (*cmp)(SmileObject a, SmileObject b, void *param), void *param)
{
	SmileList p, q, e, tail;
	Int stepSize, numMerges, psize, qsize, i;

	if (SMILE_KIND(list) == SMILE_KIND_NULL)
		return list;

	stepSize = 1;

	for (;;) {
		p = list;
		list = NullList;
		tail = NullList;

		numMerges = 0;				// Count number of merges we do in this pass.

		while (SMILE_KIND(p) == SMILE_KIND_LIST) {
			numMerges++;			// There exists a merge to be done.

			// Step 'stepSize' places along from p.
			q = p;
			psize = 0;
			for (i = 0; i < stepSize; i++) {
				psize++;
				q = (SmileList)q->d;
				if (SMILE_KIND(q) != SMILE_KIND_LIST) break;
			}

			// If q hasn't fallen off the end, we have two lists to merge.
			qsize = stepSize;

			// Now we have two lists; merge them.
			while (psize > 0 || (qsize > 0 && SMILE_KIND(q) == SMILE_KIND_LIST)) {

				if (psize == 0) {
					e = q; q = (SmileList)q->d; qsize--;
				}
				else if (qsize == 0 || SMILE_KIND(q) != SMILE_KIND_LIST) {
					e = p; p = (SmileList)p->d; psize--;
				}
				else {
					if (cmp(p->a, q->a, param) <= 0) {
						e = p; p = (SmileList)p->d; psize--;
					}
					else {
						e = q; q = (SmileList)q->d; qsize--;
					}
				}

				if (SMILE_KIND(tail) == SMILE_KIND_LIST) {
					tail->d = (SmileObject)e;
				}
				else {
					list = e;
				}
				tail = e;
			}

			// Now p has stepped 'stepSize' places along, and q has too.
			p = q;
		}
		tail->d = NullObject;

		if (numMerges <= 1)
			return list;

		stepSize <<= 1;
	}
}

struct InterruptibleListSortInfoStruct {
	SmileList list, p, q, e, tail;
	Int stepSize, psize, qsize;
	Byte state, numMerges;
	Byte reserved;
	SByte cmpResult;
	SmileObject cmpA, cmpB;
};

enum {
	INITIAL,
	OUTER_LOOP,
	AFTER_OUTER_LOOP,
	STEP_LOOP,
	AFTER_STEP_LOOP,
	MERGE_LOOP,
	COMPARE_ITEMS,
	AFTER_COMPARE_ITEMS,
	GOT_NEXT_LOWEST,
	AFTER_MERGE_LOOP,
};

#define STATE(s) case s: label_##s
#define GOTO_STATE(s) goto label_##s

InterruptibleListSortInfo InterruptibleListSort_Start(SmileList list)
{
	InterruptibleListSortInfo sortInfo = GC_MALLOC_STRUCT(struct InterruptibleListSortInfoStruct);
	if (sortInfo == NULL)
		Smile_Abort_OutOfMemory();

	sortInfo->list = list;
	sortInfo->state = INITIAL;

	return sortInfo;
}

Bool InterruptibleListSort_Continue(InterruptibleListSortInfo sortInfo, Int64 cmpResult,
	SmileObject *cmpA, SmileObject *cmpB, SmileList *sortResult)
{
	switch (sortInfo->state) {

		STATE(INITIAL):
			if (SMILE_KIND(sortInfo->list) == SMILE_KIND_NULL) {
				*sortResult = sortInfo->list;
				return False;
			}

			sortInfo->stepSize = 1;
			GOTO_STATE(OUTER_LOOP);

		STATE(OUTER_LOOP):
			sortInfo->p = sortInfo->list;
			sortInfo->list = NullList;
			sortInfo->tail = NullList;

			sortInfo->numMerges = 0;				// Count number of merges we do in this pass.
			GOTO_STATE(STEP_LOOP);

		STATE(STEP_LOOP):
			if (SMILE_KIND(sortInfo->p) != SMILE_KIND_LIST)
				GOTO_STATE(AFTER_STEP_LOOP);

			sortInfo->numMerges++;			// There exists a merge to be done.

			// Step 'stepSize' places along from p.
			sortInfo->q = sortInfo->p;
			sortInfo->psize = 0;
			{
				Int i;
				for (i = 0; i < sortInfo->stepSize; i++) {
					sortInfo->psize++;
					sortInfo->q = (SmileList)sortInfo->q->d;
					if (SMILE_KIND(sortInfo->q) != SMILE_KIND_LIST) break;
				}
			}

			// If q hasn't fallen off the end, we have two lists to merge.
			sortInfo->qsize = sortInfo->stepSize;
			GOTO_STATE(MERGE_LOOP);

		STATE(MERGE_LOOP):
			// Now we have two lists; merge them.
			if (!(sortInfo->psize > 0 || (sortInfo->qsize > 0 && SMILE_KIND(sortInfo->q) == SMILE_KIND_LIST)))
				GOTO_STATE(AFTER_MERGE_LOOP);

			if (sortInfo->psize == 0) {
				sortInfo->cmpResult = +1;
				GOTO_STATE(GOT_NEXT_LOWEST);
			}
			else if (sortInfo->qsize == 0 || SMILE_KIND(sortInfo->q) != SMILE_KIND_LIST) {
				sortInfo->cmpResult = -1;
				GOTO_STATE(GOT_NEXT_LOWEST);
			}
			else {
				GOTO_STATE(COMPARE_ITEMS);
			}

		STATE(GOT_NEXT_LOWEST):
			if (sortInfo->cmpResult <= 0) {
				sortInfo->e = sortInfo->p;
				sortInfo->p = (SmileList)sortInfo->p->d;
				sortInfo->psize--;
			}
			else {
				sortInfo->e = sortInfo->q;
				sortInfo->q = (SmileList)sortInfo->q->d;
				sortInfo->qsize--;
			}
			if (SMILE_KIND(sortInfo->tail) == SMILE_KIND_LIST) {
				sortInfo->tail->d = (SmileObject)sortInfo->e;
			}
			else {
				sortInfo->list = sortInfo->e;
			}
			sortInfo->tail = sortInfo->e;
	
			GOTO_STATE(MERGE_LOOP);

		STATE(AFTER_MERGE_LOOP):
			// Now p has stepped 'stepSize' places along, and q has too.
			sortInfo->p = sortInfo->q;
			GOTO_STATE(STEP_LOOP);

		STATE(AFTER_STEP_LOOP):
			sortInfo->tail->d = NullObject;

			if (sortInfo->numMerges <= 1) {
				*sortResult = sortInfo->list;
				return False;
			}

			sortInfo->stepSize <<= 1;
			GOTO_STATE(OUTER_LOOP);

		STATE(COMPARE_ITEMS):
			sortInfo->state = AFTER_COMPARE_ITEMS;
			*cmpA = sortInfo->p->a;
			*cmpB = sortInfo->q->a;
			return True;

		STATE(AFTER_COMPARE_ITEMS):
			sortInfo->cmpResult =
				  cmpResult < 0 ? -1
				: cmpResult > 0 ? +1
				: 0;
			GOTO_STATE(GOT_NEXT_LOWEST);
	}

	if (sortInfo) goto label_INITIAL;
	if (sortInfo) goto label_AFTER_COMPARE_ITEMS;

	return False;
}
