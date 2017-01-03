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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>

SMILE_EASY_OBJECT_NO_SECURITY(SmileFunction);

SmileFunction SmileFunction_CreateUserFunction(SmileList args, SmileObject body, struct ClosureInfoStruct *closureInfo)
{
	UNUSED(args);
	UNUSED(body);
	UNUSED(closureInfo);

	return NULL;
}

SmileFunction SmileFunction_CreateExternalFunction(ExternalFunction externalFunction, void *param,
	const char *name, const char *argNames, Int argCheckFlags, Int minArgs, Int maxArgs, const Byte *argTypeChecks)
{
	UNUSED(externalFunction);
	UNUSED(param);
	UNUSED(name);
	UNUSED(argNames);
	UNUSED(argCheckFlags);
	UNUSED(minArgs);
	UNUSED(maxArgs);
	UNUSED(argTypeChecks);

	return NULL;
}

Bool SmileUserFunction_CompareEqual(SmileFunction self, SmileObject other)
{
	return (SMILE_KIND(other) == SMILE_KIND_FUNCTION
		&& self->vtable == other->vtable
		&& self->u.closureInfo == ((SmileFunction)other)->u.closureInfo);
}

Bool SmileExternalFunction_CompareEqual(SmileFunction self, SmileObject other)
{
	return (SMILE_KIND(other) == SMILE_KIND_FUNCTION
		&& self->vtable == other->vtable
		&& self->u.externalFunctionInfo->externalFunction == ((SmileFunction)other)->u.externalFunctionInfo->externalFunction
		&& self->u.externalFunctionInfo->param == ((SmileFunction)other)->u.externalFunctionInfo->param);
}

UInt32 SmileUserFunction_Hash(SmileFunction self)
{
	return ((PtrInt)(self->u.closureInfo) & 0xFFFFFFFF) ^ Smile_HashOracle;
}

UInt32 SmileExternalFunction_Hash(SmileFunction self)
{
	return ((PtrInt)(self->u.externalFunctionInfo->externalFunction) ^ (PtrInt)(self->u.externalFunctionInfo->param) & 0xFFFFFFFF) ^ Smile_HashOracle;
}

SmileObject SmileUserFunction_GetProperty(SmileFunction self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.arguments)
		return (SmileObject)self->args;
	else if (propertyName == Smile_KnownSymbols.body)
		return self->body;
	else {
		return self->base->vtable->getProperty((SmileObject)self, propertyName);
	}
}

SmileObject SmileExternalFunction_GetProperty(SmileFunction self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.arguments)
		return (SmileObject)self->args;
	else if (propertyName == Smile_KnownSymbols.body)
		return (SmileObject)SmileString_Create(String_Format("<%S>", self->u.externalFunctionInfo->name));
	else {
		return self->base->vtable->getProperty((SmileObject)self, propertyName);
	}
}

void SmileFunction_SetProperty(SmileFunction self, Symbol propertyName, SmileObject value)
{
	UNUSED(self);
	UNUSED(propertyName);
	UNUSED(value);
	Smile_ThrowException(Smile_KnownSymbols.property_error,
		String_Format("Cannot set property \"%S\" on a function; functions are read-only.",
		SymbolTable_GetName(Smile_SymbolTable, propertyName)));
}

Bool SmileFunction_HasProperty(SmileFunction self, Symbol propertyName)
{
	UNUSED(self);
	return (propertyName == Smile_KnownSymbols.arguments || propertyName == Smile_KnownSymbols.body);
}

SmileList SmileFunction_GetPropertyNames(SmileFunction self)
{
	SmileList head, tail;

	UNUSED(self);

	LIST_INIT(head, tail);
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.arguments));
	LIST_APPEND(head, tail, SmileSymbol_Create(Smile_KnownSymbols.body));

	return head;
}

Bool SmileFunction_ToBool(SmileFunction self)
{
	UNUSED(self);
	return True;
}

Int32 SmileFunction_ToInteger32(SmileFunction self)
{
	UNUSED(self);
	return 0;
}

Float64 SmileFunction_ToFloat64(SmileFunction self)
{
	UNUSED(self);
	return 0.0;
}

Real64 SmileFunction_ToReal64(SmileFunction self)
{
	UNUSED(self);
	return Real64_Zero;
}

String SmileUserFunction_ToString(SmileFunction self)
{
	// TODO: FIXME: This should return the user's original source code.
	UNUSED(self);
	return String_Format("<fn>");
}

String SmileExternalFunction_ToString(SmileFunction self)
{
	return String_Format("<%S>", self->u.externalFunctionInfo->name);
}

Bool SmileUserFunction_Call(SmileFunction self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	return True;
}

Bool SmileExternalFunction_Call(SmileFunction self, Int argc)
{
	UNUSED(self);
	UNUSED(argc);

	return True;
}

SMILE_VTABLE(SmileUserFunction_VTable, SmileFunction)
{
	SmileUserFunction_CompareEqual,
	SmileUserFunction_Hash,

	SmileFunction_SetSecurityKey,
	SmileFunction_SetSecurity,
	SmileFunction_GetSecurity,

	SmileUserFunction_GetProperty,
	SmileFunction_SetProperty,
	SmileFunction_HasProperty,
	SmileFunction_GetPropertyNames,

	SmileFunction_ToBool,
	SmileFunction_ToInteger32,
	SmileFunction_ToFloat64,
	SmileFunction_ToReal64,
	SmileUserFunction_ToString,

	SmileUserFunction_Call,
};

SMILE_VTABLE(SmileExternalFunction_VTable, SmileFunction)
{
	SmileExternalFunction_CompareEqual,
	SmileExternalFunction_Hash,

	SmileFunction_SetSecurityKey,
	SmileFunction_SetSecurity,
	SmileFunction_GetSecurity,

	SmileExternalFunction_GetProperty,
	SmileFunction_SetProperty,
	SmileFunction_HasProperty,
	SmileFunction_GetPropertyNames,

	SmileFunction_ToBool,
	SmileFunction_ToInteger32,
	SmileFunction_ToFloat64,
	SmileFunction_ToReal64,
	SmileExternalFunction_ToString,

	SmileExternalFunction_Call,
};
