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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/collections/smilesymbolmap.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/eval/eval.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _symbolMapChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
};

static Byte _symbolMapMemberChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_SYMBOLMAP,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
	0, 0,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_SYMBOLMAP)
		return SmileUnboxedBool_From(Int32Dict_Count(&((SmileSymbolMap)argv[0].obj)->dict) > 0);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_SYMBOLMAP)
		return SmileUnboxedInteger64_From(Int32Dict_Count(&((SmileSymbolMap)argv[0].obj)->dict));

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(symbolMap, "SymbolMap");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_SYMBOLMAP)
		return SmileArg_From((SmileObject)String_Format("SymbolMap of %d", Int32Dict_Count(&((SmileSymbolMap)argv[0].obj)->dict)));

	return SmileArg_From((SmileObject)symbolMap);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	return SmileUnboxedInteger64_From((PtrInt)argv[0].obj ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Instantiation

SMILE_EXTERNAL_FUNCTION(Create)
{
	SmileSymbolMap symbolMap = SmileSymbolMap_Create();

	return SmileArg_From((SmileObject)symbolMap);
}

//-------------------------------------------------------------------------------------------------
// Member-access

SMILE_EXTERNAL_FUNCTION(GetMember)
{
	SmileSymbolMap symbolMap = (SmileSymbolMap)argv[0].obj;
	Symbol symbol = argv[1].unboxed.symbol;

	void *value;
	if (!Int32Dict_TryGetValue(&symbolMap->dict, symbol, &value))
		return SmileArg_From(NullObject);

	return SmileArg_From((SmileObject)value);
}

SMILE_EXTERNAL_FUNCTION(SetMember)
{
	SmileSymbolMap symbolMap = (SmileSymbolMap)argv[0].obj;
	Symbol symbol = argv[1].unboxed.symbol;
	SmileObject value = SmileArg_Box(argv[2]);

	Int32Dict_SetValue(&symbolMap->dict, symbol, (void *)value);

	return argv[2];
}

//-------------------------------------------------------------------------------------------------
// Map-specific methods.

SMILE_EXTERNAL_FUNCTION(Add)
{
	SmileSymbolMap symbolMap = (SmileSymbolMap)argv[0].obj;
	Symbol symbol = argv[1].unboxed.symbol;
	SmileObject value = SmileArg_Box(argv[2]);

	return SmileUnboxedBool_From(Int32Dict_Add(&symbolMap->dict, symbol, (void *)value));
}

SMILE_EXTERNAL_FUNCTION(Remove)
{
	SmileSymbolMap symbolMap;
	Symbol symbol;
	Int i;
	Bool result = True;
	
	if (SMILE_KIND(argv[0].obj) != SMILE_KIND_SYMBOLMAP)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument 1 to 'remove' should be a SymbolMap but is a %S.",
			SmileKind_GetName(SMILE_KIND(argv[0].obj))));
	symbolMap = (SmileSymbolMap)argv[0].obj;

	// Remvoe all of the provided symbols, at once.
	for (i = 1; i < argc; i++) {
		if (SMILE_KIND(argv[i].obj) != SMILE_KIND_UNBOXED_SYMBOL) {
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("Argument %d to 'remove' should be a Symbol but is a %S.",
				i + 1, SmileKind_GetName(SMILE_KIND(argv[i].obj))));
		}

		symbol = argv[i].unboxed.symbol;
		result &= Int32Dict_Remove(&symbolMap->dict, symbol);
	}

	// True if all of them were removed, false if any of them didn't exist.
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Key)
{
	SmileSymbolMap symbolMap = (SmileSymbolMap)argv[0].obj;
	Symbol symbol = argv[1].unboxed.symbol;

	return SmileUnboxedBool_From(Int32Dict_ContainsKey(&symbolMap->dict, symbol));
}

SMILE_EXTERNAL_FUNCTION(Keys)
{
	SmileSymbolMap symbolMap = (SmileSymbolMap)argv[0].obj;
	SmileList head = NullList, tail = NullList;

	INT32DICT_WALK(&symbolMap->dict, {
		LIST_APPEND(head, tail, SmileSymbol_Create(node->key));
	})

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(Values)
{
	SmileSymbolMap symbolMap = (SmileSymbolMap)argv[0].obj;
	SmileList head = NullList, tail = NullList;

	INT32DICT_WALK(&symbolMap->dict, {
		LIST_APPEND(head, tail, (SmileObject)node->value);
	})

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(Pairs)
{
	SmileSymbolMap symbolMap = (SmileSymbolMap)argv[0].obj;
	SmileList head = NullList, tail = NullList;

	INT32DICT_WALK(&symbolMap->dict, {
		LIST_APPEND(head, tail, SmileList_CreateTwo(SmileSymbol_Create(node->key), (SmileObject)node->value));
	})

	return SmileArg_From((SmileObject)head);
}

//-------------------------------------------------------------------------------------------------

void SmileSymbolMap_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "map", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "map", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "map", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "map", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("create", Create, NULL, "", ARG_CHECK_MIN | ARG_CHECK_MAX, 0, 1, 0, NULL);

	SetupFunction("get-member", GetMember, NULL, "map symbol", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolMapMemberChecks);
	SetupFunction("set-member", SetMember, NULL, "map symbol value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _symbolMapMemberChecks);

	SetupFunction("add", Add, NULL, "map symbol value", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _symbolMapMemberChecks);
	SetupFunction("remove", Remove, NULL, "map symbols...", ARG_CHECK_MIN, 2, 0, 0, NULL);

	SetupFunction("key?", Key, NULL, "map symbol", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _symbolMapMemberChecks);
	SetupSynonym("key?", "has-key?");
	SetupSynonym("key?", "contains-key?");

	SetupFunction("keys", Keys, NULL, "map", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _symbolMapChecks);
	SetupSynonym("keys", "keys-of");
	SetupFunction("values", Values, NULL, "map", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _symbolMapChecks);
	SetupSynonym("values", "values-of");
	SetupFunction("pairs", Pairs, NULL, "map", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _symbolMapChecks);
	SetupSynonym("pairs", "pairs-of");
}
