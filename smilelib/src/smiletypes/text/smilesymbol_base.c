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
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smilefunction.h>
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
	STATIC_STRING(_symbol, "Symbol");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_SYMBOL) {
		Symbol symbol = argv[0].unboxed.symbol;
		return SmileArg_From((SmileObject)SymbolTable_GetName(Smile_SymbolTable, symbol));
	}

	return SmileArg_From((SmileObject)_symbol);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileSymbol obj = (SmileSymbol)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_SYMBOL)
		return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)argv[0].unboxed.symbol));

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)obj));
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_SYMBOL
		&& argv[0].unboxed.symbol == argv[1].unboxed.symbol);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_SYMBOL
		|| argv[0].unboxed.symbol != argv[1].unboxed.symbol);
}

Inline Int Cmp(Symbol a, Symbol b)
{
	String as = SymbolTable_GetName(Smile_SymbolTable, a);
	String bs = SymbolTable_GetName(Smile_SymbolTable, b);
	return String_Compare(as, bs);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(Cmp(argv[0].unboxed.symbol, argv[1].unboxed.symbol) < 0);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(Cmp(argv[0].unboxed.symbol, argv[1].unboxed.symbol) > 0);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(Cmp(argv[0].unboxed.symbol, argv[1].unboxed.symbol) <= 0);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(Cmp(argv[0].unboxed.symbol, argv[1].unboxed.symbol) >= 0);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Int cmp = Cmp(argv[0].unboxed.symbol, argv[1].unboxed.symbol);
	return SmileUnboxedInteger64_From((Int64)cmp);
}

//-------------------------------------------------------------------------------------------------

void SmileSymbol_Setup(SmileUserObject base)
{
	SmileUnboxedSymbol_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "ch", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "ch", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "ch", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "ch", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolChecks);
	SetupSynonym("compare", "cmp");
}
