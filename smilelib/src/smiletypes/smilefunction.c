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

extern SmileVTable SmileExternalFunction_Raw_VTable;

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

extern SmileVTable SmileExternalFunction_StateMachineNoCheck_VTable;
extern SmileVTable SmileExternalFunction_StateMachineMinCheck_VTable;
extern SmileVTable SmileExternalFunction_StateMachineMaxCheck_VTable;
extern SmileVTable SmileExternalFunction_StateMachineMinMaxCheck_VTable;
extern SmileVTable SmileExternalFunction_StateMachineExactCheck_VTable;

extern SmileVTable SmileExternalFunction_StateMachineTypesCheck_VTable;
extern SmileVTable SmileExternalFunction_StateMachineMinTypesCheck_VTable;
extern SmileVTable SmileExternalFunction_StateMachineMaxTypesCheck_VTable;
extern SmileVTable SmileExternalFunction_StateMachineMinMaxTypesCheck_VTable;
extern SmileVTable SmileExternalFunction_StateMachineExactTypesCheck_VTable;

SMILE_EASY_OBJECT_READONLY_SECURITY(SmileFunction);

static Bool UserFunctionArg_Init(UserFunctionArg arg, SmileObject obj, String *errorMessage)
{
	SmileList argList;
	SmileArg nullArg;

	nullArg.obj = NullObject;
	arg->defaultValue = nullArg;
	arg->flags = USER_ARG_NORMAL;
	arg->typeName = 0;

	if (SMILE_KIND(obj) == SMILE_KIND_SYMBOL) {
		// Common shorthand:  The argument is just a symbol.
		arg->name = ((SmileSymbol)obj)->symbol;
		*errorMessage = NULL;
		return True;
	}
	
	if (SMILE_KIND(obj) != SMILE_KIND_LIST) {
		// The argument must be a list, if it's not a symbol.
		*errorMessage = String_Format("Invalid argument form.");
		return False;
	}

	// The first element of the list must be a symbol:  The argument's name.
	argList = (SmileList)obj;
	if (SMILE_KIND(argList->a) != SMILE_KIND_SYMBOL) {
		*errorMessage = String_Format("Argument sub-list must start with a symbol.");
		return False;
	}
	arg->name = ((SmileSymbol)argList->a)->symbol;

	// Walk the list of optional modifiers on this argument.
	for (argList = LIST_REST(argList); SMILE_KIND(argList) != SMILE_KIND_NULL; ) {
		Symbol modifier;
	
		if (SMILE_KIND(argList->a) != SMILE_KIND_SYMBOL) {
			*errorMessage = String_Format("Argument modifier for '%S' must start with 'type', 'default', or 'rest'.",
					SymbolTable_GetName(Smile_SymbolTable, arg->name));
			return False;
		}
	
		modifier = ((SmileSymbol)argList->a)->symbol;
		if (modifier == Smile_KnownSymbols.type) {
			// Handle the [type name] modifier.
			argList = LIST_REST(argList);
			if (SMILE_KIND(argList) != SMILE_KIND_LIST || SMILE_KIND(argList->a) != SMILE_KIND_SYMBOL) {
				*errorMessage = String_Format("Argument 'type' modifier for '%S' must be followed by a name of a type.",
					SymbolTable_GetName(Smile_SymbolTable, arg->name));
				return False;
			}
			arg->typeName = ((SmileSymbol)argList->a)->symbol;
			argList = LIST_REST(argList);
		}
		else if (modifier == Smile_KnownSymbols.default_) {
			// Handle the [default value] modifier.
			argList = LIST_REST(argList);
			if (SMILE_KIND(argList) != SMILE_KIND_LIST) {
				*errorMessage = String_Format("Argument 'default' modifier for '%S' must be followed by a default value.",
					SymbolTable_GetName(Smile_SymbolTable, arg->name));
				return False;
			}
			arg->defaultValue = SmileArg_Unbox(argList->a);
			argList = LIST_REST(argList);
		}
		else if (modifier == Smile_KnownSymbols.rest) {
			// Handle the [rest] modifier.
			arg->flags |= USER_ARG_REST;
			argList = LIST_REST(argList);
		}
		else {
			*errorMessage = String_Format("Argument modifier for '%S' must be 'type', 'default', or 'rest', not '%S'.",
				SymbolTable_GetName(Smile_SymbolTable, arg->name), SymbolTable_GetName(Smile_SymbolTable, modifier));
			return False;
		}
	}

	*errorMessage = NULL;
	return True;
}

static Bool UserFunctionInfo_ApplyArgs(UserFunctionInfo userFunctionInfo, SmileList argList, String *errorMessage)
{
	UserFunctionArg arg, argArray;
	Int numArgs, minArgs, maxArgs, argIndex;
	Int16 flags = 0;
	Bool haveRest = False, haveOptional = False;

	numArgs = SmileList_Length(argList);
	minArgs = 0;
	maxArgs = 0;

	if (numArgs <= 0) {
		argArray = NULL;
	}
	else {
		argArray = GC_MALLOC_STRUCT_ARRAY(struct UserFunctionArgStruct, numArgs);

		for (argIndex = 0; SMILE_KIND(argList) != SMILE_KIND_NULL; argList = LIST_REST(argList), argIndex++) {

			arg = &argArray[argIndex];

			if (!UserFunctionArg_Init(arg, argList->a, errorMessage))
				return False;
		
			flags |= (Int16)arg->flags;
		
			if (haveRest) {
				*errorMessage = String_Format("Function argument '%S' cannot appear after the 'rest...' argument.",
					SymbolTable_GetName(Smile_SymbolTable, arg->name));
				return False;
			}

			if (arg->flags & USER_ARG_OPTIONAL) {
				haveOptional = True;
				maxArgs++;
			}
			else if (arg->flags & USER_ARG_REST) {
				haveRest = True;
				maxArgs = Int16Max;
			}
			else {
				if (haveOptional) {
					*errorMessage = String_Format("Required function argument '%S' cannot appear after an optional argument.",
						SymbolTable_GetName(Smile_SymbolTable, arg->name));
					return False;
				}
				maxArgs = ++minArgs;
			}
		}
	}

	userFunctionInfo->flags = flags;
	userFunctionInfo->args = argArray;
	userFunctionInfo->numArgs = (Int16)numArgs;
	userFunctionInfo->minArgs = (Int16)minArgs;
	userFunctionInfo->maxArgs = (Int16)maxArgs;

	*errorMessage = NULL;
	return True;
}

UserFunctionInfo UserFunctionInfo_Create(UserFunctionInfo parent, LexerPosition position, SmileList args, SmileObject body, String *errorMessage)
{
	UserFunctionInfo userFunctionInfo;

	userFunctionInfo = GC_MALLOC_STRUCT(struct UserFunctionInfoStruct);
	if (userFunctionInfo == NULL)
		Smile_Abort_OutOfMemory();

	MemZero(userFunctionInfo, sizeof(struct UserFunctionInfoStruct));

	userFunctionInfo->parent = parent;
	userFunctionInfo->position = position;
	userFunctionInfo->argList = args;
	userFunctionInfo->body = body;

	return UserFunctionInfo_ApplyArgs(userFunctionInfo, args, errorMessage) ? userFunctionInfo : NULL;
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
	smileFunction->base = (SmileObject)Smile_KnownBases.Fn;

	smileFunction->u.u.userFunctionInfo = userFunctionInfo;
	smileFunction->u.u.declaringClosure = declaringClosure;

	return smileFunction;
}

Inline SmileVTable GetExternalFunctionVTableByFlags(Int argCheckFlags)
{
	if (argCheckFlags & ARG_MODE_RAW) return SmileExternalFunction_Raw_VTable;

	// Choose a VTable with a function that has optimized hardwired argument checks
	// for the types of checks they want.
	switch (argCheckFlags) {

		case 0:
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

		case ARG_STATE_MACHINE:
			return SmileExternalFunction_StateMachineNoCheck_VTable;
		case ARG_STATE_MACHINE | ARG_CHECK_MIN:
			return SmileExternalFunction_StateMachineMinCheck_VTable;
		case ARG_STATE_MACHINE | ARG_CHECK_MAX:
			return SmileExternalFunction_StateMachineMaxCheck_VTable;
		case ARG_STATE_MACHINE | ARG_CHECK_MIN | ARG_CHECK_MAX:
			return SmileExternalFunction_StateMachineMinMaxCheck_VTable;
		case ARG_STATE_MACHINE | ARG_CHECK_EXACT:
			return SmileExternalFunction_StateMachineExactCheck_VTable;

		case ARG_STATE_MACHINE | ARG_CHECK_TYPES:
			return SmileExternalFunction_StateMachineTypesCheck_VTable;
		case ARG_STATE_MACHINE | ARG_CHECK_TYPES | ARG_CHECK_MIN:
			return SmileExternalFunction_StateMachineMinTypesCheck_VTable;
		case ARG_STATE_MACHINE | ARG_CHECK_TYPES | ARG_CHECK_MAX:
			return SmileExternalFunction_StateMachineMaxTypesCheck_VTable;
		case ARG_STATE_MACHINE | ARG_CHECK_TYPES | ARG_CHECK_MIN | ARG_CHECK_MAX:
			return SmileExternalFunction_StateMachineMinMaxTypesCheck_VTable;
		case ARG_STATE_MACHINE | ARG_CHECK_TYPES | ARG_CHECK_EXACT:
			return SmileExternalFunction_StateMachineExactTypesCheck_VTable;

		default:
			Smile_Abort_FatalError("Unsupported external function argument configuration.");
			return NULL;
	}
}

static Int WrapFunctionName(char *dest, const char *name)
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

	smileFunction->kind = SMILE_KIND_FUNCTION | SMILE_FLAG_EXTERNAL_FUNCTION;
	smileFunction->vtable = GetExternalFunctionVTableByFlags(argCheckFlags);
	smileFunction->base = (SmileObject)Smile_KnownBases.Fn;

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

Bool SmileUserFunction_CompareEqual(SmileFunction self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	UNUSED(selfData);
	UNUSED(otherData);

	return (SmileObject)self == other;
}

Bool SmileExternalFunction_CompareEqual(SmileFunction self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData)
{
	UNUSED(selfData);
	UNUSED(otherData);

	return (SmileObject)self == other;
}

Bool SmileUserFunction_DeepEqual(SmileFunction self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	UNUSED(selfData);
	UNUSED(otherData);
	UNUSED(visitedPointers);

	return (SMILE_KIND(other) == SMILE_KIND_FUNCTION
		&& self->vtable == other->vtable
		&& self->u.u.userFunctionInfo == ((SmileFunction)other)->u.u.userFunctionInfo
		&& self->u.u.declaringClosure == ((SmileFunction)other)->u.u.declaringClosure);
}

Bool SmileExternalFunction_DeepEqual(SmileFunction self, SmileUnboxedData selfData, SmileObject other, SmileUnboxedData otherData, PointerSet visitedPointers)
{
	UNUSED(selfData);
	UNUSED(otherData);
	UNUSED(visitedPointers);

	return (SMILE_KIND(other) == SMILE_KIND_FUNCTION
		&& self->vtable == other->vtable
		&& self->u.externalFunctionInfo.externalFunction == ((SmileFunction)other)->u.externalFunctionInfo.externalFunction
		&& self->u.externalFunctionInfo.param == ((SmileFunction)other)->u.externalFunctionInfo.param);
}

UInt32 SmileUserFunction_Hash(SmileFunction self)
{
	return (UInt32)(((PtrInt)(self->u.u.userFunctionInfo) & 0xFFFFFFFF) ^ Smile_HashOracle);
}

UInt32 SmileExternalFunction_Hash(SmileFunction self)
{
	return (UInt32)(((PtrInt)(self->u.externalFunctionInfo.externalFunction) ^ (PtrInt)(self->u.externalFunctionInfo.param) & 0xFFFFFFFF) ^ Smile_HashOracle);
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

SmileObject SmileExternalFunction_GetProperty(SmileFunction self, Symbol propertyName)
{
	if (propertyName == Smile_KnownSymbols.arguments) {
		SmileList head, tail;
		String *pieces;
		Int numPieces = String_Split(self->u.externalFunctionInfo.argNames, String_Space, &pieces);
		Int i;
	
		LIST_INIT(head, tail);
		for (i = 0; i < numPieces; i++) {
			LIST_APPEND(head, tail, pieces[i]);
		}
	
		return (SmileObject)head;
	}
	else if (propertyName == Smile_KnownSymbols.body) {
		return (SmileObject)String_Format("<%S>", self->u.externalFunctionInfo.name);
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

Bool SmileFunction_ToBool(SmileFunction self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return True;
}

Int32 SmileFunction_ToInteger32(SmileFunction self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return 0;
}

Float64 SmileFunction_ToFloat64(SmileFunction self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return 0.0;
}

Real64 SmileFunction_ToReal64(SmileFunction self, SmileUnboxedData unboxedData)
{
	UNUSED(self);
	UNUSED(unboxedData);
	return Real64_Zero;
}

String SmileUserFunction_ToString(SmileFunction self, SmileUnboxedData unboxedData)
{
	// TODO: FIXME: Should this maybe return the user's original source code?
	UNUSED(self);
	UNUSED(unboxedData);
	return String_Format("<fn>");
}

String SmileExternalFunction_ToString(SmileFunction self, SmileUnboxedData unboxedData)
{
	UNUSED(unboxedData);

	return String_Format("<%S>", self->u.externalFunctionInfo.name);
}

LexerPosition SmileUserFunction_GetSourceLocation(SmileFunction self)
{
	return self->u.u.userFunctionInfo->position;
}

LexerPosition SmileExternalFunction_GetSourceLocation(SmileFunction self)
{
	UNUSED(self);
	return NULL;
}

#define USER_FUNCTION_VTABLE(__checkKind__) \
	\
	extern void SmileUserFunction_##__checkKind__##_Call(SmileFunction self, Int argc, Int extra); \
	\
	SMILE_VTABLE(SmileUserFunction_##__checkKind__##_VTable, SmileFunction) \
	{ \
		SmileUserFunction_CompareEqual, \
		SmileUserFunction_DeepEqual, \
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
		SmileUserFunction_ToString, \
		\
		SmileUserFunction_##__checkKind__##_Call, \
		SmileUserFunction_GetSourceLocation, \
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
	extern void SmileExternalFunction_##__checkKind__##_Call(SmileFunction self, Int argc, Int extra); \
	\
	SMILE_VTABLE(SmileExternalFunction_##__checkKind__##_VTable, SmileFunction) \
	{ \
		SmileExternalFunction_CompareEqual, \
		SmileExternalFunction_DeepEqual, \
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
		SmileExternalFunction_ToString, \
		\
		SmileExternalFunction_##__checkKind__##_Call, \
		SmileExternalFunction_GetSourceLocation, \
	}

EXTERNAL_FUNCTION_VTABLE(Raw);

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

EXTERNAL_FUNCTION_VTABLE(StateMachineNoCheck);
EXTERNAL_FUNCTION_VTABLE(StateMachineMinCheck);
EXTERNAL_FUNCTION_VTABLE(StateMachineMaxCheck);
EXTERNAL_FUNCTION_VTABLE(StateMachineMinMaxCheck);
EXTERNAL_FUNCTION_VTABLE(StateMachineExactCheck);

EXTERNAL_FUNCTION_VTABLE(StateMachineTypesCheck);
EXTERNAL_FUNCTION_VTABLE(StateMachineMinTypesCheck);
EXTERNAL_FUNCTION_VTABLE(StateMachineMaxTypesCheck);
EXTERNAL_FUNCTION_VTABLE(StateMachineMinMaxTypesCheck);
EXTERNAL_FUNCTION_VTABLE(StateMachineExactTypesCheck);
