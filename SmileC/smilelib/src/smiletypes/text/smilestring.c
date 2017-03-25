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

#include <smile/numeric/real.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(SmileString);

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileString)
SMILE_EASY_OBJECT_NO_CALL(SmileString)
SMILE_EASY_OBJECT_NO_SOURCE(SmileString)
SMILE_EASY_OBJECT_NO_UNBOX(SmileString)

SmileString SmileString_Create(String string)
{
	SmileString str = GC_MALLOC_STRUCT(struct SmileStringInt);
	if (str == NULL) Smile_Abort_OutOfMemory();
	str->base = (SmileObject)Smile_KnownBases.String;
	str->kind = SMILE_KIND_STRING;
	str->vtable = SmileString_VTable;
	str->string.text = ((struct StringInt *)string)->text;
	str->string.length = ((struct StringInt *)string)->length;
	return str;
}

Bool SmileString_CompareEqual(SmileString self, SmileUnboxedData selfUnboxed, SmileObject other, SmileUnboxedData otherUnboxed)
{
	SmileString otherString;
	Int length;

	if (SMILE_KIND(other) != SMILE_KIND_STRING) return False;
	otherString = (SmileString)other;

	if (otherString == self) return True;

	length = self->string.length;
	if (length != otherString->string.length) return False;

	return !MemCmp(self->string.text, otherString->string.text, length);
}

Bool SmileString_DeepEqual(SmileString self, SmileUnboxedData selfUnboxed, SmileObject other, SmileUnboxedData otherUnboxed, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	return SmileString_CompareEqual(self, selfUnboxed, other, otherUnboxed);
}

UInt32 SmileString_Hash(SmileString self)
{
	String str = SmileString_ToString(self, (SmileUnboxedData){ 0 });
	return String_Hash(str);
}

SmileObject SmileString_GetProperty(SmileString self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.length) {
		#if SizeofInt <= 4
			return (SmileObject)SmileInteger32_Create(self->string.length);
		#else
			Int length = self->string.length;
			return (SmileObject)SmileInteger32_Create(length < Int32Max ? (Int32)length : Int32Max);
		#endif
	}
	return self->base->vtable->getProperty(self->base, propertyName);
}

void SmileString_SetProperty(SmileString self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a string, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileString_HasProperty(SmileString self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.length);
}

SmileList SmileString_GetPropertyNames(SmileString self)
{
	SmileList head, tail;

	LIST_INIT(head, tail);

	UNUSED(self);

	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.length));

	return head;
}

Bool SmileString_ToBool(SmileString self, SmileUnboxedData unboxedData)
{
	Bool result;
	return String_ParseBool(SmileString_GetString(self), &result) ? result : False;
}

Int32 SmileString_ToInteger32(SmileString self, SmileUnboxedData unboxedData)
{
	Int64 result;
	return String_ParseInteger(SmileString_GetString(self), 10, &result) ? (Int32)result : 0;
}

Float64 SmileString_ToFloat64(SmileString self, SmileUnboxedData unboxedData)
{
	Float64 result;
	return String_ParseFloat(SmileString_GetString(self), 10, &result) ? result : 0.0;
}

Real64 SmileString_ToReal64(SmileString self, SmileUnboxedData unboxedData)
{
	Real128 result;
	return String_ParseReal(SmileString_GetString(self), 10, &result) ? Real128_ToReal64(result) : Real64_Zero;
}

String SmileString_ToString(SmileString str, SmileUnboxedData unboxedData)
{
	return String_Format("\"%S\"", String_AddCSlashes((String)&(str->string)));
}
