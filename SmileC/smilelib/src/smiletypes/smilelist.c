//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>

SmileList SmileList_Cons(SmileObject a, SmileObject d)
{
	SmileList smileList = GC_MALLOC_STRUCT(struct SmileListInt);
	if (smileList == NULL) Smile_Abort_OutOfMemory();
	smileList->base = Smile_KnownObjects.Object;
	smileList->kind = SMILE_KIND_LIST;
	smileList->vtable = SmileList_VTable;
	smileList->a = a;
	smileList->d = d;
	return smileList;
}

Bool SmileList_CompareEqual(SmileList self, SmileObject other)
{
	SmileList otherList;

	if (other->kind != SMILE_KIND_LIST) return False;
	otherList = (SmileList)other;

	return self->a == otherList->a && self->d == otherList->d;
}

UInt32 SmileList_Hash(SmileList self)
{
	return (UInt32)(PtrInt)self;
}

void SmileList_SetSecurity(SmileList self, Int security)
{
	UNUSED(self);
	UNUSED(security);
	Smile_ThrowException(Smile_KnownSymbols.object_security_error,
		String_Format("Cannot alter security of a list cell to be anything other than read/write."));
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
		return (SmileObject)Smile_KnownObjects.Null;
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
	UNUSED(self);
	DECLARE_LIST_BUILDER(listBuilder);
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

Real64 SmileList_ToReal64(SmileList self)
{
	UNUSED(self);
	return 1;
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
	SmileList_SetSecurity,
	SmileList_GetSecurity,

	SmileList_GetProperty,
	SmileList_SetProperty,
	SmileList_HasProperty,
	SmileList_GetPropertyNames,

	SmileList_ToBool,
	SmileList_ToInteger32,
	SmileList_ToReal64,
	SmileList_ToString,
};
