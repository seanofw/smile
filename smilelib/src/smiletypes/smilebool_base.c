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
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/range/smilecharrange.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _boolChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
};

static Byte _boolComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BOOL,
	0, 0,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BOOL)
		return argv[0];

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BOOL)
		return SmileUnboxedInteger64_From(argv[0].unboxed.b ? 1 : 0);

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(_bool, "Bool");
	STATIC_STRING(_true, "true");
	STATIC_STRING(_false , "false");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BOOL)
		return SmileArg_From((SmileObject)(argv[0].unboxed.b ? _true : _false));

	return SmileArg_From((SmileObject)_bool);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileBool obj = (SmileBool)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_BOOL)
		return SmileUnboxedInteger64_From(argv[0].unboxed.b ? 1 : 0);

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)obj));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic

SMILE_EXTERNAL_FUNCTION(And)
{
	return SmileUnboxedBool_From(argv[0].unboxed.b & argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Or)
{
	return SmileUnboxedBool_From(argv[0].unboxed.b | argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Xor)
{
	return SmileUnboxedBool_From(argv[0].unboxed.b ^ argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Nand)
{
	return SmileUnboxedBool_From(!(argv[0].unboxed.b & argv[1].unboxed.b));
}

SMILE_EXTERNAL_FUNCTION(Nor)
{
	return SmileUnboxedBool_From(!(argv[0].unboxed.b | argv[1].unboxed.b));
}

SMILE_EXTERNAL_FUNCTION(Xnor)
{
	return SmileUnboxedBool_From(!(argv[0].unboxed.b ^ argv[1].unboxed.b));
}

SMILE_EXTERNAL_FUNCTION(Not)
{
	return SmileUnboxedBool_From(!argv[0].unboxed.b);
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_BOOL
		&& argv[0].unboxed.b == argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_BOOL
		|| argv[0].unboxed.b != argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.b < argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.b > argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(argv[0].unboxed.b <= argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(argv[0].unboxed.b >= argv[1].unboxed.b);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Bool x = argv[0].unboxed.b;
	Bool y = argv[1].unboxed.b;

	if (x == y)
		return SmileUnboxedInteger64_From(0);
	else if (x < y)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

//-------------------------------------------------------------------------------------------------
// Conversions and value-testing.

SMILE_EXTERNAL_FUNCTION(IsTrue)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(IsFalse)
{
	return SmileUnboxedBool_From(!argv[0].unboxed.b);
}

//-------------------------------------------------------------------------------------------------

void SmileBool_Setup(SmileUserObject base)
{
	SmileUnboxedBool_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "bool", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "bool", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "bool", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "bool", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("and", And, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupSynonym("and", "*");
	SetupFunction("or", Or, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupSynonym("or", "+");
	SetupFunction("xor", Xor, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupSynonym("xor", "^");
	SetupFunction("nand", Nand, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupFunction("nor", Nand, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupFunction("xnor", Xnor, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupFunction("not", Not, NULL, "bool", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _boolChecks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _boolChecks);
	SetupSynonym("compare", "cmp");
}
