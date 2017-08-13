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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/range/smilebyterange.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileByteRange);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileByteRange)
SMILE_EASY_OBJECT_NO_CALL(SmileByteRange, "A ByteRange object")
SMILE_EASY_OBJECT_NO_SOURCE(SmileByteRange)
SMILE_EASY_OBJECT_NO_UNBOX(SmileByteRange)

SMILE_EASY_OBJECT_TOBOOL(SmileByteRange, True)
SMILE_EASY_OBJECT_TOINT(SmileByteRange, ((Int32)obj->end - (Int32)obj->start))
SMILE_EASY_OBJECT_TOREAL(SmileByteRange, Real64_FromInt64((Int32)obj->end - (Int32)obj->start))
SMILE_EASY_OBJECT_TOFLOAT(SmileByteRange, (Float64)((Int32)obj->end - (Int32)obj->start))
SMILE_EASY_OBJECT_TOSTRING(SmileByteRange, 			((obj->end >= obj->start && obj->stepping != +1
				|| obj->end < obj->start && obj->stepping != -1)
				? String_Format("%S..%S step %S",
					String_CreateFromInteger(obj->start, 10, False),
					String_CreateFromInteger(obj->end, 10, False),
					String_CreateFromInteger(obj->stepping, 10, False))
				: String_Format("%S..%S",
					String_CreateFromInteger(obj->start, 10, False),
					String_CreateFromInteger(obj->end, 10, False)))
)

SmileByteRange SmileByteRange_Create(Byte start, Byte end, Byte stepping)
{
	// We MALLOC_ATOMIC here because the base is a known pointer that will never be collected.
	SmileByteRange smileRange = (SmileByteRange)GC_MALLOC_ATOMIC(sizeof(struct SmileByteRangeInt));
	if (smileRange == NULL) Smile_Abort_OutOfMemory();
	smileRange->base = (SmileObject)Smile_KnownBases.ByteRange;
	smileRange->kind = SMILE_KIND_BYTERANGE;
	smileRange->vtable = SmileByteRange_VTable;
	smileRange->start = start;
	smileRange->end = end;
	smileRange->stepping = stepping;
	return smileRange;
}

static UInt32 SmileByteRange_Hash(SmileByteRange range)
{
	UInt32 result;
		Byte start = range->start;
		Byte end = range->end;
		Byte stepping = range->stepping;
		result = (UInt32)((UInt32)start ^ (UInt32)(end << 8) ^ (UInt32)(stepping << 16));

	return result;
}

static SmileObject SmileByteRange_GetProperty(SmileByteRange self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.start)
		return (SmileObject)SmileByte_Create(self->start);
	else if (propertyName == Smile_KnownSymbols.end)
		return (SmileObject)SmileByte_Create(self->end);
	else if (propertyName == Smile_KnownSymbols.stepping)
		return (SmileObject)SmileByte_Create(self->stepping);
	else if (propertyName == Smile_KnownSymbols.length)
		return (SmileObject)SmileByte_Create(self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1);
	else
		return self->base->vtable->getProperty(self->base, propertyName);
}

static void SmileByteRange_SetProperty(SmileByteRange self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on ByteRange: Ranges are read-only objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on ByteRange: This property does not exist, and ranges are not appendable objects.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

static Bool SmileByteRange_HasProperty(SmileByteRange self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.start || propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.stepping || propertyName == Smile_KnownSymbols.length);
}

static SmileList SmileByteRange_GetPropertyNames(SmileByteRange self)
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

static Bool SmileByteRange_CompareEqual(SmileByteRange a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData)
{
	if (SMILE_KIND(b) == SMILE_KIND_BYTERANGE) {
		return ((SmileByteRange)a)->start == ((SmileByteRange)b)->start
			&& ((SmileByteRange)a)->end == ((SmileByteRange)b)->end
			&& ((SmileByteRange)a)->stepping == ((SmileByteRange)b)->stepping;
	}
	else return False;
}

static Bool SmileByteRange_DeepEqual(SmileByteRange a, SmileUnboxedData aData, SmileObject b, SmileUnboxedData bData, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	if (SMILE_KIND(b) == SMILE_KIND_BYTERANGE) {
		return ((SmileByteRange)a)->start == ((SmileByteRange)b)->start
			&& ((SmileByteRange)a)->end == ((SmileByteRange)b)->end
			&& ((SmileByteRange)a)->stepping == ((SmileByteRange)b)->stepping;
	}
	else return False;
}
