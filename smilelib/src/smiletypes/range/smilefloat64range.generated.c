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

#include <smile/smiletypes/numeric/smilefloat64.h>
#include <smile/smiletypes/range/smilefloat64range.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileFloat64Range);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileFloat64Range)
SMILE_EASY_OBJECT_NO_CALL(SmileFloat64Range, "A Float64Range object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileFloat64Range)
SMILE_EASY_OBJECT_NO_UNBOX(SmileFloat64Range)

SMILE_EASY_OBJECT_TOBOOL(SmileFloat64Range, True)
SMILE_EASY_OBJECT_TOSTRING(SmileFloat64Range, ((obj->end >= obj->start && obj->stepping != +1
	|| obj->end < obj->start && obj->stepping != -1)
	? String_Format("(%S)..(%S) step %S",
		Float64_ToStringEx(obj->start, 0, 0, False),
		Float64_ToStringEx(obj->end, 0, 0, False),
		Float64_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("%S..%S",
		Float64_ToStringEx(obj->start, 0, 0, False),
		Float64_ToStringEx(obj->end, 0, 0, False)))
)

SmileFloat64Range SmileFloat64Range_Create(Float64 start, Float64 end, Float64 stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileFloat64Range smileRange = (SmileFloat64Range)GC_MALLOC_ATOMIC(sizeof(struct SmileFloat64RangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.Float64Range;
	smileRange->kind = SMILE_KIND_FLOAT64RANGE;
	smileRange->vtable = SmileFloat64Range_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileFloat64Range_Hash(SmileFloat64Range range)
{
	UInt32 result;
		UInt64 start = *(UInt64 *)&range->start;
		UInt64 end = *(UInt64 *)&range->end;
		UInt64 stepping = *(UInt64 *)&range->stepping;
		result = Smile_ApplyHashOracle((UInt32)(start ^ (start >> 32)) ^ (UInt32)(end ^ (end >> 32)) ^ (UInt32)(stepping ^ (stepping >> 32)));

	return result;
}

static SmileObject SmileFloat64Range_GetProperty(SmileFloat64Range self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileFloat64_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileFloat64_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileFloat64_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileFloat64_Create(self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1);
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileFloat64Range_SetProperty(SmileFloat64Range self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Float64Range: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Float64Range: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileFloat64Range_HasProperty(SmileFloat64Range self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileFloat64Range_GetPropertyNames(SmileFloat64Range self)
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

static Bool SmileFloat64Range_CompareEqual(SmileFloat64Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_FLOAT64RANGE) {
		return ((SmileFloat64Range)a)->start == ((SmileFloat64Range)b)->start
			&& ((SmileFloat64Range)a)->end == ((SmileFloat64Range)b)->end
			&& ((SmileFloat64Range)a)->stepping == ((SmileFloat64Range)b)->stepping;
	}
	else return False;
}

static Bool SmileFloat64Range_DeepEqual(SmileFloat64Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_FLOAT64RANGE) {
		return ((SmileFloat64Range)a)->start == ((SmileFloat64Range)b)->start
			&& ((SmileFloat64Range)a)->end == ((SmileFloat64Range)b)->end
			&& ((SmileFloat64Range)a)->stepping == ((SmileFloat64Range)b)->stepping;
	}
	else return False;
}
