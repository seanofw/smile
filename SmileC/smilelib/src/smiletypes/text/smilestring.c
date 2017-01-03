//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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

Bool SmileString_CompareEqual(SmileString self, SmileObject other)
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

UInt32 SmileString_Hash(SmileString self)
{
	String str = SmileString_ToString(self);
	return String_Hash(str);
}

void SmileString_SetSecurityKey(SmileString self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileString_SetSecurity(SmileString self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileString_GetSecurity(SmileString self)
{
	UNUSED(self);
	return SMILE_SECURITY_READONLY;
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
	return self->base->vtable->getProperty((SmileObject)self, propertyName);
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

Bool SmileString_ToBool(SmileString self)
{
	Bool result;
	return String_ParseBool(SmileString_ToString(self), &result) ? result : False;
}

Int32 SmileString_ToInteger32(SmileString self)
{
	Int64 result;
	return String_ParseInteger(SmileString_ToString(self), 10, &result) ? (Int32)result : 0;
}

Float64 SmileString_ToFloat64(SmileString self)
{
	Float64 result;
	return String_ParseFloat(SmileString_ToString(self), 10, &result) ? result : 0.0;
}

Real64 SmileString_ToReal64(SmileString self)
{
	Real128 result;
	return String_ParseReal(SmileString_ToString(self), 10, &result) ? Real128_ToReal64(result) : Real64_Zero;
}

Bool SmileString_Call(SmileString self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	Smile_ThrowException(Smile_KnownSymbols.eval_error, Smile_KnownStrings.invalidFunctionError);

	return True;
}

SMILE_VTABLE(SmileString_VTable, SmileString)
{
	SmileString_CompareEqual,
	SmileString_Hash,

	SmileString_SetSecurityKey,
	SmileString_SetSecurity,
	SmileString_GetSecurity,

	SmileString_GetProperty,
	SmileString_SetProperty,
	SmileString_HasProperty,
	SmileString_GetPropertyNames,

	SmileString_ToBool,
	SmileString_ToInteger32,
	SmileString_ToFloat64,
	SmileString_ToReal64,
	SmileString_ToString,

	SmileString_Call,
};
