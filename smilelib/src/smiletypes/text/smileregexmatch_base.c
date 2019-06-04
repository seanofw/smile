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
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/range/smileinteger64range.h>
#include <smile/dict/stringintdict.h>
#include <smile/regex.h>
#include <smile/internal/staticstring.h>
#include <smile/smiletypes/smileuserobject.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _handleComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	0, 0,
};

static Byte _getMemberChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	0, 0,
};

//-------------------------------------------------------------------------------------------------
// Virtual method overrides

static Bool RegexMatch_ToBool(SmileHandle handle, SmileUnboxedData unboxedData)
{
	return ((RegexMatch)handle->ptr)->isMatch;
}

static String RegexMatch_ToString(SmileHandle handle, SmileUnboxedData unboxedData)
{
	RegexMatch regexMatch = (RegexMatch)handle->ptr;
	RegexMatchRange range;

	if (!regexMatch->isMatch || regexMatch->numIndexedCaptures <= 0)
		return String_Empty;

	range = regexMatch->indexedCaptures;
	return String_Substring(regexMatch->input, range->start, range->length);
}

static SmileObject RegexMatch_GetProperty(SmileHandle handle, Symbol propertyName)
{
	RegexMatch regexMatch;
	RegexMatchRange range;

	if (propertyName == Smile_KnownSymbols.start) {
		regexMatch = (RegexMatch)handle->ptr;
		range = regexMatch->indexedCaptures;
		if (!regexMatch->isMatch || regexMatch->numIndexedCaptures <= 0)
			return (SmileObject)SmileInteger64_Create(0);
		return (SmileObject)SmileInteger64_Create(range->start);
	}
	else if (propertyName == Smile_KnownSymbols.length) {
		regexMatch = (RegexMatch)handle->ptr;
		range = regexMatch->indexedCaptures;
		if (!regexMatch->isMatch || regexMatch->numIndexedCaptures <= 0)
			return (SmileObject)SmileInteger64_Create(0);
		return (SmileObject)SmileInteger64_Create(range->length);
	}
	else if (propertyName == Smile_KnownSymbols.end) {
		regexMatch = (RegexMatch)handle->ptr;
		range = regexMatch->indexedCaptures;
		if (!regexMatch->isMatch || regexMatch->numIndexedCaptures <= 0)
			return (SmileObject)SmileInteger64_Create(0);
		return (SmileObject)SmileInteger64_Create(range->start + range->length - 1);
	}
	else
		return handle->base->vtable->getProperty(handle->base, propertyName);
}

static Bool RegexMatch_HasProperty(SmileHandle handle, Symbol propertyName)
{
	return (propertyName == Smile_KnownSymbols.start
		|| propertyName == Smile_KnownSymbols.end
		|| propertyName == Smile_KnownSymbols.length);
}

static SmileList RegexMatch_GetPropertyNames(SmileHandle handle)
{
	return SmileList_CreateThree(
		SmileSymbol_Create(Smile_KnownSymbols.start),
		SmileSymbol_Create(Smile_KnownSymbols.end),
		SmileSymbol_Create(Smile_KnownSymbols.length)
	);
}

struct SmileHandleMethodsStruct RegexMatchMethods = {
	.toBool = RegexMatch_ToBool,
	.toString = RegexMatch_ToString,
	.getProperty = RegexMatch_GetProperty,
	.hasProperty = RegexMatch_HasProperty,
	.getPropertyNames = RegexMatch_GetPropertyNames,
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
	handle = SmileHandle_Create((SmileObject)Smile_KnownBases.Regex, NULL, Smile_KnownSymbols.Regex_, regex);
	return SmileArg_From((SmileObject)handle);
}

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	SmileHandle handle;
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_HANDLE) {
		handle = (SmileHandle)argv[0].obj;
		if (handle->handleKind == Smile_KnownSymbols.RegexMatch_) {
			RegexMatch regexMatch = (RegexMatch)handle->ptr;
			return SmileUnboxedBool_From(regexMatch->isMatch);
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	SmileHandle handle;
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_HANDLE) {
		handle = (SmileHandle)argv[0].obj;
		if (handle->handleKind == Smile_KnownSymbols.RegexMatch_) {
			RegexMatch regexMatch = (RegexMatch)handle->ptr;
			return SmileUnboxedInteger64_From(regexMatch->isMatch
				? regexMatch->numIndexedCaptures + StringIntDict_Count(&regexMatch->namedCaptures)
				: 0);
		}
	}

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	SmileHandle handle;
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_HANDLE) {
		handle = (SmileHandle)argv[0].obj;
		if (handle->handleKind == Smile_KnownSymbols.RegexMatch_) {
			RegexMatch regexMatch = (RegexMatch)handle->ptr;
			RegexMatchRange range;

			if (!regexMatch->isMatch || regexMatch->numIndexedCaptures <= 0)
				return SmileArg_From((SmileObject)String_Empty);

			range = regexMatch->indexedCaptures;
			return SmileArg_From((SmileObject)String_Substring(regexMatch->input, range->start, range->length));
		}
	}

	return SmileArg_From((SmileObject)SymbolTable_GetName(Smile_SymbolTable, Smile_KnownSymbols.RegexMatch_));
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileHandle handle;
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_HANDLE) {
		handle = (SmileHandle)argv[0].obj;
		if (handle->handleKind == Smile_KnownSymbols.RegexMatch_) {
			RegexMatch regexMatch = (RegexMatch)handle->ptr;
			return SmileUnboxedInteger64_From(Smile_ApplyHashOracle(regexMatch->isMatch
				? regexMatch->numIndexedCaptures + StringIntDict_Count(&regexMatch->namedCaptures)
				: 0));
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
		&& a->handleKind == Smile_KnownSymbols.RegexMatch_
		&& (b = (SmileHandle)obj2)->handleKind == Smile_KnownSymbols.RegexMatch_
		&& a->ptr == b->ptr
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

STATIC_STRING(_handleException, "First argument to 'RegexMatch.%s' must be a RegexMatch handle, not a '%S' handle.");

static RegexMatch GetRegexMatchObject(SmileObject obj, const char *methodName)
{
	SmileHandle handle = (SmileHandle)obj;
	if (handle->handleKind != Smile_KnownSymbols.RegexMatch_) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FormatString(_handleException, methodName,
				SymbolTable_GetName(Smile_SymbolTable, handle->handleKind)));
	}

	return (RegexMatch)handle->ptr;
}

static RegexMatchRange ParseIndex(SmileArg arg, RegexMatch regexMatch, const char *methodName)
{
	Int64 index;
	String stringKey;

	switch (arg.obj->kind) {
		case SMILE_KIND_UNBOXED_INTEGER64:
			// Integer key means to retrieve one of the indexed captures (0 == whole match).
			index = arg.unboxed.i64;
			if (index < 0 || index >= regexMatch->numIndexedCaptures)
				return NULL;
			return regexMatch->indexedCaptures + (Int)index;

		case SMILE_KIND_UNBOXED_SYMBOL:
			stringKey = SymbolTable_GetName(Smile_SymbolTable, arg.unboxed.symbol);
			goto stringCommon;
		case SMILE_KIND_STRING:
			stringKey = (String)arg.obj;
		stringCommon:
			if (StringIntDict_Count(&regexMatch->namedCaptures) == 0)
				return NULL;
			index = StringIntDict_GetValue(&regexMatch->namedCaptures, stringKey);
			return regexMatch->indexedCaptures + index;

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_Format("Second argument to 'RegexMatch.%s' must be an Integer64, a String, or a Symbol.", methodName));
	}
}

SMILE_EXTERNAL_FUNCTION(GetMember)
{
	RegexMatch regexMatch;
	RegexMatchRange range;
	String matchedSubstring;

	regexMatch = GetRegexMatchObject(argv[0].obj, "get-member");
	range = ParseIndex(argv[1], regexMatch, "get-member");

	if (range == NULL)
		return SmileArg_From(NullObject);

	matchedSubstring = String_Substring(regexMatch->input, range->start, range->length);
	return SmileArg_From((SmileObject)matchedSubstring);
}

SMILE_EXTERNAL_FUNCTION(List)
{
	RegexMatch regexMatch;
	RegexMatchRange range;
	String capture;
	SmileList head, tail;
	Int i;

	regexMatch = GetRegexMatchObject(argv[0].obj, "list");

	if (!regexMatch->isMatch)
		return SmileArg_From(NullObject);

	head = tail = NullList;
	for (i = 0; i < regexMatch->numIndexedCaptures; i++) {
		range = regexMatch->indexedCaptures + i;
		capture = String_Substring(regexMatch->input, range->start, range->length);
		LIST_APPEND(head, tail, capture);
	}

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(Count)
{
	RegexMatch regexMatch = GetRegexMatchObject(argv[0].obj, "count");

	if (!regexMatch->isMatch)
		return SmileUnboxedInteger64_From(0);

	return SmileUnboxedInteger64_From(regexMatch->numIndexedCaptures);
}

SMILE_EXTERNAL_FUNCTION(CountNames)
{
	RegexMatch regexMatch = GetRegexMatchObject(argv[0].obj, "count-names");

	if (!regexMatch->isMatch)
		return SmileUnboxedInteger64_From(0);

	return SmileUnboxedInteger64_From(StringIntDict_Count(&regexMatch->namedCaptures));
}

SMILE_EXTERNAL_FUNCTION(Names)
{
	RegexMatch regexMatch;
	SmileList head, tail;
	Int i, count;
	String *keys;

	regexMatch = GetRegexMatchObject(argv[0].obj, "names");

	if (!regexMatch->isMatch)
		return SmileArg_From(NullObject);

	head = tail = NullList;
	if ((count = StringIntDict_Count(&regexMatch->namedCaptures)) > 0) {
		keys = StringIntDict_GetKeys(&regexMatch->namedCaptures);
		for (i = 0; i < count; i++) {
			LIST_APPEND(head, tail, keys[i]);
		}
	}

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(NamedMatches)
{
	RegexMatch regexMatch;
	RegexMatchRange range;
	SmileList head, tail;
	Int i, count, index;
	String *keys;
	String key, value;
	SmileList tuple;

	regexMatch = GetRegexMatchObject(argv[0].obj, "named-matches");

	if (!regexMatch->isMatch)
		return SmileArg_From(NullObject);

	head = tail = NullList;

	if ((count = StringIntDict_Count(&regexMatch->namedCaptures)) > 0) {
		keys = StringIntDict_GetKeys(&regexMatch->namedCaptures);
		for (i = 0; i < count; i++) {
			key = keys[i];
			index = StringIntDict_GetValue(&regexMatch->namedCaptures, key);
			range = regexMatch->indexedCaptures + index;
			value = String_Substring(regexMatch->input, range->start, range->length);
			tuple = SmileList_Cons((SmileObject)key,
				(SmileObject)SmileList_Cons((SmileObject)value,
					NullObject));
			LIST_APPEND(head, tail, tuple);
		}
	}

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(Range)
{
	RegexMatch regexMatch;
	SmileInteger64Range range;
	RegexMatchRange matchRange;

	regexMatch = GetRegexMatchObject(argv[0].obj, "range");

	if (!regexMatch->isMatch)
		return SmileArg_From(NullObject);

	if (argc > 1) {
		matchRange = ParseIndex(argv[1], regexMatch, "range");
	}
	else {
		matchRange = regexMatch->indexedCaptures;
	}
	if (matchRange == NULL)
		return SmileArg_From(NullObject);

	range = SmileInteger64Range_Create(matchRange->start, matchRange->start + matchRange->length - 1, 1);
	return SmileArg_From((SmileObject)range);
}

SMILE_EXTERNAL_FUNCTION(Object)
{
	RegexMatch regexMatch;
	RegexMatchRange range;
	Int i, count, index;
	String *keys;
	String key, value;
	SmileUserObject obj;

	regexMatch = GetRegexMatchObject(argv[0].obj, "object");

	if (!regexMatch->isMatch)
		return SmileArg_From(NullObject);

	obj = SmileUserObject_Create((SmileObject)Smile_KnownBases.Object, 0);

	if ((count = StringIntDict_Count(&regexMatch->namedCaptures)) > 0) {
		keys = StringIntDict_GetKeys(&regexMatch->namedCaptures);
		for (i = 0; i < count; i++) {
			key = keys[i];
			index = StringIntDict_GetValue(&regexMatch->namedCaptures, key);
			range = regexMatch->indexedCaptures + index;
			value = String_Substring(regexMatch->input, range->start, range->length);
			SmileUserObject_Set(obj, SymbolTable_GetSymbol(Smile_SymbolTable, key), value);
		}
	}

	return SmileArg_From((SmileObject)obj);
}

SMILE_EXTERNAL_FUNCTION(Before)
{
	RegexMatch regexMatch;
	RegexMatchRange range;
	String before;

	regexMatch = GetRegexMatchObject(argv[0].obj, "before");

	if (!regexMatch->isMatch)
		return SmileArg_From(NullObject);

	range = regexMatch->indexedCaptures;
	before = String_Substring(regexMatch->input, 0, range->start);

	return SmileArg_From((SmileObject)before);
}

SMILE_EXTERNAL_FUNCTION(After)
{
	RegexMatch regexMatch;
	RegexMatchRange range;
	String after;

	regexMatch = GetRegexMatchObject(argv[0].obj, "before");

	if (!regexMatch->isMatch)
		return SmileArg_From(NullObject);

	range = regexMatch->indexedCaptures;
	after = String_SubstringAt(regexMatch->input, range->start + range->length);

	return SmileArg_From((SmileObject)after);
}

//-------------------------------------------------------------------------------------------------

void SmileRegexMatch_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "regex-match", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "regex-match", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "regex-match", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "regex-match", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _handleComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _handleComparisonChecks);

	SetupFunction("get-member", GetMember, NULL, "regex-match item", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _getMemberChecks);
	SetupFunction("range", Range, NULL, "regex-match", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _getMemberChecks);

	SetupFunction("list", List, NULL, "regex-match", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleComparisonChecks);
	SetupFunction("count", Count, NULL, "regex-match", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleComparisonChecks);

	SetupFunction("count-names", CountNames, NULL, "regex-match", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleComparisonChecks);
	SetupFunction("names", Names, NULL, "regex-match", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleComparisonChecks);
	SetupFunction("named-matches", NamedMatches, NULL, "regex-match", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleComparisonChecks);
	SetupFunction("object", Object, NULL, "regex-match", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleComparisonChecks);

	SetupFunction("before", Before, NULL, "regex-match", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleComparisonChecks);
	SetupFunction("after", After, NULL, "regex-match", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleComparisonChecks);
}
