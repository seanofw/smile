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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>

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

static Byte _stringComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	0, 0,
};

STATIC_STRING(_invalidTypeError, "All arguments to 'String.%s' must be of type 'String'");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

static SmileObject ToBool(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_STRING)
		return ((SmileString)argv[0])->string.length ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject ToInt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_STRING)
		return (SmileObject)SmileInteger64_Create(((SmileString)argv[0])->string.length);

	return (SmileObject)Smile_KnownObjects.ZeroInt64;
}

STATIC_STRING(_String, "String");

static SmileObject ToString(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_STRING)
		return argv[0];

	return (SmileObject)SmileString_Create(_String);
}

static SmileObject Hash(Int argc, SmileObject *argv, void *param)
{
	Int64 hash;

	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_STRING) {
		hash = String_Hash64(SmileString_GetString((SmileString)argv[0]));
		return (SmileObject)SmileInteger64_Create(hash);
	}

	return (SmileObject)SmileInteger64_Create(((PtrInt)argv[0]) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Arithmetic operators

static SmileObject Plus(Int argc, SmileObject *argv, void *param)
{
	String x;
	Int i;

	UNUSED(param);

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			x = SmileString_GetString((SmileString)argv[0]);
			x = String_Concat(x, SmileString_GetString((SmileString)argv[1]));
			return (SmileObject)SmileString_Create(x);
	
		case 3:
			x = SmileString_GetString((SmileString)argv[0]);
			x = String_Concat(x, SmileString_GetString((SmileString)argv[1]));
			x = String_Concat(x, SmileString_GetString((SmileString)argv[2]));
			return (SmileObject)SmileString_Create(x);

		case 4:
			x = SmileString_GetString((SmileString)argv[0]);
			x = String_Concat(x, SmileString_GetString((SmileString)argv[1]));
			x = String_Concat(x, SmileString_GetString((SmileString)argv[2]));
			x = String_Concat(x, SmileString_GetString((SmileString)argv[3]));
			return (SmileObject)SmileString_Create(x);

		default:
			x = SmileString_GetString((SmileString)argv[0]);
			for (i = 1; i < argc; i++) {
				x = String_Concat(x, SmileString_GetString((SmileString)argv[i]));
			}
			return (SmileObject)SmileString_Create(x);
	}
}

static SmileObject Minus(Int argc, SmileObject *argv, void *param)
{
	String x;

	UNUSED(param);

	if (argc == 2) {
		// Subtract:  Remove string y from string x.
		x = SmileString_GetString((SmileString)argv[0]);
		x = String_Replace(x, SmileString_GetString((SmileString)argv[1]), String_Empty);
		return (SmileObject)SmileString_Create(x);
	}
	else {
		// Negate:  Reverse string x.
		x = SmileString_GetString((SmileString)argv[0]);
		x = String_Reverse(x);
		return (SmileObject)SmileString_Create(x);
	}
}

//-------------------------------------------------------------------------------------------------
// Comparisons

static SmileObject Eq(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_STRING
		|| !String_Equals(SmileString_GetString((SmileString)argv[0]), SmileString_GetString((SmileString)argv[1])))
		return (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject Ne(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[1]) != SMILE_KIND_STRING
		|| !String_Equals(SmileString_GetString((SmileString)argv[0]), SmileString_GetString((SmileString)argv[1])))
		return (SmileObject)Smile_KnownObjects.TrueObj;

	return (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Lt(Int argc, SmileObject *argv, void *param)
{
	String x = SmileString_GetString((SmileString)argv[0]);
	String y = SmileString_GetString((SmileString)argv[1]);
	Int cmp = String_Compare(x, y);

	UNUSED(argc);
	UNUSED(param);

	return cmp < 0 ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Gt(Int argc, SmileObject *argv, void *param)
{
	String x = SmileString_GetString((SmileString)argv[0]);
	String y = SmileString_GetString((SmileString)argv[1]);
	Int cmp = String_Compare(x, y);

	UNUSED(argc);
	UNUSED(param);

	return cmp > 0 ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Le(Int argc, SmileObject *argv, void *param)
{
	String x = SmileString_GetString((SmileString)argv[0]);
	String y = SmileString_GetString((SmileString)argv[1]);
	Int cmp = String_Compare(x, y);

	UNUSED(argc);
	UNUSED(param);

	return cmp <= 0 ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Ge(Int argc, SmileObject *argv, void *param)
{
	String x = SmileString_GetString((SmileString)argv[0]);
	String y = SmileString_GetString((SmileString)argv[1]);
	Int cmp = String_Compare(x, y);

	UNUSED(argc);
	UNUSED(param);

	return cmp >= 0 ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject Compare(Int argc, SmileObject *argv, void *param)
{
	String x = SmileString_GetString((SmileString)argv[0]);
	String y = SmileString_GetString((SmileString)argv[1]);
	Int cmp = String_Compare(x, y);

	UNUSED(argc);
	UNUSED(param);

	if (cmp == 0)
		return (SmileObject)Smile_KnownObjects.ZeroInt64;
	else if (cmp < 0)
		return (SmileObject)Smile_KnownObjects.NegOneInt64;
	else
		return (SmileObject)Smile_KnownObjects.OneInt64;
}

//-------------------------------------------------------------------------------------------------

void SmileString_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("+", Plus, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _stringChecks);
	SetupFunction("-", Minus, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _stringChecks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
}
