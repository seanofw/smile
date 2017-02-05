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

#include <smile/eval/eval.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/text/smilesymbol.h>

extern Closure _closure;
extern CompiledTables _compiledTables;
extern ByteCodeSegment _segment;
extern ByteCode _byteCode;

//-------------------------------------------------------------------------------------------------
// External function checking helpers

static Int PerformTypeChecks(Int argc, SmileObject *argv, Int numTypeChecks, const Byte *typeChecks)
{
	Int i;

	if (argc > numTypeChecks) {
		Byte mask, compare;

		// If there are more args than type checks, perform the type checks that are defined...
		for (i = 0; i < numTypeChecks; i++) {
			if ((argv[i]->kind & typeChecks[i * 2]) != typeChecks[i * 2 + 1])
				return i;
		}

		mask = typeChecks[i * 2 - 2];
		compare = typeChecks[i * 2 - 1];

		// ...and then repeatedly use the last type check for the rest of the arguments.
		for (; i < argc; i++) {
			if ((argv[i]->kind & mask) != compare)
				return i;
		}

		return -1;
	}

	// If there are fewer or equal args to type checks, perform the checks as-is,
	// wherever they match positions.  We use a switch statement (computed goto) to
	// optimize away the comparison loop if possible.
	switch (argc) {

		default:
			for (i = 0; i < argc; i++) {
				if ((argv[i]->kind & typeChecks[i * 2]) != typeChecks[i * 2 + 1])
					return i;
			}
			return -1;

		case 7:
			if ((argv[6]->kind & typeChecks[12]) != typeChecks[13])
				return 6;
		case 6:
			if ((argv[5]->kind & typeChecks[10]) != typeChecks[11])
				return 5;
		case 5:
			if ((argv[4]->kind & typeChecks[8]) != typeChecks[9])
				return 4;
		case 4:
			if ((argv[3]->kind & typeChecks[6]) != typeChecks[7])
				return 3;
		case 3:
			if ((argv[2]->kind & typeChecks[4]) != typeChecks[5])
				return 2;
		case 2:
			if ((argv[1]->kind & typeChecks[2]) != typeChecks[3])
				return 1;
		case 1:
			if ((argv[0]->kind & typeChecks[0]) != typeChecks[1])
				return 0;

		case 0:
			return -1;
	}
}

static void ThrowMinCheckError(SmileFunction self, Int argc)
{
	Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' requires at least %d arguments, but was called with %d.",
		self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.minArgs, argc));
}

static void ThrowMaxCheckError(SmileFunction self, Int argc)
{
	Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' allows at most %d arguments, but was called with %d.",
		self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.maxArgs, argc));
}

static void ThrowExactCheckError(SmileFunction self, Int argc)
{
	Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'%S' requires exactly %d arguments, but was called with %d.",
		self->u.externalFunctionInfo.name, (int)self->u.externalFunctionInfo.minArgs, argc));
}

static void ThrowTypeMismatchException(SmileFunction self, Int failingArg)
{
	Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument %d to '%S' is of the wrong type.",
		failingArg + 1, self->u.externalFunctionInfo.name));
}

//-------------------------------------------------------------------------------------------------
// External function declaration macros

// Declare the local variables required for this external function invocation to work.
#define INVOKE_DECL(__closure__, __argv__) \
	SmileObject *__argv__; \
	Int failingArg; \
	SmileObject stateMachineResult; \
	\
	UNUSED(failingArg); \
	UNUSED(stateMachineResult); \
	\
	((__argv__) = ((__closure__)->stackTop - argc))

// Check the given argument count against the external function's required minimum number of arguments.
#define DO_MIN_CHECK(__self__, __argc__) \
	if ((__argc__) < (__self__)->u.externalFunctionInfo.minArgs) ThrowMinCheckError((__self__), (__argc__))

// Check the given argument count against the external function's allowed maximum number of arguments.
#define DO_MAX_CHECK(__self__, __argc__) \
	if ((__argc__) > (__self__)->u.externalFunctionInfo.maxArgs) ThrowMaxCheckError((__self__), (__argc__))

// Check the given argument count against the external function's exact required number of arguments.
#define DO_EXACT_CHECK(__self__, __argc__) \
	if ((__argc__) != (__self__)->u.externalFunctionInfo.minArgs) ThrowExactCheckError((__self__), (__argc__))

// Check the given arguments against this external function's required argument types.
#define DO_TYPE_CHECK(__self__, __argc__, __argv__) \
	if ((failingArg = PerformTypeChecks((__argc__), (__argv__), (__self__)->u.externalFunctionInfo.numArgsToTypeCheck, (__self__)->u.externalFunctionInfo.argTypeChecks)) >= 0) \
		ThrowTypeMismatchException((__self__), failingArg)

// Invoke the function itself, passing the stack from the closure, and pushing the result back on the stack.
#define INVOKE(__self__, __argc__, __argv__, __closure__) \
	( ((__closure__)->stackTop = (__argv__)), \
	  (*(__closure__)->stackTop++ = (__self__)->u.externalFunctionInfo.externalFunction((__argc__), (__argv__), (__self__)->u.externalFunctionInfo.param)) )

// Invoke the function as a state machine initiator, pushing onto the stack only if the state machine returns non-NULL.
#define INVOKE_STATE_MACHINE(__self__, __argc__, __argv__, __closure__) \
	(__closure__)->stackTop = (__argv__); \
	stateMachineResult = (__self__)->u.externalFunctionInfo.externalFunction((__argc__), (__argv__), (__self__)->u.externalFunctionInfo.param); \
	if (stateMachineResult != NULL) { \
		/* Didn't start the state machine, and instead produced a result directly. */ \
		*(__closure__)->stackTop++ = stateMachineResult; \
	}

//-------------------------------------------------------------------------------------------------
// External functions (normal kind)

void SmileExternalFunction_NoCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_MinCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MIN_CHECK(self, argc);
	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_MaxCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MAX_CHECK(self, argc);
	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_MinMaxCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MIN_CHECK(self, argc);
	DO_MAX_CHECK(self, argc);

	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_ExactCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_EXACT_CHECK(self, argc);
	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_TypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_TYPE_CHECK(self, argc, argv);
	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_MinTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MIN_CHECK(self, argc);
	DO_TYPE_CHECK(self, argc, argv);
	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_MaxTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MAX_CHECK(self, argc);
	DO_TYPE_CHECK(self, argc, argv);
	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_MinMaxTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MIN_CHECK(self, argc);
	DO_MAX_CHECK(self, argc);
	DO_TYPE_CHECK(self, argc, argv);
	INVOKE(self, argc, argv, _closure);
}

void SmileExternalFunction_ExactTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_EXACT_CHECK(self, argc);
	DO_TYPE_CHECK(self, argc, argv);
	INVOKE(self, argc, argv, _closure);
}

//-------------------------------------------------------------------------------------------------
// State-machine external functions

void SmileExternalFunction_StateMachineNoCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineMinCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MIN_CHECK(self, argc);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineMaxCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MAX_CHECK(self, argc);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineMinMaxCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MIN_CHECK(self, argc);
	DO_MAX_CHECK(self, argc);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineExactCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_EXACT_CHECK(self, argc);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_TYPE_CHECK(self, argc, argv);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineMinTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MIN_CHECK(self, argc);
	DO_TYPE_CHECK(self, argc, argv);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineMaxTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MAX_CHECK(self, argc);
	DO_TYPE_CHECK(self, argc, argv);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineMinMaxTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_MIN_CHECK(self, argc);
	DO_MAX_CHECK(self, argc);
	DO_TYPE_CHECK(self, argc, argv);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

void SmileExternalFunction_StateMachineExactTypesCheck_Call(SmileFunction self, Int argc)
{
	INVOKE_DECL(_closure, argv);

	DO_EXACT_CHECK(self, argc);
	DO_TYPE_CHECK(self, argc, argv);
	INVOKE_STATE_MACHINE(self, argc, argv, _closure);
}

//-------------------------------------------------------------------------------------------------
// State machine construction.

static struct ByteCodeStruct _stateMachineByteCode[] = {
	{ Op_StateMachStart },
	{ Op_StateMachBody },
};

static struct ByteCodeSegmentStruct _stateMachineSegment = {
	_stateMachineByteCode, 2, 2,
};

ClosureStateMachine Eval_BeginStateMachine(StateMachine stateMachineStart, StateMachine stateMachineBody)
{
	ClosureStateMachine closureStateMachine;

	// Create a new state-machine closure.
	closureStateMachine = Closure_CreateStateMachine(stateMachineStart, stateMachineBody,
		_closure, _segment, _byteCode - _segment->byteCodes);

	// We now have the state machine, so set up the globals for running inside it.
	// We'll loop over the one instruction inside it forever.
	_closure = (Closure)closureStateMachine;
	_segment = &_stateMachineSegment;
	_byteCode = &_segment->byteCodes[0];

	return closureStateMachine;
}
