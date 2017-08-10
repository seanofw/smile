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
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/range/smileinteger64range.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>
#include <smile/eval/eval.h>

SMILE_IGNORE_UNUSED_VARIABLES

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Integer64'");
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
STATIC_STRING(_integer64TypeError, "%s argument to '%s' must be of type 'Integer64'");
STATIC_STRING(_argCountError, "Too many arguments to 'Integer64Range.%s'");

static Byte _eachChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64RANGE,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

static Byte _integer64Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64RANGE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_INTEGER64RANGE)
		return SmileUnboxedBool_From(True);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_INTEGER64RANGE) {
		SmileInteger64Range obj = (SmileInteger64Range)argv[0].obj;
		return SmileUnboxedInteger64_From(obj->end - obj->start);
	}

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	Int64 numericBase;
	STATIC_STRING(integer64range, "Integer64Range");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_INTEGER64RANGE) {
		String string;
		SmileInteger64Range range = (SmileInteger64Range)argv[0].obj;

		if (argc == 2) {
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_INTEGER64)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)argv[1].unboxed.i64;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;

		if (range->end >= range->start && range->stepping != +1
			|| range->end < range->start && range->stepping != -1) {
			string = String_Format("%S..%S step %S",
				String_CreateFromInteger(range->start, (Int)numericBase, False),
				String_CreateFromInteger(range->end, (Int)numericBase, False),
				String_CreateFromInteger(range->stepping, (Int)numericBase, False));
		}
		else {
			string = String_Format("%S..%S",
				String_CreateFromInteger(range->start, (Int)numericBase, False),
				String_CreateFromInteger(range->end, (Int)numericBase, False));
		}
		return SmileArg_From((SmileObject)string);
	}

	return SmileArg_From((SmileObject)integer64range);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_INTEGER64RANGE) {
		SmileInteger64Range range = (SmileInteger64Range)argv[0].obj;
		UInt64 start = (UInt64)range->start;
		UInt64 end = (UInt64)range->end;
		return SmileUnboxedInteger64_From((UInt32)(start ^ (start >> 32)) ^ (UInt32)(end ^ (end >> 32)));
	}

	return SmileUnboxedInteger64_From((PtrInt)argv[0].obj ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Construction

SMILE_EXTERNAL_FUNCTION(Of)
{
	Int i = 0;
	Int64 start, end, stepping;

	if (argv[i].obj == (SmileObject)param)
		i++;

	if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_INTEGER64)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_integer64TypeError, "First", "of"));
	start = argv[i++].unboxed.i64;

	if (i >= argc || SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_INTEGER64)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_integer64TypeError, "Second", "of"));
	end = argv[i++].unboxed.i64;

	if (i < argc) {
		if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_INTEGER64)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_integer64TypeError, "Third", "of"));
		stepping = argv[i++].unboxed.i64;
	}
	else stepping = end >= start ? +1 : -1;

	if (i != argc)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_argCountError, "of"));

	return SmileArg_From((SmileObject)SmileInteger64Range_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Step)
{
	if (argv[1].unboxed.i64 == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument to 'Integer64Range.step' cannot be zero."));

	return SmileArg_From((SmileObject)SmileInteger64Range_Create(
		((SmileInteger64Range)argv[0].obj)->start,
		((SmileInteger64Range)argv[0].obj)->end,
		argv[1].unboxed.i64)
	);
}

SMILE_EXTERNAL_FUNCTION(Reverse)
{
	return SmileArg_From((SmileObject)SmileInteger64Range_Create(
		((SmileInteger64Range)argv[0].obj)->end,
		((SmileInteger64Range)argv[0].obj)->start,
		-((SmileInteger64Range)argv[0].obj)->stepping)
	);
}

//-------------------------------------------------------------------------------------------------

typedef struct EachInfoStruct {
	SmileInteger64Range range;
	SmileFunction function;
	Int64 current;
	Int64 step;
	Int64 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *EachInfo;

static Int EachStateMachine(ClosureStateMachine closure)
{
	EachInfo eachInfo = (EachInfo)closure->state;

	// If we've run out of values, we're done.
	if (eachInfo->done) {
		Closure_Pop(closure);	// Pop the previous return value
		Closure_PushBoxed(closure, eachInfo->range);	// and push 'range' as the new return value.
		return -1;
	}

	// Set up to call the user's function with the next value.
	Closure_Pop(closure);
	Closure_PushBoxed(closure, eachInfo->function);
	Closure_PushUnboxedInt64(closure, eachInfo->current);
	if (eachInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, eachInfo->index);

	// Move to the next spot.
	if (eachInfo->up) {
		if (eachInfo->end - eachInfo->step >= eachInfo->current)
			eachInfo->current += eachInfo->step;
		else eachInfo->done = True;
	}
	else {
		if (eachInfo->end - eachInfo->step <= eachInfo->current)
			eachInfo->current += eachInfo->step;
		else eachInfo->done = True;
	}
	eachInfo->index++;

	return eachInfo->numArgs;
}

SMILE_EXTERNAL_FUNCTION(Each)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileInteger64Range range = (SmileInteger64Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	EachInfo eachInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(EachStateMachine, EachStateMachine);

	eachInfo = (EachInfo)closure->state;
	eachInfo->range = range;
	eachInfo->function = function;
	eachInfo->index = 0;
	eachInfo->current = range->start;
	eachInfo->step = range->stepping;
	eachInfo->end = range->end;
	eachInfo->up = range->end >= range->start;
	eachInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	eachInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	Closure_PushBoxed(closure, NullObject);	// Initial "return" value from 'each'.

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct MapInfoStruct {
	SmileFunction function;
	SmileList resultHead, resultTail;
	Int64 current;
	Int64 step;
	Int64 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *MapInfo;

static Int MapStart(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedInt64(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int MapBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileArg_Box(fnResult));

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (loopInfo->end - loopInfo->step >= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	else {
		if (loopInfo->end - loopInfo->step <= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return MapStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Map)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileInteger64Range range = (SmileInteger64Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	MapInfo loopInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(MapStart, MapBody);

	loopInfo = (MapInfo)closure->state;
	loopInfo->resultHead = loopInfo->resultTail = NullList;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = range->end >= range->start;
	loopInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct WhereInfoStruct {
	SmileFunction function;
	SmileList resultHead, resultTail;
	Int64 current;
	Int64 step;
	Int64 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *WhereInfo;

static Int WhereStart(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedInt64(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int WhereBody(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, keep this list element.
	if (booleanResult) {
		LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileInteger64_Create(loopInfo->current));
	}

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (loopInfo->end - loopInfo->step >= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	else {
		if (loopInfo->end - loopInfo->step <= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return WhereStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Where)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileInteger64Range range = (SmileInteger64Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	WhereInfo loopInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(WhereStart, WhereBody);

	loopInfo = (WhereInfo)closure->state;
	loopInfo->resultHead = loopInfo->resultTail = NullList;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = range->end >= range->start;
	loopInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct CountInfoStruct {
	SmileFunction function;
	Int64 count;
	Int64 current;
	Int64 step;
	Int64 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *CountInfo;

static Int CountStart(ClosureStateMachine closure)
{
	register CountInfo loopInfo = (CountInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushUnboxedInt64(closure, loopInfo->count);	// Push 'count' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedInt64(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int CountBody(ClosureStateMachine closure)
{
	register CountInfo loopInfo = (CountInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, keep this element.
	if (booleanResult)
		loopInfo->count++;

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (loopInfo->end - loopInfo->step >= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	else {
		if (loopInfo->end - loopInfo->step <= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return CountStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Count)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileInteger64Range range = (SmileInteger64Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	CountInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, they just want to find out how many values this range describes.
	if (argc == 1) {
		if (range->end >= range->start) {
			if (range->stepping <= 0) return SmileUnboxedInteger64_From(0);
			return SmileUnboxedInteger64_From((range->end - range->start) / range->stepping + 1);
		}
		else {
			if (range->stepping >= 0) return SmileUnboxedInteger64_From(0);
			return SmileUnboxedInteger64_From((range->start - range->end) / -range->stepping + 1);
		}
	}

	function = (SmileFunction)argv[1].obj;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(CountStart, CountBody);

	loopInfo = (CountInfo)closure->state;
	loopInfo->count = 0;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = range->end >= range->start;
	loopInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct FirstInfoStruct {
	SmileFunction function;
	Int64 current;
	Int64 step;
	Int64 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *FirstInfo;

static Int FirstStart(ClosureStateMachine closure)
{
	register FirstInfo loopInfo = (FirstInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushBoxed(closure, NullObject);	// Push 'null' as the output, since we didn't find the answer.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedInt64(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int FirstBody(ClosureStateMachine closure)
{
	register FirstInfo loopInfo = (FirstInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, we found the result!
	if (booleanResult) {
		Closure_PushUnboxedInt64(closure, loopInfo->current);
		return -1;
	}

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (loopInfo->end - loopInfo->step >= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	else {
		if (loopInfo->end - loopInfo->step <= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return FirstStart(closure);
}

SMILE_EXTERNAL_FUNCTION(First)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileInteger64Range range = (SmileInteger64Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	FirstInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, this is just a synonym for the 'start' property.
	if (argc == 1)
		return SmileUnboxedInteger64_From(range->start);

	function = (SmileFunction)argv[1].obj;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(FirstStart, FirstBody);

	loopInfo = (FirstInfo)closure->state;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = range->end >= range->start;
	loopInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct IndexOfInfoStruct {
	SmileFunction function;
	Int64 current;
	Int64 step;
	Int64 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *IndexOfInfo;

static Int IndexOfStart(ClosureStateMachine closure)
{
	register IndexOfInfo loopInfo = (IndexOfInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushBoxed(closure, NullObject);	// Didn't find it.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedInt64(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int IndexOfBody(ClosureStateMachine closure)
{
	register IndexOfInfo loopInfo = (IndexOfInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, we found its position.
	if (booleanResult) {
		Closure_PushUnboxedInt64(closure, loopInfo->index);
		return -1;
	}

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (loopInfo->end - loopInfo->step >= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	else {
		if (loopInfo->end - loopInfo->step <= loopInfo->current)
			loopInfo->current += loopInfo->step;
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return IndexOfStart(closure);
}

SMILE_EXTERNAL_FUNCTION(IndexOf)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileInteger64Range range = (SmileInteger64Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	IndexOfInfo loopInfo;
	ClosureStateMachine closure;

	function = (SmileFunction)argv[1].obj;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(IndexOfStart, IndexOfBody);

	loopInfo = (IndexOfInfo)closure->state;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = range->end >= range->start;
	loopInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

void SmileInteger64Range_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "range start end", ARG_CHECK_MIN | ARG_CHECK_MAX, 2, 4, 0, NULL);

	SetupFunction("step", Step, (void *)base, "range stepping", ARG_CHECK_EXACT, 2, 2, 2, _integer64Checks);
	SetupFunction("reverse", Reverse, NULL, "range", ARG_CHECK_EXACT, 1, 1, 1, _integer64Checks);

	SetupFunction("each", Each, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("map", Map, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("where", Where, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);

	SetupFunction("count", Count, NULL, "range fn", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 1, 2, 2, _eachChecks);
	SetupFunction("first", First, NULL, "range fn", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 1, 2, 2, _eachChecks);
	SetupFunction("index-of", IndexOf, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
}
