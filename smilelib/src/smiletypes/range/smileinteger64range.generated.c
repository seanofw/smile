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

#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/range/smileinteger64range.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileInteger64Range);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileInteger64Range)
SMILE_EASY_OBJECT_NO_CALL(SmileInteger64Range, "An Integer64Range object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileInteger64Range)
SMILE_EASY_OBJECT_NO_UNBOX(SmileInteger64Range)

SMILE_EASY_OBJECT_TOBOOL(SmileInteger64Range, True)
SMILE_EASY_OBJECT_TOSTRING(SmileInteger64Range, 		((obj->end >= obj->start && obj->stepping != +1
			|| obj->end < obj->start && obj->stepping != -1)
			? String_Format("%S..%S step %S",
				String_CreateFromInteger(obj->start, 10, False),
				String_CreateFromInteger(obj->end, 10, False),
				String_CreateFromInteger(obj->stepping, 10, False))
			: String_Format("%S..%S",
				String_CreateFromInteger(obj->start, 10, False),
				String_CreateFromInteger(obj->end, 10, False)))
)

SmileInteger64Range SmileInteger64Range_Create(Int64 start, Int64 end, Int64 stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileInteger64Range smileRange = (SmileInteger64Range)GC_MALLOC_ATOMIC(sizeof(struct SmileInteger64RangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.Integer64Range;
	smileRange->kind = SMILE_KIND_INTEGER64RANGE;
	smileRange->vtable = SmileInteger64Range_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileInteger64Range_Hash(SmileInteger64Range range)
{
	UInt32 result;
		UInt64 start = (UInt64)range->start;
		UInt64 end = (UInt64)range->end;
		UInt64 stepping = (UInt64)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)(start ^ (start >> 32)) ^ (UInt32)(end ^ (end >> 32)) ^ (UInt32)(stepping ^ (stepping >> 32)));

	return result;
}

static SmileObject SmileInteger64Range_GetProperty(SmileInteger64Range self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileInteger64_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileInteger64_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileInteger64_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileInteger64_Create(self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1);
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileInteger64Range_SetProperty(SmileInteger64Range self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Integer64Range: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Integer64Range: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileInteger64Range_HasProperty(SmileInteger64Range self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileInteger64Range_GetPropertyNames(SmileInteger64Range self)
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

static Bool SmileInteger64Range_CompareEqual(SmileInteger64Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_INTEGER64RANGE) {
		return ((SmileInteger64Range)a)->start == ((SmileInteger64Range)b)->start
			&& ((SmileInteger64Range)a)->end == ((SmileInteger64Range)b)->end
			&& ((SmileInteger64Range)a)->stepping == ((SmileInteger64Range)b)->stepping;
	}
	else return False;
}

static Bool SmileInteger64Range_DeepEqual(SmileInteger64Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_INTEGER64RANGE) {
		return ((SmileInteger64Range)a)->start == ((SmileInteger64Range)b)->start
			&& ((SmileInteger64Range)a)->end == ((SmileInteger64Range)b)->end
			&& ((SmileInteger64Range)a)->stepping == ((SmileInteger64Range)b)->stepping;
	}
	else return False;
}
