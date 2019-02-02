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

#include <smile/smiletypes/numeric/smilereal32.h>
#include <smile/smiletypes/range/smilereal32range.h>

SMILE_IGNORE_UNUSED_VARIABLES

typedef enum {
	FindMode_First,
	FindMode_IndexOf,
	FindMode_Count,
	FindMode_Where,
	FindMode_Any,
	FindMode_All,
} FindMode;

STATIC_STRING(_stringTypeError, "Second argument to 'string' must be of type 'Real32'");
STATIC_STRING(_real32TypeError, "%s argument to '%s' must be of type 'Real32'");
STATIC_STRING(_argCountError, "Too many arguments to 'Real32Range.%s'");

static Byte _eachChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_REAL32RANGE,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

static Byte _findChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_REAL32RANGE,
	0, 0,
};

static Byte _real32Checks[] = {
	SMILE_KIND_MASK, SMILE_KIND_REAL32RANGE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
};

static Byte _stepChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_REAL32RANGE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_REAL32,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_REAL32RANGE)
		return SmileUnboxedBool_From(True);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_REAL32RANGE) {
		SmileReal32Range obj = (SmileReal32Range)argv[0].obj;
		return SmileUnboxedInteger64_From(Real32_ToInt64(Real32_Sub(obj->end, obj->start)));
	}

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(real32range, "Real32Range");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_REAL32RANGE) {
		String string;
		SmileReal32Range obj = (SmileReal32Range)argv[0].obj;

		string = ((Real32_Ge(obj->end, obj->start) && Real32_Ne(obj->stepping, Real32_One)
	|| Real32_Lt(obj->end, obj->start) && Real32_Ne(obj->stepping, Real32_One))
	? String_Format("(%S)..(%S) step %S",
		Real32_ToStringEx(obj->start, 0, 0, False),
		Real32_ToStringEx(obj->end, 0, 0, False),
		Real32_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("(%S)..(%S)",
		Real32_ToStringEx(obj->start, 0, 0, False),
		Real32_ToStringEx(obj->end, 0, 0, False)))
;

		return SmileArg_From((SmileObject)string);
	}

	return SmileArg_From((SmileObject)real32range);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_REAL32RANGE) {
		SmileReal32Range range = (SmileReal32Range)argv[0].obj;
		UInt32 result;
		UInt32 start = *(UInt32 *)&range->start;
		UInt32 end = *(UInt32 *)&range->end;
		UInt32 stepping = *(UInt32 *)&range->stepping;
		result = Smile_ApplyHashOracle(start ^ end ^ stepping);

		return SmileUnboxedInteger64_From(result);
	}

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)argv[0].obj));
}

//-------------------------------------------------------------------------------------------------
// Construction

SMILE_EXTERNAL_FUNCTION(Of)
{
	Int i = 0;
	Real32 start, end;
	Real32 stepping;

	if (argv[i].obj == (SmileObject)param)
		i++;

	if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_REAL32)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_real32TypeError, "First", "of"));
	start = argv[i++].unboxed.r32;

	if (i >= argc || SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_REAL32)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_real32TypeError, "Second", "of"));
	end = argv[i++].unboxed.r32;

	if (i < argc) {
		if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_REAL32)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_real32TypeError, "Third", "of"));
		stepping = argv[i++].unboxed.r32;
	}
	else stepping = Real32_Ge(end, start) ? Real32_One : Real32_NegOne;

	if (i != argc)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_argCountError, "of"));

	return SmileArg_From((SmileObject)SmileReal32Range_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Step)
{
	Real32 stepping = argv[1].unboxed.r32;
	Real32 start = ((SmileReal32Range)argv[0].obj)->start;
	Real32 end = ((SmileReal32Range)argv[0].obj)->end;

	if (Real32_IsZero(stepping))
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument to 'Real32Range.step' cannot be zero."));
	if (Real32_Lt(start, end) && Real32_IsNeg(stepping))
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot apply a negative step to a forward range."));
	if (Real32_Lt(end, start) && Real32_IsPos(stepping))
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Cannot apply a positive step to a reverse range."));

	return SmileArg_From((SmileObject)SmileReal32Range_Create(start, end, stepping));
}

SMILE_EXTERNAL_FUNCTION(Reverse)
{
	return SmileArg_From((SmileObject)SmileReal32Range_Create(
		((SmileReal32Range)argv[0].obj)->end,
		((SmileReal32Range)argv[0].obj)->start,
		Real32_Neg(((SmileReal32Range)argv[0].obj)->stepping)
	));
}

//-------------------------------------------------------------------------------------------------

static SmileArg FindFixedValue(SmileReal32Range range, SmileArg valueArg, FindMode fixedMode)
{
	Real32 current = range->start;
	Real32 step = range->stepping;
	Real32 end = range->end;
	Bool up = Real32_Gt(range->end, range->start);
	Real32 value;

	if (!up) {
		// Handle the downward case by swapping endpoints and directions.
		Real32 temp;
		step = Real32_Neg(step);
		temp = current;
		current = end;
		end = temp;
	}

	// An Real32 range cannot contain non-Real32 values, so only test if the input value was of a sane type.
	if (SMILE_KIND(valueArg.obj) == SMILE_KIND_UNBOXED_REAL32) {
		value = valueArg.unboxed.r32;

		// Use a shortcut for a step of +1 on a forward range.
		if (Real32_Eq(step, Real32_One)) {
			if (Real32_Le(value, end)) {
				// Found it.
				switch (fixedMode) {
					case FindMode_IndexOf:
						return SmileUnboxedInteger64_From((Int64)Real32_ToInt64(Real32_Sub(value, current)));
					case FindMode_First:
					case FindMode_Where:
						return SmileUnboxedReal32_From(value);
					case FindMode_Count:
						return SmileUnboxedInteger64_From(1);
					case FindMode_Any:
						return SmileUnboxedBool_From(True);
					case FindMode_All:
						return SmileUnboxedBool_From(Real32_Eq(current, end));	// 'All' can only be true if there's one value total.
				}
			}
		}
		else {
			// General case:  Do some math and see if the target is something we'd hit by iterating.
			Real32 delta = Real32_Sub(value, current);
			if (Real32_IsZero(Real32_Mod(delta, step))) {
				switch (fixedMode) {
					case FindMode_IndexOf:
						return SmileUnboxedInteger64_From(Real32_ToInt64(Real32_Div(delta, step)));
					case FindMode_First:
					case FindMode_Where:
						return SmileUnboxedReal32_From(value);
					case FindMode_Count:
						return SmileUnboxedInteger64_From(1);
					case FindMode_Any:
						return SmileUnboxedBool_From(True);
					case FindMode_All:
						return SmileUnboxedBool_From(Real32_Eq(current, end));	// 'All' can only be true if there's one value total.
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

typedef struct EachInfoReal32Struct {
	SmileReal32Range range;
	SmileFunction function;
	Real32 current;
	Real32 step;
	Real32 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *EachInfoReal32;

static Int EachStateMachine(ClosureStateMachine closure)
{
	EachInfoReal32 eachInfo = (EachInfoReal32)closure->state;

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
		Closure_PushUnboxedReal32(closure, eachInfo->current);
		if (eachInfo->numArgs > 1)
			Closure_PushUnboxedInt64(closure, eachInfo->index);
	}

	// Move to the next spot.
	if (eachInfo->up) {
		if (Real32_Ge(Real32_Sub(eachInfo->end, eachInfo->step), eachInfo->current))
			eachInfo->current = Real32_Add(eachInfo->current, eachInfo->step);
		else eachInfo->done = True;
	}
	else {
		if (Real32_Le(Real32_Sub(eachInfo->end, eachInfo->step), eachInfo->current))
			eachInfo->current = Real32_Add(eachInfo->current, eachInfo->step);
		else eachInfo->done = True;
	}
	eachInfo->index++;

	return eachInfo->numArgs;
}

SMILE_EXTERNAL_FUNCTION(Each)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal32Range range = (SmileReal32Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	EachInfoReal32 eachInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(EachStateMachine, EachStateMachine);

	eachInfo = (EachInfoReal32)closure->state;
	eachInfo->range = range;
	eachInfo->function = function;
	eachInfo->index = 0;
	eachInfo->current = range->start;
	eachInfo->step = range->stepping;
	eachInfo->end = range->end;
	eachInfo->up = Real32_Ge(range->end, range->start);
	eachInfo->done = eachInfo->up ? Real32_IsNegOrZero(range->stepping) : Real32_IsPosOrZero(range->stepping);
	eachInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	Closure_PushBoxed(closure, NullObject);	// Initial "return" value from 'each'.

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct MapInfoReal32Struct {
	SmileFunction function;
	SmileList resultHead, resultTail;
	Real32 current;
	Real32 step;
	Real32 end;
	Int64 index;
	Byte numArgs;
	Bool done;
	Bool up;
} *MapInfoReal32;

static Int MapStart(ClosureStateMachine closure)
{
	register MapInfoReal32 loopInfo = (MapInfoReal32)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of values, we're done.
	if (loopInfo->done) {
		Closure_PushBoxed(closure, loopInfo->resultHead);	// Push 'resultHead' as the output.
		return -1;
	}

	// Body: Set up to call the user's function with the first value.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedReal32(closure, loopInfo->current);
	if (loopInfo->numArgs > 1)
		Closure_PushUnboxedInt64(closure, loopInfo->index);

	return loopInfo->numArgs;
}

static Int MapBody(ClosureStateMachine closure)
{
	register MapInfoReal32 loopInfo = (MapInfoReal32)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileArg_Box(fnResult));

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (Real32_Ge(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real32_Le(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return MapStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Map)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal32Range range = (SmileReal32Range)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	MapInfoReal32 loopInfo;
	ClosureStateMachine closure;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(MapStart, MapBody);

	loopInfo = (MapInfoReal32)closure->state;
	loopInfo->resultHead = loopInfo->resultTail = NullList;
	loopInfo->function = function;
	loopInfo->index = 0;
	loopInfo->current = range->start;
	loopInfo->step = range->stepping;
	loopInfo->end = range->end;
	loopInfo->up = Real32_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real32_IsNegOrZero(range->stepping) : Real32_IsPosOrZero(range->stepping);
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct WhereInfoStruct {
	SmileFunction function;
	SmileList resultHead, resultTail;
	Real32 current;
	Real32 step;
	Real32 end;
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
	Closure_PushUnboxedReal32(closure, loopInfo->current);
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
		LIST_APPEND(loopInfo->resultHead, loopInfo->resultTail, SmileReal32_Create(loopInfo->current));
	}

	// Next: Move the iterator to the next item.
	if (loopInfo->up) {
		if (Real32_Ge(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real32_Le(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return WhereStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Where)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal32Range range = (SmileReal32Range)argv[0].obj;
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
	loopInfo->up = Real32_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real32_IsNegOrZero(range->stepping) : Real32_IsPosOrZero(range->stepping);
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct CountInfoStruct {
	SmileFunction function;
	Int64 index;
	Int64 count;
	Real32 current;
	Real32 end;
	Real32 step;
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
	Closure_PushUnboxedReal32(closure, loopInfo->current);
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
		if (Real32_Ge(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real32_Le(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return CountStart(closure);
}

SMILE_EXTERNAL_FUNCTION(Count)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal32Range range = (SmileReal32Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	CountInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, they just want to find out how many values this range describes.
	if (argc == 1) {
		if (Real32_Ge(range->end, range->start)) {
			if (Real32_IsNegOrZero(range->stepping)) return SmileUnboxedReal32_From(Real32_Zero);
			return SmileUnboxedReal32_From(Real32_Div(
				Real32_Sub(range->end, range->start),
				Real32_Add(range->stepping, Real32_One))
			);
		}
		else {
			if (Real32_IsPosOrZero(range->stepping)) return SmileUnboxedReal32_From(Real32_Zero);
			return SmileUnboxedReal32_From(Real32_Div(
				Real32_Sub(range->start, range->end),
				Real32_Add(Real32_Neg(range->stepping), Real32_One))
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
	loopInfo->up = Real32_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real32_IsNegOrZero(range->stepping) : Real32_IsPosOrZero(range->stepping);
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct FindInfoStruct {
	SmileFunction function;
	Real32 current;
	Real32 step;
	Real32 end;
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
	Closure_PushUnboxedReal32(closure, loopInfo->current);
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
				Closure_PushUnboxedReal32(closure, loopInfo->current);
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
		if (Real32_Ge(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real32_Le(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return FindStart(closure);
}

SMILE_EXTERNAL_FUNCTION(First)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal32Range range = (SmileReal32Range)argv[0].obj;
	SmileFunction function;
	Int minArgs, maxArgs;
	FindInfo loopInfo;
	ClosureStateMachine closure;

	// With no predicate, this is just a synonym for the 'start' property.
	if (argc == 1)
		return SmileUnboxedReal32_From(range->start);

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
	loopInfo->up = Real32_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real32_IsNegOrZero(range->stepping) : Real32_IsPosOrZero(range->stepping);
	loopInfo->findMode = FindMode_First;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(IndexOf)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal32Range range = (SmileReal32Range)argv[0].obj;
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
	loopInfo->up = Real32_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real32_IsNegOrZero(range->stepping) : Real32_IsPosOrZero(range->stepping);
	loopInfo->findMode = FindMode_IndexOf;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

SMILE_EXTERNAL_FUNCTION(Any)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal32Range range = (SmileReal32Range)argv[0].obj;
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
	loopInfo->up = Real32_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real32_IsNegOrZero(range->stepping) : Real32_IsPosOrZero(range->stepping);
	loopInfo->findMode = FindMode_Any;
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct AllInfoStruct {
	SmileFunction function;
	Real32 current;
	Real32 step;
	Real32 end;
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
	Closure_PushUnboxedReal32(closure, loopInfo->current);
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
		if (Real32_Ge(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	else {
		if (Real32_Le(Real32_Sub(loopInfo->end, loopInfo->step), loopInfo->current))
			loopInfo->current = Real32_Add(loopInfo->current, loopInfo->step);
		else loopInfo->done = True;
	}
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	return AllStart(closure);
}

SMILE_EXTERNAL_FUNCTION(All)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileReal32Range range = (SmileReal32Range)argv[0].obj;
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
	loopInfo->up = Real32_Ge(range->end, range->start);
	loopInfo->done = loopInfo->up ? Real32_IsNegOrZero(range->stepping) : Real32_IsPosOrZero(range->stepping);
	loopInfo->numArgs = (Byte)(maxArgs <= 2 ? maxArgs : 2);

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

void SmileReal32Range_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "range start end", ARG_CHECK_MIN | ARG_CHECK_MAX, 3, 4, 0, NULL);

	SetupFunction("step", Step, (void *)base, "range stepping", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stepChecks);
	SetupFunction("reverse", Reverse, NULL, "range", ARG_CHECK_EXACT, 1, 1, 1, _real32Checks);

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
