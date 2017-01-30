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
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilefunction.h>

#define Setup(__name__, __value__) \
	(SmileUserObject_QuickSet(base, (__name__), (__value__)))

#define SetupFunction(__name__, __function__, __param__, __argNames__, __argCheckFlags__, __minArgs__, __maxArgs__, __numArgsToTypeCheck__, __argTypeChecks__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), (__argCheckFlags__), (__minArgs__), (__maxArgs__), (__numArgsToTypeCheck__), (__argTypeChecks__))))

#define SetupSimpleFunction(__name__, __function__, __argNames__, __numArgs__) \
	(Setup((__name__), (SmileObject)SmileFunction_CreateExternalFunction((__function__), (__param__), \
		(__name__), (__argNames__), ARG_CHECK_EXACT, (__numArgs__), (__numArgs__), NULL)))

static Byte _listChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_LIST,
	SMILE_KIND_MASK, SMILE_KIND_LIST,
	SMILE_KIND_MASK, SMILE_KIND_LIST,
	SMILE_KIND_MASK, SMILE_KIND_LIST,
	SMILE_KIND_MASK, SMILE_KIND_LIST,
	SMILE_KIND_MASK, SMILE_KIND_LIST,
	SMILE_KIND_MASK, SMILE_KIND_LIST,
	SMILE_KIND_MASK, SMILE_KIND_LIST,
};

static Byte _joinChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_LIST,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
};

//-------------------------------------------------------------------------------------------------
// Generic type conversion

static SmileObject ToBool(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_LIST)
		return (SmileObject)Smile_KnownObjects.TrueObj;

	if (SMILE_KIND(argv[0]) == SMILE_KIND_NULL)
		return (SmileObject)Smile_KnownObjects.FalseObj;

	return (SmileObject)Smile_KnownObjects.TrueObj;
}

static SmileObject ToInt(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_LIST)
		return (SmileObject)SmileInteger64_Create(SmileList_Length(((SmileList)argv[0])));

	return (SmileObject)Smile_KnownObjects.ZeroInt64;
}

STATIC_STRING(_List, "List");
STATIC_STRING(_null, "null");

static SmileObject ToString(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_LIST) {
		return (SmileObject)SmileString_Create(SmileObject_Stringify(argv[0]));
	}
	else if (SMILE_KIND(argv[0]) == SMILE_KIND_NULL) {
		return (SmileObject)SmileString_Create(_null);
	}

	return (SmileObject)SmileString_Create(_List);
}

static SmileObject Hash(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	if (SMILE_KIND(argv[0]) == SMILE_KIND_NULL) {
		return (SmileObject)Smile_KnownObjects.ZeroInt64;
	}

	return (SmileObject)SmileInteger64_Create(((PtrInt)argv[0]) ^ Smile_HashOracle);
}

//-------------------------------------------------------------------------------------------------
// Construction functions.

static SmileObject Of(Int argc, SmileObject *argv, void *param)
{
	SmileList head = NullList, tail = NullList;
	SmileUserObject base = (SmileUserObject)param;
	Int i;

	i = 0;
	if (argv[i] == (SmileObject)base)
		i++;

	for (; i < argc; i++) {
		LIST_APPEND(head, tail, argv[i]);
	}

	return (SmileObject)head;
}

static SmileObject Cons(Int argc, SmileObject *argv, void *param)
{
	UNUSED(param);

	if (argc == 2)
		return (SmileObject)SmileList_Cons(argv[0], argv[1]);
	return (SmileObject)SmileList_Cons(argv[1], argv[2]);
}

STATIC_STRING(_cycleError, "List has infinite length because it contains a cycle.");

static SmileObject Length(Int argc, SmileObject *argv, void *param)
{
	Int length;

	UNUSED(argc);
	UNUSED(param);

	length = SmileList_SafeLength((SmileList)argv[0]);
	if (length < 0) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, _cycleError);
	}

	return (SmileObject)SmileInteger64_Create(length);
}

static SmileObject HasCycle(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return SmileList_HasCycle(argv[0]) ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

static SmileObject IsWellFormed(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return SmileList_IsWellFormed(argv[0]) ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
}

STATIC_STRING(_malformedListError, "The list passed to '%s' is malformed or contains a cycle.");

static SmileObject Join(Int argc, SmileObject *argv, void *param)
{
	String glue, result;

	UNUSED(param);

	if (argc <= 1)
		glue = String_Empty;
	else
		glue = (String)&((SmileString)argv[1])->string;
	
	result = SmileList_Join((SmileList)argv[0], glue);

	if (result == NULL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FormatString(_malformedListError, "join"));
	}

	return (SmileObject)SmileString_Create(result);
}

//-------------------------------------------------------------------------------------------------

static SmileObject Car(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileList)argv[0])->a;
}

static SmileObject Cdr(Int argc, SmileObject *argv, void *param)
{
	UNUSED(argc);
	UNUSED(param);

	return ((SmileList)argv[0])->d;
}

static SmileObject Caar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Cadr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Cdar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cddr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Caaar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Caadr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Cadar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Caddr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->a;
}

static SmileObject Cdaar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cdadr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cddar(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->a;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cdddr(Int argc, SmileObject *argv, void *param)
{
	SmileObject obj;

	UNUSED(argc);
	UNUSED(param);

	obj = ((SmileList)argv[0])->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	obj = ((SmileList)obj)->d;
	if (SMILE_KIND(obj) != SMILE_KIND_LIST)
		return NullObject;
	return ((SmileList)obj)->d;
}

static SmileObject Cxr(Int argc, SmileObject *argv, void *param)
{
	UInt32 flags;
	SmileObject obj = argv[0];

	UNUSED(argc);
	UNUSED(param);

	for (flags = (UInt32)(PtrInt)param; flags; flags >>= 3) {
		if (SMILE_KIND(obj) != SMILE_KIND_LIST)
			return NullObject;
		if ((flags & 3) == 1) {
			obj = ((SmileList)obj)->a;
		}
		else if ((flags & 3) == 2) {
			obj = ((SmileList)obj)->d;
		}
	}

	return obj;
}

//-------------------------------------------------------------------------------------------------

void SmileList_Setup(SmileUserObject base)
{
	SetupFunction("bool", ToBool, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "list", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "list", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("of", Of, (void *)base, "items", ARG_CHECK_MIN, 1, 0, 0, NULL);
	SetupFunction("cons", Cons, NULL, "a b", ARG_CHECK_MIN | ARG_CHECK_MAX, 2, 3, 0, NULL);

	SetupFunction("join", Join, NULL, "list", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _joinChecks);

	SetupFunction("length", Length, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cycle?", HasCycle, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("well-formed?", IsWellFormed, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);

	SetupFunction("car", Car, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdr", Cdr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("first", Car, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("rest", Cdr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);

	SetupFunction("caar", Caar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadr", Cadr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdar", Cdar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddr", Cddr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);

	SetupFunction("caaar", Caaar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caadr", Caadr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadar", Cadar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caddr", Caddr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdaar", Cdaar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdadr", Cdadr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddar", Cddar, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdddr", Cdddr, NULL, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);

	SetupFunction("caaaar", Cxr, (void *)0x00001111, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caaadr", Cxr, (void *)0x00001112, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caadar", Cxr, (void *)0x00001121, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caaddr", Cxr, (void *)0x00001122, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadaar", Cxr, (void *)0x00001211, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadadr", Cxr, (void *)0x00001212, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("caddar", Cxr, (void *)0x00001221, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cadddr", Cxr, (void *)0x00001222, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdaaar", Cxr, (void *)0x00002111, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdaadr", Cxr, (void *)0x00002112, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdadar", Cxr, (void *)0x00002121, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdaddr", Cxr, (void *)0x00002122, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddaar", Cxr, (void *)0x00002211, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddadr", Cxr, (void *)0x00002212, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cdddar", Cxr, (void *)0x00002221, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
	SetupFunction("cddddr", Cxr, (void *)0x00002222, "list", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _listChecks);
}
