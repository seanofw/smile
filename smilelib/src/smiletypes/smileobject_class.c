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

#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/eval/eval.h>
#include <smile/smiletypes/base.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

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
	STATIC_STRING(object, "Object");

	return SmileArg_From((SmileObject)object);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)argv[0].obj));
}

//-------------------------------------------------------------------------------------------------
// Construction functions.

SMILE_EXTERNAL_FUNCTION(Base)
{
	SmileObject target = argv[argc - 1].obj;
	return SmileArg_From(target == Smile_KnownBases.Primitive ? target : target->base);
}

static Int PropertyNameComparer(SmileObject a, SmileObject b, void *param)
{
	SmileSymbol sa = (SmileSymbol)a;
	SmileSymbol sb = (SmileSymbol)b;

	String na = SymbolTable_GetName(Smile_SymbolTable, sa->symbol);
	String nb = SymbolTable_GetName(Smile_SymbolTable, sb->symbol);

	return String_Compare(na, nb);
}

SMILE_EXTERNAL_FUNCTION(PropertyNames)
{
	SmileObject target = argv[argc - 1].obj;
	SmileList propertyNames = SMILE_VCALL(target, getPropertyNames);
	propertyNames = SmileList_Sort(propertyNames, PropertyNameComparer, NULL);
	return SmileArg_From((SmileObject)propertyNames);
}

SMILE_EXTERNAL_FUNCTION(GetProperty)
{
	SmileObject target = argv[argc - 2].obj;
	SmileObject value;
	Symbol propertyName;

	if (SMILE_KIND(argv[argc - 1].obj) != SMILE_KIND_UNBOXED_SYMBOL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FromC("Object.get-property must be passed a target and a name."));
	}

	propertyName = argv[argc - 1].unboxed.symbol;
	value = SMILE_VCALL1(target, getProperty, propertyName);

	return SmileArg_From(value);
}

SMILE_EXTERNAL_FUNCTION(HasProperty)
{
	SmileObject target = argv[argc - 2].obj;
	Bool result;
	Symbol propertyName;

	if (SMILE_KIND(argv[argc - 1].obj) != SMILE_KIND_UNBOXED_SYMBOL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FromC("Object.has-property must be passed a target and a name."));
	}

	propertyName = argv[argc - 1].unboxed.symbol;
	result = SMILE_VCALL1(target, hasProperty, propertyName);

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(SetProperty)
{
	SmileObject target = argv[argc - 3].obj;
	SmileObject value = SmileArg_Box(argv[argc - 1]);
	Symbol propertyName;

	if (SMILE_KIND(argv[argc - 2].obj) != SMILE_KIND_UNBOXED_SYMBOL) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FromC("Object.set-property must be passed a target, a name, and a value."));
	}

	propertyName = argv[argc - 2].unboxed.symbol;
	SMILE_VCALL2(target, setProperty, propertyName, value);

	return argv[argc - 1];
}

SMILE_EXTERNAL_FUNCTION(GetAccess)
{
	switch (argv[0].obj->kind & SMILE_SECURITY_READWRITEAPPEND) {
		case SMILE_SECURITY_READONLY:
			return SmileUnboxedSymbol_From(Smile_KnownSymbols.read_only);
		case SMILE_SECURITY_WRITABLE:
			return SmileUnboxedSymbol_From(Smile_KnownSymbols.read_write);
		case SMILE_SECURITY_APPENDABLE:
			return SmileUnboxedSymbol_From(Smile_KnownSymbols.read_append);
		case SMILE_SECURITY_READWRITEAPPEND:
			return SmileUnboxedSymbol_From(Smile_KnownSymbols.read_write_append);
		default:
			Smile_Abort_FatalError("Illegal object access setting.");
			return SmileArg_From(NullObject);
	}
}

SMILE_EXTERNAL_FUNCTION(SetAccessKey)
{
	SmileObject target = argv[0].obj;
	SmileObject newKey = SmileArg_Box(argv[1]);
	SmileObject oldKey = argc > 2 ? SmileArg_Box(argv[2]) : NullObject;

	Bool result = SMILE_VCALL2(target, setSecurityKey, newKey, oldKey);
	if (!result)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, String_InvalidSecurityKey);

	return SmileArg_From(target);
}

SMILE_EXTERNAL_FUNCTION(SetAccess)
{
	SmileObject target, mode, key;
	Symbol modeSymbol;
	Bool result;
	Int newMode;

	STATIC_STRING(invalidMode, "Invalid security mode.");

	target = argv[0].obj;
	mode = argv[1].obj;
	key = argc > 2 ? SmileArg_Box(argv[2]) : NullObject;

	if (SMILE_KIND(mode) != SMILE_KIND_UNBOXED_SYMBOL)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, invalidMode);

	if (!(target->kind & SMILE_SECURITY_UNFROZEN))
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, String_InvalidSecurityKey);

	modeSymbol = argv[1].unboxed.symbol;
	if (modeSymbol == Smile_KnownSymbols.read_only)
		newMode = SMILE_SECURITY_READONLY;
	else if (modeSymbol == Smile_KnownSymbols.read_write)
		newMode = SMILE_SECURITY_WRITABLE;
	else if (modeSymbol == Smile_KnownSymbols.read_append)
		newMode = SMILE_SECURITY_APPENDABLE;
	else if (modeSymbol == Smile_KnownSymbols.read_write_append)
		newMode = SMILE_SECURITY_READWRITEAPPEND;
	else
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, invalidMode);

	result = SMILE_VCALL2(target, setSecurity, newMode, key);
	if (!result)
		Smile_ThrowException(Smile_KnownSymbols.object_security_error, String_InvalidSecurityKey);

	return SmileArg_From(target);
}

SMILE_EXTERNAL_FUNCTION(Freeze)
{
	SmileObject target, key;

	target = argv[0].obj;
	key = argc > 1 ? SmileArg_Box(argv[1]) : NullObject;

	if (!(target->kind & SMILE_SECURITY_UNFROZEN))
		return argv[0];

	if (SMILE_VCALL2(target, setSecurity, SMILE_SECURITY_READONLY, key))
		argv[0].obj->kind &= ~SMILE_SECURITY_UNFROZEN;

	return SmileArg_From(target);
}

//-------------------------------------------------------------------------------------------------

void SmileObject_Setup(SmileUserObject base)
{
	base->name = Smile_KnownSymbols.Object_;

	SetupFunction("bool", ToBool, NULL, "pair", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "pair", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "pair", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "pair", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("base", Base, (void *)base, "obj target", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("property-names", PropertyNames, (void *)base, "obj target", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("get-property", GetProperty, (void *)base, "obj target name", ARG_CHECK_MIN | ARG_CHECK_MAX, 2, 3, 0, NULL);
	SetupFunction("has-property", HasProperty, (void *)base, "obj target name", ARG_CHECK_MIN | ARG_CHECK_MAX, 2, 3, 0, NULL);
	SetupFunction("set-property", SetProperty, (void *)base, "obj target name value", ARG_CHECK_MIN | ARG_CHECK_MAX, 3, 4, 0, NULL);

	SetupFunction("get-access", GetAccess, (void *)base, "obj", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("set-access", SetAccess, (void *)base, "obj mode key", ARG_CHECK_MIN | ARG_CHECK_MAX, 2, 3, 0, NULL);
	SetupFunction("set-access-key", SetAccessKey, (void *)base, "obj key old-key", ARG_CHECK_MIN | ARG_CHECK_MAX, 2, 3, 0, NULL);
	SetupFunction("freeze", Freeze, (void *)base, "obj key", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
}
