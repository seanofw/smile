// ===================================================
//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!
// ===================================================

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

#include <smile/numeric/float64.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/range/smileinteger16range.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>
#include <smile/eval/eval.h>
#include <math.h>

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

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Integer16'");
#if 1
STATIC_STRING(_numericBaseError, "Valid numeric base must be in the range of 2..36");
#endif
STATIC_STRING(_integer16TypeError, "%s argument to '%s' must be of type 'Integer16'");
STATIC_STRING(_argCountError, "Too many arguments to 'Integer16Range.%s'");

static Byte _eachChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16RANGE,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

static Byte _findChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16RANGE,
	0, 0,
};

static Byte _integer16Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_INTEGER16RANGE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER16,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_INTEGER16RANGE)
		return SmileUnboxedBool_From(True);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_INTEGER16RANGE) {
		SmileInteger16Range obj = (SmileInteger16Range)argv[0].obj;
		return SmileUnboxedInteger64_From((Int64)(obj->end - obj->start));
	}

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(integer16range, "Integer16Range");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_INTEGER16RANGE) {
		String string;
		SmileInteger16Range obj = (SmileInteger16Range)argv[0].obj;

#if 1
		Int16 numericBase;

		if (argc == 2) {
			if (SMILE_KIND(argv[1].obj) != SMILE_KIND_INTEGER16)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _stringTypeError);
			numericBase = (Int)argv[1].unboxed.i16;
			if (numericBase < 2 || numericBase > 36)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _numericBaseError);
		}
		else numericBase = 10;
#endif

		string = 		((obj->end >= obj->start && obj->stepping != +1
			|| obj->end < obj->start && obj->stepping != -1)
			? String_Format("%S..%S step %S",
				String_CreateFromInteger(obj->start, (Int)numericBase, False),
				String_CreateFromInteger(obj->end, (Int)numericBase, False),
				String_CreateFromInteger(obj->stepping, (Int)numericBase, False))
			: String_Format("%S..%S",
				String_CreateFromInteger(obj->start, (Int)numericBase, False),
				String_CreateFromInteger(obj->end, (Int)numericBase, False)))
;

		return SmileArg_From((SmileObject)string);
	}

	return SmileArg_From((SmileObject)integer16range);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_INTEGER16RANGE) {
		SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
		UInt32 result;
		UInt16 start = (UInt16)range->start;
		UInt16 end = (UInt16)range->end;
		UInt16 stepping = (UInt16)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)((UInt32)start ^ (UInt32)(end << 8) ^ (UInt32)(stepping << 16)));

		return SmileUnboxedInteger64_From(result);
	}

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)argv[0].obj));
}

//-------------------------------------------------------------------------------------------------
// Construction

SMILE_EXTERNAL_FUNCTION(Of)
{
	Int i = 0;
	Int16 start, end, stepping;

	if (argv[i].obj == (SmileObject)param)
		i++;

	if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_INTEGER16)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_integer16TypeError, "First", "of"));
	start = argv[i++].unboxed.i16;

	if (i >= argc || SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_INTEGER16)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_integer16TypeError, "Second", "of"));
	end = argv[i++].unboxed.i16;

	if (i < argc) {
		if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_INTEGER16)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_integer16TypeError, "Third", "of"));
		stepping = argv[i++].unboxed.i16;
	}
	else stepping = end >= start ? (Int16)+1 : (Int16)-1;

	if (i != argc)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_argCountError, "of"));

	return SmileArg_From((SmileObject)SmileInteger16Range_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Step)
{
	Int16 stepping = argv[1].unboxed.i16;
	Int16 start = ((SmileInteger16Range)argv[0].obj)->start;
	Int16 end = ((SmileInteger16Range)argv[0].obj)->end;

	if (stepping == 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument to 'Integer16Range.step' cannot be zero."));
	if (start < end && stepping < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot apply a negative step to a forward range."));
	if (end < start && stepping > 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot apply a positive step to a reverse range."));

	return SmileArg_From((SmileObject)SmileInteger16Range_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Reverse)
{
	return SmileArg_From((SmileObject)SmileInteger16Range_Create(
		((SmileInteger16Range)argv[0].obj)->end,
		((SmileInteger16Range)argv[0].obj)->start,
		-((SmileInteger16Range)argv[0].obj)->stepping)
	);
}

//-------------------------------------------------------------------------------------------------

static SmileArg FindFixedValue(SmileInteger16Range range, SmileArg valueArg, FindMode fixedMode)
{
	Int16 current = range->start;
	Int16 step = range->stepping;
	Int16 end = range->end;
	Bool up = range->end > range->start;
	Int16 value;

	if (!up) {
		// Handle the downward case by swapping endpoints and directions.
		Int16 temp;
		step = -step;
		temp = current;
		current = end;
		end = temp;
	}

	// An Integer16 range cannot contain non-Integer16 values, so only test if the input value was of a sane type.
	if (SMILE_KIND(valueArg.obj) == SMILE_KIND_UNBOXED_INTEGER16) {
		value = valueArg.unboxed.i16;

		// Use a shortcut for a step of +1 on a forward range.
		if (step == 1) {
			if (value <= end) {
				// Found it.
				switch (fixedMode) {
					case FindMode_IndexOf:
						return SmileUnboxedInteger64_From((Int64)(value - current));
					case FindMode_First:
					case FindMode_Where:
						return SmileUnboxedInteger16_From(value);
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
			UInt16 delta = (UInt16)(value - current);
			if (Modulus(delta, (UInt16)step) == 0) {
				switch (fixedMode) {
					case FindMode_IndexOf:
						return SmileUnboxedInteger64_From((Int64)(delta / (UInt16)step));
					case FindMode_First:
					case FindMode_Where:
						return SmileUnboxedInteger16_From(value);
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

typedef struct EachInfoInteger16Struct {
	SmileInteger16Range range;
	SmileFunction function;
	Int16 current;
	Int16 step;
	Int16 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *EachInfoInteger16;

static Int EachStateMachine(ClosureStateMachine closure)
{
	EachInfoInteger16 eachInfo = (EachInfoInteger16)closure->state;

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
		Closure_PushUnboxedInt16(closure, eachInfo->current);
		if (eachInfo->numArgs > 1)
			Closure_PushUnboxedInt64(closure, eachInfo->index);
	}

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
	SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	EachInfoInteger16 eachInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(EachStateMachine, EachStateMachine);

	eachInfo = (EachInfoInteger16)closure->state;
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

typedef struct MapInfoInteger16Struct {
	SmileFunction function;
	SmileList resultHead, resultTail;
	Int16 current;
	Int16 step;
	Int16 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *MapInfoInteger16;

static Int MapStart(ClosureStateMachine closure)
{
	register MapInfoInteger16 loopInfo = (MapInfoInteger16)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedInt16(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int MapBody(ClosureStateMachine closure)
{
	register MapInfoInteger16 loopInfo = (MapInfoInteger16)closure->state;

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
	SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	MapInfoInteger16 loopInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(MapStart, MapBody);

	loopInfo = (MapInfoInteger16)closure->state;
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
	Int16 current;
	Int16 step;
	Int16 end;
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
	Closure_PushUnboxedInt16(closure, loopInfo->current);
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
		LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileInteger16_Create(loopInfo->current));
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
	SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
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
	Int16 count;
	Int16 current;
	Int16 step;
	Int16 end;
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
		Closure_PushUnboxedInt16(closure, loopInfo->count);	// Push 'count' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedInt16(closure, loopInfo->current);
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
	SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	CountInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, they just want to find out how many values this range describes.
	if (argc == 1) {
		if (range->end >= range->start) {
			if (range->stepping <= 0) return SmileUnboxedInteger16_From(0);
			return SmileUnboxedInteger16_From((range->end - range->start) / range->stepping + 1);
		}
		else {
			if (range->stepping >= 0) return SmileUnboxedInteger16_From(0);
			return SmileUnboxedInteger16_From((range->start - range->end) / -range->stepping + 1);
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
	Int16 current;
	Int16 step;
	Int16 end;
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
	Closure_PushUnboxedInt16(closure, loopInfo->current);
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
				Closure_PushUnboxedInt16(closure, loopInfo->current);
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

	return FindStart(closure);
}

SMILE_EXTERNAL_FUNCTION(First)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	FindInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, this is just a synonym for the 'start' property.
	if (argc == 1)
		return SmileUnboxedInteger16_From(range->start);

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
	SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
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
	SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
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
	Int16 current;
	Int16 step;
	Int16 end;
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
	Closure_PushUnboxedInt16(closure, loopInfo->current);
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

	return AllStart(closure);
}

SMILE_EXTERNAL_FUNCTION(All)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileInteger16Range range = (SmileInteger16Range)argv[0].obj;
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

void SmileInteger16Range_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
#if 1
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
#else
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
#endif
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "range start end", ARG_CHECK_MIN | ARG_CHECK_MAX, 3, 4, 0, NULL);

	SetupFunction("step", Step, (void *)base, "range stepping", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _integer16Checks);
	SetupFunction("reverse", Reverse, NULL, "range", ARG_CHECK_EXACT, 1, 1, 1, _integer16Checks);

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
