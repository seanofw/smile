#ifndef __SMILE_STRING_H__
#define __SMILE_STRING_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_MEM_H__
#include <smile/mem.h>
#endif

#ifndef __SMILE_ARRAY_H__
#include <smile/array.h>
#endif

#ifndef __SMILE_SMILETYPES_PREDECL_H__
#include <smile/smiletypes/predecl.h>
#endif

#ifndef __SMILE_SMILETYPES_KIND_H__
#include <smile/smiletypes/kind.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Public type declarations

// Predeclare these two types.  We can't simply #include "smileobject.h" to get them
// because that file needs to include this file!
struct SmileVTableInt;
struct SmileObjectInt;

/// <summary>
/// A String, which is an IMMUTABLE array of characters, with many functions to operate on it.
/// Strings are also real Smile objects, with a virtual table and base pointer.
/// </summary>
typedef struct StringStruct {
	UInt32 kind;	// What kind of native object this is, from the SMILE_KIND enumeration
	struct SmileVTableInt *vtable;	// A pointer to this object's virtual table, which must match SMILE_VTABLE_TYPE.
	struct SmileObjectInt *base;	// A pointer to the String base object.

	struct {
		Int length;	// The length of the text array (in bytes, not Unicode code points).
		Byte text[1024];	// The actual bytes of the string (nul-terminated).  Not actually an array of 1024 bytes, either.
	} _opaque;
} *String;

/// <summary>
/// A StringBuilder, which is a tool for efficiently constructing strings by repeatedly
/// appending text to them.  This is an incomplete type, as declared here.
/// </summary>
typedef struct StringBuilderStruct *StringBuilder;

/// <summary>
/// Options for use in String_Split().
/// </summary>
enum StringSplitOptions {
	StringSplitOptions_None = 0,
	StringSplitOptions_RemoveEmptyEntries = (1 << 0),
};

/// <summary>
/// Options for use in String_WildcardMatch().
/// </summary>
enum StringWildcardOptions {
	StringWildcardOptions_None = 0,
	StringWildcardOptions_FilenameMode = (1 << 0),
	StringWildcardOptions_BackslashEscapes = (1 << 1),
	StringWildcardOptions_CaseInsensitive = (1 << 2),
};

//-------------------------------------------------------------------------------------------------
//  Special common preallocated strings (static, not on the heap).

// The special one-and-only empty string.
SMILE_API_DATA String String_Empty;

// Common ASCII formatting/control codes, as single-character strings.
SMILE_API_DATA String String_Nul;	// 0x00
SMILE_API_DATA String String_Bell;	// 0x07
SMILE_API_DATA String String_Backspace;	// 0x08
SMILE_API_DATA String String_Tab;	// 0x09
SMILE_API_DATA String String_Newline;	// 0x0A
SMILE_API_DATA String String_VerticalTab;	// 0x0B
SMILE_API_DATA String String_FormFeed;	// 0x0C
SMILE_API_DATA String String_CarriageReturn;	// 0x0D
SMILE_API_DATA String String_Escape;	// 0x1B
SMILE_API_DATA String String_Space;	// 0x20

// ASCII punctuation, as single-character strings.
SMILE_API_DATA String String_ExclamationPoint;
SMILE_API_DATA String String_QuotationMark;
SMILE_API_DATA String String_PoundSign;
SMILE_API_DATA String String_Dollar;
SMILE_API_DATA String String_Percent;
SMILE_API_DATA String String_Ampersand;
SMILE_API_DATA String String_Apostrophe;
SMILE_API_DATA String String_LeftParenthesis;
SMILE_API_DATA String String_RightParenthesis;
SMILE_API_DATA String String_Star;
SMILE_API_DATA String String_Plus;
SMILE_API_DATA String String_Comma;
SMILE_API_DATA String String_Hyphen;
SMILE_API_DATA String String_Period;
SMILE_API_DATA String String_Slash;
SMILE_API_DATA String String_Colon;
SMILE_API_DATA String String_Semicolon;
SMILE_API_DATA String String_LessThan;
SMILE_API_DATA String String_Equal;
SMILE_API_DATA String String_GreaterThan;
SMILE_API_DATA String String_QuestionMark;
SMILE_API_DATA String String_AtSign;
SMILE_API_DATA String String_LeftBracket;
SMILE_API_DATA String String_Backslash;
SMILE_API_DATA String String_RightBracket;
SMILE_API_DATA String String_Caret;
SMILE_API_DATA String String_Underscore;
SMILE_API_DATA String String_Backtick;
SMILE_API_DATA String String_LeftBrace;
SMILE_API_DATA String String_VerticalBar;
SMILE_API_DATA String String_RightBrace;
SMILE_API_DATA String String_Tilde;

// ASCII numbers, as single-character strings.
SMILE_API_DATA String String_Zero;
SMILE_API_DATA String String_One;
SMILE_API_DATA String String_Two;
SMILE_API_DATA String String_Three;
SMILE_API_DATA String String_Four;
SMILE_API_DATA String String_Five;
SMILE_API_DATA String String_Six;
SMILE_API_DATA String String_Seven;
SMILE_API_DATA String String_Eight;
SMILE_API_DATA String String_Nine;

// An array of the ASCII numbers, as strings.
SMILE_API_DATA String String_Number[10];

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (core)

SMILE_API_FUNC String String_Create(const Byte *text, Int length);
SMILE_API_FUNC String String_CreateInternal(Int length);
SMILE_API_FUNC String String_CreateRepeat(Byte b, Int repeatCount);

SMILE_API_FUNC String String_ConcatMany(const String *strs, Int numStrs);
SMILE_API_FUNC String String_Join(const String glue, const String *strs, Int numStrs);
SMILE_API_FUNC String String_SlashAppend(const String *strs, Int numStrs);

SMILE_API_FUNC Bool String_Equals(const String str, const String other);
SMILE_API_FUNC Bool String_EqualsC(const String str, const char *other);
SMILE_API_FUNC Bool String_EqualsInternal(const String str, const Byte *otherText, Int otherLength);
SMILE_API_FUNC Int String_Compare(const String a, const String b);
SMILE_API_FUNC Int String_CompareRange(const String a, Int astart, Int alength, const String b, Int bstart, Int blength);

SMILE_API_FUNC String String_SubstringAt(const String str, Int start);
SMILE_API_FUNC String String_Substring(const String str, Int start, Int length);
SMILE_API_FUNC String String_SubstringByRange(const String str, Int64 start, Int64 end, Int64 step);
SMILE_API_FUNC String String_Concat(const String str, const String other);
SMILE_API_FUNC String String_ConcatByte(const String str, Byte ch);

SMILE_API_FUNC Int String_IndexOf(const String str, const String pattern, Int start);
SMILE_API_FUNC Int String_LastIndexOf(const String str, const String pattern, Int start);
SMILE_API_FUNC Bool String_StartsWithC(const String str, const char *pattern);
SMILE_API_FUNC Bool String_StartsWith(const String str, const String pattern);
SMILE_API_FUNC Bool String_EndsWithC(const String str, const char *pattern);
SMILE_API_FUNC Bool String_EndsWith(const String str, const String pattern);
SMILE_API_FUNC Int String_IndexOfChar(const String str, Byte ch, Int start);
SMILE_API_FUNC Int String_LastIndexOfChar(const String str, Byte ch, Int start);
SMILE_API_FUNC Int String_IndexOfAnyChar(const String str, const Byte *chars, Int numChars, Int start);
SMILE_API_FUNC String String_Replace(const String str, const String pattern, const String replacement);
SMILE_API_FUNC String String_ReplaceChar(const String str, Byte pattern, Byte replacement);

SMILE_API_FUNC String String_Format(const char *format, ...);
SMILE_API_FUNC String String_FormatV(const char *format, va_list v);
SMILE_API_FUNC String String_FormatString(const String format, ...);
SMILE_API_FUNC String String_FormatStringV(const String format, va_list v);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (Extra)

SMILE_API_FUNC Int String_SplitWithOptions(const String str, const String pattern, Int limit, Int options, String **pieces);

SMILE_API_FUNC String String_RawReverse(const String str);
SMILE_API_FUNC String String_Reverse(const String str);
SMILE_API_FUNC String String_Repeat(const String str, Int count);
SMILE_API_FUNC String String_PadStart(const String str, Int minLength, Byte padChar);
SMILE_API_FUNC String String_PadEnd(const String str, Int minLength, Byte padChar);
SMILE_API_FUNC String String_PadCenter(const String str, Int minLength, Byte padChar);
SMILE_API_FUNC String String_TrimWhitespace(const String str, Bool trimStart, Bool trimEnd);
SMILE_API_FUNC String String_CompactWhitespace(const String str);
SMILE_API_FUNC String String_AddCSlashes(const String str);
SMILE_API_FUNC String String_StripCSlashes(const String str);
SMILE_API_FUNC String String_Rot13(const String str);
SMILE_API_FUNC String String_RegexEscape(const String str);
SMILE_API_FUNC Bool String_WildcardMatch(const String pattern, const String text, Int wildcardOptions);
SMILE_API_FUNC String String_JoinEnglishNames(const String *items, Int numItems, const String conjunction);
SMILE_API_FUNC Bool String_IsNullOrWhitespace(const String str);
SMILE_API_FUNC SmileList String_SplitCommandLine(const String commandLine);

SMILE_API_FUNC String String_FromUtf16(const UInt16 *text, Int length);
SMILE_API_FUNC UInt16 *String_ToUtf16(const String str, Int *length);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (HTML-specific transformations)

SMILE_API_FUNC String String_HtmlEncode(const String str);
SMILE_API_FUNC String String_HtmlEncodeToAscii(const String str);
SMILE_API_FUNC String String_HtmlDecode(const String str);
SMILE_API_FUNC String String_UrlEncode(const String str);
SMILE_API_FUNC String String_UrlQueryEncode(const String str);
SMILE_API_FUNC String String_UrlDecode(const String str);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (identifier conversion)

SMILE_API_FUNC String String_CamelCase(String string, Bool uppercaseInitialLetter, Bool lowercaseAcronyms);
SMILE_API_FUNC String String_Hyphenize(String string, Byte separator);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (Parsing of other types)

SMILE_API_FUNC Bool String_ParseBool(const String str, Bool *result);
SMILE_API_FUNC Bool String_ParseInteger(const String str, Int numericBase, Int64 *result);
SMILE_API_FUNC Bool String_ParseReal(const String str, Int numericBase, Real128 *result);
SMILE_API_FUNC Bool String_ParseFloat(const String str, Int numericBase, Float64 *result);

SMILE_API_FUNC String String_CreateFromInteger(Int64 value, Int numericBase, Bool includeSign);
SMILE_API_FUNC String String_CreateFromUnsignedInteger(UInt64 value, Int numericBase);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (Unicode support)

SMILE_API_FUNC Int String_CompareRangeI(const String a, Int astart, Int alength, const String b, Int bstart, Int blength, Bool *usedSlowConversion);
SMILE_API_FUNC Int String_IndexOfI(const String str, const String pattern, Int start);
SMILE_API_FUNC Int String_LastIndexOfI(const String str, const String pattern, Int start);
SMILE_API_FUNC Bool String_ContainsI(const String str, const String pattern);
SMILE_API_FUNC Bool String_StartsWithI(const String str, const String pattern);
SMILE_API_FUNC Bool String_EndsWithI(const String str, const String pattern);

SMILE_API_FUNC String String_ToLowerRange(const String str, Int start, Int length);
SMILE_API_FUNC String String_ToTitleRange(const String str, Int start, Int length);
SMILE_API_FUNC String String_ToUpperRange(const String str, Int start, Int length);
SMILE_API_FUNC String String_CaseFoldRange(const String str, Int start, Int length);
SMILE_API_FUNC String String_DecomposeRange(const String str, Int start, Int length);
SMILE_API_FUNC String String_ComposeRange(const String str, Int start, Int length);
SMILE_API_FUNC String String_NormalizeRange(const String str, Int start, Int length);

SMILE_API_FUNC Int32 String_ExtractUnicodeCharacter(const String str, Int *index);
SMILE_API_FUNC Int32 String_ExtractUnicodeCharacterInternal(const Byte **start, const Byte *end);

SMILE_API_FUNC String String_ConvertUtf8ToCodePageRange(const String str, Int start, Int length, const Byte **utf8ToCodePageTables, Int numTables);
SMILE_API_FUNC String String_ConvertCodePageToUtf8Range(const String str, Int start, Int length, const UInt16 *codePageToUtf8Table);

//-------------------------------------------------------------------------------------------------
//  Known (supported) legacy code pages.

enum {
	LEGACY_CODE_PAGE_ISO_8859_1 = 1,
	LEGACY_CODE_PAGE_ISO_8859_2 = 2,
	LEGACY_CODE_PAGE_ISO_8859_3 = 3,
	LEGACY_CODE_PAGE_ISO_8859_4 = 4,
	LEGACY_CODE_PAGE_ISO_8859_5 = 5,
	LEGACY_CODE_PAGE_ISO_8859_6 = 6,
	LEGACY_CODE_PAGE_ISO_8859_7 = 7,
	LEGACY_CODE_PAGE_ISO_8859_8 = 8,
	LEGACY_CODE_PAGE_ISO_8859_9 = 9,
	LEGACY_CODE_PAGE_ISO_8859_10 = 10,
	LEGACY_CODE_PAGE_ISO_8859_11 = 11,
	// There is no such thing as ISO 8859-12.
	LEGACY_CODE_PAGE_ISO_8859_13 = 13,
	LEGACY_CODE_PAGE_ISO_8859_14 = 14,
	LEGACY_CODE_PAGE_ISO_8859_15 = 15,
	LEGACY_CODE_PAGE_ISO_8859_16 = 16,

	LEGACY_CODE_PAGE_CP437 = 437,

	LEGACY_CODE_PAGE_WIN1250 = 1250,
	LEGACY_CODE_PAGE_WIN1251 = 1251,
	LEGACY_CODE_PAGE_WIN1252 = 1252,
	LEGACY_CODE_PAGE_WIN1253 = 1253,
	LEGACY_CODE_PAGE_WIN1254 = 1254,
	LEGACY_CODE_PAGE_WIN1255 = 1255,
	LEGACY_CODE_PAGE_WIN1256 = 1256,
	LEGACY_CODE_PAGE_WIN1257 = 1257,
	LEGACY_CODE_PAGE_WIN1258 = 1258,
};

SMILE_API_FUNC String String_ConvertUtf8ToKnownCodePageRange(const String str, Int start, Int length, Int legacyCodePageID);
SMILE_API_FUNC String String_ConvertKnownCodePageToUtf8Range(const String str, Int start, Int length, Int legacyCodePageID);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

// Foreign reference to String's VTable so that we can statically instantiate strings.
struct String_VTableInt;
SMILE_API_DATA struct String_VTableInt String_VTableData;

// Foreign reference to String's base object so that we can statically instantiate strings.
SMILE_API_DATA struct SmileUserObjectInt String_BaseObjectStruct;

/// <summary>
/// Retrieve a byte from the string at the given index.  The index must
/// be valid, or you may read past the end of the string.
/// </summary>
/// <param name="__str__">The string to read one byte from.</param>
/// <param name="__index__">The index within that string of the byte to read.</param>
/// <returns>The byte at the given index.</returns>
#define String_At(__str__, __index__) \
	((const Byte)(__str__)->_opaque.text[(__index__)])

/// <summary>
/// Retrieve a pointer to the underlying byte array in the string.  Note that strings
/// are immutable, so you should *not* change any data found at this pointer or you
/// risk dangerous side-effects!  The bytes will be nul-terminated (a C-compatible string),
/// but may legally contain nul (zero) values within them; you should use String_Length()
/// to get the actual length of the string.
/// </summary>
/// <param name="__str__">The string to obtain the raw bytes of.</param>
/// <returns>The raw bytes of the string.</returns>
#define String_GetBytes(__str__) \
	((const Byte *)(__str__)->_opaque.text)

/// <summary>
/// Retrieve a pointer to the underlying byte array in the string.  Note that strings
/// are immutable, so you should *not* change any data found at this pointer or you
/// risk dangerous side-effects!  The bytes will be nul-terminated (a C-compatible string),
/// but may legally contain nul (zero) values within them; you should use String_Length()
/// to get the actual length of the string.
/// </summary>
/// <param name="__str__">The string to obtain the raw bytes of.</param>
/// <returns>The raw bytes of the string.</returns>
#define String_ToC(__str__) \
	((const char *)(__str__)->_opaque.text)

/// <summary>
/// Get the length of the given string, in bytes.
/// </summary>
/// <param name="str">The string to obtain the length of.</param>
/// <returns>The length of that string.</returns>
#define String_Length(__str__) \
	((const Int)(__str__)->_opaque.length)

/// <summary>
/// Create a String instance from a C-style (nul-terminated) string.
/// </summary>
/// <param name="text">A pointer to a C-style string that you wish to create a String from.</param>
/// <returns>A new String instance that contains a copy of the C-style string.</returns>
Inline String String_FromC(const char *text)
{
	return String_Create((const Byte *)text, StrLen(text));
}

/// <summary>
/// Determine if the given string is NULL or the empty string.
/// </summary>
/// <param name="str">The string to test.</param>
/// <returns>True if the string is NULL or the empty string, or False if it actually is a
/// valid string with content.</returns>
Inline Bool String_IsNullOrEmpty(const String str)
{
	return str == NULL || String_Length(str) == 0;
}

/// <summary>
/// Compute a 32-bit hash code for the given string, based on its bytes.
/// </summary>
/// <param name="str">The string to hash.</param>
/// <returns>A reasonably-unique hash value for that string.</returns>
Inline UInt32 String_Hash(const String str)
{
	return Smile_Hash(str->_opaque.text, String_Length(str));
}

/// <summary>
/// Compute a 32-bit hash code for the given string bytes.
/// </summary>
/// <param name="text">The text of the string to hash.</param>
/// <param name="length">The length of the text of the string to hash.</param>
/// <returns>A reasonably-unique hash value for that string.</returns>
Inline UInt32 String_HashInternal(const Byte *text, Int length)
{
	return Smile_Hash(text, length);
}

/// <summary>
/// Compute a 64-bit hash code for the given string, based on its bytes.
/// </summary>
/// <param name="str">The string to hash.</param>
/// <returns>A reasonably-unique hash value for that string.</returns>
Inline UInt64 String_Hash64(const String str)
{
	return Smile_Hash64(str->_opaque.text, String_Length(str));
}

/// <summary>
/// Search through the given string looking for the given pattern.
/// </summary>
/// <param name="str">The string to search through.</param>
/// <param name="pattern">The pattern to test the string against.</param>
/// <returns>True if any part of the string exactly matches the pattern; False if no part of the string exactly matches the pattern.</returns>
Inline Bool String_Contains(const String str, const String pattern)
{
	return String_IndexOf(str, pattern, 0) >= 0;
}

/// <summary>
/// Convert a string to lowercase, if it is not already lowercase.
/// </summary>
/// <param name="str">The string you would like to convert.</param>
/// <returns>The whole string converted to lowercase.</returns>
Inline String String_ToLower(const String str)
{
	return str != NULL ? String_ToLowerRange(str, 0, String_Length(str)) : (String )str;
}

/// <summary>
/// Convert a string to titlecase, if it is not already titlecase.
/// </summary>
/// <param name="str">The string you would like to convert.</param>
/// <returns>The whole string converted to titlecase.</returns>
Inline String String_ToTitle(const String str)
{
	return str != NULL ? String_ToTitleRange(str, 0, String_Length(str)) : (String )str;
}

/// <summary>
/// Convert a string to uppercase, if it is not already uppercase.
/// </summary>
/// <param name="str">The string you would like to convert.</param>
/// <returns>The whole string converted to uppercase.</returns>
Inline String String_ToUpper(const String str)
{
	return str != NULL ? String_ToUpperRange(str, 0, String_Length(str)) : (String )str;
}

/// <summary>
/// Case-fold a given string so that its characters are suitable for case-insensitive comparison.
/// </summary>
/// <param name="str">The string you would like to convert.</param>
/// <returns>The whole string, case-folded.</returns>
Inline String String_CaseFold(const String str)
{
	return str != NULL ? String_CaseFoldRange(str, 0, String_Length(str)) : (String )str;
}

/// <summary>
/// Decompose all composed characters within a given string, so that all combining diacritics and
/// compound characters are exposed as separate Unicode code points.
/// </summary>
/// <param name="str">The string you would like to decompose.</param>
/// <returns>The whole string, decomposed.</returns>
Inline String String_Decompose(const String str)
{
	return str != NULL ? String_DecomposeRange(str, 0, String_Length(str)) : (String )str;
}

/// <summary>
/// Compose all decomposed characters within a given string, so that all combining diacritics and
/// compound characters are joined to result in as few Unicode code points as possible.
/// </summary>
/// <param name="str">The string you would like to compose.</param>
/// <returns>The whole string, composed.</returns>
Inline String String_Compose(const String str)
{
	return str != NULL ? String_ComposeRange(str, 0, String_Length(str)) : (String )str;
}

/// <summary>
/// Normalize a string, so that its combining characters are in canonical order.
/// </summary>
/// <param name="str">The string you would like to normalize.</param>
/// <returns>The whole string, normalized.</returns>
Inline String String_Normalize(const String str)
{
	return str != NULL ? String_NormalizeRange(str, 0, String_Length(str)) : (String )str;
}

/// <summary>
/// Convert a string from UTF-8 encoding to that described by the given code-page tables.
/// </summary>
/// <param name="str">The string you would like to convert to a code page.</param>
/// <param name="utf8ToCodePageTables">A pointer to a table of pointers to 256-byte tables that describe how to
/// convert each Unicode code point.</param>
/// <param name="numTables">The number of 256-byte tables described.</param>
/// <returns>The whole string, converted to that code page.</returns>
Inline String String_ConvertUtf8ToCodePage(const String str, const Byte **utf8ToCodePageTables, Int numTables)
{
	return str != NULL ? String_ConvertUtf8ToCodePageRange(str, 0, String_Length(str), utf8ToCodePageTables, numTables) : (String )str;
}

/// <summary>
/// Convert a string from a specific code-page encoding to standard UTF-8.
/// </summary>
/// <param name="str">The string you would like to convert to UTF-8.</param>
/// <param name="codePageToUtf8Table">A 256-entry table describing the resulting Unicode code point for each byte in the string.</param>
/// <returns>The whole string, converted to UTF-8.</returns>
Inline String String_ConvertCodePageToUtf8(const String str, const UInt16 *codePageToUtf8Table)
{
	return str != NULL ? String_ConvertCodePageToUtf8Range(str, 0, String_Length(str), codePageToUtf8Table) : (String )str;
}

/// <summary>
/// Convert a string from UTF-8 encoding to that described by the given well-known legacy code page.
/// </summary>
/// <param name="str">The string you would like to convert to a code page.</param>
/// <param name="legacyCodePageID">The ID of the known legacy code-page.  If the code page is unknown, this will return the empty string.</param>
/// <returns>The whole string, converted to that code page.</returns>
Inline String String_ConvertUtf8ToKnownCodePage(const String str, Int legacyCodePageID)
{
	return str != NULL ? String_ConvertUtf8ToKnownCodePageRange(str, 0, String_Length(str), legacyCodePageID) : (String )str;
}

/// <summary>
/// Convert a string from a specific well-known legacy code-page encoding to standard UTF-8.
/// </summary>
/// <param name="str">The string you would like to convert to UTF-8.</param>
/// <param name="legacyCodePageID">The ID of the known legacy code-page.  If the code page is unknown, this will return the empty string.</param>
/// <returns>The whole string, converted to UTF-8.</returns>
Inline String String_ConvertKnownCodePageToUtf8(const String str, Int legacyCodePageID)
{
	return str != NULL ? String_ConvertKnownCodePageToUtf8Range(str, 0, String_Length(str), legacyCodePageID) : (String )str;
}

/// <summary>
/// Compare two strings, case-insensitive, to lexically order those strings,
/// without converting the strings to new string instances if possible.
/// </summary>
/// <param name="a">The first string to compare.</param>
/// <param name="b">The second string to compare.</param>
/// <returns>0 if the strings were equal; -1 if a comes before b; or +1 if b comes before a.</returns>
Inline Int String_CompareI(const String a, const String b)
{
	Bool usedSlowConversion;
	return String_CompareRangeI(a, 0, String_Length(a), b, 0, String_Length(b), &usedSlowConversion);
}

/// <summary>
/// Split a string by a given pattern string.
/// </summary>
/// <param name="str">The string to split.</param>
/// <param name="pattern">The substring on which to split that string.  If this is the empty string, no splitting will be performed.</param>
/// <param name="pieces">If non-NULL, this will be set to an array containing the resulting split strings.</param>
/// <returns>The number of strings resulting from the split (i.e., the number of strings returned in the 'pieces' array).</returns>
Inline Int String_Split(const String str, const String pattern, String **pieces)
{
	return String_SplitWithOptions(str, pattern, 0, StringSplitOptions_None, pieces);
}

/// <summary>
/// Trim whitespace from the ends of the string.  Whitespace is defined for this function
/// as byte values in the range of 0 to 32, inclusive.
/// </summary>
/// <param name="str">The string to trim.</param>
/// <returns>The string, with all whitespace trimmed from both ends of it.</returns>
Inline String String_Trim(const String str)
{
	return String_TrimWhitespace(str, True, True);
}

/// <summary>
/// Trim whitespace from the start of the string.  Whitespace is defined for this function
/// as byte values in the range of 0 to 32, inclusive.
/// </summary>
/// <param name="str">The string to trim.</param>
/// <returns>The string, with all whitespace trimmed from the start of it.</returns>
Inline String String_TrimStart(const String str)
{
	return String_TrimWhitespace(str, True, False);
}

/// <summary>
/// Trim whitespace from the end of the string.  Whitespace is defined for this function
/// as byte values in the range of 0 to 32, inclusive.
/// </summary>
/// <param name="str">The string to trim.</param>
/// <returns>The string, with all whitespace trimmed from the end of it.</returns>
Inline String String_TrimEnd(const String str)
{
	return String_TrimWhitespace(str, False, True);
}

/// <summary>
/// Determine if this string contains a '\0' (nul) character — in other words, determine
/// if it can be safely passed to a standard C string function, or if it would break.
/// </summary>
Inline Bool String_ContainsNul(const String str)
{
	return (String_IndexOfChar(str, '\0', 0) >= 0);
}

#endif