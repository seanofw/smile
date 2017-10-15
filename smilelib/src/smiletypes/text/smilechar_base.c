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
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _charChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
};

static Byte _charComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
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
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_BYTE)
		return SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.ch);

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(_char, "Char");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_CHAR) {
		return SmileArg_From((SmileObject)String_CreateRepeat(argv[0].unboxed.ch, 1));
	}

	return SmileArg_From((SmileObject)_char);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileChar obj = (SmileChar)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_CHAR)
		return SmileUnboxedInteger64_From((UInt32)obj->ch);

	return SmileUnboxedInteger64_From((UInt32)((PtrInt)obj ^ Smile_HashOracle));
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return SmileUnboxedInteger32_From((Int32)(UInt32)argv[0].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	return SmileUnboxedInteger16_From((Int16)(UInt16)argv[0].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	return SmileUnboxedByte_From((Byte)argv[0].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(ToChar)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(ToUni)
{
	return SmileUnboxedUni_From(argv[0].unboxed.ch);
}

//-------------------------------------------------------------------------------------------------
// Pseudo-arithmetic

SMILE_EXTERNAL_FUNCTION(Plus)
{
	switch (SMILE_KIND(argv[1].obj)) {

		case SMILE_KIND_UNBOXED_INTEGER64:
			return SmileUnboxedChar_From((Byte)(argv[0].unboxed.ch + argv[1].unboxed.i64));
		case SMILE_KIND_UNBOXED_INTEGER32:
			return SmileUnboxedChar_From((Byte)(argv[0].unboxed.ch + argv[1].unboxed.i32));
		case SMILE_KIND_UNBOXED_INTEGER16:
			return SmileUnboxedChar_From((Byte)(argv[0].unboxed.ch + argv[1].unboxed.i16));
		case SMILE_KIND_UNBOXED_BYTE:
			return SmileUnboxedChar_From((Byte)(argv[0].unboxed.ch + argv[1].unboxed.i8));

		case SMILE_KIND_STRING:
			{
				String other = (String)(argv[1].obj);
				String self = String_CreateRepeat(argv[0].unboxed.ch, 1);
				return SmileArg_From((SmileObject)String_Concat(self, other));
			}

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("Second argument to Char.+ must be an integer or a string."));
			break;
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	switch (SMILE_KIND(argv[1].obj)) {

		case SMILE_KIND_UNBOXED_INTEGER64:
			return SmileUnboxedChar_From((Byte)(argv[0].unboxed.ch - argv[1].unboxed.i64));
		case SMILE_KIND_UNBOXED_INTEGER32:
			return SmileUnboxedChar_From((Byte)(argv[0].unboxed.ch - argv[1].unboxed.i32));
		case SMILE_KIND_UNBOXED_INTEGER16:
			return SmileUnboxedChar_From((Byte)(argv[0].unboxed.ch - argv[1].unboxed.i16));
		case SMILE_KIND_UNBOXED_BYTE:
			return SmileUnboxedChar_From((Byte)(argv[0].unboxed.ch - argv[1].unboxed.i8));

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("Second argument to Char.- must be an integer."));
			break;
	}
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_CHAR
		&& argv[0].unboxed.ch == argv[1].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_CHAR
		|| argv[0].unboxed.ch != argv[1].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.ch < argv[1].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.ch > argv[1].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(argv[0].unboxed.ch <= argv[1].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(argv[0].unboxed.ch >= argv[1].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	Byte x = argv[0].unboxed.ch;
	Byte y = argv[1].unboxed.ch;

	if (x == y)
		return SmileUnboxedInteger64_From(0);
	else if (x < y)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

//-------------------------------------------------------------------------------------------------

void SmileChar_Setup(SmileUserObject base)
{
	SmileUnboxedChar_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "ch", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "ch", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "ch", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "ch", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("int64", ToInt64, NULL, "ch", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _charChecks);
	SetupFunction("int32", ToInt32, NULL, "ch", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _charChecks);
	SetupFunction("int16", ToInt16, NULL, "ch", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _charChecks);
	SetupFunction("byte", ToByte, NULL, "ch", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _charChecks);

	SetupFunction("char", ToChar, NULL, "ch", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _charChecks);
	SetupFunction("uni", ToUni, NULL, "ch", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _charChecks);

	SetupFunction("+", Plus, NULL, "ch other", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charComparisonChecks);
	SetupFunction("-", Minus, NULL, "ch other", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charComparisonChecks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _charChecks);
	SetupSynonym("compare", "cmp");
}
