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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/text/smilesymbol.h>

extern SmileVTable SmileUserObject_VTable_ReadOnly;
extern SmileVTable SmileUserObject_VTable_ReadWrite;
extern SmileVTable SmileUserObject_VTable_ReadAppend;
extern SmileVTable SmileUserObject_VTable_ReadWriteAppend;

SmileUserObject SmileUserObject_CreateWithSize(SmileObject base, Int initialSize)
{
	SmileUserObject userObject = GC_MALLOC_STRUCT(struct SmileUserObjectInt);
	if (userObject == NULL || initialSize >= Int32Max) Smile_Abort_OutOfMemory();

	userObject->base = base;
	userObject->kind = SMILE_KIND_USEROBJECT | SMILE_SECURITY_READWRITEAPPEND;
	userObject->vtable = SmileUserObject_VTable_ReadWriteAppend;
	userObject->securityKey = NullObject;

	Int32Dict_ClearWithSize((Int32Dict)&userObject->dict, (Int32)initialSize);

	return userObject;
}

void SmileUserObject_InitWithSize(SmileUserObject userObject, SmileObject base, Int initialSize)
{
	if (initialSize >= Int32Max) Smile_Abort_OutOfMemory();

	userObject->base = base;
	userObject->kind = SMILE_KIND_USEROBJECT | SMILE_SECURITY_READWRITEAPPEND;
	userObject->vtable = SmileUserObject_VTable_ReadWriteAppend;
	userObject->securityKey = NullObject;

	Int32Dict_ClearWithSize((Int32Dict)&userObject->dict, (Int32)initialSize);
}

SmileUserObject SmileUserObject_CreateFromArgPairs(SmileObject base, SmileArg *argPairs, Int numArgPairs)
{
	SmileUserObject userObject = SmileUserObject_CreateWithSize(base, numArgPairs);
	Int i;

	for (i = 0; i < numArgPairs; i++) {
		Symbol propertyName = argPairs[i << 1].unboxed.symbol;
		SmileObject value = SmileArg_Box(argPairs[(i << 1) + 1]);
		Int32Dict_SetValue((Int32Dict)&userObject->dict, (Int32)propertyName, value);
	}

	return userObject;
}

Bool SmileUserObject_CompareEqual(SmileUserObject self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	UNUSED(selfData);
	UNUSED(otherData);

	return ((SmileObject)self == other);
}

Bool SmileUserObject_DeepEqual(SmileUserObject self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	SmileUserObject otherUserObject;
	Symbol *keys;
	Symbol key;
	Int i, numKeys, numOtherKeys;
	SmileObject value, otherValue;

	UNUSED(selfData);
	UNUSED(otherData);

	if ((SmileObject)self == other) return True;
	if (SMILE_KIND(other) != SMILE_KIND_USEROBJECT) return False;
	otherUserObject = (SmileUserObject)other;

	numKeys = Int32Dict_Count((Int32Dict)&self->dict);
	numOtherKeys = Int32Dict_Count((Int32Dict)&otherUserObject->dict);
	if (numKeys != numOtherKeys) return False;

	keys = (Symbol *)Int32Dict_GetKeys((Int32Dict)&self->dict);

	for (i = 0; i < numKeys; i++) {
		key = keys[i];
		if (!Int32Dict_TryGetValue((Int32Dict)&otherUserObject->dict, key, &otherValue))
			return False;
		value = (SmileObject)Int32Dict_GetValue((Int32Dict)&self->dict, key);
		
		if (PointerSet_Add(visitedPointers, value)) {
			if (!SMILE_VCALL4(value, deepEqual, (SmileUnboxedData){ 0 }, otherValue, (SmileUnboxedData){ 0 }, visitedPointers))
				return False;
		}
	}

	return True;
}

UInt32 SmileUserObject_Hash(SmileUserObject self)
{
	return ((PtrInt)self & 0xFFFFFFFF) ^ Smile_HashOracle;
}

void SmileUserObject_SetSecurityKey(SmileUserObject self, SmileObject newSecurityKey, SmileObject oldSecurityKey)
{
	Bool isValidSecurityKey = self->securityKey->vtable->compareEqual(self->securityKey, (SmileUnboxedData){ 0 }, oldSecurityKey, (SmileUnboxedData){ 0 });
	if (!isValidSecurityKey)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, String_InvalidSecurityKey);

	self->securityKey = newSecurityKey;
}

void SmileUserObject_SetSecurity(SmileUserObject self, Int security, SmileObject securityKey)
{
	Bool isValidSecurityKey = self->securityKey->vtable->compareEqual(self->securityKey, (SmileUnboxedData){ 0 }, securityKey, (SmileUnboxedData){ 0 });
	if (!isValidSecurityKey)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, String_InvalidSecurityKey);

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
		return self->base->vtable->getProperty(self->base, propertyName);
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
	if (SmileObject_IsNull(value)) {
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
	if (SmileObject_IsNull(value)) {
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
	SmileList head, tail;
	Symbol *keys;
	Int i, numKeys;

	keys = (Symbol *)Int32Dict_GetKeys((Int32Dict)&self->dict);
	numKeys = Int32Dict_Count((Int32Dict)&self->dict);

	LIST_INIT(head, tail);
	for (i = 0; i < numKeys; i++) {
		LIST_APPEND(head, tail, SmileSymbol_Create(keys[i]));
	}

	return head;
}

Bool SmileUserObject_ToBool(SmileUserObject self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return True;
}

Int32 SmileUserObject_ToInteger32(SmileUserObject self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return 0;
}

Float64 SmileUserObject_ToFloat64(SmileUserObject self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return 0.0;
}

Real64 SmileUserObject_ToReal64(SmileUserObject self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return Real64_Zero;
}

String SmileUserObject_ToString(SmileUserObject self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return String_Format("user object");
}

void SmileUserObject_Call(SmileUserObject self, Int argc, Int extra)
{
	SmileObject fn;
	if (Int32Dict_TryGetValue((Int32Dict)&self->dict, (Int32)Smile_KnownSymbols._fn, &fn)
		&& SMILE_KIND(fn) == SMILE_KIND_FUNCTION) {
		// This has a 'fn' property that is a function.  Invoke that instead, with the same args.
		SMILE_VCALL2(fn, call, argc, extra);
	}
	else {
		// This has no 'fn' property, so go ask the base object to do the call instead, since this isn't itself callable.
		SMILE_VCALL2(self->base, call, argc, extra);
	}
}

void SmileUserObject_SetC(SmileUserObject self, const char *name, SmileObject value)
{
	Symbol symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, name);
	Int32Dict_SetValue((Int32Dict)&self->dict, symbol, value);
}

void SmileUserObject_SetupFunction(SmileUserObject self, ExternalFunction function, void *param,
	const char *name, const char *argNames, Int argCheckFlags, Int minArgs, Int maxArgs, Int numArgsToTypeCheck, const Byte *argTypeChecks)
{
	SmileFunction smileFunction = SmileFunction_CreateExternalFunction(function, param,
		name, argNames, argCheckFlags, minArgs, maxArgs, numArgsToTypeCheck, argTypeChecks);
	Symbol symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, name);
	Int32Dict_SetValue((Int32Dict)&self->dict, symbol, (SmileObject)smileFunction);
}

void SmileUserObject_SetupSynonym(SmileUserObject self, const char *oldName, const char *newName)
{
	Symbol oldSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, oldName);
	Symbol newSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, newName);
	SmileObject oldObject = SmileUserObject_Get(self, oldSymbol);
	Int32Dict_SetValue((Int32Dict)&self->dict, newSymbol, (SmileObject)oldObject);
}

static LexerPosition SmileUserObject_GetSourceLocation(SmileUserObject self)
{
	UNUSED(self);
	return NULL;
}

SMILE_VTABLE(SmileUserObject_VTable_ReadWriteAppend, SmileUserObject)
{
	SmileUserObject_CompareEqual,
	SmileUserObject_DeepEqual,
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
	SmileUserObject_ToFloat64,
	SmileUserObject_ToReal64,
	SmileUserObject_ToString,

	SmileUserObject_Call,
	SmileUserObject_GetSourceLocation,
};

SMILE_VTABLE(SmileUserObject_VTable_ReadWrite, SmileUserObject)
{
	SmileUserObject_CompareEqual,
	SmileUserObject_DeepEqual,
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
	SmileUserObject_ToFloat64,
	SmileUserObject_ToReal64,
	SmileUserObject_ToString,

	SmileUserObject_Call,
	SmileUserObject_GetSourceLocation,
};

SMILE_VTABLE(SmileUserObject_VTable_ReadAppend, SmileUserObject)
{
	SmileUserObject_CompareEqual,
	SmileUserObject_DeepEqual,
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
	SmileUserObject_ToFloat64,
	SmileUserObject_ToReal64,
	SmileUserObject_ToString,

	SmileUserObject_Call,
	SmileUserObject_GetSourceLocation,
};

SMILE_VTABLE(SmileUserObject_VTable_ReadOnly, SmileUserObject)
{
	SmileUserObject_CompareEqual,
	SmileUserObject_DeepEqual,
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
	SmileUserObject_ToFloat64,
	SmileUserObject_ToReal64,
	SmileUserObject_ToString,

	SmileUserObject_Call,
	SmileUserObject_GetSourceLocation,
};
