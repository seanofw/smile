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
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_EASY_OBJECT_VTABLE(String);

SMILE_EASY_OBJECT_READONLY_SECURITY(String)
SMILE_EASY_OBJECT_NO_CALL(String)
SMILE_EASY_OBJECT_NO_SOURCE(String)
SMILE_EASY_OBJECT_NO_UNBOX(String)

Bool String_CompareEqual(String self, SmileUnboxedData selfUnboxed, SmileObject other, SmileUnboxedData otherUnboxed)
{
	String otherString;
	Int length;

	if (SMILE_KIND(other) != SMILE_KIND_STRING) return False;
	otherString = (String)other;

	if (otherString == self) return True;

	length = String_Length(self);
	if (length != String_Length(otherString)) return False;

	return !MemCmp(String_GetBytes(self), String_GetBytes(otherString), length);
}

Bool String_DeepEqual(String self, SmileUnboxedData selfUnboxed, SmileObject other, SmileUnboxedData otherUnboxed, PointerSet visitedPointers)
{
	UNUSED(visitedPointers);

	return String_CompareEqual(self, selfUnboxed, other, otherUnboxed);
}

SmileObject String_GetProperty(String self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.length) {
		#if SizeofInt <= 4
			return (SmileObject)SmileInteger32_Create(String_Length(self));
		#else
			Int length = String_Length(self);
			return (SmileObject)SmileInteger32_Create(length < Int32Max ? (Int32)length : Int32Max);
		#endif
	}
	return self->base->vtable->getProperty(self->base, propertyName);
}

void String_SetProperty(String self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot set property \"%S\" on a string, which is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool String_HasProperty(String self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.length);
}

SmileList String_GetPropertyNames(String self)
{
	SmileList head, tail;

	LIST_INIT(head, tail);

	UNUSED(self);

	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.length));

	return head;
}

Bool String_ToBool(String self, SmileUnboxedData unboxedData)
{
	Bool result;
	return String_ParseBool(self, &result) ? result : False;
}

Int32 String_ToInteger32(String self, SmileUnboxedData unboxedData)
{
	Int64 result;
	return String_ParseInteger(self, 10, &result) ? (Int32)result : 0;
}

Float64 String_ToFloat64(String self, SmileUnboxedData unboxedData)
{
	Float64 result;
	return String_ParseFloat(self, 10, &result) ? result : 0.0;
}

Real64 String_ToReal64(String self, SmileUnboxedData unboxedData)
{
	Real128 result;
	return String_ParseReal(self, 10, &result) ? Real128_ToReal64(result) : Real64_Zero;
}

String String_ToString(String str, SmileUnboxedData unboxedData)
{
	return String_Format("\"%S\"", String_AddCSlashes(str));
}
