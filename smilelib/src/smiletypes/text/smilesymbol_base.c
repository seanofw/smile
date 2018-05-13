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
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _symbolChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
};

static Byte _symbolComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	0, 0,
};

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(From)
{
	String string;
	Symbol symbol;

	if (argv[0].obj == param && argc == 2) {
		if (SMILE_KIND(argv[1].obj) != SMILE_KIND_STRING)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument 2 to 'from' should be a String but is a %S.",
				SmileKind_GetName(SMILE_KIND(argv[1].obj))));
		string = (String)argv[1].obj;
	}
	else {
		if (SMILE_KIND(argv[0].obj) != SMILE_KIND_STRING)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument 1 to 'from' should be a String but is a %S.",
				SmileKind_GetName(SMILE_KIND(argv[0].obj))));
		string = (String)argv[0].obj;
	}

	symbol = SymbolTable_GetSymbol(Smile_SymbolTable, string);
	return SmileUnboxedSymbol_From(symbol);
}

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(symbol, "Symbol");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_SYMBOL)
		return SmileArg_From((SmileObject)SymbolTable_GetName(Smile_SymbolTable, argv[0].unboxed.symbol));

	return SmileArg_From((SmileObject)symbol);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_SYMBOL)
		return SmileUnboxedInteger64_From(argv[0].unboxed.symbol ^ Smile_HashOracle);

	return SmileUnboxedInteger64_From(((PtrInt)argv[0].obj) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_SYMBOL
		|| argv[0].unboxed.symbol != argv[1].unboxed.symbol)
		return SmileUnboxedBool_From(False);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_SYMBOL
		|| argv[0].unboxed.symbol != argv[1].unboxed.symbol)
		return SmileUnboxedBool_From(True);

	return SmileUnboxedBool_From(False);
}

#define RELATIVE_COMPARE(__name__, __op__) \
	SMILE_EXTERNAL_FUNCTION(__name__) \
	{ \
		Symbol x = argv[0].unboxed.symbol; \
		Symbol y = argv[1].unboxed.symbol; \
		Int cmp = x == y ? 0 \
			: String_Compare(SymbolTable_GetName(Smile_SymbolTable, x), SymbolTable_GetName(Smile_SymbolTable, y)); \
		\
		return SmileUnboxedBool_From(cmp __op__ 0); \
	}

RELATIVE_COMPARE(Lt, <)
	RELATIVE_COMPARE(Gt, >)
	RELATIVE_COMPARE(Le, <= )
	RELATIVE_COMPARE(Ge, >= )

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Symbol x = argv[0].unboxed.symbol;
	Symbol y = argv[1].unboxed.symbol;
	
	if (x == y) return SmileUnboxedInteger64_From(0);

	if (String_Compare(SymbolTable_GetName(Smile_SymbolTable, x), SymbolTable_GetName(Smile_SymbolTable, y)) < 0)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

//-------------------------------------------------------------------------------------------------

void SmileSymbol_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("from", From, (void *)base, "string", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);
	SetupSynonym("compare", "cmp");
}
