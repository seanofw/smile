// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

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

#include <smile/numeric/real64.h>
#include <smile/numeric/float64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>

#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/range/smileunirange.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileUniRange);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileUniRange)
SMILE_EASY_OBJECT_NO_CALL(SmileUniRange, "A UniRange object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileUniRange)
SMILE_EASY_OBJECT_NO_UNBOX(SmileUniRange)

SMILE_EASY_OBJECT_TOBOOL(SmileUniRange, True)
SMILE_EASY_OBJECT_TOSTRING(SmileUniRange, 	((obj->end >= obj->start && obj->stepping != +1
		|| obj->end < obj->start && obj->stepping != -1)
		? String_Format("'\\u%X'..'\\u%X' step %ld", obj->start, obj->end, obj->stepping)
		: String_Format("'\\u%X'..'\\u%X'", obj->start, obj->end))
)

SmileUniRange SmileUniRange_Create(UInt32 start, UInt32 end, Int64 stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileUniRange smileRange = (SmileUniRange)GC_MALLOC_ATOMIC(sizeof(struct SmileUniRangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.UniRange;
	smileRange->kind = SMILE_KIND_UNIRANGE;
	smileRange->vtable = SmileUniRange_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileUniRange_Hash(SmileUniRange range)
{
	UInt32 result;
		UInt32 start = range->start;
		UInt32 end = range->end;
		UInt32 stepping = (UInt32)(UInt64)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)(start ^ (end << 8) ^ (stepping << 16)));

	return result;
}

static SmileObject SmileUniRange_GetProperty(SmileUniRange self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileUni_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileUni_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileInteger64_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileUni_Create(self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1);
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileUniRange_SetProperty(SmileUniRange self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on UniRange: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on UniRange: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileUniRange_HasProperty(SmileUniRange self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileUniRange_GetPropertyNames(SmileUniRange self)
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

static Bool SmileUniRange_CompareEqual(SmileUniRange a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_UNIRANGE) {
		return ((SmileUniRange)a)->start == ((SmileUniRange)b)->start
			&& ((SmileUniRange)a)->end == ((SmileUniRange)b)->end
			&& ((SmileUniRange)a)->stepping == ((SmileUniRange)b)->stepping;
	}
	else return False;
}

static Bool SmileUniRange_DeepEqual(SmileUniRange a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_UNIRANGE) {
		return ((SmileUniRange)a)->start == ((SmileUniRange)b)->start
			&& ((SmileUniRange)a)->end == ((SmileUniRange)b)->end
			&& ((SmileUniRange)a)->stepping == ((SmileUniRange)b)->stepping;
	}
	else return False;
}
