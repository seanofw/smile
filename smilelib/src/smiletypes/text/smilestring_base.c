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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/range/smileinteger64range.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/raw/smilebytearray.h>
#include <smile/internal/staticstring.h>

SMILE_IGNORE_UNUSED_VARIABLES

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
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
};

static Byte _eachChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_FUNCTION,
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
	if (SMILE_KIND(argv[0].obj) == SMILE_KIND_STRING)
		return SmileUnboxedInteger64_From(String_Length((String)argv[0].obj));

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

	return SmileUnboxedInteger64_From(((PtrInt)argv[0].obj) ^ Smile_HashOracle);
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
			case SMILE_KIND_UNBOXED_BYTE:
				x = String_ConcatByte(x, argv[1].unboxed.i8);
				return SmileArg_From((SmileObject)x);
			case SMILE_KIND_UNBOXED_INTEGER16:
				value = argv[1].unboxed.i16;
				break;
			case SMILE_KIND_INTEGER32:
				value = argv[1].unboxed.i32;
				break;
			case SMILE_KIND_INTEGER64:
				value = argv[1].unboxed.i64;
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
		case SMILE_KIND_UNBOXED_BYTE:
			StringBuilder_AppendByte(stringBuilder, argv[1].unboxed.i8);
			break;
		case SMILE_KIND_UNBOXED_INTEGER16:
			StringBuilder_AppendUnicode(stringBuilder, argv[1].unboxed.i16);
			break;
		case SMILE_KIND_UNBOXED_INTEGER32:
			value = argv[1].unboxed.i32;
			goto append_unicode;
		case SMILE_KIND_UNBOXED_INTEGER64:
			value = argv[1].unboxed.i64;
		append_unicode:
			if (value < 0 || value >= 0x110000) {
				Smile_ThrowException(Smile_KnownSymbols.native_method_error, _plusIllegalUnicodeChar);
			}
			StringBuilder_AppendUnicode(stringBuilder, (UInt32)value);
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

SMILE_EXTERNAL_FUNCTION(StartsWith)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;
	Bool result = String_StartsWith(x, y);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(StartsWithI)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;
	Bool result = String_StartsWithI(x, y);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(EndsWith)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;
	Bool result = String_EndsWith(x, y);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(EndsWithI)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;
	Bool result = String_EndsWithI(x, y);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Contains)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;
	Bool result = String_Contains(x, y);
	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(ContainsI)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;
	Bool result = String_ContainsI(x, y);
	return SmileUnboxedBool_From(result);
}

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

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(IndexOf)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int stringLength = String_Length(x);
	Int result;

	result = startIndex < stringLength ? String_IndexOf(x, y, (Int)startIndex) : -1;
	if (result < 0) return SmileArg_From(NullObject);

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(IndexOfI)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int stringLength = String_Length(x);
	Int result;

	result = startIndex < stringLength ? String_IndexOfI(x, y, (Int)startIndex) : -1;
	if (result < 0) return SmileArg_From(NullObject);

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(LastIndexOf)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int stringLength = String_Length(x);
	Int result;

	result = startIndex < stringLength ? String_LastIndexOf(x, y, (Int)startIndex) : -1;
	if (result < 0) return SmileArg_From(NullObject);

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(LastIndexOfI)
{
	String x = (String)argv[0].obj;
	String y = (String)argv[1].obj;

	Int64 startIndex = argc > 2 ? argv[2].unboxed.i64 : 0;
	Int stringLength = String_Length(x);
	Int result;

	result = startIndex < stringLength ? String_LastIndexOfI(x, y, (Int)startIndex) : -1;
	if (result < 0) return SmileArg_From(NullObject);

	return SmileUnboxedInteger64_From(result);
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

			return SmileUnboxedByte_From(ch);

		case SMILE_KIND_INTEGER64RANGE:
			range = (SmileInteger64Range)argv[1].obj;
			return SmileArg_From((SmileObject)String_SubstringByRange(x, range->start, range->end, range->stepping));

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, indexTypeError);
	}
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
		case SMILE_KIND_UNBOXED_BYTE:
			StringBuilder_AppendByte(loopInfo->result, fnResult.unboxed.i8);
			break;
		case SMILE_KIND_UNBOXED_INTEGER16:
			StringBuilder_AppendUnicode(loopInfo->result, fnResult.unboxed.i16);
			break;
		case SMILE_KIND_UNBOXED_INTEGER32:
			StringBuilder_AppendUnicode(loopInfo->result, fnResult.unboxed.i32);
			break;
		case SMILE_KIND_UNBOXED_INTEGER64:
			StringBuilder_AppendUnicode(loopInfo->result, (UInt32)fnResult.unboxed.i64);
			break;
		case SMILE_KIND_STRING:
			StringBuilder_AppendString(loopInfo->result, (String)fnResult.obj);
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("'map' projection must return a String or Integer type."));
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
		case SMILE_KIND_UNBOXED_BYTE:
			StringBuilder_AppendByte(loopInfo->result, fnResult.unboxed.i8);
			break;
		case SMILE_KIND_UNBOXED_INTEGER16:
			StringBuilder_AppendUnicode(loopInfo->result, fnResult.unboxed.i16);
			break;
		case SMILE_KIND_UNBOXED_INTEGER32:
			StringBuilder_AppendUnicode(loopInfo->result, fnResult.unboxed.i32);
			break;
		case SMILE_KIND_UNBOXED_INTEGER64:
			StringBuilder_AppendUnicode(loopInfo->result, (UInt32)fnResult.unboxed.i64);
			break;
		case SMILE_KIND_STRING:
			StringBuilder_AppendString(loopInfo->result, (String)fnResult.obj);
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_FromC("'map' projection must return a String or Integer type."));
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

		if (SMILE_KIND(argv[1].obj) != SMILE_KIND_UNBOXED_BYTE)
			return SmileUnboxedInteger64_From(0);

		// Degenerate form:  Count up any bytes that are equal to the given byte.
		value = argv[1].unboxed.i8;
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

void String_Setup(SmileUserObject base)
{
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

	SetupFunction("index-of", IndexOf, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);
	SetupFunction("index-of~", IndexOfI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);
	SetupFunction("last-index-of", LastIndexOf, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);
	SetupFunction("last-index-of~", LastIndexOfI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 3, 3, _indexOfChecks);

	SetupFunction("trim", Trim, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("trim-start", TrimStart, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("trim-end", TrimEnd, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("compact-whitespace", CompactWhitespace, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("remove-bom", RemoveBOM, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("trim-bom", TrimBOM, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("add-bom", AddBOM, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("pad-start", PadStart, NULL, "string length char", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _padChecks);
	SetupFunction("pad-end", PadEnd, NULL, "string length char", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _padChecks);
	SetupFunction("pad-center", PadCenter, NULL, "string length char", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _padChecks);

	SetupFunction("case-fold", CaseFold, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupSynonym("case-fold", "fold");
	SetupFunction("lowercase", Lowercase, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("uppercase", Uppercase, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("titlecase", Titlecase, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("decompose", Decompose, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("compose", Compose, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("normalize-diacritics", NormalizeDiacritics, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);

	SetupFunction("starts-with?", StartsWith, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("starts-with~?", StartsWithI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("ends-with?", EndsWith, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("ends-with~?", EndsWithI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("contains?", Contains, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("contains~?", ContainsI, NULL, "x y", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupFunction("empty?", IsEmpty, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("whitespace?", IsWhitespace, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);

	SetupFunction("rot13", Rot13, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("add-c-slashes", AddCSlashes, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("strip-c-slashes", StripCSlashes, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("html-encode", HtmlEncode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("html-decode", HtmlDecode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("url-encode", UrlEncode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("url-query-encode", UrlQueryEncode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("url-decode", UrlDecode, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("regex-escape", RegexEscape, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);

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

	SetupFunction("each", Each, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("map", Map, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("where", Where, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES | ARG_STATE_MACHINE, 2, 2, 2, _eachChecks);
	SetupFunction("count", Count, NULL, "string", ARG_STATE_MACHINE, 0, 0, 0, NULL);

	SetupFunction("byte-array", ByteArray, NULL, "string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
}
