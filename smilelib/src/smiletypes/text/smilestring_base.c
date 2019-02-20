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
#include <smile/eval/eval.h>
#include <smile/smiletypes/smileobject.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smileuni.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/range/smileinteger64range.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/raw/smilebytearray.h>
#include <smile/smiletypes/smilehandle.h>
#include <smile/regex.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

extern SmileArg RegexReplaceStateMachine_Start(Regex regex, String input, SmileFunction function, Int startOffset, Int limit, Bool demoteMatchToString);

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

static Byte _stringReplaceChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	0, 0,
	0, 0,
};

static Byte _stringNumberChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
};

static Byte _padChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
};

static Byte _indexOfChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	0, 0,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
};

static Byte _eachChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
};

static Byte _hyphenizeChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
};

static Byte _wildcardChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
};

STATIC_STRING(_invalidTypeError, "All arguments to 'String.%s' must be of type 'String'");

//-------------------------------------------------------------------------------------------------
// Generic type conversion

SMILE_EXTERNAL_FUNCTION(ToBool)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING)
		return SmileUnboxedBool_From(String_Length((String)argv[0].obj) > 0);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(ToInt)
{
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING) {
		Int64 result;
		if (!String_ParseInteger((String)argv[0].obj, 10, &result))
			result = 0;
		return SmileUnboxedInteger64_From(result);
	}

	return SmileUnboxedInteger64_From(0);
}

SMILE_EXTERNAL_FUNCTION(ToString)
{
	STATIC_STRING(string, "String");

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING)
		return argv[0];

	return SmileArg_From((SmileObject)string);
}

SMILE_EXTERNAL_FUNCTION(Hash)
{
	Int64 hash;

	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING) {
		hash = String_Hash64((String)argv[0].obj);
		return SmileUnboxedInteger64_From(hash);
	}

	return SmileUnboxedInteger64_From(Smile_ApplyHashOracle((PtrInt)argv[0].obj));
}

//-------------------------------------------------------------------------------------------------
// Arithmetic operators

STATIC_STRING(_plusSuccessiveTypeError, "'+' requires its successive arguments to be Strings or Integers.");
STATIC_STRING(_plusIllegalUnicodeChar, "'+' cannot append an illegal Unicode character (>= 0x110000).");

SMILE_EXTERNAL_FUNCTION(Plus)
{
	String x;
	Int i;
	Int64 value;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);

	if (SMILE_KIND(argv[0].obj) != SMILE_KIND_STRING)
		goto concat_many;

	if (argc == 1)
		return argv[0];

	if (argc == 2) {
		// Concatenating exactly two things: a string + (a string | a char | a Unicode char).
		x = (String)argv[0].obj;
	
		switch (SMILE_KIND(argv[1].obj)) {
			case SMILE_KIND_STRING:
				x = String_Concat(x, (String)argv[1].obj);
				return SmileArg_From((SmileObject)x);
			case SMILE_KIND_UNBOXED_CHAR:
				x = String_ConcatByte(x, argv[1].unboxed.ch);
				return SmileArg_From((SmileObject)x);
			case SMILE_KIND_UNBOXED_UNI:
				value = argv[1].unboxed.uni;
				break;
			default:
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _plusSuccessiveTypeError);
		}
	
		if (value < 256 && value >= 0) {
			x = String_ConcatByte(x, (Byte)value);
			return SmileArg_From((SmileObject)x);
		}
		if (value < 0 || value >= 0x110000) {
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _plusIllegalUnicodeChar);
		}
	
		INIT_INLINE_STRINGBUILDER(stringBuilder);
		StringBuilder_AppendString(stringBuilder, x);
		StringBuilder_AppendUnicode(stringBuilder, (UInt32)value);
		return SmileArg_From((SmileObject)StringBuilder_ToString(stringBuilder));
	}

concat_many:
	// Concatenating many things, which should all be Strings or Integers (chars or Unicode chars).
	// We allow for the special case of the first thing not being a String, so that this can be
	// used in a "static" fashion.
	INIT_INLINE_STRINGBUILDER(stringBuilder);

	for (i = 0; i < argc; i++) {
	
		switch (SMILE_KIND(argv[i].obj)) {
		
		case SMILE_KIND_STRING:
			StringBuilder_AppendString(stringBuilder, (String)argv[i].obj);
			break;
		case SMILE_KIND_UNBOXED_CHAR:
			StringBuilder_AppendByte(stringBuilder, argv[1].unboxed.ch);
			break;
		case SMILE_KIND_UNBOXED_UNI:
			StringBuilder_AppendUnicode(stringBuilder, argv[1].unboxed.uni);
			break;		
		default:
			if (i == 0) continue;
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, _plusSuccessiveTypeError);
			break;
		}
	}

	return SmileArg_From((SmileObject)StringBuilder_ToString(stringBuilder));
}

SMILE_EXTERNAL_FUNCTION(Remove)
{
	String x;

	if (argc == 2) {
		// Subtract:  Remove string y from string x.
		x = (String)argv[0].obj;
		x = String_Replace(x, (String)argv[1].obj, String_Empty);
		return SmileArg_From((SmileObject)x);
	}
	else {
		// Negate:  Reverse string x.
		x = (String)argv[0].obj;
		x = String_Reverse(x);
		return SmileArg_From((SmileObject)x);
	}
}

/// <summary>
/// Remove all Byte-Order-Marks from string x.  This is needed so often for interoperability
/// that we provide a special optimized method for it.
/// </summary>
SMILE_EXTERNAL_FUNCTION(RemoveBOM)
{
	String x;
	STATIC_STRING(bom, "\xEF\xBB\xBF");

	x = (String)argv[0].obj;
	x = String_Replace(x, bom, String_Empty);
	return SmileArg_From((SmileObject)x);
}

/// <summary>
/// Remove a single, initial Byte-Order-Mark from string x, if one exists.
/// </summary>
SMILE_EXTERNAL_FUNCTION(TrimBOM)
{
	String x;
	STATIC_STRING(bom, "\xEF\xBB\xBF");

	x = (String)argv[0].obj;
	if (String_StartsWith(x, bom))
		x = String_SubstringAt(x, 3);
	return SmileArg_From((SmileObject)x);
}

/// <summary>
/// Add a single, initial Byte-Order-Mark to string x, if one does not already exist.
/// </summary>
SMILE_EXTERNAL_FUNCTION(AddBOM)
{
	String x;
	STATIC_STRING(bom, "\xEF\xBB\xBF");

	x = (String)argv[0].obj;
	if (!String_StartsWith(x, bom))
		x = String_Concat(bom, x);
	return SmileArg_From((SmileObject)x);
}

/// <summary>
/// Pad the start of a string to ensure it is at least the given length.
/// </summary>
SMILE_EXTERNAL_FUNCTION(PadStart)
{
	String x, padded;
	Int64 minLength;

	x = (String)argv[0].obj;
	minLength = argv[1].unboxed.i64;
	if (minLength > IntMax) minLength = IntMax;

	padded = String_PadStart(x, (Int)minLength, argc > 2 ? argv[2].unboxed.i8 : ' ');

	return SmileArg_From((SmileObject)padded);
}

/// <summary>
/// Pad the end of a string to ensure it is at least the given length.
/// </summary>
SMILE_EXTERNAL_FUNCTION(PadEnd)
{
	String x, padded;
	Int64 minLength;

	x = (String)argv[0].obj;
	minLength = argv[1].unboxed.i64;
	if (minLength > IntMax) minLength = IntMax;

	padded = String_PadEnd(x, (Int)minLength, argc > 2 ? argv[2].unboxed.i8 : ' ');

	return SmileArg_From((SmileObject)padded);
}

/// <summary>
/// Pad both ends of a string to ensure it is at least the given length.  If more
/// padding is required on one side than the other, the extra character will be added
/// to the end of the string, not the start.
/// </summary>
SMILE_EXTERNAL_FUNCTION(PadCenter)
{
	String x, padded;
	Int64 minLength;

	x = (String)argv[0].obj;
	minLength = argv[1].unboxed.i64;
	if (minLength > IntMax) minLength = IntMax;

	padded = String_PadCenter(x, (Int)minLength, argc > 2 ? argv[2].unboxed.i8 : ' ');

	return SmileArg_From((SmileObject)padded);
}

SMILE_EXTERNAL_FUNCTION(Reverse)
{
	String x = (String)argv[0].obj;
	return SmileArg_From((SmileObject)String_RawReverse(x));
}

SMILE_EXTERNAL_FUNCTION(UniReverse)
{
	String x = (String)argv[0].obj;
	return SmileArg_From((SmileObject)String_Reverse(x));
}

SMILE_EXTERNAL_FUNCTION(Repeat)
{
	String x;

	x = (String)argv[0].obj;
	x = String_Repeat(x, (Int)argv[1].unboxed.i64);
	return SmileArg_From((SmileObject)x);
}

SMILE_EXTERNAL_FUNCTION(SlashAppend)
{
	String x;
	Int i, j;
	String strs[16];

	switch (argc) {
		case 1:
			return argv[0];
		
		case 2:
			strs[0] = (String)argv[0].obj;
			strs[1] = (String)argv[1].obj;
			x = String_SlashAppend(strs, 2);
			return SmileArg_From((SmileObject)x);
	
		case 3:
			strs[0] = (String)argv[0].obj;
			strs[1] = (String)argv[1].obj;
			strs[2] = (String)argv[2].obj;
			x = String_SlashAppend(strs, 3);
			return SmileArg_From((SmileObject)x);

		case 4:
			strs[0] = (String)argv[0].obj;
			strs[1] = (String)argv[1].obj;
			strs[2] = (String)argv[2].obj;
			strs[3] = (String)argv[3].obj;
			x = String_SlashAppend(strs, 4);
			return SmileArg_From((SmileObject)x);

		default:
			// Smash together the strings in batches of up to 16 strings each.
			x = (String)argv[0].obj;
			for (i = 1; i < argc; i++) {
				strs[0] = x;
				for (j = 1; j < 16 && i < argc; j++, i++) {
					strs[j] = (String)argv[i].obj;
				}
				x = String_SlashAppend(strs, j);
			}
			return SmileArg_From((SmileObject)x);
	}
}

//-------------------------------------------------------------------------------------------------
// Comparisons

SMILE_EXTERNAL_FUNCTION(Eq)
{
	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_STRING
		|| !String_Equals((String)argv[0].obj, (String)argv[1].obj))
		return SmileUnboxedBool_From(False);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(Ne)
{
	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_STRING
		|| !String_Equals((String)argv[0].obj, (String)argv[1].obj))
		return SmileUnboxedBool_From(True);

	return SmileUnboxedBool_From(False);
}

SMILE_EXTERNAL_FUNCTION(EqI)
{
	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_STRING
		|| String_CompareI((String)argv[0].obj, (String)argv[1].obj) != 0)
		return SmileUnboxedBool_From(False);

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(NeI)
{
	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_STRING
		|| String_CompareI((String)argv[0].obj, (String)argv[1].obj) != 0)
		return SmileUnboxedBool_From(True);

	return SmileUnboxedBool_From(False);
}

#define RELATIVE_COMPARE(__name__, __func__, __op__) \
	SMILE_EXTERNAL_FUNCTION(__name__) \
	{ \
		String x = (String)argv[0].obj; \
		String y = (String)argv[1].obj; \
		Int cmp = __func__(x, y); \
		\
		return SmileUnboxedBool_From(cmp __op__ 0); \
	}

RELATIVE_COMPARE(Lt, String_Compare, <)
RELATIVE_COMPARE(Gt, String_Compare, >)
RELATIVE_COMPARE(Le, String_Compare, <=)
RELATIVE_COMPARE(Ge, String_Compare, >=)
RELATIVE_COMPARE(LtI, String_CompareI, <)
RELATIVE_COMPARE(GtI, String_CompareI, >)
RELATIVE_COMPARE(LeI, String_CompareI, <=)
RELATIVE_COMPARE(GeI, String_CompareI, >=)

SMILE_EXTERNAL_FUNCTION(Compare)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;
	Int cmp = String_Compare(x, y);

	if (cmp == 0)
		return SmileUnboxedInteger64_From(0);
	else if (cmp < 0)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

SMILE_EXTERNAL_FUNCTION(CompareI)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;
	Int cmp = String_CompareI(x, y);

	if (cmp == 0)
		return SmileUnboxedInteger64_From(0);
	else if (cmp < 0)
		return SmileUnboxedInteger64_From(-1);
	else
		return SmileUnboxedInteger64_From(+1);
}

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(IsEmpty)
{
	String str = (String)argv[0].obj;
	Bool result = String_IsNullOrEmpty(str);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsWhitespace)
{
	String str = (String)argv[0].obj;
	Bool result = String_IsNullOrWhitespace(str);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IsAlpha)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	const Byte *end = src + String_Length(str);

	if (src >= end)
		return SmileUnboxedBool_From(True);

	while (src < end) {
		Byte ch = *src++;
		if (!(ch >= 'a' && ch <= 'z'
			|| ch >= 'A' && ch <= 'Z')) {
			return SmileUnboxedBool_From(False);
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(IsUppercase)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	const Byte *end = src + String_Length(str);

	if (src >= end)
		return SmileUnboxedBool_From(True);

	while (src < end) {
		Byte ch = *src++;
		if (!(ch >= 'A' && ch <= 'Z')) {
			return SmileUnboxedBool_From(False);
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(IsLowercase)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	const Byte *end = src + String_Length(str);

	if (src >= end)
		return SmileUnboxedBool_From(True);

	while (src < end) {
		Byte ch = *src++;
		if (!(ch >= 'a' && ch <= 'z')) {
			return SmileUnboxedBool_From(False);
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(IsAlnum)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	const Byte *end = src + String_Length(str);

	if (src >= end)
		return SmileUnboxedBool_From(True);

	while (src < end) {
		Byte ch = *src++;
		if (!(ch >= 'a' && ch <= 'z'
			|| ch >= 'A' && ch <= 'Z'
			|| ch >= '0' && ch <= '9')) {
			return SmileUnboxedBool_From(False);
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(IsDigits)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	const Byte *end = src + String_Length(str);

	if (src >= end)
		return SmileUnboxedBool_From(True);

	while (src < end) {
		Byte ch = *src++;
		if (!(ch >= '0' && ch <= '9')) {
			return SmileUnboxedBool_From(False);
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(IsHex)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	const Byte *end = src + String_Length(str);

	if (src >= end)
		return SmileUnboxedBool_From(True);

	while (src < end) {
		Byte ch = *src++;
		if (!(ch >= 'a' && ch <= 'f'
			|| ch >= 'A' && ch <= 'F'
			|| ch >= '0' && ch <= '9')) {
			return SmileUnboxedBool_From(False);
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(IsOctal)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	const Byte *end = src + String_Length(str);

	if (src >= end)
		return SmileUnboxedBool_From(True);

	while (src < end) {
		Byte ch = *src++;
		if (!(ch >= '0' && ch <= '7')) {
			return SmileUnboxedBool_From(False);
		}
	}

	return SmileUnboxedBool_From(True);
}

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(IsCIdent)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	const Byte *end = src + String_Length(str);
	Byte ch;

	if (src >= end)
		return SmileUnboxedBool_From(False);

	ch = *src++;
	if (!(ch >= 'a' && ch <= 'z'
		|| ch >= 'A' && ch <= 'Z'
		|| ch == '_')) {
		return SmileUnboxedBool_From(False);
	}

	while (src < end) {
		ch = *src++;
		if (!(ch >= 'a' && ch <= 'z'
			|| ch >= 'A' && ch <= 'Z'
			|| ch >= '0' && ch <= '9'
			|| ch == '_')) {
			return SmileUnboxedBool_From(False);
		}
	}

	return SmileUnboxedBool_From(True);
}

SMILE_EXTERNAL_FUNCTION(IsIdent)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	Int length = String_Length(str);
	const Byte *end = src + length;
	Lexer lexer = Lexer_Create(str, 0, length, String_Empty, 1, 1);
	Int token;
	Bool isIdentifier;

	if (length == 0
		|| (*src >= '\0' && *src <= ' ')
		|| (end[-1] >= '\0' && end[-1] <= ' '))
		return SmileUnboxedBool_From(False);

	token = Lexer_Next(lexer);
	if (Lexer_Next(lexer) != TOKEN_EOI)
		return SmileUnboxedBool_From(False);

	isIdentifier = token == TOKEN_ALPHANAME || token == TOKEN_UNKNOWNALPHANAME
		|| token == TOKEN_PUNCTNAME || token == TOKEN_UNKNOWNPUNCTNAME;

	return SmileUnboxedBool_From(isIdentifier);
}

SMILE_EXTERNAL_FUNCTION(IsAlphaIdent)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	Int length = String_Length(str);
	const Byte *end = src + length;
	Lexer lexer = Lexer_Create(str, 0, length, String_Empty, 1, 1);
	Int token;
	Bool isIdentifier;

	if (length == 0
		|| (*src >= '\0' && *src <= ' ')
		|| (end[-1] >= '\0' && end[-1] <= ' '))
		return SmileUnboxedBool_From(False);

	token = Lexer_Next(lexer);
	if (Lexer_Next(lexer) != TOKEN_EOI)
		return SmileUnboxedBool_From(False);

	isIdentifier = token == TOKEN_ALPHANAME || token == TOKEN_UNKNOWNALPHANAME;

	return SmileUnboxedBool_From(isIdentifier);
}

SMILE_EXTERNAL_FUNCTION(IsPunctIdent)
{
	String str = (String)argv[0].obj;
	const Byte *src = String_GetBytes(str);
	Int length = String_Length(str);
	const Byte *end = src + length;
	Lexer lexer = Lexer_Create(str, 0, length, String_Empty, 1, 1);
	Int token;
	Bool isIdentifier;

	if (length == 0
		|| (*src >= '\0' && *src <= ' ')
		|| (end[-1] >= '\0' && end[-1] <= ' '))
		return SmileUnboxedBool_From(False);

	token = Lexer_Next(lexer);
	if (Lexer_Next(lexer) != TOKEN_EOI)
		return SmileUnboxedBool_From(False);

	isIdentifier = token == TOKEN_PUNCTNAME || token == TOKEN_UNKNOWNPUNCTNAME;

	return SmileUnboxedBool_From(isIdentifier);
}

//-------------------------------------------------------------------------------------------------

Inline void DecodePattern(SmileArg arg, String *pattern, Regex *regexPattern, const char *methodName, const char *paramName)
{
	Int patternKind = SMILE_KIND(arg.obj);

	*pattern = NULL;
	*regexPattern = NULL;

	// Decode the pattern.
	if (patternKind == SMILE_KIND_STRING)
		*pattern = (String)arg.obj;
	else if (patternKind == SMILE_KIND_CHAR)
		*pattern = String_CreateRepeat(arg.unboxed.i8, 1);
	else if (patternKind == SMILE_KIND_UNI)
		*pattern = String_CreateFromUnicode(arg.unboxed.uni);
	else if (patternKind == SMILE_KIND_HANDLE && ((SmileHandle)arg.obj)->handleKind == Smile_KnownSymbols.Regex_)
		*regexPattern = (Regex)((SmileHandle)arg.obj)->ptr;
	else {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_Format("The %s to 'String.%s' must be a String, Char, Uni, or Regex.", paramName, methodName));
	}
}

Inline String DecodePatternWithoutRegex(SmileArg arg, const char *methodName, const char *paramName)
{
	Int patternKind = SMILE_KIND(arg.obj);

	// Decode the pattern.
	if (patternKind == SMILE_KIND_STRING)
		return (String)arg.obj;
	else if (patternKind == SMILE_KIND_CHAR)
		return String_CreateRepeat(arg.unboxed.i8, 1);
	else if (patternKind == SMILE_KIND_UNI)
		return String_CreateFromUnicode(arg.unboxed.uni);
	else {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_Format("The %s to 'String.%s' must be a String, Char, or Uni.", paramName, methodName));
	}
}

SMILE_EXTERNAL_FUNCTION(StartsWith)
{
	String str = (String)argv[0].obj;
	String pattern;
	Regex regexPattern;
	Bool result;

	DecodePattern(argv[1], &pattern, &regexPattern, "starts-with", "pattern");

	if (pattern != NULL) {
		result = String_StartsWith(str, pattern);
	}
	else if (regexPattern != NULL) {
		result = Regex_Test(Regex_WithStartAnchor(regexPattern), str, 0);
	}
	else result = False;

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(StartsWithI)
{
	String str = (String)argv[0].obj;
	String pattern;
	Regex regexPattern;
	Bool result;

	DecodePattern(argv[1], &pattern, &regexPattern, "starts-with~", "pattern");

	if (pattern != NULL) {
		result = String_StartsWithI(str, pattern);
	}
	else if (regexPattern != NULL) {
		result = Regex_Test(Regex_AsCaseInsensitive(Regex_WithStartAnchor(regexPattern)), str, 0);
	}
	else result = False;

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(EndsWith)
{
	String str = (String)argv[0].obj;
	String pattern;
	Regex regexPattern;
	Bool result;

	DecodePattern(argv[1], &pattern, &regexPattern, "ends-with", "pattern");

	if (pattern != NULL) {
		result = String_EndsWith(str, pattern);
	}
	else if (regexPattern != NULL) {
		result = Regex_Test(Regex_WithEndAnchor(regexPattern), str, 0);
	}
	else result = False;

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(EndsWithI)
{
	String str = (String)argv[0].obj;
	String pattern;
	Regex regexPattern;
	Bool result;

	DecodePattern(argv[1], &pattern, &regexPattern, "ends-with~", "pattern");

	if (pattern != NULL) {
		result = String_EndsWithI(str, pattern);
	}
	else if (regexPattern != NULL) {
		result = Regex_Test(Regex_AsCaseInsensitive(Regex_WithEndAnchor(regexPattern)), str, 0);
	}
	else result = False;

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Contains)
{
	String str = (String)argv[0].obj;
	String pattern;
	Regex regexPattern;
	Bool result;

	DecodePattern(argv[1], &pattern, &regexPattern, "contains", "pattern");

	if (pattern != NULL) {
		result = String_Contains(str, pattern);
	}
	else if (regexPattern != NULL) {
		result = Regex_Test(regexPattern, str, 0);
	}
	else result = False;

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(ContainsI)
{
	String str = (String)argv[0].obj;
	String pattern;
	Regex regexPattern;
	Bool result;

	DecodePattern(argv[1], &pattern, &regexPattern, "contains~", "pattern");

	if (pattern != NULL) {
		result = String_ContainsI(str, pattern);
	}
	else if (regexPattern != NULL) {
		result = Regex_Test(Regex_AsCaseInsensitive(regexPattern), str, 0);
	}
	else result = False;

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(IndexOf)
{
	String str = (String)argv[0].obj;
	String pattern = NULL;
	Regex regexPattern = NULL;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int stringLength = String_Length(str);
	Int result;

	DecodePattern(argv[1], &pattern, &regexPattern, "index-of", "pattern");

	if (startIndex < 0 || startIndex >= stringLength)
		return SmileArg_From(NullObject);

	if (pattern != NULL) {
		result = String_IndexOf(str, pattern, (Int)startIndex);
		if (result < 0) return SmileArg_From(NullObject);
	}
	else {
		RegexMatch match = Regex_Match(regexPattern, str, 0);
		if (!match->isMatch) return SmileArg_From(NullObject);
		result = match->indexedCaptures[0].start;
	}

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(IndexOfI)
{
	String str = (String)argv[0].obj;
	String pattern = NULL;
	Regex regexPattern = NULL;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int stringLength = String_Length(str);
	Int result;

	DecodePattern(argv[1], &pattern, &regexPattern, "index-of~", "pattern");

	if (startIndex < 0 || startIndex >= stringLength)
		return SmileArg_From(NullObject);

	if (pattern != NULL) {
		result = String_IndexOfI(str, pattern, (Int)startIndex);
		if (result < 0) return SmileArg_From(NullObject);
	}
	else {
		RegexMatch match = Regex_Match(Regex_AsCaseInsensitive(regexPattern), str, 0);
		if (!match->isMatch) return SmileArg_From(NullObject);
		result = match->indexedCaptures[0].start;
	}

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(LastIndexOf)
{
	String str = (String)argv[0].obj;
	String pattern;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : String_Length(str) - 1;
	Int stringLength = String_Length(str);
	Int result;

	pattern = DecodePatternWithoutRegex(argv[1], "last-index-of", "pattern");

	result = startIndex < stringLength ? String_LastIndexOf(str, pattern, (Int)startIndex) : -1;
	if (result < 0) return SmileArg_From(NullObject);

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(LastIndexOfI)
{
	String str = (String)argv[0].obj;
	String pattern;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : String_Length(str) - 1;
	Int stringLength = String_Length(str);
	Int result;

	pattern = DecodePatternWithoutRegex(argv[1], "last-index-of~", "pattern");

	result = startIndex < stringLength ? String_LastIndexOfI(str, pattern, (Int)startIndex) : -1;
	if (result < 0) return SmileArg_From(NullObject);

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(CountOf)
{
	String str = (String)argv[0].obj;
	String pattern = NULL;
	Regex regexPattern = NULL;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int stringLength = String_Length(str);
	Int result;

	DecodePattern(argv[1], &pattern, &regexPattern, "count-of", "pattern");

	if (startIndex < 0 || startIndex >= stringLength)
		return SmileArg_From(NullObject);

	if (pattern != NULL) {
		result = String_CountOf(str, pattern, (Int)startIndex);
	}
	else {
		result = Regex_Count(regexPattern, str, startIndex, 0);
	}

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(CountOfI)
{
	String str = (String)argv[0].obj;
	String pattern = NULL;
	Regex regexPattern = NULL;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int stringLength = String_Length(str);
	Int result;

	DecodePattern(argv[1], &pattern, &regexPattern, "count-of~", "pattern");

	if (startIndex < 0 || startIndex >= stringLength)
		return SmileArg_From(NullObject);

	if (pattern != NULL) {
		result = String_CountOf(str, pattern, (Int)startIndex);
	}
	else {
		result = Regex_Count(Regex_AsCaseInsensitive(regexPattern), str, startIndex, 0);
	}

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(Replace)
{
	String str = (String)argv[0].obj;
	String pattern, replacement;
	Regex regexPattern;
	Int patternKind = SMILE_KIND(argv[1].obj);
	Int replacementKind = SMILE_KIND(argv[2].obj);

	// Special efficient case: If they're replacing a char with a char, use an optimized routine.
	if (patternKind == SMILE_KIND_UNBOXED_CHAR && replacementKind == SMILE_KIND_UNBOXED_CHAR) {
		if (argc > 3) {
			Int64 limit = argv[3].unboxed.i64;
			if (limit < 0) limit = 0;
			if (limit > String_Length(str)) limit = String_Length(str);
			str = String_ReplaceCharWithLimit(str, argv[1].unboxed.i8, argv[2].unboxed.i8, (Int)limit);
		}
		else
			str = String_ReplaceChar(str, argv[1].unboxed.i8, argv[2].unboxed.i8);
		return SmileArg_From((SmileObject)str);
	}

	DecodePattern(argv[1], &pattern, &regexPattern, "replace", "pattern");

	if (SMILE_KIND(argv[2].obj) == SMILE_KIND_FUNCTION) {
		Int64 limit;
		if (argc > 3) {
			limit = argv[3].unboxed.i64;
			if (limit < 0) limit = 0;
			if (limit > String_Length(str)) limit = String_Length(str);
		}
		else limit = 0;
		if (regexPattern == NULL)
			regexPattern = Regex_Create(String_RegexEscape(pattern), String_Empty, NULL);
		return RegexReplaceStateMachine_Start(regexPattern, str, (SmileFunction)argv[2].obj, 0, limit, pattern != NULL);
	}

	replacement = DecodePatternWithoutRegex(argv[2], "replace", "replacement");

	if (pattern != NULL) {
		if (argc > 3) {
			Int64 limit = argv[3].unboxed.i64;
			if (limit < 0) limit = 0;
			if (limit > String_Length(str)) limit = String_Length(str);
			str = String_ReplaceWithLimit(str, pattern, replacement, (Int)limit);
		}
		else {
			str = String_Replace(str, pattern, replacement);
		}
	}
	else if (regexPattern != NULL) {
		Int64 limit;
		if (argc > 3) {
			limit = argv[3].unboxed.i64;
			if (limit < 0) limit = 0;
			if (limit > String_Length(str)) limit = String_Length(str);
		}
		else limit = 0;
		str = Regex_Replace(regexPattern, str, replacement, 0, limit);
	}

	return SmileArg_From((SmileObject)str);
}

SMILE_EXTERNAL_FUNCTION(ReplaceI)
{
	String str = (String)argv[0].obj;
	String pattern, replacement;
	Regex regexPattern;
	STATIC_STRING(i, "i");

	DecodePattern(argv[1], &pattern, &regexPattern, "replace~", "pattern");

	if (SMILE_KIND(argv[2].obj) == SMILE_KIND_FUNCTION) {
		Int64 limit;
		if (argc > 3) {
			limit = argv[3].unboxed.i64;
			if (limit < 0) limit = 0;
			if (limit > String_Length(str)) limit = String_Length(str);
		}
		else limit = 0;
		regexPattern = (regexPattern != NULL
			? Regex_AsCaseInsensitive(regexPattern)
			: Regex_Create(String_RegexEscape(pattern), i, NULL));
		return RegexReplaceStateMachine_Start(regexPattern, str, (SmileFunction)argv[2].obj, 0, limit, pattern != NULL);
	}

	replacement = DecodePatternWithoutRegex(argv[2], "replace~", "replacement");

	if (pattern != NULL) {
		if (argc > 3) {
			Int64 limit = argv[3].unboxed.i64;
			if (limit < 0) limit = 0;
			if (limit > String_Length(str)) limit = String_Length(str);
			str = String_ReplaceWithLimitI(str, pattern, replacement, (Int)limit);
		}
		else {
			str = String_ReplaceI(str, pattern, replacement);
		}
	}
	else if (regexPattern != NULL) {
		Int64 limit;
		if (argc > 3) {
			limit = argv[3].unboxed.i64;
			if (limit < 0) limit = 0;
			if (limit > String_Length(str)) limit = String_Length(str);
		}
		else limit = 0;
		str = Regex_Replace(Regex_AsCaseInsensitive(regexPattern), str, replacement, 0, limit);
	}

	return SmileArg_From((SmileObject)str);
}

SMILE_EXTERNAL_FUNCTION(Split)
{
	String str = (String)argv[0].obj;
	Int limit;
	Int numPieces;
	Int i;
	String *pieces;
	SmileList head, tail;
	String pattern = NULL;
	Regex regexPattern = NULL;

	// Decode the pattern.
	DecodePattern(argv[1], &pattern, &regexPattern, "split", "pattern");

	// If there's a limit, get the limit.
	if (argc > 2) {
		Int64 limit64 = argv[2].unboxed.i64;
		if (limit64 < 0) limit64 = 0;
		if (limit64 > String_Length(str) + 1) limit64 = String_Length(str) + 1;
		limit = (Int)limit64;
	}
	else {
		limit = -1;
	}

	// Do the actual splitting.
	if (pattern != NULL) {
		numPieces = String_SplitWithOptions(str, pattern, limit, StringSplitOptions_None, &pieces);
	}
	else if (regexPattern != NULL) {
		numPieces = Regex_Split(regexPattern, str, &pieces, False, limit);
	}
	else {
		numPieces = 0;
		pieces = NULL;
	}

	// Make a list of the resulting pieces.
	head = tail = NullList;
	for (i = 0; i < numPieces; i++) {
		LIST_APPEND(head, tail, pieces[i]);
	}

	// And we're done.
	return SmileArg_From((SmileObject)head);
}

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(GetMember)
{
	String x = (String)argv[0].obj;
	Int64 index;
	SmileInteger64Range range;
	Byte ch;
	STATIC_STRING(indexOutOfRangeError, "Index to 'get-member' is beyond the length of the string.");
	STATIC_STRING(indexTypeError, "Index to 'get-member' must be an Integer or an IntegerRange.");

	switch (SMILE_KIND(argv[1].obj)) {
		case SMILE_KIND_UNBOXED_INTEGER64:
			index = argv[1].unboxed.i64;

			if (index < 0 || index >= String_Length(x))
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, indexOutOfRangeError);

			ch = String_At(x, (Int)index);

			return SmileUnboxedChar_From(ch);

		case SMILE_KIND_INTEGER64RANGE:
			range = (SmileInteger64Range)argv[1].obj;
			return SmileArg_From((SmileObject)String_SubstringByRange(x, range->start, range->end, range->stepping));

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, indexTypeError);
	}
}

SMILE_EXTERNAL_FUNCTION(UniAt)
{
	String str = (String)argv[0].obj;
	Int64 index = argv[1].unboxed.i64;
	const Byte *text = String_GetBytes(str);
	const Byte *src;
	Int length = String_Length(str);
	Int32 value;

	STATIC_STRING(indexOutOfRangeError, "Index to 'uni-at' is beyond the length of the string.");

	if (index < 0 || index >= length)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, indexOutOfRangeError);

	src = text + (Int)index;

	value = String_ExtractUnicodeCharacterInternal(&src, src + length);
	if (value < 0) value = 0xFFFD;
	return SmileUnboxedUni_From(value);
}

SMILE_EXTERNAL_FUNCTION(UniLengthAt)
{
	String str = (String)argv[0].obj;
	Int64 index = argv[1].unboxed.i64;
	const Byte *text = String_GetBytes(str);
	const Byte *src, *start;
	Int length = String_Length(str);

	STATIC_STRING(indexOutOfRangeError, "Index to 'uni-length-at' is beyond the length of the string.");

	if (index < 0 || index >= length)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, indexOutOfRangeError);

	start = src = text + (Int)index;

	String_ExtractUnicodeCharacterInternal(&src, src + length);
	return SmileUnboxedInteger64_From(src - start);
}

SMILE_EXTERNAL_FUNCTION(UniNext)
{
	String str = (String)argv[0].obj;
	Int64 index = argv[1].unboxed.i64;
	const Byte *text = String_GetBytes(str);
	const Byte *src;
	Int length = String_Length(str);

	if (index < 0 || index >= length)
		return SmileUnboxedInteger64_From(-1);

	src = text + (Int)index;

	String_ExtractUnicodeCharacterInternal(&src, src + length);
	return SmileUnboxedInteger64_From(src < text + length ? src - text : -1);
}

SMILE_EXTERNAL_FUNCTION(Substr)
{
	STATIC_STRING(indexTypeError, "Index to 'substr' must be an Integer or an IntegerRange.");
	String x = (String)argv[0].obj;
	Int64 index, length;
	SmileInteger64Range range;

	if (argc == 2) {
		switch (SMILE_KIND(argv[1].obj)) {
			case SMILE_KIND_UNBOXED_INTEGER64:
				index = argv[1].unboxed.i64;
				return SmileArg_From((SmileObject)String_SubstringAt(x, (Int)index));

			case SMILE_KIND_INTEGER64RANGE:
				range = (SmileInteger64Range)argv[1].obj;
				return SmileArg_From((SmileObject)String_SubstringByRange(x, range->start, range->end, range->stepping));

			default:
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, indexTypeError);
		}
	}
	else {
		if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_INTEGER64
			|| SMILE_KIND(argv[2].obj) != SMILE_KIND_UNBOXED_INTEGER64)
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, indexTypeError);
		index = argv[1].unboxed.i64;
		length = argv[2].unboxed.i64;
		return SmileArg_From((SmileObject)String_Substring(x, (Int)index, (Int)length));
	}
}

SMILE_EXTERNAL_FUNCTION(Left)
{
	String x = (String)argv[0].obj;
	Int64 length = argv[1].unboxed.i64;
	Int stringLength = String_Length(x);

	return length >= stringLength ? argv[0]
		: SmileArg_From((SmileObject)String_Substring(x, 0, (Int)length));
}

SMILE_EXTERNAL_FUNCTION(Right)
{
	String x = (String)argv[0].obj;
	Int64 length = argv[1].unboxed.i64;
	Int stringLength = String_Length(x);

	return length >= stringLength ? argv[0]
		: SmileArg_From((SmileObject)String_SubstringAt(x, (Int)stringLength - (Int)length));
}

SMILE_EXTERNAL_FUNCTION(Chip)
{
	String str;
	Int64 count;

	str = (String)argv[0].obj;

	// Chip away 'count' characters from the start of 'str'.  If 'count'
	// is not provided, it's 1.
	count = (argc == 2 ? argv[1].unboxed.i64 : 1);

	if (count >= (Int64)String_Length(str))
		str = String_Empty;
	else if (count > 0)
		str = String_SubstringAt(str, (Int)count);

	return SmileArg_From((SmileObject)str);
}

SMILE_EXTERNAL_FUNCTION(Chop)
{
	String str;
	Int64 count, length;

	str = (String)argv[0].obj;

	// Chop off 'count' characters from the end of 'str'.  If 'count'
	// is not provided, it's 1.
	count = (argc == 2 ? argv[1].unboxed.i64 : 1);

	length = (Int64)String_Length(str);
	if (count >= length)
		str = String_Empty;
	else if (count > 0)
		str = String_Substring(str, 0, (Int)(length - count));

	return SmileArg_From((SmileObject)str);
}

SMILE_EXTERNAL_FUNCTION(ReplaceNewlines)
{
	String str = (String)argv[0].obj;
	String replacement = (String)argv[1].obj;

	str = String_ReplaceNewlines(str, replacement);

	return SmileArg_From((SmileObject)str);
}

SMILE_EXTERNAL_FUNCTION(NewlinesToBreaks)
{
	STATIC_STRING(_break, "<br />");

	String str = (String)argv[0].obj;
	String prefix = argc > 1 ? (String)argv[1].obj : _break;

	str = String_PrefixNewlines(str, prefix);

	return SmileArg_From((SmileObject)str);
}

//-------------------------------------------------------------------------------------------------

#define UnaryProxyFunction(__functionName__, __stringName__) \
	SMILE_EXTERNAL_FUNCTION(__functionName__) \
	{ \
		String x = (String)argv[0].obj; \
		return SmileArg_From((SmileObject)__stringName__(x)); \
	}

UnaryProxyFunction(CaseFold, String_CaseFold)
UnaryProxyFunction(Uppercase, String_ToUpper)
UnaryProxyFunction(Lowercase, String_ToLower)
UnaryProxyFunction(Titlecase, String_ToTitle)
UnaryProxyFunction(Decompose, String_Decompose)
UnaryProxyFunction(Compose, String_Compose)
UnaryProxyFunction(NormalizeDiacritics, String_Normalize)

UnaryProxyFunction(Trim, String_Trim)
UnaryProxyFunction(TrimStart, String_TrimStart)
UnaryProxyFunction(TrimEnd, String_TrimEnd)
UnaryProxyFunction(CompactWhitespace, String_CompactWhitespace)
UnaryProxyFunction(SplitNewlines, String_SplitNewlines)

UnaryProxyFunction(Rot13, String_Rot13)
UnaryProxyFunction(AddCSlashes, String_AddCSlashes)
UnaryProxyFunction(StripCSlashes, String_StripCSlashes)
UnaryProxyFunction(HtmlEncode, String_HtmlEncode)
UnaryProxyFunction(HtmlDecode, String_HtmlDecode)
UnaryProxyFunction(UrlEncode, String_UrlEncode)
UnaryProxyFunction(UrlQueryEncode, String_UrlQueryEncode)
UnaryProxyFunction(UrlDecode, String_UrlDecode)
UnaryProxyFunction(RegexEscape, String_RegexEscape)

UnaryProxyFunction(ByteArray, String_ToByteArray)

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(CamelCase)
{
	String str = (String)argv[0].obj;
	PtrInt paramValue = (PtrInt)param;
	return SmileArg_From((SmileObject)String_CamelCase(str, paramValue & 1, (paramValue >> 1) & 1));
}

SMILE_EXTERNAL_FUNCTION(Hyphenize)
{
	String str = (String)argv[0].obj;
	Byte separator = (argc > 1 ? argv[1].unboxed.i8 : (Byte)(PtrInt)param);
	return SmileArg_From((SmileObject)String_Hyphenize(str, separator));
}

//-------------------------------------------------------------------------------------------------

typedef struct EachInfoStruct {
	String initialString;
	const Byte *ptr, *end;
	SmileFunction function;
	Int index;
} *EachInfo;

static Int EachWithOneArg(ClosureStateMachine closure)
{
	EachInfo eachInfo = (EachInfo)closure->state;

	// If we've run out of list nodes, we're done.
	if (eachInfo->ptr >= eachInfo->end) {
		Closure_Pop(closure);
		Closure_PushBoxed(closure, eachInfo->initialString);	// Pop the previous return value and push 'initialList'.
		return -1;
	}

	// Set up to call the user's function with the next list item.
	Closure_Pop(closure);
	Closure_PushBoxed(closure, eachInfo->function);
	Closure_PushUnboxedByte(closure, *eachInfo->ptr);

	eachInfo->ptr++;	// Move the iterator to the next item.
	eachInfo->index++;

	return 1;
}

static Int EachWithTwoArgs(ClosureStateMachine closure)
{
	EachInfo eachInfo = (EachInfo)closure->state;

	// If we've run out of list nodes, we're done.
	if (eachInfo->ptr >= eachInfo->end) {
		Closure_Pop(closure);
		Closure_PushBoxed(closure, eachInfo->initialString);	// Pop the previous return value and push 'initialList'.
		return -1;
	}

	// Set up to call the user's function with the next list item and index.
	Closure_Pop(closure);
	Closure_PushBoxed(closure, eachInfo->function);
	Closure_PushUnboxedByte(closure, *eachInfo->ptr);
	Closure_PushUnboxedInt64(closure, eachInfo->index);

	eachInfo->ptr++;	// Move the iterator to the next item.
	eachInfo->index++;

	return 2;
}

SMILE_EXTERNAL_FUNCTION(Each)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	String smileString = (String)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	EachInfo eachInfo;
	ClosureStateMachine closure;
	StateMachine stateMachine;

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	stateMachine = maxArgs <= 1 ? EachWithOneArg : EachWithTwoArgs;
	closure = Eval_BeginStateMachine(stateMachine, stateMachine);

	eachInfo = (EachInfo)closure->state;
	eachInfo->function = function;
	eachInfo->initialString = smileString;
	eachInfo->index = 0;
	eachInfo->ptr = String_GetBytes(smileString);
	eachInfo->end = eachInfo->ptr + String_Length(smileString);

	Closure_PushBoxed(closure, NullObject);	// Initial "return" value from 'each'.

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct MapInfoStruct {
	StringBuilder result;
	const Byte *ptr, *end;
	SmileFunction function;
	Int index;
} *MapInfo;

static Int MapWithOneArgStart(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToString(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

static Int MapWithOneArgBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output array.
	SmileArg fnResult = Closure_Pop(closure);
	switch (SMILE_KIND(fnResult.obj)) {
		case SMILE_KIND_UNBOXED_CHAR:
			StringBuilder_AppendByte(loopInfo->result, fnResult.unboxed.ch);
			break;
		case SMILE_KIND_UNBOXED_UNI:
			StringBuilder_AppendUnicode(loopInfo->result, fnResult.unboxed.uni);
			break;
		case SMILE_KIND_STRING:
			StringBuilder_AppendString(loopInfo->result, (String)fnResult.obj);
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("'map' projection must return a String, a Char, or a Uni."));
			break;
	}

	// Next: Move the iterator to the next item.
	loopInfo->ptr++;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToString(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

static Int MapWithTwoArgsStart(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToString(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

static Int MapWithTwoArgsBody(ClosureStateMachine closure)
{
	register MapInfo loopInfo = (MapInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	switch (SMILE_KIND(fnResult.obj)) {
		case SMILE_KIND_UNBOXED_CHAR:
			StringBuilder_AppendByte(loopInfo->result, fnResult.unboxed.ch);
			break;
		case SMILE_KIND_UNBOXED_UNI:
			StringBuilder_AppendUnicode(loopInfo->result, fnResult.unboxed.uni);
			break;
		case SMILE_KIND_STRING:
			StringBuilder_AppendString(loopInfo->result, (String)fnResult.obj);
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("'map' projection must return a String, a Char, or a Uni."));
			break;
	}

	// Next: Move the iterator to the next item.
	loopInfo->ptr++;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToString(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

SMILE_EXTERNAL_FUNCTION(Map)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	String smileString = (String)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	MapInfo mapInfo;
	ClosureStateMachine closure;
	const Byte *ptr;
	Int length;

	ptr = String_GetBytes(smileString);
	length = String_Length(smileString);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(
		maxArgs <= 1 ? MapWithOneArgStart : MapWithTwoArgsStart,
		maxArgs <= 1 ? MapWithOneArgBody : MapWithTwoArgsBody);

	mapInfo = (MapInfo)closure->state;
	mapInfo->function = function;
	mapInfo->ptr = ptr;
	mapInfo->end = ptr + length;
	mapInfo->result = StringBuilder_Create();
	mapInfo->index = 0;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct WhereInfoStruct {
	StringBuilder result;
	const Byte *ptr, *end;
	SmileFunction function;
	Int index;
} *WhereInfo;

static Int WhereWithOneArgStart(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of bytes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToString(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

static Int WhereWithOneArgBody(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, keep this list element.
	if (booleanResult) {
		StringBuilder_AppendByte(loopInfo->result, *loopInfo->ptr);
	}

	// Next: Move the iterator to the next item.
	loopInfo->ptr++;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToString(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

static Int WhereWithTwoArgsStart(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToString(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the first list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

static Int WhereWithTwoArgsBody(ClosureStateMachine closure)
{
	register WhereInfo loopInfo = (WhereInfo)closure->state;

	// Body: Append the user function's most recent result to the output list.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If it's truthy, keep this list element.
	if (booleanResult) {
		StringBuilder_AppendByte(loopInfo->result, *loopInfo->ptr);
	}

	// Next: Move the iterator to the next item.
	loopInfo->ptr++;
	loopInfo->index++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushBoxed(closure, StringBuilder_ToString(loopInfo->result));
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	Closure_PushUnboxedInt64(closure, loopInfo->index);
	return 2;
}

SMILE_EXTERNAL_FUNCTION(Where)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	String smileString = (String)argv[0].obj;
	SmileFunction function = (SmileFunction)argv[1].obj;
	Int minArgs, maxArgs;
	WhereInfo whereInfo;
	ClosureStateMachine closure;
	const Byte *ptr;
	Int length;

	ptr = String_GetBytes(smileString);
	length = String_Length(smileString);

	SmileFunction_GetArgCounts(function, &minArgs, &maxArgs);

	closure = Eval_BeginStateMachine(
		maxArgs <= 1 ? WhereWithOneArgStart : WhereWithTwoArgsStart,
		maxArgs <= 1 ? WhereWithOneArgBody : WhereWithTwoArgsBody);

	whereInfo = (WhereInfo)closure->state;
	whereInfo->function = function;
	whereInfo->ptr = ptr;
	whereInfo->end = ptr + length;
	whereInfo->result = StringBuilder_Create();
	whereInfo->index = 0;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

typedef struct CountInfoStruct {
	const Byte *ptr, *end;
	SmileFunction function;
	Int count;
} *CountInfo;

static Int CountStart(ClosureStateMachine closure)
{
	register CountInfo loopInfo = (CountInfo)closure->state;

	//---------- begin first for-loop iteration ----------

	// Condition: If we've run out of string bytes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushUnboxedInt64(closure, loopInfo->count);
		return -1;
	}

	// Body: Set up to call the user's function with the next character.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

static Int CountBody(ClosureStateMachine closure)
{
	register CountInfo loopInfo = (CountInfo)closure->state;

	// Body: Get the value from the user's condition.
	SmileArg fnResult = Closure_Pop(closure);
	Bool booleanResult = SMILE_VCALL1(fnResult.obj, toBool, fnResult.unboxed);

	// If we found a hit, add it to the count.  (We always add here to avoid the possibility
	// of a branch misprediction from an if-statement.)
	loopInfo->count += booleanResult;

	// Next: Move the iterator to the next byte.
	loopInfo->ptr++;

	//---------- end previous for-loop iteration ----------

	//---------- begin next for-loop iteration ----------

	// Condition: If we've run out of list nodes, we're done.
	if (loopInfo->ptr >= loopInfo->end) {
		Closure_PushUnboxedInt64(closure, loopInfo->count);
		return -1;
	}

	// Body: Set up to call the user's function with the next list item.
	Closure_PushBoxed(closure, loopInfo->function);
	Closure_PushUnboxedByte(closure, *loopInfo->ptr);
	return 1;
}

SMILE_EXTERNAL_FUNCTION(Count)
{
	// We use Eval's state-machine construct to avoid recursing deeper on the C stack.
	String smileString = (String)argv[0].obj;
	CountInfo countInfo;
	ClosureStateMachine closure;
	Byte value;
	const Byte *ptr, *end;
	Int count;
	Int length;

	if (argc < 1) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'count' requires at least 1 argument, but was called with %d.", argc));
	}
	if (SMILE_KIND(argv[0].obj) != SMILE_KIND_STRING) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("Argument 1 to 'count' is of the wrong type."));
	}

	ptr = String_GetBytes(smileString);
	length = String_Length(smileString);

	if (argc == 1) {
		// Degenerate form: Just return the length of the string.
		return SmileUnboxedInteger64_From(length);
	}

	if (argc > 2) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format("'count' allows at most 2 arguments, but was called with %d.", argc));
	}

	if (SMILE_KIND(argv[1].obj) != SMILE_KIND_FUNCTION) {

		if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_CHAR)
			return SmileUnboxedInteger64_From(0);

		// Degenerate form:  Count up any bytes that are equal to the given byte.
		value = argv[1].unboxed.ch;
		end = ptr + length;
		for (count = 0; ptr < end; ptr++) {
			if (*ptr == value) count++;
		}
		return SmileUnboxedInteger64_From(count);
	}

	closure = Eval_BeginStateMachine(CountStart, CountBody);

	countInfo = (CountInfo)closure->state;
	countInfo->function = (SmileFunction)argv[1].obj;
	countInfo->ptr = ptr;
	countInfo->end = ptr + length;

	return (SmileArg){ NULL };	// We have to return something, but this value will be ignored.
}

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(SplitCommandLine)
{
	String x = (String)argv[0].obj;
	SmileList args = String_SplitCommandLine(x);
	return SmileArg_From((SmileObject)args);
}

static struct {
	Symbol backslashEscapes;
	Symbol caseInsensitive;
	Symbol caseSensitive;
	Symbol filenameMode;
} _wildcardMatchingSymbols;

SMILE_EXTERNAL_FUNCTION(WildcardMatches)
{
	String str = (String)argv[0].obj;
	String pattern = (String)argv[1].obj;
	Int options = StringWildcardOptions_None;
	Int i;

	if (argc > 2) {
		for (i = 2; i < argc; i++) {
			Symbol symbol = argv[i].unboxed.symbol;
			if (symbol == _wildcardMatchingSymbols.backslashEscapes)
				options |= StringWildcardOptions_BackslashEscapes;
			else if (symbol == _wildcardMatchingSymbols.caseInsensitive)
				options |= StringWildcardOptions_CaseInsensitive;
			else if (symbol == _wildcardMatchingSymbols.caseSensitive)
				options &= ~StringWildcardOptions_CaseInsensitive;
			else if (symbol == _wildcardMatchingSymbols.filenameMode)
				options |= StringWildcardOptions_FilenameMode;
			else {
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_Format("wildcard-matches?: Unknown option \"%S\"",
						SymbolTable_GetName(Smile_SymbolTable, symbol)));
			}
		}
	}

	Bool isMatch = String_WildcardMatch(pattern, str, options);

	return SmileUnboxedBool_From(isMatch);
}

//-------------------------------------------------------------------------------------------------

void String_Setup(SmileUserObject base)
{
	_wildcardMatchingSymbols.backslashEscapes = SymbolTable_GetSymbolC(Smile_SymbolTable, "backslash-escapes");
	_wildcardMatchingSymbols.caseInsensitive = SymbolTable_GetSymbolC(Smile_SymbolTable, "case-insensitive");
	_wildcardMatchingSymbols.caseSensitive = SymbolTable_GetSymbolC(Smile_SymbolTable, "case-sensitive");
	_wildcardMatchingSymbols.filenameMode = SymbolTable_GetSymbolC(Smile_SymbolTable, "filename-mode");

	SetupFunction("bool", ToBool, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("int", ToInt, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);
	SetupFunction("string", ToString, NULL, "value", ARG_CHECK_MIN | ARG_CHECK_MAX, 1, 2, 0, NULL);
	SetupFunction("hash", Hash, NULL, "value", ARG_CHECK_EXACT, 1, 1, 0, NULL);

	SetupFunction("+", Plus, NULL, "x y", ARG_CHECK_MIN, 1, 0, 0, NULL);
	SetupSynonym("+", "concat");
	SetupFunction("remove", Remove, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _stringChecks);
	SetupSynonym("remove", "-");
	SetupFunction("repeat", Repeat, NULL, "x count", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringNumberChecks);
	SetupSynonym("repeat", "*");
	SetupFunction("/", SlashAppend, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 8, _stringChecks);

	SetupFunction("substr", Substr, NULL, "x y", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 2, _stringComparisonChecks);
	SetupSynonym("substr", "mid");
	SetupSynonym("substr", "substring");
	SetupFunction("left", Left, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringNumberChecks);
	SetupFunction("right", Right, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringNumberChecks);
	SetupFunction("chip", Chip, NULL, "str count", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _stringNumberChecks);
	SetupFunction("chop", Chop, NULL, "str count", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _stringNumberChecks);

	SetupFunction("starts-with?", StartsWith, NULL, "str pattern", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _indexOfChecks);
	SetupFunction("starts-with~?", StartsWithI, NULL, "str pattern", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _indexOfChecks);
	SetupFunction("ends-with?", EndsWith, NULL, "str pattern", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _indexOfChecks);
	SetupFunction("ends-with~?", EndsWithI, NULL, "str pattern", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _indexOfChecks);
	SetupFunction("contains?", Contains, NULL, "str pattern", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _indexOfChecks);
	SetupFunction("contains~?", ContainsI, NULL, "str pattern", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _indexOfChecks);
	SetupFunction("index-of", IndexOf, NULL, "str pattern", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);
	SetupFunction("index-of~", IndexOfI, NULL, "str pattern", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);
	SetupFunction("last-index-of", LastIndexOf, NULL, "str pattern", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);
	SetupFunction("last-index-of~", LastIndexOfI, NULL, "str pattern", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);
	SetupFunction("count-of", CountOf, NULL, "str pattern", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _indexOfChecks);
	SetupFunction("count-of~", CountOfI, NULL, "str pattern", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _indexOfChecks);

	SetupFunction("split", Split, NULL, "str pattern limit", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);

	SetupFunction("replace", Replace, NULL, "str pattern replacement", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _stringReplaceChecks);
	SetupFunction("replace~", ReplaceI, NULL, "str pattern replacement", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 3, 3, 3, _stringReplaceChecks);
	SetupSynonym("replace", "subst");
	SetupSynonym("replace~", "subst~");

	SetupFunction("trim", Trim, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("trim-start", TrimStart, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("trim-end", TrimEnd, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("compact-whitespace", CompactWhitespace, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("replace-newlines", ReplaceNewlines, NULL, "string replacement", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("split-newlines", SplitNewlines, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("remove-bom", RemoveBOM, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("trim-bom", TrimBOM, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("add-bom", AddBOM, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("pad-start", PadStart, NULL, "string length char", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _padChecks);
	SetupFunction("pad-end", PadEnd, NULL, "string length char", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _padChecks);
	SetupFunction("pad-center", PadCenter, NULL, "string length char", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _padChecks);
	SetupSynonym("pad-start", "left-pad");
	SetupSynonym("pad-end", "right-pad");
	SetupFunction("reverse", Reverse, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("uni-reverse", UniReverse, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);

	SetupFunction("case-fold", CaseFold, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupSynonym("case-fold", "fold");
	SetupFunction("lowercase", Lowercase, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("uppercase", Uppercase, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("titlecase", Titlecase, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("decompose", Decompose, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("compose", Compose, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("normalize-diacritics", NormalizeDiacritics, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);

	SetupFunction("camelCase", CamelCase, (void *)0, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("CamelCase", CamelCase, (void *)1, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("camelCase-acronyms", CamelCase, (void *)2, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("CamelCase-acronyms", CamelCase, (void *)3, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("hyphenize", Hyphenize, (void *)'-', "string", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _hyphenizeChecks);
	SetupFunction("underscorize", Hyphenize, (void *)'_', "string", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _hyphenizeChecks);

	SetupFunction("empty?", IsEmpty, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("whitespace?", IsWhitespace, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("alpha?", IsAlpha, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("lowercase?", IsLowercase, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("uppercase?", IsUppercase, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("alnum?", IsAlnum, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("digits?", IsDigits, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("hex?", IsHex, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("octal?", IsOctal, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);

	SetupFunction("c-ident?", IsCIdent, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("ident?", IsIdent, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("alpha-ident?", IsAlphaIdent, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("punct-ident?", IsPunctIdent, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);

	SetupFunction("rot13", Rot13, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("add-c-slashes", AddCSlashes, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("strip-c-slashes", StripCSlashes, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("html-encode", HtmlEncode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("html-decode", HtmlDecode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("url-encode", UrlEncode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("url-query-encode", UrlQueryEncode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("url-decode", UrlDecode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("regex-escape", RegexEscape, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("newlines-to-breaks", NewlinesToBreaks, NULL, "string break", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 1, 2, 2, _stringChecks);

	SetupFunction("==", Eq, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringComparisonChecks);
	SetupFunction("==~", EqI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringComparisonChecks);
	SetupFunction("!=", Ne, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringComparisonChecks);
	SetupFunction("!=~", NeI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringComparisonChecks);
	SetupFunction("<", Lt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("<~", LtI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction(">", Gt, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction(">~", GtI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("<=", Le, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("<=~", LeI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction(">=", Ge, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction(">=~", GeI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);

	SetupFunction("compare", Compare, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupSynonym("compare", "cmp");
	SetupFunction("compare~", CompareI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupSynonym("compare~", "cmp~");

	SetupFunction("get-member", GetMember, NULL, "str index", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringComparisonChecks);
	SetupSynonym("get-member", "at");
	SetupFunction("uni-at", UniAt, NULL, "str index", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 2, 2, _stringNumberChecks);
	SetupFunction("uni-length-at", UniLengthAt, NULL, "str index", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 2, 2, _stringNumberChecks);
	SetupFunction("uni-next", UniNext, NULL, "str index", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 2, 2, _stringNumberChecks);

	SetupFunction("each", Each, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("map", Map, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("where", Where, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("count", Count, NULL, "string", ARG_STATE_MACHINE, 0, 0, 0, NULL);

	SetupFunction("byte-array", ByteArray, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);

	SetupFunction("split-command-line", SplitCommandLine, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("wildcard-matches?", WildcardMatches, NULL, "string", ARG_CHECK_MIN | ARG_CHECK_TYPES, 2, 2, 3, _wildcardChecks);

	// Missing:
	//    match, matches?
	//    set-member
	//    splice
	//    uni-digits?, uni-letters?, uni-letters-digits?, uni-lowercase?, uni-uppercase?, uni-titlecase?
	//    sprintf, symbol
	//    each-uni, map-uni, where-uni, count-uni
	//    uni-array, char-array
}
