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

#include <smile/types.h>
#include <smile/numeric/real64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>

SmileList SmileList_Cons(SmileObject a, SmileObject d)
{
	SmileList smileList = GC_MALLOC_STRUCT(struct SmileListInt);
	if (smileList == NULL) Smile_Abort_OutOfMemory();
	smileList->base = Smile_KnownObjects.Object;
	smileList->kind = SMILE_KIND_LIST | SMILE_SECURITY_WRITABLE;
	smileList->vtable = SmileList_VTable;
	smileList->a = a;
	smileList->d = d;
	return smileList;
}

SmileList SmileList_ConsWithSource(SmileObject a, SmileObject d, LexerPosition position)
{
	struct SmileListWithSourceInt *smileList = GC_MALLOC_STRUCT(struct SmileListWithSourceInt);
	if (smileList == NULL) Smile_Abort_OutOfMemory();
	smileList->base = Smile_KnownObjects.Object;
	smileList->kind = SMILE_KIND_LIST | SMILE_FLAG_LISTWITHSOURCE | SMILE_SECURITY_WRITABLE;
	smileList->vtable = SmileList_VTable;
	smileList->a = a;
	smileList->d = d;
	smileList->position = position;
	return (SmileList)smileList;
}

SmileList SmileList_CreateListFromArray(SmileObject *objects, Int numObjects)
{
	SmileList head, tail;

	LIST_INIT(head, tail);

	while (numObjects-- > 0) {
		LIST_APPEND(head, tail, *objects++);
	}

	return head;
}

SmileList SmileList_CreateList(SmileObject firstObject, ...)
{
	SmileList result;
	va_list v;

	va_start(v, firstObject);
	result = SmileList_CreateListv(firstObject, v);
	va_end(v);

	return result;
}

SmileList SmileList_CreateListv(SmileObject firstObject, va_list v)
{
	SmileList head, tail;
	SmileObject obj;

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, firstObject);

	while ((obj = va_arg(v, SmileObject)) != NULL) {
		LIST_APPEND(head, tail, obj);
	}

	return head;
}

Bool SmileList_CompareEqual(SmileList self, SmileObject other)
{
	SmileList otherList;

	do {
		if (!SmileObject_IsList(other)) return False;
		otherList = (SmileList)other;

		// Compare this list element.
		if (!SMILE_VCALL1(self->a, compareEqual, otherList->a))
			return False;

		self = (SmileList)self->d;
		other = otherList->d;

		// We use a loop to unroll the ->d recursion, where possible.
	} while (SMILE_KIND(self) == SMILE_KIND_LIST);

	// Compare the last ->d objects, whatever they are.
	return SMILE_VCALL1((SmileObject)self, compareEqual, other);
}

UInt32 SmileList_Hash(SmileList self)
{
	return (UInt32)(PtrInt)self;
}

void SmileList_SetSecurityKey(SmileList self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	UNUSED(self);
	UNUSED(newSecurityKey);
	UNUSED(oldSecurityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

void SmileList_SetSecurity(SmileList self, Int security, SmileObject securityKey)
{
	UNUSED(self);
	UNUSED(security);
	UNUSED(securityKey);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);
}

Int SmileList_GetSecurity(SmileList self)
{
	UNUSED(self);
	return SMILE_SECURITY_WRITABLE;
}

SmileObject SmileList_GetProperty(SmileList self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.a)
		return self->a;
	else if (propertyName == Smile_KnownSymbols.d)
		return self->d;
	else
		return NullObject;
}

void SmileList_SetProperty(SmileList self, Symbol propertyName, SmileObject value)
{
	if (propertyName == Smile_KnownSymbols.a)
		self->a = value;
	else if (propertyName == Smile_KnownSymbols.d)
		self->d = value;
	else {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on list cell: This property does not exist, and list cells are not appendable objects.",
			SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

Bool SmileList_HasProperty(SmileList self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.a || propertyName == Smile_KnownSymbols.d);
}

SmileList SmileList_GetPropertyNames(SmileList self)
{
	DECLARE_LIST_BUILDER(listBuilder);

	UNUSED(self);

	LIST_BUILDER_APPEND(listBuilder, SmileSymbol_Create(Smile_KnownSymbols.a));
	LIST_BUILDER_APPEND(listBuilder, SmileSymbol_Create(Smile_KnownSymbols.d));

	return LIST_BUILDER_HEAD(listBuilder);
}

Bool SmileList_ToBool(SmileList self)
{
	UNUSED(self);
	return True;
}

Int32 SmileList_ToInteger32(SmileList self)
{
	UNUSED(self);
	return 1;
}

Float64 SmileList_ToFloat64(SmileList self)
{
	UNUSED(self);
	return 1.0;
}

Real64 SmileList_ToReal64(SmileList self)
{
	UNUSED(self);
	return Real64_One;
}

String SmileList_ToString(SmileList self)
{
	UNUSED(self);
	return String_Format("List");
}

SMILE_VTABLE(SmileList_VTable, SmileList)
{
	SmileList_CompareEqual,
	SmileList_Hash,

	SmileList_SetSecurityKey,
	SmileList_SetSecurity,
	SmileList_GetSecurity,

	SmileList_GetProperty,
	SmileList_SetProperty,
	SmileList_HasProperty,
	SmileList_GetPropertyNames,

	SmileList_ToBool,
	SmileList_ToInteger32,
	SmileList_ToFloat64,
	SmileList_ToReal64,
	SmileList_ToString,
};
