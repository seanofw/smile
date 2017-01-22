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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>

extern SmileVTable SmileUserFunction_NoArgs_VTable;
extern SmileVTable SmileUserFunction_Fast1_VTable;
extern SmileVTable SmileUserFunction_Fast2_VTable;
extern SmileVTable SmileUserFunction_Fast3_VTable;
extern SmileVTable SmileUserFunction_Fast4_VTable;
extern SmileVTable SmileUserFunction_Fast5_VTable;
extern SmileVTable SmileUserFunction_Fast6_VTable;
extern SmileVTable SmileUserFunction_Fast7_VTable;
extern SmileVTable SmileUserFunction_Fast8_VTable;
extern SmileVTable SmileUserFunction_Slow_VTable;
extern SmileVTable SmileUserFunction_Optional_VTable;
extern SmileVTable SmileUserFunction_Rest_VTable;
extern SmileVTable SmileUserFunction_Checked_VTable;
extern SmileVTable SmileUserFunction_CheckedRest_VTable;

extern SmileVTable SmileExternalFunction_NoCheck_VTable;
extern SmileVTable SmileExternalFunction_MinCheck_VTable;
extern SmileVTable SmileExternalFunction_MaxCheck_VTable;
extern SmileVTable SmileExternalFunction_MinMaxCheck_VTable;
extern SmileVTable SmileExternalFunction_ExactCheck_VTable;
extern SmileVTable SmileExternalFunction_TypesCheck_VTable;
extern SmileVTable SmileExternalFunction_MinTypesCheck_VTable;
extern SmileVTable SmileExternalFunction_MaxTypesCheck_VTable;
extern SmileVTable SmileExternalFunction_MinMaxTypesCheck_VTable;
extern SmileVTable SmileExternalFunction_ExactTypesCheck_VTable;

extern Bool SmileUserFunction_Call(SmileFunction self, Int argc);

SMILE_EASY_OBJECT_NO_SECURITY(SmileFunction);

UserFunctionInfo UserFunctionInfo_Create(SmileList args, SmileObject body)
{
	UserFunctionInfo userFunctionInfo;

	userFunctionInfo = GC_MALLOC_STRUCT(struct UserFunctionInfoStruct);
	if (userFunctionInfo == NULL)
		Smile_Abort_OutOfMemory();

	MemZero(userFunctionInfo, sizeof(struct UserFunctionInfoStruct));

	userFunctionInfo->argList = args;
	userFunctionInfo->body = body;

	

	return userFunctionInfo;
}

Inline SmileVTable GetUserFunctionVTableByFlags(Int flags, Int numArgs)
{
	switch (flags) {
		case 0:
			switch (numArgs) {
				case 0:	return SmileUserFunction_NoArgs_VTable;
				case 1:	return SmileUserFunction_Fast1_VTable;
				case 2:	return SmileUserFunction_Fast2_VTable;
				case 3:	return SmileUserFunction_Fast3_VTable;
				case 4:	return SmileUserFunction_Fast4_VTable;
				case 5:	return SmileUserFunction_Fast5_VTable;
				case 6:	return SmileUserFunction_Fast6_VTable;
				case 7:	return SmileUserFunction_Fast7_VTable;
				case 8:	return SmileUserFunction_Fast8_VTable;
				default:	return SmileUserFunction_Slow_VTable;
			}

		case USER_ARG_OPTIONAL:
			return SmileUserFunction_Optional_VTable;
		case USER_ARG_REST:
		case USER_ARG_REST | USER_ARG_OPTIONAL:
			return SmileUserFunction_Rest_VTable;
		case USER_ARG_TYPECHECK:
		case USER_ARG_TYPECHECK | USER_ARG_OPTIONAL:
			return SmileUserFunction_Checked_VTable;
		case USER_ARG_REST | USER_ARG_TYPECHECK:
		case USER_ARG_REST | USER_ARG_TYPECHECK | USER_ARG_OPTIONAL:
			return SmileUserFunction_CheckedRest_VTable;
	}

	return NULL;	// Shouldn't be able to get here.
}

SmileFunction SmileFunction_CreateUserFunction(UserFunctionInfo userFunctionInfo, Closure declaringClosure)
{
	SmileFunction smileFunction;

	smileFunction = GC_MALLOC_STRUCT(struct SmileFunctionInt);
	if (smileFunction == NULL)
		Smile_Abort_OutOfMemory();

	smileFunction->kind = SMILE_KIND_FUNCTION;
	smileFunction->vtable = GetUserFunctionVTableByFlags(userFunctionInfo->flags, userFunctionInfo->numArgs);
	smileFunction->base = (SmileObject)Smile_KnownBases.Function;
	smileFunction->assignedSymbol = 0;

	smileFunction->u.u.userFunctionInfo = userFunctionInfo;
	smileFunction->u.u.declaringClosure = declaringClosure;

	return smileFunction;
}

Inline SmileVTable GetExternalFunctionVTableByFlags(Int argCheckFlags)
{
	// Choose a VTable with a function that has optimized hardwired argument checks
	// for the types of checks they want.
	switch (argCheckFlags) {
		default:
			return SmileExternalFunction_NoCheck_VTable;
		case ARG_CHECK_MIN:
			return SmileExternalFunction_MinCheck_VTable;
		case ARG_CHECK_MAX:
			return SmileExternalFunction_MaxCheck_VTable;
		case ARG_CHECK_MIN | ARG_CHECK_MAX:
			return SmileExternalFunction_MinMaxCheck_VTable;
		case ARG_CHECK_EXACT:
			return SmileExternalFunction_ExactCheck_VTable;
		case ARG_CHECK_TYPES:
			return SmileExternalFunction_TypesCheck_VTable;
		case ARG_CHECK_TYPES | ARG_CHECK_MIN:
			return SmileExternalFunction_MinTypesCheck_VTable;
		case ARG_CHECK_TYPES | ARG_CHECK_MAX:
			return SmileExternalFunction_MaxTypesCheck_VTable;
		case ARG_CHECK_TYPES | ARG_CHECK_MIN | ARG_CHECK_MAX:
			return SmileExternalFunction_MinMaxTypesCheck_VTable;
		case ARG_CHECK_TYPES | ARG_CHECK_EXACT:
			return SmileExternalFunction_ExactTypesCheck_VTable;
	}
}

static int WrapFunctionName(char *dest, const char *name)
{
	Int maxlen = 256;
	char *start = dest;

	*dest++ = '<';

	while (*name && maxlen--) {
		*dest++ = *name++;
	}

	*dest++ = '>';
	*dest = '\0';

	return dest - start;
}

SmileFunction SmileFunction_CreateExternalFunction(ExternalFunction externalFunction, void *param,
	const char *name, const char *argNames, Int argCheckFlags, Int minArgs, Int maxArgs, Int numArgsToTypeCheck, const Byte *argTypeChecks)
{
	SmileFunction smileFunction;

	// First, correct for degenerate configurations.
	if (minArgs == 0)
		argCheckFlags &= ~ARG_CHECK_MIN;
	if (minArgs == maxArgs && (argCheckFlags & (ARG_CHECK_MIN | ARG_CHECK_MAX)) == (ARG_CHECK_MIN | ARG_CHECK_MAX)) {
		argCheckFlags &= ~(ARG_CHECK_MIN | ARG_CHECK_MAX);
		argCheckFlags |= ARG_CHECK_EXACT;
	}
	if (numArgsToTypeCheck == 0)
		argCheckFlags &= ~ARG_CHECK_TYPES;

	smileFunction = GC_MALLOC_STRUCT(struct SmileFunctionInt);
	if (smileFunction == NULL)
		Smile_Abort_OutOfMemory();

	smileFunction->kind = SMILE_KIND_FUNCTION;
	smileFunction->vtable = GetExternalFunctionVTableByFlags(argCheckFlags);
	smileFunction->base = (SmileObject)Smile_KnownBases.Function;
	smileFunction->assignedSymbol = 0;

	smileFunction->u.externalFunctionInfo.argNames = String_FromC(argNames);
	smileFunction->u.externalFunctionInfo.name = String_FromC(name);
	smileFunction->u.externalFunctionInfo.externalFunction = externalFunction;
	smileFunction->u.externalFunctionInfo.param = param;
	smileFunction->u.externalFunctionInfo.argCheckFlags = (UInt16)argCheckFlags;
	smileFunction->u.externalFunctionInfo.numArgsToTypeCheck = (UInt16)numArgsToTypeCheck;
	smileFunction->u.externalFunctionInfo.minArgs = (UInt16)minArgs;
	smileFunction->u.externalFunctionInfo.maxArgs = (UInt16)maxArgs;
	smileFunction->u.externalFunctionInfo.argTypeChecks = argTypeChecks;

	return smileFunction;
}

Bool SmileUserFunction_CompareEqual(SmileFunction self, SmileObject other)
{
	return (SMILE_KIND(other) == SMILE_KIND_FUNCTION
		&& self->vtable == other->vtable
		&& self->u.u.userFunctionInfo == ((SmileFunction)other)->u.u.userFunctionInfo
		&& self->u.u.declaringClosure == ((SmileFunction)other)->u.u.declaringClosure);
}

Bool SmileExternalFunction_CompareEqual(SmileFunction self, SmileObject other)
{
	return (SMILE_KIND(other) == SMILE_KIND_FUNCTION
		&& self->vtable == other->vtable
		&& self->u.externalFunctionInfo.externalFunction == ((SmileFunction)other)->u.externalFunctionInfo.externalFunction
		&& self->u.externalFunctionInfo.param == ((SmileFunction)other)->u.externalFunctionInfo.param);
}

UInt32 SmileUserFunction_Hash(SmileFunction self)
{
	return ((PtrInt)(self->u.u.userFunctionInfo) & 0xFFFFFFFF) ^ Smile_HashOracle;
}

UInt32 SmileExternalFunction_Hash(SmileFunction self)
{
	return ((PtrInt)(self->u.externalFunctionInfo.externalFunction) ^ (PtrInt)(self->u.externalFunctionInfo.param) & 0xFFFFFFFF) ^ Smile_HashOracle;
}

SmileObject SmileUserFunction_GetProperty(SmileFunction self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.arguments)
		return (SmileObject)self->u.u.userFunctionInfo->argList;
	else if (propertyName == Smile_KnownSymbols.body)
		return self->u.u.userFunctionInfo->body;
	else {
		return self->base->vtable->getProperty(self->base, propertyName);
	}
}

STATIC_STRING(_spaceSplitter, " ");

SmileObject SmileExternalFunction_GetProperty(SmileFunction self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.arguments) {
		SmileList head, tail;
		String *pieces;
		Int numPieces = String_Split(self->u.externalFunctionInfo.argNames, _spaceSplitter, &pieces);
		Int i;
	
		LIST_INIT(head, tail);
		for (i = 0; i < numPieces; i++) {
			LIST_APPEND(head, tail, SmileString_Create(pieces[i]));
		}
	
		return (SmileObject)head;
	}
	else if (propertyName == Smile_KnownSymbols.body) {
		return (SmileObject)SmileString_Create(String_Format("<%S>", self->u.externalFunctionInfo.name));
	}
	else {
		return self->base->vtable->getProperty(self->base, propertyName);
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
	return String_Format("<%S>", self->u.externalFunctionInfo.name);
}

#define USER_FUNCTION_VTABLE(__checkKind__) \
	\
	extern Bool SmileUserFunction_##__checkKind__##_Call(SmileFunction self, Int argc); \
	\
	SMILE_VTABLE(SmileUserFunction_##__checkKind__##_VTable, SmileFunction) \
	{ \
		SmileUserFunction_CompareEqual, \
		SmileUserFunction_Hash, \
		\
		SmileFunction_SetSecurityKey, \
		SmileFunction_SetSecurity, \
		SmileFunction_GetSecurity, \
		\
		SmileUserFunction_GetProperty, \
		SmileFunction_SetProperty, \
		SmileFunction_HasProperty, \
		SmileFunction_GetPropertyNames, \
		\
		SmileFunction_ToBool, \
		SmileFunction_ToInteger32, \
		SmileFunction_ToFloat64, \
		SmileFunction_ToReal64, \
		SmileUserFunction_ToString, \
		\
		SmileUserFunction_##__checkKind__##_Call, \
	}

USER_FUNCTION_VTABLE(NoArgs);	// Function with no arguments
USER_FUNCTION_VTABLE(Fast1);	// Function of exactly 1 argument, no rest, no type checks
USER_FUNCTION_VTABLE(Fast2);	// Function of exactly 2 arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Fast3);	// Function of exactly 3 arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Fast4);	// Function of exactly 4 arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Fast5);	// Function of exactly 5 arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Fast6);	// Function of exactly 6 arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Fast7);	// Function of exactly 7 arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Fast8);	// Function of exactly 8 arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Slow);	// Function of 9+ arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Optional);	// Function with optional arguments, no rest, no type checks
USER_FUNCTION_VTABLE(Rest);	// Function with a 'rest' argument, no type checks
USER_FUNCTION_VTABLE(Checked);	// Function with at least one type-checked argument, no rest
USER_FUNCTION_VTABLE(CheckedRest);	// Function with at least one type-checked argument, and a 'rest' argument

#define EXTERNAL_FUNCTION_VTABLE(__checkKind__) \
	\
	extern Bool SmileExternalFunction_##__checkKind__##_Call(SmileFunction self, Int argc); \
	\
	SMILE_VTABLE(SmileExternalFunction_##__checkKind__##_VTable, SmileFunction) \
	{ \
		SmileExternalFunction_CompareEqual, \
		SmileExternalFunction_Hash, \
		\
		SmileFunction_SetSecurityKey, \
		SmileFunction_SetSecurity, \
		SmileFunction_GetSecurity, \
		\
		SmileExternalFunction_GetProperty, \
		SmileFunction_SetProperty, \
		SmileFunction_HasProperty, \
		SmileFunction_GetPropertyNames, \
		\
		SmileFunction_ToBool, \
		SmileFunction_ToInteger32, \
		SmileFunction_ToFloat64, \
		SmileFunction_ToReal64, \
		SmileExternalFunction_ToString, \
		\
		SmileExternalFunction_##__checkKind__##_Call, \
	}

EXTERNAL_FUNCTION_VTABLE(NoCheck);
EXTERNAL_FUNCTION_VTABLE(MinCheck);
EXTERNAL_FUNCTION_VTABLE(MaxCheck);
EXTERNAL_FUNCTION_VTABLE(MinMaxCheck);
EXTERNAL_FUNCTION_VTABLE(ExactCheck);
EXTERNAL_FUNCTION_VTABLE(TypesCheck);
EXTERNAL_FUNCTION_VTABLE(MinTypesCheck);
EXTERNAL_FUNCTION_VTABLE(MaxTypesCheck);
EXTERNAL_FUNCTION_VTABLE(MinMaxTypesCheck);
EXTERNAL_FUNCTION_VTABLE(ExactTypesCheck);
