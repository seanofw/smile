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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/eval/eval.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _listChecks[] = {
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
};

static Byte _joinChecks[] = {
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
};

static Byte _eachChecks[] = {
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

static Byte _indexOfChecks[] = {
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
	0, 0,
};

static Byte _combineChecks[] = {
	0, 0,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST_BIT, SMILE_KIND_NULL,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[0].obj) != SMILE_KIND_NULL);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_LIST)
		return SmileUnboxedInteger64_From(SmileList_Length((SmileList)argv[0].obj));

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(list, "List");
	STATIC_STRING(null, "null");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_LIST) {
		return SmileArg_From((SmileObject)SmileObject_Stringify(argv[0].obj));
	}
	else if (SMILE_KIND(argv[0].obj) == SMILE_KIND_NULL) {
		return SmileArg_From((SmileObject)null);
	}

	return SmileArg_From((SmileObject)list);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_NULL)
		return SmileUnboxedInteger64_From(0);

	return SmileUnboxedInteger64_From((PtrInt)argv[0].obj ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Construction functions.

SMILE_EXTERNAL_FUNCTION(Of)
{
	SmileList head = NullList, tail = NullList;
	SmileUserObject base = (SmileUserObject)param;
	Int i;

	i = 0;
	if (argv[i].obj == (SmileObject)base)
		i++;

	for (; i < argc; i++) {
		LIST_APPEND(head, tail, SmileArg_Box(argv[i]));
	}

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(Cons)
{
	if (argc == 2)
		return SmileArg_From((SmileObject)SmileList_Cons(SmileArg_Box(argv[0]), SmileArg_Box(argv[1])));
	return SmileArg_From((SmileObject)SmileList_Cons(SmileArg_Box(argv[1]), SmileArg_Box(argv[2])));
}

SMILE_EXTERNAL_FUNCTION(Combine)
{
	SmileList head = NullList, tail = NullList;
	SmileList source;
	SmileUserObject base = (SmileUserObject)param;
	STATIC_STRING(wellFormedError, "Object cannot be used with List.combine because it is not a List or is not well-formed.");
	Int i;

	i = 0;
	if (argv[i].obj == (SmileObject)base)
		i++;

	for (; i < argc; i++) {
		source = (SmileList)argv[i].obj;
		if (!SmileList_IsWellFormed((SmileObject)source))
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, wellFormedError);
		for (; SMILE_KIND(source) != SMILE_KIND_NULL; source = (SmileList)source->d) {
			LIST_APPEND(head, tail, source->a);
		}
	}

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(Length)
{
	Int length;
	STATIC_STRING(cycleError, "List has infinite length because it contains a cycle.");

	length = SmileList_SafeLength((SmileList)argv[0].obj);
	if (length < 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, cycleError);
	}

	return SmileUnboxedInteger64_From(length);
}

SMILE_EXTERNAL_FUNCTION(HasCycle)
{
	return SmileUnboxedBool_From(SmileList_HasCycle(argv[0].obj));
}

SMILE_EXTERNAL_FUNCTION(IsWellFormed)
{
	return SmileUnboxedBool_From(SmileList_IsWellFormed(argv[0].obj));
}

SMILE_EXTERNAL_FUNCTION(Join)
{
	String glue, result;
	STATIC_STRING(malformedListError, "The list passed to '%s' is malformed or contains a cycle.");

	if (argc <= 1)
		glue = String_Empty;
	else
		glue = (String)argv[1].obj;
	
	result = SmileList_Join((SmileList)argv[0].obj, glue);

	if (result == NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(malformedListError, "join"));
	}

	return SmileArg_From((SmileObject)result);
}

//-------------------------------------------------------------------------------------------------

typedef struct EachInfoStruct {
	SmileList initialList;
	SmileList list;
	SmileFunction function;
	Int index;
} *EachInfo;

static Int EachWithOneArg(ClosureStateMachine closure)
{
	EachInfo eachInfo = (EachInfo)closure->state;
	SmileList list = eachInfo->list;

	// If we've run out of list nodes, we're done.
	if (SMILE_KIND(list) != SMILE_KIND_LIST) {
		Closure_Pop(closure);	// Pop the previous return value
		Closure_PushBoxed(closure, eachInfo->initialList);	// and push 'initialList' as the new return value.
		return -1;
	}

	// Set up to call the user's function with the next list item.
	Closure_Pop(closure);
	Closure_PushBoxed(closure, eachInfo->function);
	Closure_UnboxAndPush(closure, list->a);

	eachInfo->list = (SmileList)list->d;	// Move the iterator to the next item.
	eachInfo->index++;

	return 1;
}

static Int EachWithTwoArgs(ClosureStateMachine closure)
{
	EachInfo eachInfo = (EachInfo)closure->state;
	SmileList list = eachInfo->list;

	// If we've run out of list nodes, we're done.
	if (SMILE_KIND(list) != SMILE_KIND_LIST) {
		Closure_Pop(closure);	// Pop the previous return value
		Closure_PushBoxed(closure, eachInfo->initialList);	// and push 'initialList' as the new return value.
		return -1;
	}

	// Set up to call the user's function with the next list item and index.
	Closure_Pop(closure);
	Closure_PushBoxed(closure, eachInfo->function);
	Closure_UnboxAndPush(closure, list->a);
	Closure_PushUnboxedInt64(closure, eachInfo->index);

	eachInfo->list = (SmileList)list->d;	// Move the iterator to the next item.
	eachInfo->index++;

	return 2;
}

SMILE_EXTERNAL_FUNCTION(Each)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	EachInfo eachInfo;
	ClosureStateMachine closure;
	StateMachine stateMachine;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	stateMachine = maxArgs <= 1 ? EachWithOneArg : EachWithTwoArgs;
	closure = Eval_BeginStateMachine(stateMachine, stateMachine);

	eachInfo = (EachInfo)closure->state;
	eachInfo->function = function;
	eachInfo->list = eachInfo->initialList = list;
	eachInfo->index = 0;

	Closure_PushBoxed(closure, NullObject);	// Initial "return" value from 'each'.

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct MapInfoStruct {
	SmileList resultHead, resultTail;
	SmileList list;
	SmileFunction function;
	Int index;
} *MapInfo;

static Int MapWithOneArgStart(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

static Int MapWithOneArgBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileArg_Box(fnResult));

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

static Int MapWithTwoArgsStart(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

static Int MapWithTwoArgsBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileArg_Box(fnResult));

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

SMILE_EXTERNAL_FUNCTION(Map)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	MapInfo loopInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(
		maxArgs <= 1 ? MapWithOneArgStart : MapWithTwoArgsStart,
		maxArgs <= 1 ? MapWithOneArgBody : MapWithTwoArgsBody);

	loopInfo = (MapInfo)closure->state;
	loopInfo->function = function;
	loopInfo->list = list;
	loopInfo->resultHead = loopInfo->resultTail = NullList;
	loopInfo->index = 0;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct WhereInfoStruct {
	SmileList resultHead, resultTail;
	SmileList list;
	SmileFunction function;
	Int index;
} *WhereInfo;

static Int WhereWithOneArgStart(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

static Int WhereWithOneArgBody(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, keep this list element.
	if (booleanResult) {
		LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, loopInfo->list->a);
	}

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

static Int WhereWithTwoArgsStart(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

static Int WhereWithTwoArgsBody(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, keep this list element.
	if (booleanResult) {
		LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, loopInfo->list->a);
	}

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

SMILE_EXTERNAL_FUNCTION(Where)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	WhereInfo whereInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(
		maxArgs <= 1 ? WhereWithOneArgStart : WhereWithTwoArgsStart,
		maxArgs <= 1 ? WhereWithOneArgBody : WhereWithTwoArgsBody);

	whereInfo = (WhereInfo)closure->state;
	whereInfo->function = function;
	whereInfo->list = list;
	whereInfo->resultHead = whereInfo->resultTail = NullList;
	whereInfo->index = 0;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct AnyAllInfoStruct {
	SmileList list;
	SmileFunction function;
	Bool complement;
} *AnyAllInfo;

static Int AnyAllStart(ClosureStateMachine closure)
{
	register AnyAllInfo loopInfo = (AnyAllInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushUnboxedBool(closure, loopInfo->complement);
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

static Int AnyAllBody(ClosureStateMachine closure)
{
	register AnyAllInfo loopInfo = (AnyAllInfo)closure->state;

	// Body: Get the value from the user's condition.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	if (booleanResult ^ loopInfo->complement) {
		// We found a hit (for any, or a miss for all).  Stop now.
		Closure_PushUnboxedBool(closure, !loopInfo->complement);
		return -1;
	}

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushUnboxedBool(closure, loopInfo->complement);
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

SMILE_EXTERNAL_FUNCTION(Contains)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	AnyAllInfo anyAllInfo;
	ClosureStateMachine closure;
	SmileArg arg;

	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Check to see if any values are super-equal to the given value.
		arg = argv[1];
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (SMILE_VCALL3(list->a, compareEqual, (SmileUnboxedData){ 0 }, arg.obj, arg.unboxed)) {
				return SmileUnboxedBool_From(True);
			}
		}
		return SmileUnboxedBool_From(False);
	}

	closure = Eval_BeginStateMachine(AnyAllStart, AnyAllBody);

	anyAllInfo = (AnyAllInfo)closure->state;
	anyAllInfo->function = function;
	anyAllInfo->list = list;
	anyAllInfo->complement = False;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(Empty)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[0].obj) == SMILE_KIND_NULL);
}

SMILE_EXTERNAL_FUNCTION(Any)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	AnyAllInfo anyAllInfo;
	ClosureStateMachine closure;
	SmileArg arg;

	if (argc < 1) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'any?' requires at least 1 argument, but was called with %d.", argc));
	}
	if ((SMILE_KIND(argv[0].obj) & ~SMILE_KIND_LIST_BIT) != SMILE_KIND_NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument 1 to 'any?' is of the wrong type."));
	}

	if (argc == 1) {
		return SmileUnboxedBool_From(SMILE_KIND(argv[0].obj) != SMILE_KIND_NULL);
	}

	if (argc > 2) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'any?' allows at most 2 arguments, but was called with %d.", argc));
	}

	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Check to see if any values are super-equal to the given value.
		arg = argv[1];
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (SMILE_VCALL3(list->a, compareEqual, (SmileUnboxedData){ 0 }, arg.obj, arg.unboxed)) {
				return SmileUnboxedBool_From(True);
			}
		}
		return SmileUnboxedBool_From(False);
	}

	closure = Eval_BeginStateMachine(AnyAllStart, AnyAllBody);

	anyAllInfo = (AnyAllInfo)closure->state;
	anyAllInfo->function = function;
	anyAllInfo->list = list;
	anyAllInfo->complement = False;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(All)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	AnyAllInfo anyAllInfo;
	ClosureStateMachine closure;
	SmileArg arg;

	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Check to make sure that all values are super-equal to the given value.
		arg = argv[1];
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (!SMILE_VCALL3(list->a, compareEqual, (SmileUnboxedData){ 0 }, arg.obj, arg.unboxed)) {
				return SmileUnboxedBool_From(False);
			}
		}
		return SmileUnboxedBool_From(True);
	}

	closure = Eval_BeginStateMachine(AnyAllStart, AnyAllBody);

	anyAllInfo = (AnyAllInfo)closure->state;
	anyAllInfo->function = function;
	anyAllInfo->list = list;
	anyAllInfo->complement = True;

	return (SmileArg){ 0 };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct CountInfoStruct {
	SmileList list;
	SmileFunction function;
	Int count;
} *CountInfo;

static Int CountStart(ClosureStateMachine closure)
{
	register CountInfo loopInfo = (CountInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushUnboxedInt64(closure, loopInfo->count);
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

static Int CountBody(ClosureStateMachine closure)
{
	register CountInfo loopInfo = (CountInfo)closure->state;

	// Body: Get the value from the user's condition.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If we found a hit, add it to the count.  (We always add here to avoid the possibility
	// of a branch misprediction from an if-statement.)
	loopInfo->count += booleanResult;

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushUnboxedInt64(closure, loopInfo->count);
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

SMILE_EXTERNAL_FUNCTION(Count)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	CountInfo countInfo;
	ClosureStateMachine closure;
	SmileArg arg;
	Int count;

	if (argc < 1) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'count' requires at least 1 argument, but was called with %d.", argc));
	}
	if ((SMILE_KIND(argv[0].obj) & ~SMILE_KIND_LIST_BIT) != SMILE_KIND_NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument 1 to 'count' is of the wrong type."));
	}

	if (argc == 1) {
		// Degenerate form: Just count the list nodes, as fast as possible.
		return SmileUnboxedInteger64_From(SmileList_Length(list));
	}

	if (argc > 2) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'count' allows at most 2 arguments, but was called with %d.", argc));
	}

	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Count up any values that are super-equal to the given value.
		arg = argv[1];
		for (count = 0; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (SMILE_VCALL3(list->a, compareEqual, (SmileUnboxedData){ 0 }, arg.obj, arg.unboxed)) {
				count++;
			}
		}
		return SmileUnboxedInteger64_From(count);
	}

	closure = Eval_BeginStateMachine(CountStart, CountBody);

	countInfo = (CountInfo)closure->state;
	countInfo->function = (SmileFunction)argv[1].obj;
	countInfo->list = list;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct FirstInfoStruct {
	SmileList list;
	SmileFunction function;
	Int index;
	Bool indexOfMode;
} *FirstInfo;

static Int FirstStart(ClosureStateMachine closure)
{
	register FirstInfo loopInfo = (FirstInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		if (loopInfo->indexOfMode)
			Closure_PushUnboxedInt64(closure, -1);
		else
			Closure_PushBoxed(closure, NullObject);
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

static Int FirstBody(ClosureStateMachine closure)
{
	register FirstInfo loopInfo = (FirstInfo)closure->state;

	// Body: Get the value from the user's condition.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	if (booleanResult) {
		// We found a hit.  Stop now.
		if (loopInfo->indexOfMode)
			Closure_PushUnboxedInt64(closure, loopInfo->index);
		else
			Closure_UnboxAndPush(closure, loopInfo->list->a);
		return -1;
	}

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		if (loopInfo->indexOfMode)
			Closure_PushUnboxedInt64(closure, -1);
		else
			Closure_PushBoxed(closure, NullObject);
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_UnboxAndPush(closure, loopInfo->list->a);
	return 1;
}

SMILE_EXTERNAL_FUNCTION(First)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	FirstInfo firstInfo;
	ClosureStateMachine closure;
	SmileArg arg;

	if (argc < 1) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'first' requires at least 1 argument, but was called with %d.", argc));
	}
	if ((SMILE_KIND(argv[0].obj) & ~SMILE_KIND_LIST_BIT) != SMILE_KIND_NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument 1 to 'first' is of the wrong type."));
	}

	if (argc == 1) {
		// Degenerate form:  This is a synonym for 'car'.
		return SmileArg_Unbox(list->a);
	}

	if (argc > 2) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'first' allows at most 2 arguments, but was called with %d.", argc));
	}

	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Return the first matching item if a matching item exists, null if none exists.
		arg = argv[1];
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (SMILE_VCALL3(list->a, compareEqual, (SmileUnboxedData){ 0 }, arg.obj, arg.unboxed)) {
				return SmileArg_Unbox(list->a);
			}
		}
		return SmileArg_From(NullObject);
	}

	// Iterating with a search predicate.
	closure = Eval_BeginStateMachine(FirstStart, FirstBody);

	firstInfo = (FirstInfo)closure->state;
	firstInfo->function = (SmileFunction)argv[1].obj;
	firstInfo->list = list;
	firstInfo->indexOfMode = False;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(IndexOf)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	FirstInfo firstInfo;
	ClosureStateMachine closure;
	SmileArg arg;
	Int index;

	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Return the index of the first matching item if a matching item exists, -1 if none exists.
		arg = argv[1];
		index = 0;
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list), index++) {
			if (SMILE_VCALL3(list->a, compareEqual, (SmileUnboxedData){ 0 }, arg.obj, arg.unboxed)) {
				return SmileUnboxedInteger64_From(index);
			}
		}
		return SmileUnboxedInteger64_From(-1);
	}

	// Iterating with a search predicate.
	closure = Eval_BeginStateMachine(FirstStart, FirstBody);

	firstInfo = (FirstInfo)closure->state;
	firstInfo->function = function;
	firstInfo->list = list;
	firstInfo->indexOfMode = True;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(Car)
{
	return SmileArg_Unbox(((SmileList)argv[0].obj)->a);
}

SMILE_EXTERNAL_FUNCTION(Cdr)
{
	return SmileArg_Unbox(((SmileList)argv[0].obj)->d);
}

SMILE_EXTERNAL_FUNCTION(Caar)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->a);
}

SMILE_EXTERNAL_FUNCTION(Cadr)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->a);
}

SMILE_EXTERNAL_FUNCTION(Cdar)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->d);
}

SMILE_EXTERNAL_FUNCTION(Cddr)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->d);
}

SMILE_EXTERNAL_FUNCTION(Caaar)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->a);
}

SMILE_EXTERNAL_FUNCTION(Caadr)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->a);
}

SMILE_EXTERNAL_FUNCTION(Cadar)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->a);
}

SMILE_EXTERNAL_FUNCTION(Caddr)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->a);
}

SMILE_EXTERNAL_FUNCTION(Cdaar)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->d);
}

SMILE_EXTERNAL_FUNCTION(Cdadr)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->d);
}

SMILE_EXTERNAL_FUNCTION(Cddar)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->d);
}

SMILE_EXTERNAL_FUNCTION(Cdddr)
{
	SmileObject obj;

	obj = ((SmileList)argv[0].obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return SmileArg_From(NullObject);
	return SmileArg_Unbox(((SmileList)obj)->d);
}

SMILE_EXTERNAL_FUNCTION(Cxr)
{
	UInt32 flags;
	SmileObject obj = argv[0].obj;

	for (flags = (UInt32)(PtrInt)param; flags; flags >>= 3) {
		if (SMILE_KIND(obj) != SMILE_KIND_LIST)
			return SmileArg_From(NullObject);
		if ((flags & 3) == 1) {
			obj = ((SmileList)obj)->a;
		}
		else if ((flags & 3) == 2) {
			obj = ((SmileList)obj)->d;
		}
	}

	return SmileArg_Unbox(obj);
}

//-------------------------------------------------------------------------------------------------

void SmileList_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "list", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "items", ARG_CHECK_MIN, 1, 0, 0, NULL);
	SetupFunction("cons", Cons, (void *)base, "a b", ARG_CHECK_MIN | ARG_CHECK_MAX, 2, 3, 0, NULL);
	SetupFunction("combine", Combine, (void *)base, "lists...", ARG_CHECK_MIN, 2, 0, 2, _combineChecks);

	SetupFunction("join", Join, NULL, "list", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _joinChecks);

	/*
	SetupFunction("append", Append, NULL, "list elements...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 1, _joinChecks);
	SetupSynonym("append", "conc");
	SetupFunction("append!", AppendInPlace, NULL, "list elements...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 2, _joinChecks);
	SetupSynonym("append!", "conc!");
	SetupFunction("append-list", AppendList, NULL, "list lists...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 1, _listChecks);
	SetupSynonym("append-list", "conc-list");
	SetupFunction("append-list!", AppendListInPlace, NULL, "list lists...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 1, _listChecks);
	SetupSynonym("append-list!", "conc-list!");
	SetupSynonym("append", "+");

	SetupFunction("get-member", GetMember, NULL, "list index", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _joinChecks);
	SetupFunction("set-member", SetMember, NULL, "list index value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 2, _joinChecks);
	SetupSynonym("get-member", "nth");
	SetupFunction("nth-cell", NthCell, NULL, "list index", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _joinChecks);

	SetupFunction("splice", Splice, NULL, "list index elements...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 1, _joinChecks);
	SetupFunction("splice!", SpliceInPlace, NULL, "list index elements...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 2, _joinChecks);
	SetupFunction("splice-list", SpliceList, NULL, "list index lists...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 1, _listChecks);
	SetupFunction("splice-list!", SpliceListInPlace, NULL, "list index lists...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 1, _listChecks);
	*/

	SetupFunction("each", Each, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("map", Map, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("where", Where, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);

	SetupFunction("empty?", Empty, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("null?", Empty, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);

	SetupFunction("any?", Any, NULL, "list", ARG_STATE_MACHINE, 0, 0, 0, NULL);
	SetupFunction("contains?", Contains, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _indexOfChecks);
	SetupFunction("all?", All, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _indexOfChecks);
	SetupFunction("first", First, NULL, "list", ARG_STATE_MACHINE, 0, 0, 0, NULL);
	SetupFunction("index-of", IndexOf, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _indexOfChecks);
	SetupFunction("count", Count, NULL, "list", ARG_STATE_MACHINE, 0, 0, 0, NULL);

	SetupFunction("length", Length, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cycle?", HasCycle, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("well-formed?", IsWellFormed, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);

	SetupFunction("car", Car, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdr", Cdr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupSynonym("cdr", "rest");

	SetupFunction("caar", Caar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadr", Cadr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdar", Cdar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddr", Cddr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);

	SetupFunction("caaar", Caaar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caadr", Caadr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadar", Cadar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caddr", Caddr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdaar", Cdaar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdadr", Cdadr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddar", Cddar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdddr", Cdddr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);

	SetupFunction("caaaar", Cxr, (void *)0x00001111, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caaadr", Cxr, (void *)0x00001112, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caadar", Cxr, (void *)0x00001121, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caaddr", Cxr, (void *)0x00001122, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadaar", Cxr, (void *)0x00001211, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadadr", Cxr, (void *)0x00001212, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caddar", Cxr, (void *)0x00001221, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadddr", Cxr, (void *)0x00001222, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdaaar", Cxr, (void *)0x00002111, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdaadr", Cxr, (void *)0x00002112, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdadar", Cxr, (void *)0x00002121, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdaddr", Cxr, (void *)0x00002122, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddaar", Cxr, (void *)0x00002211, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddadr", Cxr, (void *)0x00002212, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdddar", Cxr, (void *)0x00002221, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddddr", Cxr, (void *)0x00002222, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
}
