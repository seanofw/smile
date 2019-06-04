// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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

#include <smile/numeric/real64.h>
#include <smile/numeric/float64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>

#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/range/smilecharrange.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileCharRange);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileCharRange)
SMILE_EASY_OBJECT_NO_CALL(SmileCharRange, "A CharRange object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileCharRange)
SMILE_EASY_OBJECT_NO_UNBOX(SmileCharRange)

SMILE_EASY_OBJECT_TOBOOL(SmileCharRange, True)
SMILE_EASY_OBJECT_TOSTRING(SmileCharRange, 	((obj->end >= obj->start && obj->stepping != +1
		|| obj->end < obj->start && obj->stepping != -1)
		? String_Format("'%S'..'%S' step %ld",
			String_AddCSlashes(String_CreateRepeat(obj->start, 1)),
			String_AddCSlashes(String_CreateRepeat(obj->end, 1)),
			obj->stepping)
		: String_Format("'%S'..'%S'",
			String_AddCSlashes(String_CreateRepeat(obj->start, 1)),
			String_AddCSlashes(String_CreateRepeat(obj->end, 1))))
)

SmileCharRange SmileCharRange_Create(Byte start, Byte end, Int64 stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileCharRange smileRange = (SmileCharRange)GC_MALLOC_ATOMIC(sizeof(struct SmileCharRangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.CharRange;
	smileRange->kind = SMILE_KIND_CHARRANGE;
	smileRange->vtable = SmileCharRange_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileCharRange_Hash(SmileCharRange range)
{
	UInt32 result;
		Byte start = range->start;
		Byte end = range->end;
		UInt32 stepping = (UInt32)(UInt64)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)((UInt32)start ^ (UInt32)(end << 8) ^ (UInt32)(stepping << 16)));

	return result;
}

static SmileObject SmileCharRange_GetProperty(SmileCharRange self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileChar_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileChar_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileInteger64_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileChar_Create(self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1);
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileCharRange_SetProperty(SmileCharRange self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on CharRange: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on CharRange: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileCharRange_HasProperty(SmileCharRange self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileCharRange_GetPropertyNames(SmileCharRange self)
{
	SmileList head, tail;

	UNUSED(self);

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.start));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.end));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.stepping));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.length));

	return head;
}

static Bool SmileCharRange_CompareEqual(SmileCharRange a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_CHARRANGE) {
		return ((SmileCharRange)a)->start == ((SmileCharRange)b)->start
			&& ((SmileCharRange)a)->end == ((SmileCharRange)b)->end
			&& ((SmileCharRange)a)->stepping == ((SmileCharRange)b)->stepping;
	}
	else return False;
}

static Bool SmileCharRange_DeepEqual(SmileCharRange a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_CHARRANGE) {
		return ((SmileCharRange)a)->start == ((SmileCharRange)b)->start
			&& ((SmileCharRange)a)->end == ((SmileCharRange)b)->end
			&& ((SmileCharRange)a)->stepping == ((SmileCharRange)b)->stepping;
	}
	else return False;
}
