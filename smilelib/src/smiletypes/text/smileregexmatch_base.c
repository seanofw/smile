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
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilehandle.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/regex.h>
#include <smile/internal/staticstring.h>

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
				? regexMatch->numIndexedCaptures + StringIntDict_Count(regexMatch->namedCaptures)
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
				? regexMatch->numIndexedCaptures + StringIntDict_Count(regexMatch->namedCaptures)
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

SMILE_EXTERNAL_FUNCTION(GetMember)
{
	SmileHandle handle;
	RegexMatch regexMatch;
	String stringKey;
	Int64 index;
	RegexMatchRange range;
	String matchedSubstring;

	handle = (SmileHandle)argv[0].obj;
	if (handle->handleKind != Smile_KnownSymbols.RegexMatch_) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FormatString(_handleException, "get-member",
				SymbolTable_GetName(Smile_SymbolTable, handle->handleKind)));
	}

	regexMatch = (RegexMatch)handle->ptr;

	switch (argv[1].obj->kind) {
		case SMILE_KIND_UNBOXED_INTEGER64:
			// Integer key means to retrieve one of the indexed captures (0 == whole match).
			index = argv[1].unboxed.i64;
			if (index < 0 || index >= regexMatch->numIndexedCaptures)
				return SmileArg_From(NullObject);
			range = regexMatch->indexedCaptures + (Int)index;
			break;

		case SMILE_KIND_UNBOXED_SYMBOL:
			stringKey = SymbolTable_GetName(Smile_SymbolTable, argv[1].unboxed.symbol);
			goto stringCommon;
		case SMILE_KIND_STRING:
			stringKey = (String)argv[1].obj;
		stringCommon:
			if (regexMatch->namedCaptures == NULL)
				return SmileArg_From(NullObject);
			index = StringIntDict_GetValue(regexMatch->namedCaptures, stringKey);
			range = regexMatch->indexedCaptures + index;
			break;

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("Second argument to 'RegexMatch.get-member' must be an Integer64, a String, or a Symbol."));
	}

	matchedSubstring = String_Substring(regexMatch->input, range->start, range->length);
	return SmileArg_From((SmileObject)matchedSubstring);
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
}
