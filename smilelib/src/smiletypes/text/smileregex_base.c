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
			return SmileUnboxedInteger64_From(Smile_ApplyHashOracle(String_Hash(regex->pattern) + String_Hash(regex->flags)));
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
	Regex regex;
	Bool result;

	if (handle->handleKind != Smile_KnownSymbols.Regex_) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FormatString(_handleException, "matches?",
				SymbolTable_GetName(Smile_SymbolTable, handle->handleKind)));
	}

	regex = (Regex)handle->ptr;
	result = Regex_Test(regex, input);

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Match)
{
	SmileHandle handle = (SmileHandle)argv[0].obj;
	String input = (String)argv[1].obj;
	Regex regex;
	RegexMatch result;

	if (handle->handleKind != Smile_KnownSymbols.Regex_) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FormatString(_handleException, "match",
				SymbolTable_GetName(Smile_SymbolTable, handle->handleKind)));
	}

	regex = (Regex)handle->ptr;
	result = Regex_Match(regex, input);

	return SmileUnboxedBool_From(True);
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

	SetupFunction("matches?", Matches, NULL, "regex string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _matchChecks);
	SetupFunction("match", Match, NULL, "regex string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _matchChecks);
}
