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
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/eval/eval.h>

#define Setup(__name__, __value__) \
	(SmileUserObject_QuickSet(base, (__name__), (__value__)))

#define SetupSynonym(__newName__, __oldName__) \
	(Setup((__newName__), SmileUserObject_Get(base, SymbolTable_GetSymbolC(Smile_SymbolTable, (__oldName__)))))

#define SetupFunction(__name__, __function__, __param__, __argNames__, __argCheckFlags__, __minArgs__, __maxArgs__, __numArgsToTypeCheck__, __argTypeChecks__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), (__argCheckFlags__), (__minArgs__), (__maxArgs__), (__numArgsToTypeCheck__), (__argTypeChecks__))))

#define SetupSimpleFunction(__name__, __function__, __argNames__, __numArgs__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), ARG_CHECK_EXACT, (__numArgs__), (__numArgs__), NULL)))

static Byte _listChecks[] = {
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
};

static Byte _joinChecks[] = {
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
};

static Byte _eachChecks[] = {
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

static Byte _indexOfChecks[] = {
	SMILE_KIND_MASK & ~SMILE_KIND_LIST, SMILE_KIND_NULL,
	0, 0,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

static SmileObject ToBool(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_LIST)
		return (SmileObject)Smile_KnownObjects.TrueObj;

	if (SMILE_KIND(argv[0]) == SMILE_KIND_NULL)
		return (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject ToInt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_LIST)
		return (SmileObject)SmileInteger64_Create(SmileList_Length(((SmileList)argv[0])));

	return (SmileObject)Smile_KnownObjects.ZeroInt64;
}

STATIC_STRING(_List, "List");
STATIC_STRING(_null, "null");

static SmileObject ToString(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_LIST) {
		return (SmileObject)SmileString_Create(SmileObject_Stringify(argv[0]));
	}
	else if (SMILE_KIND(argv[0]) == SMILE_KIND_NULL) {
		return (SmileObject)SmileString_Create(_null);
	}

	return (SmileObject)SmileString_Create(_List);
}

static SmileObject Hash(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_NULL) {
		return (SmileObject)Smile_KnownObjects.ZeroInt64;
	}

	return (SmileObject)SmileInteger64_Create(((PtrInt)argv[0]) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Construction functions.

static SmileObject Of(Int argc, SmileObject *argv, void *param)
{
	SmileList head = NullList, tail = NullList;
	SmileUserObject base = (SmileUserObject)param;
	Int i;

	i = 0;
	if (argv[i] == (SmileObject)base)
		i++;

	for (; i < argc; i++) {
		LIST_APPEND(head, tail, argv[i]);
	}

	return (SmileObject)head;
}

static SmileObject Cons(Int argc, SmileObject *argv, void *param)
{
	UNUSED(param);

	if (argc == 2)
		return (SmileObject)SmileList_Cons(argv[0], argv[1]);
	return (SmileObject)SmileList_Cons(argv[1], argv[2]);
}

STATIC_STRING(_cycleError, "List has infinite length because it contains a cycle.");

static SmileObject Length(Int argc, SmileObject *argv, void *param)
{
	Int length;

	UNUSED(argc);
	UNUSED(param);

	length = SmileList_SafeLength((SmileList)argv[0]);
	if (length < 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _cycleError);
	}

	return (SmileObject)SmileInteger64_Create(length);
}

static SmileObject HasCycle(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return SmileList_HasCycle(argv[0]) ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject IsWellFormed(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return SmileList_IsWellFormed(argv[0]) ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

STATIC_STRING(_malformedListError, "The list passed to '%s' is malformed or contains a cycle.");

static SmileObject Join(Int argc, SmileObject *argv, void *param)
{
	String glue, result;

	UNUSED(param);

	if (argc <= 1)
		glue = String_Empty;
	else
		glue = (String)&((SmileString)argv[1])->string;
	
	result = SmileList_Join((SmileList)argv[0], glue);

	if (result == NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_malformedListError, "join"));
	}

	return (SmileObject)SmileString_Create(result);
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
		Closure_SetTop(closure, eachInfo->initialList);	// Pop the previous return value and push 'initialList'.
		return -1;
	}

	// Set up to call the user's function with the next list item.
	Closure_PopTemp(closure);
	Closure_PushTemp(closure, eachInfo->function);
	Closure_PushTemp(closure, list->a);

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
		Closure_SetTop(closure, eachInfo->initialList);	// Pop the previous return value and push 'initialList'.
		return -1;
	}

	// Set up to call the user's function with the next list item and index.
	Closure_PopTemp(closure);
	Closure_PushTemp(closure, eachInfo->function);
	Closure_PushTemp(closure, list->a);
	Closure_PushTemp(closure, SmileInteger64_Create(eachInfo->index));

	eachInfo->list = (SmileList)list->d;	// Move the iterator to the next item.
	eachInfo->index++;

	return 2;
}

static SmileObject Each(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	SmileFunction function = (SmileFunction)argv[1];
	Int minArgs, maxArgs;
	EachInfo eachInfo;
	ClosureStateMachine closure;
	StateMachine stateMachine;

	UNUSED(param);
	UNUSED(argc);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	stateMachine = maxArgs <= 1 ? EachWithOneArg : EachWithTwoArgs;
	closure = Eval_BeginStateMachine(stateMachine, stateMachine);

	eachInfo = (EachInfo)closure->state;
	eachInfo->function = function;
	eachInfo->list = eachInfo->initialList = list;
	eachInfo->index = 0;

	Closure_PushTemp(closure, NullObject);	// Initial "return" value from 'each'.

	return NULL;	// We have to return something, but this value will be ignored.
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
		Closure_PushTemp(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static Int MapWithOneArgBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileObject fnResult = Closure_PopTemp(closure);
	LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, fnResult);

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushTemp(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static Int MapWithTwoArgsStart(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushTemp(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	Closure_PushTemp(closure, SmileInteger64_Create(loopInfo->index));
	return 2;
}

static Int MapWithTwoArgsBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileObject fnResult = Closure_PopTemp(closure);
	LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, fnResult);

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushTemp(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	Closure_PushTemp(closure, SmileInteger64_Create(loopInfo->index));
	return 2;
}

static SmileObject Map(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	SmileFunction function = (SmileFunction)argv[1];
	Int minArgs, maxArgs;
	MapInfo loopInfo;
	ClosureStateMachine closure;

	UNUSED(param);
	UNUSED(argc);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(
		maxArgs <= 1 ? MapWithOneArgStart : MapWithTwoArgsStart,
		maxArgs <= 1 ? MapWithOneArgBody : MapWithTwoArgsBody);

	loopInfo = (MapInfo)closure->state;
	loopInfo->function = function;
	loopInfo->list = list;
	loopInfo->resultHead = loopInfo->resultTail = NullList;
	loopInfo->index = 0;

	return NULL;	// We have to return something, but this value will be ignored.
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
		Closure_PushTemp(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static Int WhereWithOneArgBody(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileObject fnResult = Closure_PopTemp(closure);
	Bool booleanResult = SMILE_VCALL(fnResult, toBool);

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
		Closure_PushTemp(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static Int WhereWithTwoArgsStart(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushTemp(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	Closure_PushTemp(closure, SmileInteger64_Create(loopInfo->index));
	return 2;
}

static Int WhereWithTwoArgsBody(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileObject fnResult = Closure_PopTemp(closure);
	Bool booleanResult = SMILE_VCALL(fnResult, toBool);

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
		Closure_PushTemp(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	Closure_PushTemp(closure, SmileInteger64_Create(loopInfo->index));
	return 2;
}

static SmileObject Where(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	SmileFunction function = (SmileFunction)argv[1];
	Int minArgs, maxArgs;
	WhereInfo whereInfo;
	ClosureStateMachine closure;

	UNUSED(param);
	UNUSED(argc);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(
		maxArgs <= 1 ? WhereWithOneArgStart : WhereWithTwoArgsStart,
		maxArgs <= 1 ? WhereWithOneArgBody : WhereWithTwoArgsBody);

	whereInfo = (WhereInfo)closure->state;
	whereInfo->function = function;
	whereInfo->list = list;
	whereInfo->resultHead = whereInfo->resultTail = NullList;
	whereInfo->index = 0;

	return NULL;	// We have to return something, but this value will be ignored.
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
		Closure_PushTemp(closure, loopInfo->complement ? Smile_KnownObjects.TrueObj : Smile_KnownObjects.FalseObj);
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static Int AnyAllBody(ClosureStateMachine closure)
{
	register AnyAllInfo loopInfo = (AnyAllInfo)closure->state;

	// Body: Get the value from the user's condition.
	SmileObject fnResult = Closure_PopTemp(closure);
	Bool booleanResult = SMILE_VCALL(fnResult, toBool);

	if (booleanResult ^ loopInfo->complement) {
		// We found a hit (for any, or a miss for all).  Stop now.
		Closure_PushTemp(closure, loopInfo->complement ? Smile_KnownObjects.FalseObj : Smile_KnownObjects.TrueObj);
		return -1;
	}

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushTemp(closure, loopInfo->complement ? Smile_KnownObjects.TrueObj : Smile_KnownObjects.FalseObj);
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static SmileObject Contains(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	SmileFunction function = (SmileFunction)argv[1];
	AnyAllInfo anyAllInfo;
	ClosureStateMachine closure;
	SmileObject value;

	UNUSED(param);
	UNUSED(argc);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Check to see if any values are super-equal to the given value.
		value = argv[1];
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (list->a == value || SMILE_VCALL1(list->a, compareEqual, value)) {
				return (SmileObject)Smile_KnownObjects.TrueObj;
			}
		}
		return (SmileObject)Smile_KnownObjects.FalseObj;
	}

	closure = Eval_BeginStateMachine(AnyAllStart, AnyAllBody);

	anyAllInfo = (AnyAllInfo)closure->state;
	anyAllInfo->function = function;
	anyAllInfo->list = list;
	anyAllInfo->complement = False;

	return NULL;	// We have to return something, but this value will be ignored.
}

static SmileObject Empty(Int argc, SmileObject *argv, void *param)
{
	UNUSED(param);
	UNUSED(argc);

	return SMILE_KIND(argv[0]) == SMILE_KIND_NULL ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Any(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	SmileFunction function = (SmileFunction)argv[1];
	AnyAllInfo anyAllInfo;
	ClosureStateMachine closure;
	SmileObject value;

	UNUSED(param);

	if (argc < 1) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'any?' requires at least 1 argument, but was called with %d.", argc));
	}
	if ((SMILE_KIND(argv[0]) & ~SMILE_KIND_LIST) != SMILE_KIND_NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument 1 to 'any?' is of the wrong type."));
	}

	if (argc == 1) {
		return SMILE_KIND(argv[0]) != SMILE_KIND_NULL ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
	}

	if (argc > 2) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'any?' allows at most 2 arguments, but was called with %d.", argc));
	}

	if (SMILE_KIND(argv[1]) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Check to see if any values are super-equal to the given value.
		value = argv[1];
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (list->a == value || SMILE_VCALL1(list->a, compareEqual, value)) {
				return (SmileObject)Smile_KnownObjects.TrueObj;
			}
		}
		return (SmileObject)Smile_KnownObjects.FalseObj;
	}

	closure = Eval_BeginStateMachine(AnyAllStart, AnyAllBody);

	anyAllInfo = (AnyAllInfo)closure->state;
	anyAllInfo->function = function;
	anyAllInfo->list = list;
	anyAllInfo->complement = False;

	return NULL;	// We have to return something, but this value will be ignored.
}

static SmileObject All(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	SmileFunction function = (SmileFunction)argv[1];
	AnyAllInfo anyAllInfo;
	ClosureStateMachine closure;
	SmileObject value;

	UNUSED(param);
	UNUSED(argc);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Check to make sure that all values are super-equal to the given value.
		value = argv[1];
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (list->a != value && !SMILE_VCALL1(list->a, compareEqual, value)) {
				return (SmileObject)Smile_KnownObjects.FalseObj;
			}
		}
		return (SmileObject)Smile_KnownObjects.TrueObj;
	}

	closure = Eval_BeginStateMachine(AnyAllStart, AnyAllBody);

	anyAllInfo = (AnyAllInfo)closure->state;
	anyAllInfo->function = function;
	anyAllInfo->list = list;
	anyAllInfo->complement = True;

	return NULL;	// We have to return something, but this value will be ignored.
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
		Closure_PushTemp(closure, SmileInteger64_Create(loopInfo->count));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static Int CountBody(ClosureStateMachine closure)
{
	register CountInfo loopInfo = (CountInfo)closure->state;

	// Body: Get the value from the user's condition.
	SmileObject fnResult = Closure_PopTemp(closure);
	Bool booleanResult = SMILE_VCALL(fnResult, toBool);

	// If we found a hit, add it to the count.  (We always add here to avoid the possibility
	// of a branch misprediction from an if-statement.)
	loopInfo->count += booleanResult;

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushTemp(closure, SmileInteger64_Create(loopInfo->count));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static SmileObject Count(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	CountInfo countInfo;
	ClosureStateMachine closure;
	SmileObject value;
	Int count;

	UNUSED(param);

	if (argc < 1) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'count' requires at least 1 argument, but was called with %d.", argc));
	}
	if ((SMILE_KIND(argv[0]) & ~SMILE_KIND_LIST) != SMILE_KIND_NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument 1 to 'count' is of the wrong type."));
	}

	if (argc == 1) {
		// Degenerate form: Just count the list nodes, as fast as possible.
		return (SmileObject)SmileInteger64_Create(SmileList_Length(list));
	}

	if (argc > 2) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'count' allows at most 2 arguments, but was called with %d.", argc));
	}

	if (SMILE_KIND(argv[1]) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Count up any values that are super-equal to the given value.
		value = argv[1];
		for (count = 0; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (list->a == value || SMILE_VCALL1(list->a, compareEqual, value)) {
				count++;
			}
		}
		return (SmileObject)SmileInteger64_Create(count);
	}

	closure = Eval_BeginStateMachine(CountStart, CountBody);

	countInfo = (CountInfo)closure->state;
	countInfo->function = (SmileFunction)argv[1];
	countInfo->list = list;

	return NULL;	// We have to return something, but this value will be ignored.
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
		Closure_PushTemp(closure, loopInfo->indexOfMode ? (SmileObject)Smile_KnownObjects.NegOneInt64 : NullObject);
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static Int FirstBody(ClosureStateMachine closure)
{
	register FirstInfo loopInfo = (FirstInfo)closure->state;

	// Body: Get the value from the user's condition.
	SmileObject fnResult = Closure_PopTemp(closure);
	Bool booleanResult = SMILE_VCALL(fnResult, toBool);

	if (booleanResult) {
		// We found a hit.  Stop now.
		Closure_PushTemp(closure, loopInfo->indexOfMode
			? (SmileObject)SmileInteger64_Create(loopInfo->index) : loopInfo->list->a);
		return -1;
	}

	// Next: Move the iterator to the next item.
	loopInfo->list = (SmileList)loopInfo->list->d;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (SMILE_KIND(loopInfo->list) != SMILE_KIND_LIST) {
		Closure_PushTemp(closure, loopInfo->indexOfMode ? (SmileObject)Smile_KnownObjects.NegOneInt64 : NullObject);
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushTemp(closure, loopInfo->function);
	Closure_PushTemp(closure, loopInfo->list->a);
	return 1;
}

static SmileObject First(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	FirstInfo firstInfo;
	ClosureStateMachine closure;
	SmileObject value;

	UNUSED(param);

	if (argc < 1) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'first' requires at least 1 argument, but was called with %d.", argc));
	}
	if ((SMILE_KIND(argv[0]) & ~SMILE_KIND_LIST) != SMILE_KIND_NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument 1 to 'first' is of the wrong type."));
	}

	if (argc == 1) {
		// Degenerate form:  This is a synonym for 'car'.
		return list->a;
	}

	if (argc > 2) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'first' allows at most 2 arguments, but was called with %d.", argc));
	}

	if (SMILE_KIND(argv[1]) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Return the first matching item if a matching item exists, null if none exists.
		value = argv[1];
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
			if (list->a == value || SMILE_VCALL1(list->a, compareEqual, value)) {
				return list->a;
			}
		}
		return NullObject;
	}

	// Iterating with a search predicate.
	closure = Eval_BeginStateMachine(FirstStart, FirstBody);

	firstInfo = (FirstInfo)closure->state;
	firstInfo->function = (SmileFunction)argv[1];
	firstInfo->list = list;
	firstInfo->indexOfMode = False;

	return NULL;	// We have to return something, but this value will be ignored.
}

static SmileObject IndexOf(Int argc, SmileObject *argv, void *param)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileList list = (SmileList)argv[0];
	SmileFunction function = (SmileFunction)argv[1];
	FirstInfo firstInfo;
	ClosureStateMachine closure;
	SmileObject value;
	Int index;

	UNUSED(param);
	UNUSED(argc);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_FUNCTION) {
		// Degenerate form:  Return the index of the first matching item if a matching item exists, -1 if none exists.
		value = argv[1];
		index = 0;
		for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list), index++) {
			if (list->a == value || SMILE_VCALL1(list->a, compareEqual, value)) {
				return (SmileObject)SmileInteger64_Create(index);
			}
		}
		return (SmileObject)Smile_KnownObjects.NegOneInt64;
	}

	// Iterating with a search predicate.
	closure = Eval_BeginStateMachine(FirstStart, FirstBody);

	firstInfo = (FirstInfo)closure->state;
	firstInfo->function = function;
	firstInfo->list = list;
	firstInfo->indexOfMode = True;

	return NULL;	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

static SmileObject Car(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileList)argv[0])->a;
}

static SmileObject Cdr(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileList)argv[0])->d;
}

static SmileObject Caar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Cadr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Cdar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cddr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Caaar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Caadr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Cadar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Caddr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Cdaar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cdadr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cddar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cdddr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cxr(Int argc, SmileObject *argv, void *param)
{
	UInt32 flags;
	SmileObject obj = argv[0];

	UNUSED(argc);
	UNUSED(param);

	for (flags = (UInt32)(PtrInt)param; flags; flags >>= 3) {
		if (SMILE_KIND(obj) != SMILE_KIND_LIST)
			return NullObject;
		if ((flags & 3) == 1) {
			obj = ((SmileList)obj)->a;
		}
		else if ((flags & 3) == 2) {
			obj = ((SmileList)obj)->d;
		}
	}

	return obj;
}

//-------------------------------------------------------------------------------------------------

void SmileList_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "list", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "items", ARG_CHECK_MIN, 1, 0, 0, NULL);
	SetupFunction("cons", Cons, NULL, "a b", ARG_CHECK_MIN | ARG_CHECK_MAX, 2, 3, 0, NULL);

	SetupFunction("join", Join, NULL, "list", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _joinChecks);

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
	SetupSynonym("rest", "cdr");

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
