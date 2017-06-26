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

#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/eval/eval.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/raw/smilebytearray.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _byteArrayChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	0, 0,
	0, 0,
};

static Byte _eachChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_BYTEARRAY)
		return SmileUnboxedBool_From(String_Length((String)argv[0].obj) > 0);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING)
		return SmileUnboxedInteger64_From(String_Length((String)argv[0].obj));

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(byteArrayString, "ByteArray");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_BYTEARRAY) {
		SmileByteArray byteArray = (SmileByteArray)(argv[0].obj);
		String result = String_CreateFromByteArray(byteArray);
		return SmileArg_From((SmileObject)result);
	}
	else {
		return SmileArg_From((SmileObject)byteArrayString);
	}
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	return SmileUnboxedInteger64_From(((PtrInt)argv[0].obj) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Get/set members

STATIC_STRING(OutOfRangeError, "Index out of range.");

SMILE_EXTERNAL_FUNCTION(GetMember)
{
	STATIC_STRING(invalidIndexType, "Index to ByteArray.get-member must be of type Integer64 or IntegerRange64.");
	SmileByteArray byteArray = (SmileByteArray)argv[0].obj;

	switch (SMILE_KIND(argv[1].obj)) {
		case SMILE_KIND_UNBOXED_INTEGER64:
		{
			Int64 offset;
			offset = argv[1].unboxed.i64;
			if (offset < 0 || offset >= byteArray->length)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, OutOfRangeError);
			return SmileUnboxedByte_From((Byte)byteArray->data[(Int)offset]);
		}
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, invalidIndexType);
	}

	return (SmileArg){ NULL };
}

SMILE_EXTERNAL_FUNCTION(SetMember)
{
	STATIC_STRING(invalidIndexType, "Index to ByteArray.set-member must be of type Integer64 or IntegerRange64.");
	STATIC_STRING(invalidValueType, "Value for ByteArray.set-member must be of type Byte or ByteArray.");
	STATIC_STRING(readOnlyError, "ByteArray is read-only.");
	SmileByteArray byteArray = (SmileByteArray)argv[0].obj;

	if (!(byteArray->kind & SMILE_SECURITY_WRITABLE))
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, readOnlyError);

	switch (SMILE_KIND(argv[1].obj)) {
		case SMILE_KIND_UNBOXED_INTEGER64:
		{
			Int64 offset;
			offset = argv[1].unboxed.i64;
			if (offset < 0 || offset >= byteArray->length)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, OutOfRangeError);
		}
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, invalidIndexType);
	}

	return (SmileArg){ NULL };
}

//-------------------------------------------------------------------------------------------------
// Construction

SMILE_EXTERNAL_FUNCTION(OfSize)
{
	STATIC_STRING(argumentError, "ByteArray.of-size accepts one Integer64 argument (and one optional Byte argument).");
	STATIC_STRING(countError, "ByteArray.of-size count must not be negative.");

	SmileUserObject base = (SmileUserObject)param;
	Int i;
	Int64 count;
	Byte value;
	SmileByteArray byteArray;

	i = 0;
	if (argv[i].obj == (SmileObject)base)
		i++;

	// Parse the Integer64 count.
	if (i >= argc || SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_INTEGER64)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, argumentError);
	count = argv[i].unboxed.i64;
	if (count < 0 || count > PtrIntMax)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, countError);
	i++;

	// Parse an optional Byte.
	if (i < argc) {
		if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_BYTE)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, argumentError);
		value = argv[i].unboxed.i8;
	}
	else value = 0;

	// Create the ByteArray.
	byteArray = SmileByteArray_Create((SmileObject)Smile_KnownBases.ByteArray, (Int)count, True);
	if (value != 0)
		MemSet(byteArray->data, value, byteArray->length);

	return SmileArg_From((SmileObject)byteArray);
}

//-------------------------------------------------------------------------------------------------

typedef struct EachInfoStruct {
	SmileByteArray byteArray;
	const Byte *ptr, *end;
	SmileFunction function;
	Int index;
} *EachInfo;

static Int EachWithOneArg(ClosureStateMachine closure)
{
	EachInfo eachInfo = (EachInfo)closure->state;

	// If we've run out of bytes, we're done.
	if (eachInfo->ptr >= eachInfo->end) {
		Closure_Pop(closure);
		Closure_PushBoxed(closure, eachInfo->byteArray);	// Pop the previous return value and push 'byteArray'.
		return -1;
	}

	// Set up to call the user's function with the next list item.
	Closure_Pop(closure);
	Closure_PushBoxed(closure, eachInfo->function);
	Closure_PushUnboxedByte(closure, *eachInfo->ptr);

	eachInfo->ptr++;	// Move the iterator to the next item.
	eachInfo->index++;

	return 1;
}

static Int EachWithTwoArgs(ClosureStateMachine closure)
{
	EachInfo eachInfo = (EachInfo)closure->state;

	// If we've run out of bytes, we're done.
	if (eachInfo->ptr >= eachInfo->end) {
		Closure_Pop(closure);
		Closure_PushBoxed(closure, eachInfo->byteArray);	// Pop the previous return value and push 'byteArray'.
		return -1;
	}

	// Set up to call the user's function with the next list item and index.
	Closure_Pop(closure);
	Closure_PushBoxed(closure, eachInfo->function);
	Closure_PushUnboxedByte(closure, *eachInfo->ptr);
	Closure_PushUnboxedInt64(closure, eachInfo->index);

	eachInfo->ptr++;	// Move the iterator to the next item.
	eachInfo->index++;

	return 2;
}

SMILE_EXTERNAL_FUNCTION(Each)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileByteArray byteArray = (SmileByteArray)argv[0].obj;
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
	eachInfo->byteArray = byteArray;
	eachInfo->index = 0;
	eachInfo->ptr = byteArray->data;
	eachInfo->end = eachInfo->ptr + byteArray->length;

	Closure_PushBoxed(closure, NullObject);	// Initial "return" value from 'each'.

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct MapInfoStruct {
	StringBuilder result;
	const Byte *ptr, *end;
	SmileFunction function;
	Int index;
} *MapInfo;

static Int MapWithOneArgStart(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToByteArray(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

static Int MapWithOneArgBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output array.
	SmileArg fnResult = Closure_Pop(closure);
	switch (SMILE_KIND(fnResult.obj)) {
		case SMILE_KIND_UNBOXED_BYTE:
			StringBuilder_AppendByte(loopInfo->result, fnResult.unboxed.i8);
			break;
		case SMILE_KIND_BYTEARRAY:
			{
				SmileByteArray baResult = (SmileByteArray)fnResult.obj;
				StringBuilder_Append(loopInfo->result, baResult->data, 0, baResult->length);
			}
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("'map' projection must return a Byte or ByteArray type."));
			break;
	}

	// Next: Move the iterator to the next item.
	loopInfo->ptr++;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToByteArray(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

static Int MapWithTwoArgsStart(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToByteArray(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

static Int MapWithTwoArgsBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	switch (SMILE_KIND(fnResult.obj)) {
		case SMILE_KIND_UNBOXED_BYTE:
			StringBuilder_AppendByte(loopInfo->result, fnResult.unboxed.i8);
			break;
		case SMILE_KIND_BYTEARRAY:
			{
				SmileByteArray baResult = (SmileByteArray)fnResult.obj;
				StringBuilder_Append(loopInfo->result, baResult->data, 0, baResult->length);
			}
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("'map' projection must return a Byte or ByteArray type."));
			break;
	}

	// Next: Move the iterator to the next item.
	loopInfo->ptr++;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToByteArray(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

SMILE_EXTERNAL_FUNCTION(Map)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileByteArray byteArray = (SmileByteArray)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	MapInfo mapInfo;
	ClosureStateMachine closure;
	const Byte *ptr;
	Int length;

	ptr = byteArray->data;
	length = byteArray->length;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(
		maxArgs <= 1 ? MapWithOneArgStart : MapWithTwoArgsStart,
		maxArgs <= 1 ? MapWithOneArgBody : MapWithTwoArgsBody);

	mapInfo = (MapInfo)closure->state;
	mapInfo->function = function;
	mapInfo->ptr = ptr;
	mapInfo->end = ptr + length;
	mapInfo->result = StringBuilder_Create();
	mapInfo->index = 0;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct WhereInfoStruct {
	StringBuilder result;
	const Byte *ptr, *end;
	SmileFunction function;
	Int index;
} *WhereInfo;

static Int WhereWithOneArgStart(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of bytes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToByteArray(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
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
		StringBuilder_AppendByte(loopInfo->result, *loopInfo->ptr);
	}

	// Next: Move the iterator to the next item.
	loopInfo->ptr++;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToByteArray(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

static Int WhereWithTwoArgsStart(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToByteArray(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
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
		StringBuilder_AppendByte(loopInfo->result, *loopInfo->ptr);
	}

	// Next: Move the iterator to the next item.
	loopInfo->ptr++;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToByteArray(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

SMILE_EXTERNAL_FUNCTION(Where)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileByteArray byteArray = (SmileByteArray)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	WhereInfo whereInfo;
	ClosureStateMachine closure;
	const Byte *ptr;
	Int length;

	ptr = byteArray->data;
	length = byteArray->length;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(
		maxArgs <= 1 ? WhereWithOneArgStart : WhereWithTwoArgsStart,
		maxArgs <= 1 ? WhereWithOneArgBody : WhereWithTwoArgsBody);

	whereInfo = (WhereInfo)closure->state;
	whereInfo->function = function;
	whereInfo->ptr = ptr;
	whereInfo->end = ptr + length;
	whereInfo->result = StringBuilder_Create();
	whereInfo->index = 0;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct CountInfoStruct {
	const Byte *ptr, *end;
	SmileFunction function;
	Int count;
} *CountInfo;

static Int CountStart(ClosureStateMachine closure)
{
	register CountInfo loopInfo = (CountInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of string bytes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushUnboxedInt64(closure, loopInfo->count);
		return -1;
	}

	// Body: Set up to call the user's function with the next character.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
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

	// Next: Move the iterator to the next byte.
	loopInfo->ptr++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushUnboxedInt64(closure, loopInfo->count);
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

SMILE_EXTERNAL_FUNCTION(Count)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	SmileByteArray byteArray = (SmileByteArray)argv[0].obj;
	CountInfo countInfo;
	ClosureStateMachine closure;
	Byte value;
	const Byte *ptr, *end;
	Int count;
	Int length;

	if (argc < 1) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'count' requires at least 1 argument, but was called with %d.", argc));
	}
	if (SMILE_KIND(argv[0].obj) != SMILE_KIND_BYTEARRAY) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument 1 to 'count' is of the wrong type."));
	}

	ptr = byteArray->data;
	length = byteArray->length;

	if (argc == 1) {
		// Degenerate form: Just return the length of the array.
		return SmileUnboxedInteger64_From(length);
	}

	if (argc > 2) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'count' allows at most 2 arguments, but was called with %d.", argc));
	}

	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_FUNCTION) {

		if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_BYTE)
			return SmileUnboxedInteger64_From(0);

		// Degenerate form:  Count up any bytes that are equal to the given byte.
		value = argv[1].unboxed.i8;
		end = ptr + length;
		for (count = 0; ptr < end; ptr++) {
			if (*ptr == value) count++;
		}
		return SmileUnboxedInteger64_From(count);
	}

	closure = Eval_BeginStateMachine(CountStart, CountBody);

	countInfo = (CountInfo)closure->state;
	countInfo->function = (SmileFunction)argv[1].obj;
	countInfo->ptr = ptr;
	countInfo->end = ptr + length;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

void SmileByteArray_Setup(SmileUserObject base)
{
	SetupFunction("of-size", OfSize, (void *)base, "count", 0, 0, 0, 0, NULL);

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("get-member", GetMember, NULL, "byte-array index", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 1, _byteArrayChecks);
	SetupFunction("set-member", SetMember, NULL, "byte-array index value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 1, _byteArrayChecks);

	SetupFunction("each", Each, NULL, "byte-array", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("map", Map, NULL, "byte-array", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("where", Where, NULL, "byte-array", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("count", Count, NULL, "byte-array", ARG_STATE_MACHINE, 0, 0, 0, NULL);
}
