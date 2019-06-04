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

#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/range/smilereal32range.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileReal32Range);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileReal32Range)
SMILE_EASY_OBJECT_NO_CALL(SmileReal32Range, "A Real32Range object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileReal32Range)
SMILE_EASY_OBJECT_NO_UNBOX(SmileReal32Range)

SMILE_EASY_OBJECT_TOBOOL(SmileReal32Range, True)
SMILE_EASY_OBJECT_TOSTRING(SmileReal32Range, ((Real32_Ge(obj->end, obj->start) && Real32_Ne(obj->stepping, Real32_One)
	|| Real32_Lt(obj->end, obj->start) && Real32_Ne(obj->stepping, Real32_One))
	? String_Format("(%S)..(%S) step %S",
		Real32_ToStringEx(obj->start, 0, 0, False),
		Real32_ToStringEx(obj->end, 0, 0, False),
		Real32_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("(%S)..(%S)",
		Real32_ToStringEx(obj->start, 0, 0, False),
		Real32_ToStringEx(obj->end, 0, 0, False)))
)

SmileReal32Range SmileReal32Range_Create(Real32 start, Real32 end, Real32 stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileReal32Range smileRange = (SmileReal32Range)GC_MALLOC_ATOMIC(sizeof(struct SmileReal32RangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.Real32Range;
	smileRange->kind = SMILE_KIND_REAL32RANGE;
	smileRange->vtable = SmileReal32Range_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileReal32Range_Hash(SmileReal32Range range)
{
	UInt32 result;
		UInt32 start = *(UInt32 *)&range->start;
		UInt32 end = *(UInt32 *)&range->end;
		UInt32 stepping = *(UInt32 *)&range->stepping;
		result = Smile_ApplyHashOracle(start ^ end ^ stepping);

	return result;
}

static SmileObject SmileReal32Range_GetProperty(SmileReal32Range self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileReal32_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileReal32_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileReal32_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileReal32_Create(Real32_Gt(self->end, self->start) ? Real32_Add(Real32_Sub(self->end, self->start), Real32_One) : Real32_Add(Real32_Sub(self->start, self->end), Real32_One));
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileReal32Range_SetProperty(SmileReal32Range self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Real32Range: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Real32Range: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileReal32Range_HasProperty(SmileReal32Range self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileReal32Range_GetPropertyNames(SmileReal32Range self)
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

static Bool SmileReal32Range_CompareEqual(SmileReal32Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_REAL32RANGE) {
		return Real32_Eq(((SmileReal32Range)a)->start, ((SmileReal32Range)b)->start)
			&& Real32_Eq(((SmileReal32Range)a)->end, ((SmileReal32Range)b)->end)
			&& Real32_Eq(((SmileReal32Range)a)->stepping, ((SmileReal32Range)b)->stepping);
	}
	else return False;
}

static Bool SmileReal32Range_DeepEqual(SmileReal32Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_REAL32RANGE) {
		return Real32_Eq(((SmileReal32Range)a)->start, ((SmileReal32Range)b)->start)
			&& Real32_Eq(((SmileReal32Range)a)->end, ((SmileReal32Range)b)->end)
			&& Real32_Eq(((SmileReal32Range)a)->stepping, ((SmileReal32Range)b)->stepping);
	}
	else return False;
}
