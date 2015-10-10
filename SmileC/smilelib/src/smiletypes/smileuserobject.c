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
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>

static SmileVTable SmileUserObject_VTable_ReadOnly;
static SmileVTable SmileUserObject_VTable_ReadWrite;
static SmileVTable SmileUserObject_VTable_ReadAppend;
static SmileVTable SmileUserObject_VTable_ReadWriteAppend;

SmileUserObject SmileUserObject_CreateWithSize(SmileObject base, Int initialSize)
{
	SmileUserObject userObject = GC_MALLOC_STRUCT(struct SmileUserObjectInt);
	if (userObject == NULL) Smile_Abort_OutOfMemory();
	userObject->base = base;
	userObject->kind = SMILE_KIND_USEROBJECT | SMILE_SECURITY_READWRITEAPPEND;
	userObject->vtable = SmileUserObject_VTable_ReadWriteAppend;
	userObject->securityKey = (SmileObject)Smile_KnownObjects.Null;
	Int32Dict_ClearWithSize((Int32Dict)&userObject->dict, initialSize);
	return userObject;
}

Bool SmileUserObject_CompareEqual(SmileUserObject self, SmileObject other)
{
	UNUSED(self);
	return ((SmileObject)self == other);
}

UInt32 SmileUserObject_Hash(SmileUserObject self)
{
	return ((PtrInt)self & 0xFFFFFFFF) ^ Smile_HashOracle;
}

void SmileUserObject_SetSecurityKey(SmileUserObject self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	Bool isValidSecurityKey = self->securityKey->vtable->compareEqual(self->securityKey, oldSecurityKey);
	if (!isValidSecurityKey)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);

	self->securityKey = newSecurityKey;
}

void SmileUserObject_SetSecurity(SmileUserObject self, Int security, SmileObject securityKey)
{
	Bool isValidSecurityKey = self->securityKey->vtable->compareEqual(self->securityKey, securityKey);
	if (!isValidSecurityKey)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, (String)&Smile_KnownStrings.InvalidSecurityKey->string);

	self->kind = (self->kind & ~SMILE_SECURITY_READWRITEAPPEND) | (security & SMILE_SECURITY_READWRITEAPPEND);

	switch (security & SMILE_SECURITY_READWRITEAPPEND) {
		case SMILE_SECURITY_READONLY:
			self->vtable = SmileUserObject_VTable_ReadOnly;
			break;
		case SMILE_SECURITY_WRITABLE:
			self->vtable = SmileUserObject_VTable_ReadWrite;
			break;
		case SMILE_SECURITY_APPENDABLE:
			self->vtable = SmileUserObject_VTable_ReadAppend;
			break;
		case SMILE_SECURITY_READWRITEAPPEND:
			self->vtable = SmileUserObject_VTable_ReadWriteAppend;
			break;
	}
}

Int SmileUserObject_GetSecurity(SmileUserObject self)
{
	return self->kind & SMILE_SECURITY_READWRITEAPPEND;
}

SmileObject SmileUserObject_GetProperty(SmileUserObject self, Symbol propertyName)
{
	SmileObject obj;
	if (Int32Dict_TryGetValue((Int32Dict)&self->dict, (Int32)propertyName, (void **)&obj)) {
		return obj;
	}
	else {
		return self->base->vtable->getProperty((SmileObject)self, propertyName);
	}
}

void SmileUserObject_SetProperty_ReadOnly(SmileUserObject self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(propertyName);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.property_error,
		String_Format("Cannot set property \"%S\" on this object; this object is read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

void SmileUserObject_SetProperty_ReadWrite(SmileUserObject self, Symbol propertyName, SmileObject value)
{
	Bool wasReplaced = Int32Dict_ReplaceValue((Int32Dict)&self->dict, (Int32)propertyName, value);
	if (!wasReplaced) {
		Smile_ThrowException(Smile_KnownSymbols.property_error,
			String_Format("Cannot set property \"%S\" on this object; this object cannot be appended to.",
			SymbolTable_GetName(Smile_SymbolTable, propertyName)));
	}
}

void SmileUserObject_SetProperty_ReadAppend(SmileUserObject self, Symbol propertyName, SmileObject value)
{
	if (value->kind == SMILE_KIND_NULL) {
		if (Int32Dict_ContainsKey((Int32Dict)&self->dict, (Int32)propertyName)) {
			Smile_ThrowException(Smile_KnownSymbols.property_error,
				String_Format("Cannot set property \"%S\" on this object; this object can only be appended to.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
		}
	}
	else {
		Bool wasAdded = Int32Dict_Add((Int32Dict)&self->dict, (Int32)propertyName, value);

		if (!wasAdded) {
			Smile_ThrowException(Smile_KnownSymbols.property_error,
				String_Format("Cannot set property \"%S\" on this object; this object can only be appended to.",
				SymbolTable_GetName(Smile_SymbolTable, propertyName)));
		}
	}
}

void SmileUserObject_SetProperty_ReadWriteAppend(SmileUserObject self, Symbol propertyName, SmileObject value)
{
	if (value->kind == SMILE_KIND_NULL) {
		Int32Dict_Remove((Int32Dict)&self->dict, (Int32)propertyName);
	}
	else {
		Int32Dict_SetValue((Int32Dict)&self->dict, (Int32)propertyName, value);
	}
}

Bool SmileUserObject_HasProperty(SmileUserObject self, Symbol propertyName)
{
	return Int32Dict_ContainsKey((Int32Dict)&self->dict, (Int32)propertyName);
}

SmileList SmileUserObject_GetPropertyNames(SmileUserObject self)
{
	DECLARE_LIST_BUILDER(result);
	Symbol *keys;
	Int i, numKeys;

	keys = (Symbol *)Int32Dict_GetKeys((Int32Dict)&self->dict);
	numKeys = Int32Dict_Count((Int32Dict)&self->dict);

	for (i = 0; i < numKeys; i++) {
		LIST_BUILDER_APPEND(result, SmileSymbol_Create(keys[i]));
	}

	return LIST_BUILDER_HEAD(result);
}

Bool SmileUserObject_ToBool(SmileUserObject self)
{
	UNUSED(self);
	return True;
}

Int32 SmileUserObject_ToInteger32(SmileUserObject self)
{
	UNUSED(self);
	return 0;
}

Real64 SmileUserObject_ToReal64(SmileUserObject self)
{
	UNUSED(self);
	return 0;
}

String SmileUserObject_ToString(SmileUserObject self)
{
	UNUSED(self);
	return String_Format("user object");
}

SMILE_VTABLE(SmileUserObject_VTable_ReadWriteAppend, SmileUserObject)
{
	SmileUserObject_CompareEqual,
	SmileUserObject_Hash,

	SmileUserObject_SetSecurityKey,
	SmileUserObject_SetSecurity,
	SmileUserObject_GetSecurity,

	SmileUserObject_GetProperty,
	SmileUserObject_SetProperty_ReadWriteAppend,
	SmileUserObject_HasProperty,
	SmileUserObject_GetPropertyNames,

	SmileUserObject_ToBool,
	SmileUserObject_ToInteger32,
	SmileUserObject_ToReal64,
	SmileUserObject_ToString,
};

SMILE_VTABLE(SmileUserObject_VTable_ReadWrite, SmileUserObject)
{
	SmileUserObject_CompareEqual,
	SmileUserObject_Hash,

	SmileUserObject_SetSecurityKey,
	SmileUserObject_SetSecurity,
	SmileUserObject_GetSecurity,

	SmileUserObject_GetProperty,
	SmileUserObject_SetProperty_ReadWrite,
	SmileUserObject_HasProperty,
	SmileUserObject_GetPropertyNames,

	SmileUserObject_ToBool,
	SmileUserObject_ToInteger32,
	SmileUserObject_ToReal64,
	SmileUserObject_ToString,
};

SMILE_VTABLE(SmileUserObject_VTable_ReadAppend, SmileUserObject)
{
	SmileUserObject_CompareEqual,
	SmileUserObject_Hash,

	SmileUserObject_SetSecurityKey,
	SmileUserObject_SetSecurity,
	SmileUserObject_GetSecurity,

	SmileUserObject_GetProperty,
	SmileUserObject_SetProperty_ReadAppend,
	SmileUserObject_HasProperty,
	SmileUserObject_GetPropertyNames,

	SmileUserObject_ToBool,
	SmileUserObject_ToInteger32,
	SmileUserObject_ToReal64,
	SmileUserObject_ToString,
};

SMILE_VTABLE(SmileUserObject_VTable_ReadOnly, SmileUserObject)
{
	SmileUserObject_CompareEqual,
	SmileUserObject_Hash,

	SmileUserObject_SetSecurityKey,
	SmileUserObject_SetSecurity,
	SmileUserObject_GetSecurity,

	SmileUserObject_GetProperty,
	SmileUserObject_SetProperty_ReadOnly,
	SmileUserObject_HasProperty,
	SmileUserObject_GetPropertyNames,

	SmileUserObject_ToBool,
	SmileUserObject_ToInteger32,
	SmileUserObject_ToReal64,
	SmileUserObject_ToString,
};
