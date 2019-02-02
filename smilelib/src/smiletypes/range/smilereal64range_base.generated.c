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

#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/range/smilereal64range.h>

SMILE_IGNORE_UNUSED_VARIABLES

typedef enum {
	FindMode_First,
	FindMode_IndexOf,
	FindMode_Count,
	FindMode_Where,
	FindMode_Any,
	FindMode_All,
} FindMode;

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Real64'");
STATIC_STRING(_real64TypeError, "%s argument to '%s' must be of type 'Real64'");
STATIC_STRING(_argCountError, "Too many arguments to 'Real64Range.%s'");

static Byte _eachChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_REAL64RANGE,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

static Byte _findChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_REAL64RANGE,
	0, 0,
};

static Byte _real64Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_REAL64RANGE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
};

static Byte _stepChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_REAL64RANGE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL64,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_REAL64RANGE)
		return SmileUnboxedBool_From(True);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_REAL64RANGE) {
		SmileReal64Range obj = (SmileReal64Range)argv[0].obj;
		return SmileUnboxedInteger64_From(Real64_ToInt64(Real64_Sub(obj->end, obj->start)));
	}

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(real64range, "Real64Range");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_REAL64RANGE) {
		String string;
		SmileReal64Range obj = (SmileReal64Range)argv[0].obj;

		string = ((Real64_Ge(obj->end, obj->start) && Real64_Ne(obj->stepping, Real64_One)
	|| Real64_Lt(obj->end, obj->start) && Real64_Ne(obj->stepping, Real64_One))
	? String_Format("(%S)..(%S) step %S",
		Real64_ToStringEx(obj->start, 0, 0, False),
		Real64_ToStringEx(obj->end, 0, 0, False),
		Real64_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("(%S)..(%S)",
		Real64_ToStringEx(obj->start, 0, 0, False),
		Real64_ToStringEx(obj->end, 0, 0, False)))
;

		return SmileArg_From((SmileObject)string);
	}

	return SmileArg_From((SmileObject)real64range);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_REAL64RANGE) {
		SmileReal64Range range = (SmileReal64Range)argv[0].obj;
		UInt32 result;
		UInt64 start = *(UInt64 *)&range->start;
		UInt64 end = *(UInt64 *)&range->end;
		UInt64 stepping = *(UInt64 *)&range->stepping;
		UInt64 hash = start ^ end ^ stepping;
		result = Smile_ApplyHashOracle((Int32)(hash ^ (hash >> 32)));

		return SmileUnboxedInteger64_From(result);
	}

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)argv[0].obj));
}

//-------------------------------------------------------------------------------------------------
// Construction

SMILE_EXTERNAL_FUNCTION(Of)
{
	Int i = 0;
	Real64 start, end;
	Real64 stepping;

	if (argv[i].obj == (SmileObject)param)
		i++;

	if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_REAL64)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_real64TypeError, "First", "of"));
	start = argv[i++].unboxed.r64;

	if (i >= argc || SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_REAL64)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_real64TypeError, "Second", "of"));
	end = argv[i++].unboxed.r64;

	if (i < argc) {
		if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_REAL64)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_real64TypeError, "Third", "of"));
		stepping = argv[i++].unboxed.r64;
	}
	else stepping = Real64_Ge(end, start) ? Real64_One : Real64_NegOne;

	if (i != argc)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_argCountError, "of"));

	return SmileArg_From((SmileObject)SmileReal64Range_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Step)
{
	Real64 stepping = argv[1].unboxed.r64;
	Real64 start = ((SmileReal64Range)argv[0].obj)->start;
	Real64 end = ((SmileReal64Range)argv[0].obj)->end;

	if (Real64_IsZero(stepping))
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument to 'Real64Range.step' cannot be zero."));
	if (Real64_Lt(start, end) && Real64_IsNeg(stepping))
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot apply a negative step to a forward range."));
	if (Real64_Lt(end, start) && Real64_IsPos(stepping))
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot apply a positive step to a reverse range."));

	return SmileArg_From((SmileObject)SmileReal64Range_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Reverse)
{
	return SmileArg_From((SmileObject)SmileReal64Range_Create(
		((SmileReal64Range)argv[0].obj)->end,
		((SmileReal64Range)argv[0].obj)->start,
		Real64_Neg(((SmileReal64Range)argv[0].obj)->stepping)
	));
}

//-------------------------------------------------------------------------------------------------

static SmileArg FindFixedValue(SmileReal64Range range, SmileArg valueArg, FindMode fixedMode)
{
	Real64 current = range->start;
	Real64 step = range->stepping;
	Real64 end = range->end;
	Bool up = Real64_Gt(range->end, range->start);
	Real64 value;

	if (!up) {
		// Handle the downward case by swapping endpoints and directions.
		Real64 temp;
		step = Real64_Neg(step);
		temp = current;
		current = end;
		end = temp;
	}

	// An Real64 range cannot contain non-Real64 values, so only test if the input value was of a sane type.
	if (SMILE_KIND(valueArg.obj) == SMILE_KIND_UNBOXED_REAL64) {
		value = valueArg.unboxed.r64;

		// Use a shortcut for a step of +1 on a forward range.
		if (Real64_Eq(step, Real64_One)) {
			if (Real64_Le(value, end)) {
				// Found it.
				switch (fixedMode) {
					case FindMode_IndexOf:
						return SmileUnboxedInteger64_From((Int64)Real64_ToInt64(Real64_Sub(value, current)));
					case FindMode_First:
					case FindMode_Where:
						return SmileUnboxedReal64_From(value);
					case FindMode_Count:
						return SmileUnboxedInteger64_From(1);
					case FindMode_Any:
						return SmileUnboxedBool_From(True);
					case FindMode_All:
						return SmileUnboxedBool_From(Real64_Eq(current, end));	// 'All' can only be true if there's one value total.
				}
			}
		}
		else {
			// General case:  Do some math and see if the target is something we'd hit by iterating.
			Real64 delta = Real64_Sub(value, current);
			if (Real64_IsZero(Real64_Mod(delta, step))) {
				switch (fixedMode) {
					case FindMode_IndexOf:
						return SmileUnboxedInteger64_From(Real64_ToInt64(Real64_Div(delta, step)));
					case FindMode_First:
					case FindMode_Where:
						return SmileUnboxedReal64_From(value);
					case FindMode_Count:
						return SmileUnboxedInteger64_From(1);
					case FindMode_Any:
						return SmileUnboxedBool_From(True);
					case FindMode_All:
						return SmileUnboxedBool_From(Real64_Eq(current, end));	// 'All' can only be true if there's one value total.
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

typedef struct EachInfoReal64Struct {
	SmileReal64Range range;
	SmileFunction function;
	Real64 current;
	Real64 step;
	Real64 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *EachInfoReal64;

static Int EachStateMachine(ClosureStateMachine closure)
{
	EachInfoReal64 eachInfo = (EachInfoReal64)closure->state;

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
		Closure_PushUnboxedReal64(closure, eachInfo->current);
		if (eachInfo->numArgs > 1)
			Closure_PushUnboxedInt64(closure, eachInfo->index);
	}

	// Move to the next spot.
	if (eachInfo->up) {
		if (Real64_Ge(Real64_Sub(eachInfo->end, eachInfo->step), eachInfo->current))
			eachInfo->current = Real64_Add(eachInfo->current, eachInfo->step);
		else eachInfo->done = True;
	}
	else {
		if (Real64_Le(Real64_Sub(eachInfo->end, eachInfo->step), eachInfo->current))
			eachInfo->current = Real64_Add(eachInfo->current, eachInfo->step);
		else eachInfo->done = True;
	}
	eachInfo->index++;

	return eachInfo->numArgs;
}

SMILE_EXTERNAL_FUNCTION(Each)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal64Range range = (SmileReal64Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	EachInfoReal64 eachInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(EachStateMachine, EachStateMachine);

	eachInfo = (EachInfoReal64)closure->state;
	eachInfo->range = range;
	eachInfo->function = function;
	eachInfo->index = 0;
	eachInfo->current = range->start;
	eachInfo->step = range->stepping;
	eachInfo->end = range->end;
	eachInfo->up = Real64_Ge(range->end, range->start);
	eachInfo->done = eachInfo->up ? Real64_IsNegOrZero(range->stepping) : Real64_IsPosOrZero(range->stepping);
	eachInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	Closure_PushBoxed(closure, NullObject);	// Initial "return" value from 'each'.

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct MapInfoReal64Struct {
	SmileFunction function;
	SmileList resultHead, resultTail;
	Real64 current;
	Real64 step;
	Real64 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *MapInfoReal64;

static Int MapStart(ClosureStateMachine closure)
{
	register MapInfoReal64 loopInfo = (MapInfoReal64)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedReal64(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int MapBody(ClosureStateMachine closure)
{
	register MapInfoReal64 loopInfo = (MapInfoReal64)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileArg_Box(fnResult));

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (Real64_Ge(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real64_Le(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return MapStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Map)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal64Range range = (SmileReal64Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	MapInfoReal64 loopInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(MapStart, MapBody);

	loopInfo = (MapInfoReal64)closure->state;
	loopInfo->resultHead = loopInfo->resultTail = NullList;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = Real64_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real64_IsNegOrZero(range->stepping) : Real64_IsPosOrZero(range->stepping);
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct WhereInfoStruct {
	SmileFunction function;
	SmileList resultHead, resultTail;
	Real64 current;
	Real64 step;
	Real64 end;
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
	Closure_PushUnboxedReal64(closure, loopInfo->current);
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
		LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileReal64_Create(loopInfo->current));
	}

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (Real64_Ge(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real64_Le(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return WhereStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Where)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal64Range range = (SmileReal64Range)argv[0].obj;
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
	loopInfo->up = Real64_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real64_IsNegOrZero(range->stepping) : Real64_IsPosOrZero(range->stepping);
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct CountInfoStruct {
	SmileFunction function;
	Int64 index;
	Int64 count;
	Real64 current;
	Real64 end;
	Real64 step;
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
	Closure_PushUnboxedReal64(closure, loopInfo->current);
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
		if (Real64_Ge(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real64_Le(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return CountStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Count)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal64Range range = (SmileReal64Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	CountInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, they just want to find out how many values this range describes.
	if (argc == 1) {
		if (Real64_Ge(range->end, range->start)) {
			if (Real64_IsNegOrZero(range->stepping)) return SmileUnboxedReal64_From(Real64_Zero);
			return SmileUnboxedReal64_From(Real64_Div(
				Real64_Sub(range->end, range->start),
				Real64_Add(range->stepping, Real64_One))
			);
		}
		else {
			if (Real64_IsPosOrZero(range->stepping)) return SmileUnboxedReal64_From(Real64_Zero);
			return SmileUnboxedReal64_From(Real64_Div(
				Real64_Sub(range->start, range->end),
				Real64_Add(Real64_Neg(range->stepping), Real64_One))
			);
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
	loopInfo->up = Real64_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real64_IsNegOrZero(range->stepping) : Real64_IsPosOrZero(range->stepping);
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct FindInfoStruct {
	SmileFunction function;
	Real64 current;
	Real64 step;
	Real64 end;
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
	Closure_PushUnboxedReal64(closure, loopInfo->current);
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
				Closure_PushUnboxedReal64(closure, loopInfo->current);
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
		if (Real64_Ge(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real64_Le(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return FindStart(closure);
}

SMILE_EXTERNAL_FUNCTION(First)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal64Range range = (SmileReal64Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	FindInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, this is just a synonym for the 'start' property.
	if (argc == 1)
		return SmileUnboxedReal64_From(range->start);

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
	loopInfo->up = Real64_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real64_IsNegOrZero(range->stepping) : Real64_IsPosOrZero(range->stepping);
	loopInfo->findMode = FindMode_First;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(IndexOf)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal64Range range = (SmileReal64Range)argv[0].obj;
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
	loopInfo->up = Real64_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real64_IsNegOrZero(range->stepping) : Real64_IsPosOrZero(range->stepping);
	loopInfo->findMode = FindMode_IndexOf;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(Any)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal64Range range = (SmileReal64Range)argv[0].obj;
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
	loopInfo->up = Real64_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real64_IsNegOrZero(range->stepping) : Real64_IsPosOrZero(range->stepping);
	loopInfo->findMode = FindMode_Any;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct AllInfoStruct {
	SmileFunction function;
	Real64 current;
	Real64 step;
	Real64 end;
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
	Closure_PushUnboxedReal64(closure, loopInfo->current);
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
		if (Real64_Ge(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real64_Le(Real64_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real64_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return AllStart(closure);
}

SMILE_EXTERNAL_FUNCTION(All)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal64Range range = (SmileReal64Range)argv[0].obj;
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
	loopInfo->up = Real64_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real64_IsNegOrZero(range->stepping) : Real64_IsPosOrZero(range->stepping);
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

void SmileReal64Range_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "range start end", ARG_CHECK_MIN | ARG_CHECK_MAX, 3, 4, 0, NULL);

	SetupFunction("step", Step, (void *)base, "range stepping", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stepChecks);
	SetupFunction("reverse", Reverse, NULL, "range", ARG_CHECK_EXACT, 1, 1, 1, _real64Checks);

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
