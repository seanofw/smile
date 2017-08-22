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
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/range/smileinteger32range.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileInteger32Range);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileInteger32Range)
SMILE_EASY_OBJECT_NO_CALL(SmileInteger32Range, "An Integer32Range object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileInteger32Range)
SMILE_EASY_OBJECT_NO_UNBOX(SmileInteger32Range)

SMILE_EASY_OBJECT_TOBOOL(SmileInteger32Range, True)
SMILE_EASY_OBJECT_TOSTRING(SmileInteger32Range, 			((obj->end >= obj->start && obj->stepping != +1
				|| obj->end < obj->start && obj->stepping != -1)
				? String_Format("%S..%S step %S",
					String_CreateFromInteger(obj->start, 10, False),
					String_CreateFromInteger(obj->end, 10, False),
					String_CreateFromInteger(obj->stepping, 10, False))
				: String_Format("%S..%S",
					String_CreateFromInteger(obj->start, 10, False),
					String_CreateFromInteger(obj->end, 10, False)))
)

SmileInteger32Range SmileInteger32Range_Create(Int32 start, Int32 end, Int32 stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileInteger32Range smileRange = (SmileInteger32Range)GC_MALLOC_ATOMIC(sizeof(struct SmileInteger32RangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.Integer32Range;
	smileRange->kind = SMILE_KIND_INTEGER32RANGE;
	smileRange->vtable = SmileInteger32Range_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileInteger32Range_Hash(SmileInteger32Range range)
{
	UInt32 result;
		UInt32 start = (UInt32)range->start;
		UInt32 end = (UInt32)range->end;
		UInt32 stepping = (UInt32)range->stepping;
		result = (UInt32)(start ^ end ^ stepping);

	return result;
}

static SmileObject SmileInteger32Range_GetProperty(SmileInteger32Range self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileInteger32_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileInteger32_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileInteger32_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileInteger32_Create(self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1);
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileInteger32Range_SetProperty(SmileInteger32Range self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Integer32Range: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Integer32Range: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileInteger32Range_HasProperty(SmileInteger32Range self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileInteger32Range_GetPropertyNames(SmileInteger32Range self)
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

static Bool SmileInteger32Range_CompareEqual(SmileInteger32Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_INTEGER32RANGE) {
		return ((SmileInteger32Range)a)->start == ((SmileInteger32Range)b)->start
			&& ((SmileInteger32Range)a)->end == ((SmileInteger32Range)b)->end
			&& ((SmileInteger32Range)a)->stepping == ((SmileInteger32Range)b)->stepping;
	}
	else return False;
}

static Bool SmileInteger32Range_DeepEqual(SmileInteger32Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_INTEGER32RANGE) {
		return ((SmileInteger32Range)a)->start == ((SmileInteger32Range)b)->start
			&& ((SmileInteger32Range)a)->end == ((SmileInteger32Range)b)->end
			&& ((SmileInteger32Range)a)->stepping == ((SmileInteger32Range)b)->stepping;
	}
	else return False;
}
