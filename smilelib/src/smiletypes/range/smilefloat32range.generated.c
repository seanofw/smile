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

#include <smile/smiletypes/numeric/smilefloat32.h>
#include <smile/smiletypes/range/smilefloat32range.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileFloat32Range);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileFloat32Range)
SMILE_EASY_OBJECT_NO_CALL(SmileFloat32Range, "A Float32Range object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileFloat32Range)
SMILE_EASY_OBJECT_NO_UNBOX(SmileFloat32Range)

SMILE_EASY_OBJECT_TOBOOL(SmileFloat32Range, True)
SMILE_EASY_OBJECT_TOSTRING(SmileFloat32Range, ((obj->end >= obj->start && obj->stepping != +1
	|| obj->end < obj->start && obj->stepping != -1)
	? String_Format("(%S)..(%S) step %S",
		Float64_ToStringEx(obj->start, 0, 0, False),
		Float64_ToStringEx(obj->end, 0, 0, False),
		Float64_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("%S..%S",
		Float64_ToStringEx(obj->start, 0, 0, False),
		Float64_ToStringEx(obj->end, 0, 0, False)))
)

SmileFloat32Range SmileFloat32Range_Create(Float32 start, Float32 end, Float32 stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileFloat32Range smileRange = (SmileFloat32Range)GC_MALLOC_ATOMIC(sizeof(struct SmileFloat32RangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.Float32Range;
	smileRange->kind = SMILE_KIND_FLOAT32RANGE;
	smileRange->vtable = SmileFloat32Range_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileFloat32Range_Hash(SmileFloat32Range range)
{
	UInt32 result;
		UInt32 start = *(UInt32 *)&range->start;
		UInt32 end = *(UInt32 *)&range->end;
		UInt32 stepping = *(UInt32 *)&range->stepping;
		result = Smile_ApplyHashOracle(start ^ end ^ stepping);

	return result;
}

static SmileObject SmileFloat32Range_GetProperty(SmileFloat32Range self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileFloat32_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileFloat32_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileFloat32_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileFloat32_Create(self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1);
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileFloat32Range_SetProperty(SmileFloat32Range self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Float32Range: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Float32Range: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileFloat32Range_HasProperty(SmileFloat32Range self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileFloat32Range_GetPropertyNames(SmileFloat32Range self)
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

static Bool SmileFloat32Range_CompareEqual(SmileFloat32Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_FLOAT32RANGE) {
		return ((SmileFloat32Range)a)->start == ((SmileFloat32Range)b)->start
			&& ((SmileFloat32Range)a)->end == ((SmileFloat32Range)b)->end
			&& ((SmileFloat32Range)a)->stepping == ((SmileFloat32Range)b)->stepping;
	}
	else return False;
}

static Bool SmileFloat32Range_DeepEqual(SmileFloat32Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_FLOAT32RANGE) {
		return ((SmileFloat32Range)a)->start == ((SmileFloat32Range)b)->start
			&& ((SmileFloat32Range)a)->end == ((SmileFloat32Range)b)->end
			&& ((SmileFloat32Range)a)->stepping == ((SmileFloat32Range)b)->stepping;
	}
	else return False;
}
