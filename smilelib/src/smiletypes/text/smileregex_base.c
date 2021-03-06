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

#include <smile/string.h>
#include <smile/stringbuilder.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilehandle.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/regex.h>
#include <smile/eval/eval.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

SmileArg RegexReplaceStateMachine_Start(Regex regex, String input, SmileFunction function, Int startOffset, Int limit, Bool demoteMatchToString);

extern struct SmileHandleMethodsStruct RegexMatchMethods;

static Byte _handleChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
};

static Byte _handleComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	0, 0,
};

static Byte _stringChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
};

static Byte _matchChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
};

static Byte _replaceChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	0, 0,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
};

//-------------------------------------------------------------------------------------------------
// Virtual method overrides

static Bool Regex_ToBool(SmileHandle handle, SmileUnboxedData unboxedData)
{
	return ((Regex)handle->ptr)->cacheId > 0;
}

static String Regex_ToStringWrapper(SmileHandle handle, SmileUnboxedData unboxedData)
{
	return Regex_ToString((Regex)handle->ptr);
}

static SmileObject Regex_GetProperty(SmileHandle handle, Symbol propertyName)
{
	Regex regex = (Regex)handle->ptr;

	if (propertyName == Smile_KnownSymbols.pattern)
		return (SmileObject)regex->pattern;
	else if (propertyName == Smile_KnownSymbols.flags)
		return (SmileObject)regex->flags;
	else
		return handle->base->vtable->getProperty(handle->base, propertyName);
}

static Bool Regex_HasProperty(SmileHandle handle, Symbol propertyName)
{
	return (propertyName == Smile_KnownSymbols.pattern
		|| propertyName == Smile_KnownSymbols.flags);
}

static SmileList Regex_GetPropertyNames(SmileHandle handle)
{
	return SmileList_CreateTwo(
		SmileSymbol_Create(Smile_KnownSymbols.pattern),
		SmileSymbol_Create(Smile_KnownSymbols.flags)
	);
}

static struct SmileHandleMethodsStruct RegexMethods = {
	.toBool = Regex_ToBool,
	.toString = Regex_ToStringWrapper,
	.getProperty = Regex_GetProperty,
	.hasProperty = Regex_HasProperty,
	.getPropertyNames = Regex_GetPropertyNames,
};

//-------------------------------------------------------------------------------------------------
// Construction

SMILE_EXTERNAL_FUNCTION(Of)
{
	String pattern, options;
	Regex regex;
	SmileHandle handle;
	Int argi;

	argi = 0;
	if (argi < argc && argv[argi].obj == param)
		argi++;
	if (argi < argc) {
		pattern = (String)argv[argi++].obj;
	}
	else {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FromC("A string pattern is required as the first argument to 'Regex.of'."));
	}
	if (String_IsNullOrEmpty(pattern)) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FromC("The first argument to 'Regex.of' cannot be an empty string."));
	}
	options = argi < argc ? (String)argv[argi++].obj : String_Empty;

	regex = Regex_Create(pattern, options, NULL);
	handle = SmileHandle_Create((SmileObject)Smile_KnownBases.Regex, &RegexMethods, Smile_KnownSymbols.Regex_, regex);
	return SmileArg_From((SmileObject)handle);
}

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	SmileHandle handle;
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_HANDLE) {
		handle = (SmileHandle)argv[0].obj;
		if (handle->handleKind == Smile_KnownSymbols.Regex_) {
			Regex regex = (Regex)handle->ptr;
			return SmileUnboxedBool_From(Regex_IsValid(regex));
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	SmileHandle handle;
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_HANDLE) {
		handle = (SmileHandle)argv[0].obj;
		if (handle->handleKind == Smile_KnownSymbols.Regex_) {
			Regex regex = (Regex)handle->ptr;
			String string = Regex_ToString(regex);
			return SmileArg_From((SmileObject)string);
		}
	}

	return SmileArg_From((SmileObject)SymbolTable_GetName(Smile_SymbolTable, Smile_KnownSymbols.Regex_));
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileHandle handle;
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_HANDLE) {
		handle = (SmileHandle)argv[0].obj;
		if (handle->handleKind == Smile_KnownSymbols.Regex_) {
			Regex regex = (Regex)handle->ptr;
			UInt32 hash = String_Hash(regex->pattern) + String_Hash(regex->flags);
			return SmileUnboxedInteger64_From(Smile_ApplyHashOracle(hash));
		}
	}

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)argv[0].obj));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

static Bool Equals(SmileObject obj1, SmileObject obj2)
{
	SmileHandle a, b;

	a = (SmileHandle)obj1;

	return (
		SMILE_KIND(obj1) == SMILE_KIND_HANDLE
		&& a->handleKind == Smile_KnownSymbols.Regex_
		&& (b = (SmileHandle)obj2)->handleKind == Smile_KnownSymbols.Regex_
		&& Regex_Equal((Regex)a->ptr, (Regex)b->ptr)
	);
}

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(Equals(argv[0].obj, argv[1].obj));
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(!Equals(argv[0].obj, argv[1].obj));
}

//-------------------------------------------------------------------------------------------------

STATIC_STRING(_handleException, "First argument to 'Regex.%s' must be a Regex handle, not a '%S' handle.");

SMILE_EXTERNAL_FUNCTION(Matches)
{
	SmileHandle handle = (SmileHandle)argv[0].obj;
	String input = (String)argv[1].obj;
	Int64 offset = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int length;
	Regex regex;
	Bool result;

	if (handle->handleKind != Smile_KnownSymbols.Regex_) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FormatString(_handleException, "matches?",
				SymbolTable_GetName(Smile_SymbolTable, handle->handleKind)));
	}

	length = String_Length(input);
	if (offset > length) offset = length;
	if (offset < 0) offset = 0;

	regex = (Regex)handle->ptr;
	result = Regex_Test(regex, input, (Int)offset);

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Match)
{
	SmileHandle handle = (SmileHandle)argv[0].obj;
	String input = (String)argv[1].obj;
	Int64 offset = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int length;
	Regex regex;
	RegexMatch result;
	SmileHandle matchHandle;

	if (handle->handleKind != Smile_KnownSymbols.Regex_) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FormatString(_handleException, "match",
				SymbolTable_GetName(Smile_SymbolTable, handle->handleKind)));
	}

	length = String_Length(input);
	if (offset > length) offset = length;
	if (offset < 0) offset = 0;

	regex = (Regex)handle->ptr;
	result = Regex_Match(regex, input, (Int)offset);

	matchHandle = SmileHandle_Create((SmileObject)Smile_KnownBases.RegexMatch, &RegexMatchMethods, Smile_KnownSymbols.RegexMatch_, result);
	return SmileArg_From((SmileObject)matchHandle);
}

static Bool _splitWithEmpty = True;
static Bool _splitWithoutEmpty = False;

SMILE_EXTERNAL_FUNCTION(Split)
{
	SmileHandle handle = (SmileHandle)argv[0].obj;
	String input = (String)argv[1].obj;
	Int64 limit = argc > 2 ? argv[2].unboxed.i64 : 0;
	Regex regex;
	String *pieces;
	Int numPieces, i;
	SmileList head, tail;
	Bool withEmpty = *(Bool *)param;

	if (handle->handleKind != Smile_KnownSymbols.Regex_) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FormatString(_handleException, "split",
				SymbolTable_GetName(Smile_SymbolTable, handle->handleKind)));
	}

	regex = (Regex)handle->ptr;
#	if SizeofInt < 8
		if (limit > IntMax) limit = IntMax;
#	endif
	numPieces = Regex_Split(regex, input, &pieces, withEmpty, (Int)limit);

	head = tail = NullList;
	for (i = 0; i < numPieces; i++) {
		LIST_APPEND(head, tail, pieces[i]);
	}

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(Replace)
{
	SmileHandle handle = (SmileHandle)argv[0].obj;
	String input = (String)argv[1].obj;
	String replacement;
	Int64 startOffset = argc > 3 ? argv[3].unboxed.i64 : 0;
	Int64 limit = argc > 4 ? argv[4].unboxed.i64 : 0;
	Int length = String_Length(input);
	Regex regex;
	String result;

	if (handle->handleKind != Smile_KnownSymbols.Regex_) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FormatString(_handleException, "replace",
				SymbolTable_GetName(Smile_SymbolTable, handle->handleKind)));
	}

	regex = (Regex)handle->ptr;

#	if SizeofInt < 8
		if (limit > IntMax) limit = IntMax;
#	endif
	if (startOffset < 0) startOffset = 0;
	if (startOffset > length) startOffset = length;

	switch (SMILE_KIND(argv[2].obj)) {
		case SMILE_KIND_STRING:
			replacement = (String)argv[2].obj;
			goto stringCase;

		case SMILE_KIND_CHAR:
			replacement = String_CreateRepeat(argv[2].unboxed.ch, 1);
			goto stringCase;

		case SMILE_KIND_UNI:
			replacement = String_CreateFromUnicode(argv[2].unboxed.uni);
			goto stringCase;

		stringCase:
			result = Regex_Replace(regex, input, replacement, (Int)startOffset, (Int)limit);
			return SmileArg_From((SmileObject)result);

		case SMILE_KIND_FUNCTION:
			return RegexReplaceStateMachine_Start(regex, input, (SmileFunction)argv[2].obj, (Int)startOffset, (Int)limit, False);

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_Format("Third argument to 'Regex.replace' must be a replacement String, Char, Uni, or Function."));
	}
}

//-------------------------------------------------------------------------------------------------

typedef struct ReplaceInfoStruct {
	RegexReplaceState state;
	SmileFunction function;
	Bool demoteMatchToString;
} *ReplaceInfo;

static Int RegexReplaceStateMachine_StartLoop(ClosureStateMachine closure)
{
	ReplaceInfo replaceInfo = (ReplaceInfo)closure->state;

	if (Regex_ReplaceLoopTop(replaceInfo->state)) {
		Closure_PushBoxed(closure, replaceInfo->function);
		if (replaceInfo->demoteMatchToString) {
			RegexMatch match = replaceInfo->state->match;
			String capture = String_Substring(match->input, match->indexedCaptures[0].start, match->indexedCaptures[0].length);
			Closure_PushBoxed(closure, capture);
		}
		else {
			SmileHandle matchHandle = SmileHandle_Create((SmileObject)Smile_KnownBases.RegexMatch, &RegexMatchMethods,
				Smile_KnownSymbols.RegexMatch_, replaceInfo->state->match);
			Closure_PushBoxed(closure, matchHandle);
		}
		return 1;
	}
	else {
		String result = Regex_EndReplace(replaceInfo->state);
		Closure_PushBoxed(closure, result);
		return -1;
	}
}

static Int RegexReplaceStateMachine_Body(ClosureStateMachine closure)
{
	ReplaceInfo replaceInfo = (ReplaceInfo)closure->state;

	SmileArg returnValue = Closure_Pop(closure);

	String replacement = (SMILE_KIND(returnValue.obj) == SMILE_KIND_STRING
		? (String)returnValue.obj
		: SMILE_VCALL1(returnValue.obj, toString, returnValue.unboxed));

	Regex_ReplaceLoopBottom(replaceInfo->state, replacement);

	return RegexReplaceStateMachine_StartLoop(closure);
}

SmileArg RegexReplaceStateMachine_Start(Regex regex, String input, SmileFunction function, Int startOffset, Int limit, Bool demoteMatchToString)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	ReplaceInfo replaceInfo;
	ClosureStateMachine closure;

	closure = Eval_BeginStateMachine(RegexReplaceStateMachine_StartLoop, RegexReplaceStateMachine_Body);

	replaceInfo = (ReplaceInfo)closure->state;
	replaceInfo->function = function;
	replaceInfo->state = Regex_BeginReplace(regex, input, startOffset, limit);
	replaceInfo->demoteMatchToString = demoteMatchToString;

	return (SmileArg) { NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

void SmileRegex_Setup(SmileUserObject base)
{
	SetupFunction("of", Of, base, "pattern options", ARG_CHECK_MAX, 1, 3, 0, NULL);

	SetupFunction("bool", ToBool, NULL, "regex", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "regex", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "regex", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "regex", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _handleComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _handleComparisonChecks);

	SetupFunction("matches?", Matches, NULL, "regex string offset", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _matchChecks);
	SetupFunction("match", Match, NULL, "regex string offset", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _matchChecks);

	SetupFunction("split", Split, &_splitWithoutEmpty, "regex string limit", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _matchChecks);
	SetupFunction("split-with-empty", Split, &_splitWithEmpty, "regex string limit", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _matchChecks);

	SetupFunction("replace", Replace, NULL, "regex string replacement offset limit", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 3, 5, 5, _replaceChecks);
}
