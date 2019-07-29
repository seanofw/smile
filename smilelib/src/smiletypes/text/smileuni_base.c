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
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/range/smileunirange.h>
#include <smile/internal/staticstring.h>
#include <smile/internal/unicode.h>

SMILE_IGNORE_UNUSED_VARIABLES

static Byte _uniChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
};

static Byte _uniComparisonChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
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
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_UNI)
		return SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.uni);

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(uni, "Uni");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_UNBOXED_UNI) {
		return SmileArg_From((SmileObject)String_CreateFromUnicode(argv[0].unboxed.uni));
	}

	return SmileArg_From((SmileObject)uni);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	SmileUni obj = (SmileUni)argv[0].obj;

	if (SMILE_KIND(obj) == SMILE_KIND_UNBOXED_UNI)
		return SmileUnboxedInteger64_From((UInt32)obj->code);

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)obj));
}

//-------------------------------------------------------------------------------------------------
// Specialized type conversion

SMILE_EXTERNAL_FUNCTION(ToInt64)
{
	return SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.uni);
}

SMILE_EXTERNAL_FUNCTION(ToInt32)
{
	return SmileUnboxedInteger32_From((Int32)(UInt32)argv[0].unboxed.uni);
}

SMILE_EXTERNAL_FUNCTION(ToInt16)
{
	UInt32 uni = argv[0].unboxed.uni;
	return SmileUnboxedInteger16_From(uni <= 0xFFFF ? (Int16)(UInt16)uni : (Int16)(UInt16)0xFFFF);
}

SMILE_EXTERNAL_FUNCTION(ToByte)
{
	UInt32 uni = argv[0].unboxed.uni;
	return SmileUnboxedByte_From(uni <= 0xFF ? (Byte)uni : (Byte)0xFF);
}

SMILE_EXTERNAL_FUNCTION(ToChar)
{
	UInt32 uni = argv[0].unboxed.uni;
	return SmileUnboxedChar_From(uni <= 0xFF ? (Byte)uni : (Byte)0xFF);
}

SMILE_EXTERNAL_FUNCTION(ToUni)
{
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(RangeTo)
{
	Int32 start, end, step;

	start = (Int32)argv[0].unboxed.uni;
	end = (Int32)argv[1].unboxed.uni;
	step = end >= start ? +1 : -1;

	return SmileArg_From((SmileObject)SmileUniRange_Create(start, end, step));
}

//-------------------------------------------------------------------------------------------------
// Pseudo-arithmetic

SMILE_EXTERNAL_FUNCTION(Plus)
{
	switch (SMILE_KIND(argv[1].obj)) {

		case SMILE_KIND_UNBOXED_INTEGER64:
			{
				Int64 i64 = argv[1].unboxed.i64;
				if (i64 > 0x110000) i64 = 0x110000;
				if (i64 < -0x110000) i64 = -0x110000;
				return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.uni + (Int32)i64);
			}
		case SMILE_KIND_UNBOXED_INTEGER32:
			{
				Int32 i32 = argv[1].unboxed.i32;
				if (i32 > 0x110000) i32 = 0x110000;
				if (i32 < -0x110000) i32 = -0x110000;
				return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.uni + i32);
			}
		case SMILE_KIND_UNBOXED_INTEGER16:
			return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.uni + argv[1].unboxed.i16);
		case SMILE_KIND_UNBOXED_BYTE:
			return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.uni + argv[1].unboxed.i8);

		case SMILE_KIND_STRING:
			{
				String other = (String)(argv[1].obj);
				String self = String_CreateFromUnicode(argv[0].unboxed.uni);
				return SmileArg_From((SmileObject)String_Concat(self, other));
			}

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("Second argument to Uni.+ must be an integer or a string."));
			break;
	}
}

SMILE_EXTERNAL_FUNCTION(Minus)
{
	switch (SMILE_KIND(argv[1].obj)) {

		case SMILE_KIND_UNBOXED_INTEGER64:
			{
				Int64 i64 = argv[1].unboxed.i64;
				if (i64 > 0x110000) i64 = 0x110000;
				if (i64 < -0x110000) i64 = -0x110000;
				return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.uni - (Int32)i64);
			}
		case SMILE_KIND_UNBOXED_INTEGER32:
			{
				Int32 i32 = argv[1].unboxed.i32;
				if (i32 > 0x110000) i32 = 0x110000;
				if (i32 < -0x110000) i32 = -0x110000;
				return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.uni - i32);
			}
		case SMILE_KIND_UNBOXED_INTEGER16:
			return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.uni - argv[1].unboxed.i16);
		case SMILE_KIND_UNBOXED_BYTE:
			return SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.uni - argv[1].unboxed.i8);

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("Second argument to Char.- must be an integer."));
			break;
	}
}

//-------------------------------------------------------------------------------------------------
// Unicode-specific conversions

SMILE_EXTERNAL_FUNCTION(Category)
{
	Symbol symbol;
	UInt32 uni = argv[0].unboxed.uni;
	Byte category = Uni_GetGeneralCategory(uni);

	switch (category) {
		case GeneralCategory_Cn: symbol = Smile_KnownSymbols.Cn; break;
		case GeneralCategory_Cc: symbol = Smile_KnownSymbols.Cc; break;
		case GeneralCategory_Cf: symbol = Smile_KnownSymbols.Cf; break;
		case GeneralCategory_Cs: symbol = Smile_KnownSymbols.Cs; break;
		case GeneralCategory_Co: symbol = Smile_KnownSymbols.Co; break;
		case GeneralCategory_Lu: symbol = Smile_KnownSymbols.Lu; break;
		case GeneralCategory_Ll: symbol = Smile_KnownSymbols.Ll; break;
		case GeneralCategory_Lt: symbol = Smile_KnownSymbols.Lt; break;
		case GeneralCategory_Mn: symbol = Smile_KnownSymbols.Mn; break;
		case GeneralCategory_Mc: symbol = Smile_KnownSymbols.Mc; break;
		case GeneralCategory_Me: symbol = Smile_KnownSymbols.Me; break;
		case GeneralCategory_Nd: symbol = Smile_KnownSymbols.Nd; break;
		case GeneralCategory_Nl: symbol = Smile_KnownSymbols.Nl; break;
		case GeneralCategory_No: symbol = Smile_KnownSymbols.No; break;
		case GeneralCategory_Zs: symbol = Smile_KnownSymbols.Zs; break;
		case GeneralCategory_Zl: symbol = Smile_KnownSymbols.Zl; break;
		case GeneralCategory_Zp: symbol = Smile_KnownSymbols.Zp; break;
		case GeneralCategory_Lm: symbol = Smile_KnownSymbols.Lm; break;
		case GeneralCategory_Lo: symbol = Smile_KnownSymbols.Lo; break;
		case GeneralCategory_Pc: symbol = Smile_KnownSymbols.Pc; break;
		case GeneralCategory_Pd: symbol = Smile_KnownSymbols.Pd; break;
		case GeneralCategory_Ps: symbol = Smile_KnownSymbols.Ps; break;
		case GeneralCategory_Pe: symbol = Smile_KnownSymbols.Pe; break;
		case GeneralCategory_Pi: symbol = Smile_KnownSymbols.Pi; break;
		case GeneralCategory_Pf: symbol = Smile_KnownSymbols.Pf; break;
		case GeneralCategory_Po: symbol = Smile_KnownSymbols.Po; break;
		case GeneralCategory_Sm: symbol = Smile_KnownSymbols.Sm; break;
		case GeneralCategory_Sc: symbol = Smile_KnownSymbols.Sc; break;
		case GeneralCategory_Sk: symbol = Smile_KnownSymbols.Sk; break;
		case GeneralCategory_So: symbol = Smile_KnownSymbols.So; break;

		default: return SmileArg_From(NullObject);
	}

	return SmileUnboxedSymbol_From(symbol);
}

SMILE_EXTERNAL_FUNCTION(Lowercase)
{
	UInt32 uni = argv[0].unboxed.uni;
	uni = Uni_ToLower(uni);
	return SmileUnboxedUni_FromSafeInt32(uni);
}

SMILE_EXTERNAL_FUNCTION(Uppercase)
{
	UInt32 uni = argv[0].unboxed.uni;
	uni = Uni_ToUpper(uni);
	return SmileUnboxedUni_FromSafeInt32(uni);
}

SMILE_EXTERNAL_FUNCTION(Titlecase)
{
	UInt32 uni = argv[0].unboxed.uni;
	uni = Uni_ToTitle(uni);
	return SmileUnboxedUni_FromSafeInt32(uni);
}

SMILE_EXTERNAL_FUNCTION(CaseFold)
{
	UInt32 uni = argv[0].unboxed.uni;
	uni = Uni_CaseFold(uni);
	return SmileUnboxedUni_FromSafeInt32(uni);
}

//-------------------------------------------------------------------------------------------------
// Unicode-specific type tests

SMILE_EXTERNAL_FUNCTION(IsLetter)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsLetter(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsCasedLetter)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsCasedLetter(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsNumber)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsNumber(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsMark)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsMark(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsPunctuation)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsPunct(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsOther)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsOther(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsSymbol)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsSymbol(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsSeparator)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsSep(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsUppercase)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsUpper(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsLowercase)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsLower(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsTitlecase)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsTitle(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsDigit)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsDigit(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsLetterOrNumber)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsLetterOrNumber(uni);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsWhitespace)
{
	UInt32 uni = argv[0].unboxed.uni;
	Bool result = Uni_IsWhitespace(uni);
	return SmileUnboxedBool_From(result);
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) == SMILE_KIND_UNBOXED_UNI
		&& argv[0].unboxed.uni == argv[1].unboxed.uni);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	return SmileUnboxedBool_From(SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_UNI
		|| argv[0].unboxed.uni != argv[1].unboxed.uni);
}

SMILE_EXTERNAL_FUNCTION(Lt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.uni < argv[1].unboxed.uni);
}

SMILE_EXTERNAL_FUNCTION(Gt)
{
	return SmileUnboxedBool_From(argv[0].unboxed.uni > argv[1].unboxed.uni);
}

SMILE_EXTERNAL_FUNCTION(Le)
{
	return SmileUnboxedBool_From(argv[0].unboxed.uni <= argv[1].unboxed.uni);
}

SMILE_EXTERNAL_FUNCTION(Ge)
{
	return SmileUnboxedBool_From(argv[0].unboxed.uni >= argv[1].unboxed.uni);
}

SMILE_EXTERNAL_FUNCTION(Compare)
{
	UInt32 x = argv[0].unboxed.uni;
	UInt32 y = argv[1].unboxed.uni;

	if (x == y)
		return SmileUnboxedInteger64_From(0);
	else if (x < y)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

//-------------------------------------------------------------------------------------------------

void SmileUni_Setup(SmileUserObject base)
{
	SmileUnboxedUni_Instance->base = (SmileObject)base;

	SetupFunction("bool", ToBool, NULL, "uni", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "uni", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "uni", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("hash", Hash, NULL, "uni", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("int64", ToInt64, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("int32", ToInt32, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("int16", ToInt16, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("byte", ToByte, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);

	SetupFunction("char", ToChar, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("uni", ToUni, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);

	SetupFunction("+", Plus, NULL, "uni other", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniComparisonChecks);
	SetupFunction("-", Minus, NULL, "uni other", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniComparisonChecks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniChecks);
	SetupSynonym("compare", "cmp");

	SetupFunction("category", Category, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);

	SetupFunction("lowercase", Lowercase, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("uppercase", Uppercase, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("titlecase", Titlecase, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("case-fold", CaseFold, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupSynonym("case-fold", "fold");

	SetupFunction("letter?", IsLetter, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("cased-letter?", IsCasedLetter, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupSynonym("cased-letter?", "alpha?");
	SetupFunction("number?", IsNumber, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("separator?", IsSeparator, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupSynonym("separator?", "sep?");
	SetupFunction("symbol?", IsSymbol, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("punctuation?", IsPunctuation, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupSynonym("punctuation?", "punct?");
	SetupFunction("other?", IsOther, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);

	SetupFunction("lowercase?", IsLowercase, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("uppercase?", IsUppercase, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("titlecase?", IsTitlecase, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupFunction("digit?", IsDigit, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);

	SetupFunction("letter-or-number?", IsLetterOrNumber, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);
	SetupSynonym("letter-or-number?", "alnum?");
	SetupFunction("whitespace?", IsWhitespace, NULL, "uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _uniChecks);

	SetupFunction("range-to", RangeTo, NULL, "start end", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _uniChecks);
}
