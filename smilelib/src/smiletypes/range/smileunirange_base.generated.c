// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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

#include <math.h>
#include <smile/numeric/float64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>
#include <smile/eval/eval.h>

#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/range/smileunirange.h>

SMILE_IGNORE_UNUSED_VARIABLES

#define Modulus(a, b) (a % b)

typedef enum {
	FindMode_First,
	FindMode_IndexOf,
	FindMode_Count,
	FindMode_Where,
	FindMode_Any,
	FindMode_All,
} FindMode;

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Uni'");
#if 0
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
#endif
STATIC_STRING(_uniTypeError, "%s argument to '%s' must be of type 'Uni'");
STATIC_STRING(_argCountError, "Too many arguments to 'UniRange.%s'");

static Byte _eachChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNIRANGE,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

static Byte _findChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNIRANGE,
	0, 0,
};

static Byte _uniChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNIRANGE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
};

static Byte _stepChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNIRANGE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNIRANGE)
		return SmileUnboxedBool_From(True);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNIRANGE) {
		SmileUniRange obj = (SmileUniRange)argv[0].obj;
		return SmileUnboxedInteger64_From((Int64)((Int64)obj->end - (Int64)obj->start));
	}

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(unirange, "UniRange");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNIRANGE) {
		String string;
		SmileUniRange obj = (SmileUniRange)argv[0].obj;

#if 0
		UInt32 numericBase;

		if (argc == 2) {
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNI)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)argv[1].unboxed.uni;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;
#endif

		string = 	((obj->end >= obj->start && obj->stepping != +1
		|| obj->end < obj->start && obj->stepping != -1)
		? String_Format("'\\u%X'..'\\u%X' step %ld", obj->start, obj->end, obj->stepping)
		: String_Format("'\\u%X'..'\\u%X'", obj->start, obj->end))
;

		return SmileArg_From((SmileObject)string);
	}

	return SmileArg_From((SmileObject)unirange);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNIRANGE) {
		SmileUniRange range = (SmileUniRange)argv[0].obj;
		UInt32 result;
		UInt32 start = range->start;
		UInt32 end = range->end;
		UInt32 stepping = (UInt32)(UInt64)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)(start ^ (end << 8) ^ (stepping << 16)));

		return SmileUnboxedInteger64_From(result);
	}

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)argv[0].obj));
}

//-------------------------------------------------------------------------------------------------
// Construction

SMILE_EXTERNAL_FUNCTION(Of)
{
	Int i = 0;
	UInt32 start, end;
	Int64 stepping;

	if (argv[i].obj == (SmileObject)param)
		i++;

	if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_UNI)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_uniTypeError, "First", "of"));
	start = argv[i++].unboxed.uni;

	if (i >= argc || SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_UNI)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_uniTypeError, "Second", "of"));
	end = argv[i++].unboxed.uni;

	if (i < argc) {
		if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_UNI)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_uniTypeError, "Third", "of"));
		stepping = argv[i++].unboxed.uni;
	}
	else stepping = end >= start ? (UInt32)+1 : (UInt32)-1;

	if (i != argc)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_argCountError, "of"));

	return SmileArg_From((SmileObject)SmileUniRange_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Step)
{
	Int64 stepping = (Int64)argv[1].unboxed.i64;
	UInt32 start = ((SmileUniRange)argv[0].obj)->start;
	UInt32 end = ((SmileUniRange)argv[0].obj)->end;

	if (stepping == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument to 'UniRange.step' cannot be zero."));
	if (start < end && stepping < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot apply a negative step to a forward range."));
	if (end < start && stepping > 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot apply a positive step to a reverse range."));

	return SmileArg_From((SmileObject)SmileUniRange_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Reverse)
{
	return SmileArg_From((SmileObject)SmileUniRange_Create(
		((SmileUniRange)argv[0].obj)->end,
		((SmileUniRange)argv[0].obj)->start,
		-((SmileUniRange)argv[0].obj)->stepping)
	);
}

//-------------------------------------------------------------------------------------------------

static SmileArg FindFixedValue(SmileUniRange range, SmileArg valueArg, FindMode fixedMode)
{
	UInt32 current = range->start;
	Int64 step = range->stepping;
	UInt32 end = range->end;
	Bool up = range->end > range->start;
	UInt32 value;

	if (!up) {
		// Handle the downward case by swapping endpoints and directions.
		UInt32 temp;
		step = -step;
		temp = current;
		current = end;
		end = temp;
	}

	// An Uni range cannot contain non-Uni values, so only test if the input value was of a sane type.
	if (SMILE_KIND(valueArg.obj) == SMILE_KIND_UNBOXED_UNI) {
		value = valueArg.unboxed.uni;

		// Use a shortcut for a step of +1 on a forward range.
		if (step == 1) {
			if (value <= end) {
				// Found it.
				switch (fixedMode) {
					case FindMode_IndexOf:
						return SmileUnboxedInteger64_From((Int64)(value - current));
					case FindMode_First:
					case FindMode_Where:
						return SmileUnboxedUni_From(value);
					case FindMode_Count:
						return SmileUnboxedInteger64_From(1);
					case FindMode_Any:
						return SmileUnboxedBool_From(True);
					case FindMode_All:
						return SmileUnboxedBool_From(current == end);	// 'All' can only be true if there's one value total.
				}
			}
		}
		else {
			// General case:  Do some math and see if the target is something we'd hit by iterating.
			UInt32 delta = (UInt32)(value - current);
			if (Modulus(delta, (UInt32)step) == 0) {
				switch (fixedMode) {
					case FindMode_IndexOf:
						return SmileUnboxedInteger64_From((Int64)(delta / (UInt32)step));
					case FindMode_First:
					case FindMode_Where:
						return SmileUnboxedUni_From(value);
					case FindMode_Count:
						return SmileUnboxedInteger64_From(1);
					case FindMode_Any:
						return SmileUnboxedBool_From(True);
					case FindMode_All:
						return SmileUnboxedBool_From(current == end);	// 'All' can only be true if there's one value total.
				}
			}
		}
	}

	// Didn't find it.
	switch (fixedMode) {
		case FindMode_Count:
			return SmileUnboxedInteger64_From(0);
		case FindMode_Any:
		case FindMode_All:
			return SmileUnboxedBool_From(False);
		default:
			return SmileArg_From(NullObject);
	}
}

//-------------------------------------------------------------------------------------------------

typedef struct EachInfoUniStruct {
	SmileUniRange range;
	SmileFunction function;
	UInt32 current;
	Int64 step;
	UInt32 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *EachInfoUni;

static Int EachStateMachine(ClosureStateMachine closure)
{
	EachInfoUni eachInfo = (EachInfoUni)closure->state;

	// If we've run out of values, we're done.
	if (eachInfo->done) {
		Closure_Pop(closure);	// Pop the previous return value
		Closure_PushBoxed(closure, eachInfo->range);	// and push 'range' as the new return value.
		return -1;
	}

	// Set up to call the user's function with the next value.
	Closure_Pop(closure);
	Closure_PushBoxed(closure, eachInfo->function);
	if (eachInfo->numArgs > 0) {
		Closure_PushUnboxedUni(closure, eachInfo->current);
		if (eachInfo->numArgs > 1)
			Closure_PushUnboxedInt64(closure, eachInfo->index);
	}

	// Move to the next spot.
	if (eachInfo->up) {
		if ((Int64)eachInfo->end - eachInfo->step >= (Int64)eachInfo->current)
			eachInfo->current = (UInt32)((Int64)eachInfo->current + eachInfo->step);
		else eachInfo->done = True;
	}
	else {
		if ((Int64)eachInfo->end - eachInfo->step <= (Int64)eachInfo->current)
			eachInfo->current = (UInt32)((Int64)eachInfo->current + eachInfo->step);
		else eachInfo->done = True;
	}
	eachInfo->index++;

	return eachInfo->numArgs;
}

SMILE_EXTERNAL_FUNCTION(Each)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileUniRange range = (SmileUniRange)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	EachInfoUni eachInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(EachStateMachine, EachStateMachine);

	eachInfo = (EachInfoUni)closure->state;
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

typedef struct MapInfoUniStruct {
	SmileFunction function;
	SmileList resultHead, resultTail;
	UInt32 current;
	Int64 step;
	UInt32 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *MapInfoUni;

static Int MapStart(ClosureStateMachine closure)
{
	register MapInfoUni loopInfo = (MapInfoUni)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedUni(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int MapBody(ClosureStateMachine closure)
{
	register MapInfoUni loopInfo = (MapInfoUni)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileArg_Box(fnResult));

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if ((Int64)loopInfo->end - loopInfo->step >= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if ((Int64)loopInfo->end - loopInfo->step <= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return MapStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Map)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileUniRange range = (SmileUniRange)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	MapInfoUni loopInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(MapStart, MapBody);

	loopInfo = (MapInfoUni)closure->state;
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
	UInt32 current;
	Int64 step;
	UInt32 end;
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
	Closure_PushUnboxedUni(closure, loopInfo->current);
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
		LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileUni_Create(loopInfo->current));
	}

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if ((Int64)loopInfo->end - loopInfo->step >= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if ((Int64)loopInfo->end - loopInfo->step <= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return WhereStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Where)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileUniRange range = (SmileUniRange)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	WhereInfo loopInfo;
	ClosureStateMachine closure;

	if (SMILE_KIND(function) != SMILE_KIND_FUNCTION)
		return FindFixedValue(range, argv[1], FindMode_Count);

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
	Int64 index;
	Int64 count;
	UInt32 current;
	UInt32 end;
	Int64 step;
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
	Closure_PushUnboxedUni(closure, loopInfo->current);
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
		if ((Int64)loopInfo->end - loopInfo->step >= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if ((Int64)loopInfo->end - loopInfo->step <= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return CountStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Count)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileUniRange range = (SmileUniRange)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	CountInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, they just want to find out how many values this range describes.
	if (argc == 1) {
		if (range->end >= range->start) {
			if (range->stepping <= 0) return SmileUnboxedUni_From(0);
			return SmileUnboxedInteger64_From((Int64)(range->end - range->start) / range->stepping + 1);
		}
		else {
			if (range->stepping >= 0) return SmileUnboxedUni_From(0);
			return SmileUnboxedInteger64_From((Int64)(range->start - range->end) / -range->stepping + 1);
		}
	}

	function = (SmileFunction)argv[1].obj;

	if (SMILE_KIND(function) != SMILE_KIND_FUNCTION)
		return FindFixedValue(range, argv[1], FindMode_Count);

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

typedef struct FindInfoStruct {
	SmileFunction function;
	UInt32 current;
	Int64 step;
	UInt32 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
	Byte findMode;
} *FindInfo;

static Int FindStart(ClosureStateMachine closure)
{
	register FindInfo loopInfo = (FindInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		switch (loopInfo->findMode) {
			case FindMode_Count:
				Closure_PushUnboxedInt64(closure, 0);
				break;
			case FindMode_Any:
				Closure_PushUnboxedBool(closure, False);
				break;
			default:
				Closure_PushBoxed(closure, NullObject);
				break;
		}
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedUni(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int FindBody(ClosureStateMachine closure)
{
	register FindInfo loopInfo = (FindInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, we found the result!
	if (booleanResult) {
		switch (loopInfo->findMode) {
			case FindMode_First:
				Closure_PushUnboxedUni(closure, loopInfo->current);
				break;
			case FindMode_IndexOf:
				Closure_PushUnboxedInt64(closure, loopInfo->index);
				break;
			case FindMode_Count:
				Closure_PushUnboxedInt64(closure, 1);
				break;
			case FindMode_Any:
				Closure_PushUnboxedBool(closure, True);
				break;
			default:
				Closure_PushBoxed(closure, NullObject);
				break;
		}
		return -1;
	}

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if ((Int64)loopInfo->end - loopInfo->step >= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if ((Int64)loopInfo->end - loopInfo->step <= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return FindStart(closure);
}

SMILE_EXTERNAL_FUNCTION(First)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileUniRange range = (SmileUniRange)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	FindInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, this is just a synonym for the 'start' property.
	if (argc == 1)
		return SmileUnboxedUni_From(range->start);

	function = (SmileFunction)argv[1].obj;

	if (SMILE_KIND(function) != SMILE_KIND_FUNCTION)
		return FindFixedValue(range, argv[1], FindMode_First);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(FindStart, FindBody);

	loopInfo = (FindInfo)closure->state;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = range->end >= range->start;
	loopInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	loopInfo->findMode = FindMode_First;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(IndexOf)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileUniRange range = (SmileUniRange)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	FindInfo loopInfo;
	ClosureStateMachine closure;

	function = (SmileFunction)argv[1].obj;

	if (SMILE_KIND(function) != SMILE_KIND_FUNCTION)
		return FindFixedValue(range, argv[1], FindMode_IndexOf);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(FindStart, FindBody);

	loopInfo = (FindInfo)closure->state;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = range->end >= range->start;
	loopInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	loopInfo->findMode = FindMode_IndexOf;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(Any)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileUniRange range = (SmileUniRange)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	FindInfo loopInfo;
	ClosureStateMachine closure;

	function = (SmileFunction)argv[1].obj;

	if (SMILE_KIND(function) != SMILE_KIND_FUNCTION)
		return FindFixedValue(range, argv[1], FindMode_Any);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(FindStart, FindBody);

	loopInfo = (FindInfo)closure->state;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = range->end >= range->start;
	loopInfo->done = range->end >= range->start ? range->stepping <= 0 : range->stepping >= 0;
	loopInfo->findMode = FindMode_Any;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct AllInfoStruct {
	SmileFunction function;
	UInt32 current;
	Int64 step;
	UInt32 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *AllInfo;

static Int AllStart(ClosureStateMachine closure)
{
	register AllInfo loopInfo = (AllInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values to test, we're done.
	if (loopInfo->done) {
		Closure_PushUnboxedBool(closure, True);
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedUni(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int AllBody(ClosureStateMachine closure)
{
	register AllInfo loopInfo = (AllInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's falsy, this element fails, and so does the set.
	if (!booleanResult) {
		Closure_PushUnboxedBool(closure, False);
		return -1;
	}

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if ((Int64)loopInfo->end - loopInfo->step >= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if ((Int64)loopInfo->end - loopInfo->step <= (Int64)loopInfo->current)
			loopInfo->current = (UInt32)((Int64)loopInfo->current + loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return AllStart(closure);
}

SMILE_EXTERNAL_FUNCTION(All)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileUniRange range = (SmileUniRange)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	AllInfo loopInfo;
	ClosureStateMachine closure;

	function = (SmileFunction)argv[1].obj;

	if (SMILE_KIND(function) != SMILE_KIND_FUNCTION)
		return FindFixedValue(range, argv[1], FindMode_All);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(AllStart, AllBody);

	loopInfo = (AllInfo)closure->state;
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

void SmileUniRange_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
#if 0
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
#else
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
#endif
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "range start end", ARG_CHECK_MIN | ARG_CHECK_MAX, 3, 4, 0, NULL);

	SetupFunction("step", Step, (void *)base, "range stepping", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stepChecks);
	SetupFunction("reverse", Reverse, NULL, "range", ARG_CHECK_EXACT, 1, 1, 1, _uniChecks);

	SetupFunction("each", Each, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("map", Map, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupSynonym("map", "select");
	SetupSynonym("map", "project");
	SetupFunction("where", Where, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _findChecks);
	SetupSynonym("where", "filter");

	SetupFunction("count", Count, NULL, "range fn", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 1, 2, 2, _findChecks);
	SetupFunction("first", First, NULL, "range fn", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 1, 2, 2, _findChecks);
	SetupFunction("index-of", IndexOf, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _findChecks);
	SetupFunction("any?", Any, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _findChecks);
	SetupFunction("all?", All, NULL, "range fn", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _findChecks);
	SetupSynonym("any?", "contains?");
}
