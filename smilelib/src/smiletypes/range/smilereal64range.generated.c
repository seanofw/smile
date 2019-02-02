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

#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/range/smilereal64range.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileReal64Range);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileReal64Range)
SMILE_EASY_OBJECT_NO_CALL(SmileReal64Range, "A Real64Range object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileReal64Range)
SMILE_EASY_OBJECT_NO_UNBOX(SmileReal64Range)

SMILE_EASY_OBJECT_TOBOOL(SmileReal64Range, True)
SMILE_EASY_OBJECT_TOSTRING(SmileReal64Range, ((Real64_Ge(obj->end, obj->start) && Real64_Ne(obj->stepping, Real64_One)
	|| Real64_Lt(obj->end, obj->start) && Real64_Ne(obj->stepping, Real64_One))
	? String_Format("(%S)..(%S) step %S",
		Real64_ToStringEx(obj->start, 0, 0, False),
		Real64_ToStringEx(obj->end, 0, 0, False),
		Real64_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("(%S)..(%S)",
		Real64_ToStringEx(obj->start, 0, 0, False),
		Real64_ToStringEx(obj->end, 0, 0, False)))
)

SmileReal64Range SmileReal64Range_Create(Real64 start, Real64 end, Real64 stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileReal64Range smileRange = (SmileReal64Range)GC_MALLOC_ATOMIC(sizeof(struct SmileReal64RangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.Real64Range;
	smileRange->kind = SMILE_KIND_REAL64RANGE;
	smileRange->vtable = SmileReal64Range_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileReal64Range_Hash(SmileReal64Range range)
{
	UInt32 result;
		UInt64 start = *(UInt64 *)&range->start;
		UInt64 end = *(UInt64 *)&range->end;
		UInt64 stepping = *(UInt64 *)&range->stepping;
		UInt64 hash = start ^ end ^ stepping;
		result = Smile_ApplyHashOracle((Int32)(hash ^ (hash >> 32)));

	return result;
}

static SmileObject SmileReal64Range_GetProperty(SmileReal64Range self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileReal64_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileReal64_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileReal64_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileReal64_Create(Real64_Gt(self->end, self->start) ? Real64_Add(Real64_Sub(self->end, self->start), Real64_One) : Real64_Add(Real64_Sub(self->start, self->end), Real64_One));
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileReal64Range_SetProperty(SmileReal64Range self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Real64Range: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on Real64Range: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileReal64Range_HasProperty(SmileReal64Range self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileReal64Range_GetPropertyNames(SmileReal64Range self)
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

static Bool SmileReal64Range_CompareEqual(SmileReal64Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_REAL64RANGE) {
		return Real64_Eq(((SmileReal64Range)a)->start, ((SmileReal64Range)b)->start)
			&& Real64_Eq(((SmileReal64Range)a)->end, ((SmileReal64Range)b)->end)
			&& Real64_Eq(((SmileReal64Range)a)->stepping, ((SmileReal64Range)b)->stepping);
	}
	else return False;
}

static Bool SmileReal64Range_DeepEqual(SmileReal64Range a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_REAL64RANGE) {
		return Real64_Eq(((SmileReal64Range)a)->start, ((SmileReal64Range)b)->start)
			&& Real64_Eq(((SmileReal64Range)a)->end, ((SmileReal64Range)b)->end)
			&& Real64_Eq(((SmileReal64Range)a)->stepping, ((SmileReal64Range)b)->stepping);
	}
	else return False;
}
