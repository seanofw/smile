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
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>
#include <smile/eval/bytecode.h>
#include <smile/eval/opcode.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _callChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
	0, 0,
};

static Byte _applyChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	return SmileUnboxedInteger64_From(0);
}

STATIC_STRING(_Fn, "Fn");

SMILE_EXTERNAL_FUNCTION(ToString)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_FUNCTION) {
		return SmileArg_From((SmileObject)SmileObject_Stringify(argv[0].obj));
	}

	return SmileArg_From((SmileObject)_Fn);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	return SmileUnboxedInteger64_From((PtrInt)argv[0].obj ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Danger Will Robinson:  The code in this section relies heavily on knowledge of how Eval()
// works, and directly manipulates Eval()'s internal state.  If Eval() or Closure is changed
// substantially, the call/apply functions will probably break.

extern Closure _closure;
extern ByteCodeSegment _segment;
extern ByteCode _byteCode;

/// <summary>
/// Construct a Closure and ClosureInfo that are big enough to hold a call containing the
/// given arguments.  We only need these for the duration of the invocation of the call to
/// some other function, but since we don't really know what they'll be used for, we have
/// no choice but to put them on the heap.
/// </summary>
static void SetupVirtualClosure(Int numArgs)
{
	static struct ByteCodeStruct byteCodes = {
		Op_Ret,
	};
	static struct ByteCodeSegmentStruct segment = {
		NULL, &byteCodes, 1, 1
	};

	ClosureInfo closureInfo;
	Closure closure;

	if (numArgs > Int16Max)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("'Fn.call' and 'Fn.apply' are limited to a maximum of 32767 function arguments."));

	closureInfo = ClosureInfo_Create(_closure->closureInfo->global, CLOSURE_KIND_LOCAL);
	closureInfo->tempSize = (Int16)numArgs;

	closure = Closure_CreateLocal(closureInfo, _closure->global, _closure, _segment, _byteCode - _segment->byteCodes);

	_closure = closure;
	_segment = &segment;
	_byteCode = &byteCodes;

	closure->stackTop = closure->variables;
}

/// <summary>
/// Some of the indirect callers may require more stack space than is availble.  So
/// this checks to see how much is available, and if there's not enough, it invokes
/// SetupVirtualClosure() to make a virtual function where there *is* enough stack
/// space available.  Returns nothing, but may mutate _closure (and friends) directly.
/// </summary>
Inline void PrepareStackForNestedCall(Int numArgs)
{
	// If we're lucky, there's room in this closure to stash the arguments.
	Int availableSpace = (_closure->variables + _closure->closureInfo->numVariables + _closure->closureInfo->tempSize)
		- _closure->stackTop;

	if (numArgs <= availableSpace) {
		// Lucky!  Just use the available space in the current closure to perform the call.
		// There's literally nothing to do here, since we already popped our two arguments
		// off the stack (it's clean).
	}
	else {
		// Not so lucky:  The call is bigger than we have stack space to put it on.  So to make
		// this work, we construct a virtual closure that is big enough for the arguments, and
		// attach the closure to a virtual single-Ret-instruction function so that everything
		// acts like there's enough space.  This isn't ideal, but the stack memory has to come
		// from *somewhere*.
	}
}

SMILE_EXTERNAL_RAW_FUNCTION(Call)
{
	SmileObject target;
	Int i;

	// Remove the target, and then move all the arguments down one position in the stack.
	target = argv[0].obj;
	if (SMILE_KIND(target) != SMILE_KIND_FUNCTION)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("First argument to 'Fn.call' must be a function."));
	for (i = 0; i < argc - 1; i++) {
		argv[i] = argv[i + 1];
	}
	_closure->stackTop--;

	// The stack is now set up as though they'd called the target directly, so we can
	// now just forward the call on as though they actually *had* called the target directly.

	// Call the target for real.  The target will leave its result at argv[0], and the compiler
	// should just turn this into a computed jump, since we don't have a return value and neither
	// does the 'call' virtual method.
	SMILE_VCALL2(target, call, argc - 1, 0);
}

SMILE_EXTERNAL_RAW_FUNCTION(CallSoft)
{
	SmileFunction target;
	Int i;
	Int minArgs, maxArgs, namedArgs;
	Int numArgs;
	Bool hasMax;

	// Get the target.
	target = (SmileFunction)argv[0].obj;
	if (SMILE_KIND(target) != SMILE_KIND_FUNCTION)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("First argument to 'Fn.call~' must be a function."));

	// Pretend to pop everything from the stack.
	_closure->stackTop = argv;

	// Find out how many arguments the target really wants to be provided.
	if (target->kind & SMILE_FLAG_EXTERNAL_FUNCTION) {
		minArgs = target->u.externalFunctionInfo.minArgs;
		maxArgs = target->u.externalFunctionInfo.maxArgs;
		namedArgs = (target->u.externalFunctionInfo.argCheckFlags & (ARG_CHECK_MAX | ARG_CHECK_EXACT)) ? maxArgs : minArgs;
		hasMax = !!(target->u.externalFunctionInfo.argCheckFlags & (ARG_CHECK_MAX | ARG_CHECK_EXACT));
	}
	else {
		minArgs = target->u.u.userFunctionInfo->minArgs;
		maxArgs = target->u.u.userFunctionInfo->maxArgs;
		namedArgs = target->u.u.userFunctionInfo->numArgs;
		if (namedArgs > 0 && (target->u.u.userFunctionInfo->args[namedArgs - 1].flags & USER_ARG_REST)) {
			namedArgs--;
			hasMax = False;
		}
		else hasMax = True;
	}

	// Argc-1 is the actual number of arguments we have available to pass to the target
	// function, so correct argc to be the *actual* number of arguments we have for the target.
	argc--;

	// If this function can't handle more than a certain number of arguments and more than that
	// were given, then simply chop off the arguments at the allowed limit.
	if (hasMax && argc > maxArgs)
		argc = maxArgs;

	if (argc <= namedArgs) {
		// If we have fewer than the target's minimum, then pad out the collection with null
		// or default values.
		PrepareStackForNestedCall(namedArgs);

		for (i = 0; i < argc; i++) {
			Closure_Push(_closure, argv[i + 1]);
		}
		if (i < namedArgs) {
			// Pad out the arguments with whatever's available.
			if (target->kind & SMILE_FLAG_EXTERNAL_FUNCTION) {
				for (; i < namedArgs; i++) {
					Closure_PushBoxed(_closure, NullObject);
				}
			}
			else {
				for (; i < namedArgs; i++) {
					Closure_Push(_closure, target->u.u.userFunctionInfo->args[i].defaultValue);
				}
			}
		}
		numArgs = namedArgs;
	}
	else {
		// We're past the limit for default arguments, so just copy everything.
		PrepareStackForNestedCall(argc);
		for (i = 0; i < argc; i++) {
			Closure_Push(_closure, argv[i + 1]);
		}
		numArgs = argc;
	}

	// The stack is now set up as though they'd called the target directly, so we can
	// now just forward the call on as though they actually *had* called the target directly.

	// Call the target for real.  The target will leave its result at argv[0], and the compiler
	// should just turn this into a computed jump, since we don't have a return value and neither
	// does the 'call' virtual method.
	SMILE_VCALL2(target, call, numArgs, 0);
}

SMILE_EXTERNAL_RAW_FUNCTION(Apply)
{
	SmileObject target;
	SmileList args;
	Int childArgc;
	STATIC_STRING(malformedApplyList, "Second argument to Fn.apply, the list of arguments, is not well-formed; it is circular or terminates incorrectly.");

	// Remove both the target and arguments from this closure's stack.
	target = argv[0].obj;
	if (SMILE_KIND(target) != SMILE_KIND_FUNCTION)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("First argument to 'Fn.apply' must be a function."));
	args = (SmileList)argv[1].obj;
	if (SMILE_KIND(args) != SMILE_KIND_LIST && SMILE_KIND(args) != SMILE_KIND_NULL)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Second argument to 'Fn.apply' must be a list."));
	_closure->stackTop -= 2;

	// Figure out how many arguments they want to pass to the target function.
	childArgc = SmileList_Length(args);
	if (childArgc < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, malformedApplyList);

	// If more stack space is needed for the arguments, go find some.  This is much
	// more complex than it may seem.
	PrepareStackForNestedCall(childArgc);

	// Push all the list members onto the closure's stack, unboxing them as we go.
	for (; SMILE_KIND(args) != SMILE_KIND_NULL; args = (SmileList)(args->d)) {
		Closure_UnboxAndPush(_closure, args->a);
	}

	// Call the target for real.  The target will leave its result at argv[0], and the compiler
	// should just turn this into a computed jump, since we don't have a return value and neither
	// does the 'call' virtual method.  If this was done on a virtual closure, then the virtual
	// function attached to it will proxy the result back down to the real caller.
	SMILE_VCALL2(target, call, childArgc, 0);
}

SMILE_EXTERNAL_RAW_FUNCTION(ApplySoft)
{
	SmileFunction target;
	Int i;
	Int minArgs, maxArgs, namedArgs;
	Int numArgs;
	Bool hasMax;
	SmileList args;
	Int childArgc;
	STATIC_STRING(malformedApplyList, "Second argument to Fn.apply~, the list of arguments, is not well-formed; it is circular or terminates incorrectly.");

	// Remove both the target and arguments from this closure's stack.
	target = (SmileFunction)argv[0].obj;
	if (SMILE_KIND(target) != SMILE_KIND_FUNCTION)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("First argument to 'Fn.apply~' must be a function."));
	args = (SmileList)argv[1].obj;
	if (SMILE_KIND(args) != SMILE_KIND_LIST && SMILE_KIND(args) != SMILE_KIND_NULL)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Second argument to 'Fn.apply~' must be a list."));
	_closure->stackTop -= 2;

	// Figure out how many arguments they want to pass to the target function.
	childArgc = SmileList_Length(args);
	if (childArgc < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, malformedApplyList);

	// Find out how many arguments the target really wants to be provided.
	if (target->kind & SMILE_FLAG_EXTERNAL_FUNCTION) {
		minArgs = target->u.externalFunctionInfo.minArgs;
		maxArgs = target->u.externalFunctionInfo.maxArgs;
		namedArgs = (target->u.externalFunctionInfo.argCheckFlags & (ARG_CHECK_MAX | ARG_CHECK_EXACT)) ? maxArgs : minArgs;
		hasMax = !!(target->u.externalFunctionInfo.argCheckFlags & (ARG_CHECK_MAX | ARG_CHECK_EXACT));
	}
	else {
		minArgs = target->u.u.userFunctionInfo->minArgs;
		maxArgs = target->u.u.userFunctionInfo->maxArgs;
		namedArgs = target->u.u.userFunctionInfo->numArgs;
		if (namedArgs > 0 && (target->u.u.userFunctionInfo->args[namedArgs - 1].flags & USER_ARG_REST)) {
			namedArgs--;
			hasMax = False;
		}
		else hasMax = True;
	}

	// If this function can't handle more than a certain number of arguments and more than that
	// were given, then simply chop off the arguments at the allowed limit.
	if (hasMax && childArgc > maxArgs)
		childArgc = maxArgs;

	if (childArgc <= namedArgs) {
		// If we have fewer than the target's minimum, then pad out the collection with null
		// or default values.
		PrepareStackForNestedCall(namedArgs);

		for (i = 0; i < childArgc; i++, args = (SmileList)(args->d)) {
			Closure_UnboxAndPush(_closure, args->a);
		}
		if (i < namedArgs) {
			// Pad out the arguments with whatever's available.
			if (target->kind & SMILE_FLAG_EXTERNAL_FUNCTION) {
				for (; i < namedArgs; i++) {
					Closure_PushBoxed(_closure, NullObject);
				}
			}
			else {
				for (; i < namedArgs; i++) {
					Closure_Push(_closure, target->u.u.userFunctionInfo->args[i].defaultValue);
				}
			}
		}
		numArgs = namedArgs;
	}
	else {
		// We're past the limit for default arguments, so just copy everything.
		PrepareStackForNestedCall(childArgc);
		for (i = 0; i < childArgc; i++, args = (SmileList)(args->d)) {
			Closure_UnboxAndPush(_closure, args->a);
		}
		numArgs = childArgc;
	}

	// The stack is now set up as though they'd called the target directly, so we can
	// now just forward the call on as though they actually *had* called the target directly.

	// Call the target for real.  The target will leave its result at argv[0], and the compiler
	// should just turn this into a computed jump, since we don't have a return value and neither
	// does the 'call' virtual method.
	SMILE_VCALL2(target, call, numArgs, 0);
}

//-------------------------------------------------------------------------------------------------

void SmileFunction_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("call", (ExternalFunction)Call, NULL, "fn args...", ARG_MODE_RAW, 0, 0, 0, NULL);
	SetupFunction("apply", (ExternalFunction)Apply, NULL, "fn arg-list", ARG_MODE_RAW, 0, 0, 0, NULL);
	SetupFunction("call~", (ExternalFunction)CallSoft, NULL, "fn args...", ARG_MODE_RAW, 0, 0, 0, NULL);
	SetupFunction("apply~", (ExternalFunction)ApplySoft, NULL, "fn arg-list", ARG_MODE_RAW, 0, 0, 0, NULL);
}
