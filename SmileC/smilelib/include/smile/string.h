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

//-------------------------------------------------------------------------------------------------
//  Internal types

/// <summary>
/// The internal implementation of a String.
/// </summary>
struct StringInt {
	Byte *text;				// A pointer to the actual bytes of the string (nul-terminated).
	Int length;				// The length of the text array (in bytes, not Unicode code points).
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A String, which is an IMMUTABLE array of characters, with many functions to operate on it.
/// </summary>
typedef struct StringStruct {
	struct StringInt _opaque;
} *String;

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
//  External parts of the implementation (core)

SMILE_API String String_Empty;

SMILE_API String String_Create(const Byte *text, Int length);
SMILE_API String String_CreateInternal(Int length);
SMILE_API String String_CreateRepeat(Byte b, Int repeatCount);

SMILE_API String String_ConcatMany(const String *strs, Int numStrs);
SMILE_API String String_Join(const String glue, const String *strs, Int numStrs);
SMILE_API String String_SlashAppend(const String *strs, Int numStrs);

SMILE_API Bool String_Equals(const String str, const String other);
SMILE_API Int String_Compare(const String a, const String b);
SMILE_API Int String_CompareRange(const String a, Int astart, Int alength, const String b, Int bstart, Int blength);

SMILE_API String String_SubstringAt(const String str, Int start);
SMILE_API String String_Substring(const String str, Int start, Int length);
SMILE_API String String_Concat(const String str, const String other);
SMILE_API String String_ConcatByte(const String str, Byte ch);

SMILE_API Int String_IndexOf(const String str, const String pattern, Int start);
SMILE_API Int String_LastIndexOf(const String str, const String pattern, Int start);
SMILE_API Bool String_StartsWith(const String str, const String pattern);
SMILE_API Bool String_EndsWith(const String str, const String pattern);
SMILE_API Int String_IndexOfChar(const String str, Byte ch, Int start);
SMILE_API Int String_LastIndexOfChar(const String str, Byte ch, Int start);
SMILE_API Int String_IndexOfAnyChar(const String str, const Byte *chars, Int numChars, Int start);
SMILE_API String String_Replace(const String str, const String pattern, const String replacement);
SMILE_API String String_ReplaceChar(const String str, Byte pattern, Byte replacement);

SMILE_API String String_Format(const char *format, ...);
SMILE_API String String_FormatV(const char *format, va_list v);
SMILE_API String String_FormatString(const String format, ...);
SMILE_API String String_FormatStringV(const String format, va_list v);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (Extra)

SMILE_API Int String_SplitWithOptions(const String str, const String pattern, Int limit, Int options, String **pieces);

SMILE_API String String_RawReverse(const String str);
SMILE_API String String_Reverse(const String str);
SMILE_API String String_Repeat(const String str, Int count);
SMILE_API String String_PadStart(const String str, Int minLength, Byte padChar);
SMILE_API String String_PadEnd(const String str, Int minLength, Byte padChar);
SMILE_API String String_TrimWhitespace(const String str, Bool trimStart, Bool trimEnd);
SMILE_API String String_CompactWhitespace(const String str);
SMILE_API String String_AddCSlashes(const String str);
SMILE_API String String_StripCSlashes(const String str);
SMILE_API String String_Rot13(const String str);
SMILE_API String String_RegexEscape(const String str);
SMILE_API Bool String_WildcardMatch(const String pattern, const String text, Int wildcardOptions);
SMILE_API String String_JoinEnglishNames(const String *items, Int numItems, const String conjunction);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (HTML-specific transformations)

SMILE_API String String_HtmlEncode(const String str);
SMILE_API String String_HtmlEncodeToAscii(const String str);
SMILE_API String String_HtmlDecode(const String str);
SMILE_API String String_UrlEncode(const String str);
SMILE_API String String_UrlQueryEncode(const String str);
SMILE_API String String_UrlDecode(const String str);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (Parsing of other types)

SMILE_API Bool String_ParseBool(const String str, Bool *result);
SMILE_API Bool String_ParseInteger(const String str, Int numericBase, Int64 *result);
SMILE_API Bool String_ParseReal(const String str, Int numericBase, Real64 *result);

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation (Unicode support)

SMILE_API Int String_CompareRangeI(const String a, Int astart, Int alength, const String b, Int bstart, Int blength, Bool *usedSlowConversion);
SMILE_API Int String_IndexOfI(const String str, const String pattern, Int start);
SMILE_API Int String_LastIndexOfI(const String str, const String pattern, Int start);
SMILE_API Bool String_ContainsI(const String str, const String pattern);
SMILE_API Bool String_StartsWithI(const String str, const String pattern);
SMILE_API Bool String_EndsWithI(const String str, const String pattern);

SMILE_API String String_ToLowerRange(const String str, Int start, Int length);
SMILE_API String String_ToTitleRange(const String str, Int start, Int length);
SMILE_API String String_ToUpperRange(const String str, Int start, Int length);
SMILE_API String String_CaseFoldRange(const String str, Int start, Int length);
SMILE_API String String_DecomposeRange(const String str, Int start, Int length);
SMILE_API String String_ComposeRange(const String str, Int start, Int length);
SMILE_API String String_NormalizeRange(const String str, Int start, Int length);

SMILE_API Int32 String_ExtractUnicodeCharacter(const String str, Int *index);
SMILE_API Int32 String_ExtractUnicodeCharacterInternal(const Byte **start, const Byte *end);

SMILE_API String String_ConvertUtf8ToCodePageRange(const String str, Int start, Int length, const Byte **utf8ToCodePageTables, Int numTables);
SMILE_API String String_ConvertCodePageToUtf8Range(const String str, Int start, Int length, const UInt16 *codePageToUtf8Table);

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

SMILE_API String String_ConvertUtf8ToKnownCodePageRange(const String str, Int start, Int length, Int legacyCodePageID);
SMILE_API String String_ConvertKnownCodePageToUtf8Range(const String str, Int start, Int length, Int legacyCodePageID);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

/// <summary>
/// Declare a static string, preallocated in const (readonly) memory, rather than on the heap.
/// </summary>
/// <param name="__name__">The name of the static string instance to declare.</param>
/// <param name="__text__">A C-style string that contains the static text.</param>
/// <param name="__textLength__">The number of bytes in the C-style string, not including the terminating nul character.</param>
#define EXTERN_STATIC_STRING(__name__, __text__) \
	static struct StringInt __name__##Struct = { (Byte *)(__text__), (sizeof(__text__) - 1) }; \
	String __name__ = (String)(&__name__##Struct)

/// <summary>
/// Declare a private (i.e., not accessible outside this source file) static string,
/// preallocated in const (readonly) memory, rather than on the heap.
/// </summary>
/// <param name="__name__">The name of the static string instance to declare.</param>
/// <param name="__text__">A C-style string that contains the static text.</param>
/// <param name="__textLength__">The number of bytes in the C-style string, not including the terminating nul character.</param>
#define STATIC_STRING(__name__, __text__) \
	static struct StringInt __name__##Struct = { (Byte *)(__text__), (sizeof(__text__) - 1) }; \
	static String __name__ = (String)(&__name__##Struct)

/// <summary>
/// Retrieve a byte from the string at the given index.  The index must
/// be valid, or you may read past the end of the string.
/// </summary>
/// <param name="str">The string to read one byte from.</param>
/// <param name="index">The index within that string of the byte to read.</param>
/// <returns>The byte at the given index.</returns>
Inline Byte String_At(const String str, Int index)
{
	return ((struct StringInt *)str)->text[index];
}

/// <summary>
/// Retrieve a pointer to the underlying byte array in the string.  Note that strings
/// are immutable, so you should *not* change any data found at this pointer or you
/// risk dangerous side-effects!  The bytes will be nul-terminated (a C-compatible string),
/// but may legally contain nul (zero) values within them; you should use String_Length()
/// to get the actual length of the string.
/// </summary>
/// <param name="str">The string to obtain the raw bytes of.</param>
/// <returns>The raw bytes of the string.</returns>
Inline const Byte *String_GetBytes(const String str)
{
	return ((struct StringInt *)str)->text;
}

/// <summary>
/// Retrieve a pointer to the underlying byte array in the string.  Note that strings
/// are immutable, so you should *not* change any data found at this pointer or you
/// risk dangerous side-effects!  The bytes will be nul-terminated (a C-compatible string),
/// but may legally contain nul (zero) values within them; you should use String_Length()
/// to get the actual length of the string.
/// </summary>
/// <param name="str">The string to obtain the raw bytes of.</param>
/// <returns>The raw bytes of the string.</returns>
Inline const char *String_ToC(const String str)
{
	return (const char *)((struct StringInt *)str)->text;
}

/// <summary>
/// Get the length of the given string, in bytes.
/// </summary>
/// <param name="str">The string to obtain the length of.</param>
/// <returns>The length of that string.</returns>
Inline Int String_Length(const String str)
{
	return ((const struct StringInt *)str)->length;
}

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
	return str == NULL || ((const struct StringInt *)str)->length == 0;
}

/// <summary>
/// Compute a 32-bit hash code for the given string, based on its bytes.
/// </summary>
/// <param name="str">The string to hash.</param>
/// <returns>A reasonably-unique hash value for that string.</returns>
Inline UInt32 String_Hash(const String str)
{
	return Smile_Hash(((const struct StringInt *)str)->text, ((const struct StringInt *)str)->length);
}

/// <summary>
/// Compute a 64-bit hash code for the given string, based on its bytes.
/// </summary>
/// <param name="str">The string to hash.</param>
/// <returns>A reasonably-unique hash value for that string.</returns>
Inline UInt64 String_Hash64(const String str)
{
	return Smile_Hash64(((const struct StringInt *)str)->text, ((const struct StringInt *)str)->length);
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
	return str != NULL ? String_ToLowerRange(str, 0, ((const struct StringInt *)str)->length) : (String )str;
}

/// <summary>
/// Convert a string to titlecase, if it is not already titlecase.
/// </summary>
/// <param name="str">The string you would like to convert.</param>
/// <returns>The whole string converted to titlecase.</returns>
Inline String String_ToTitle(const String str)
{
	return str != NULL ? String_ToTitleRange(str, 0, ((const struct StringInt *)str)->length) : (String )str;
}

/// <summary>
/// Convert a string to uppercase, if it is not already uppercase.
/// </summary>
/// <param name="str">The string you would like to convert.</param>
/// <returns>The whole string converted to uppercase.</returns>
Inline String String_ToUpper(const String str)
{
	return str != NULL ? String_ToUpperRange(str, 0, ((const struct StringInt *)str)->length) : (String )str;
}

/// <summary>
/// Case-fold a given string so that its characters are suitable for case-insensitive comparison.
/// </summary>
/// <param name="str">The string you would like to convert.</param>
/// <returns>The whole string, case-folded.</returns>
Inline String String_CaseFold(const String str)
{
	return str != NULL ? String_CaseFoldRange(str, 0, ((const struct StringInt *)str)->length) : (String )str;
}

/// <summary>
/// Decompose all composed characters within a given string, so that all combining diacritics and
/// compound characters are exposed as separate Unicode code points.
/// </summary>
/// <param name="str">The string you would like to decompose.</param>
/// <returns>The whole string, decomposed.</returns>
Inline String String_Decompose(const String str)
{
	return str != NULL ? String_DecomposeRange(str, 0, ((const struct StringInt *)str)->length) : (String )str;
}

/// <summary>
/// Compose all decomposed characters within a given string, so that all combining diacritics and
/// compound characters are joined to result in as few Unicode code points as possible.
/// </summary>
/// <param name="str">The string you would like to compose.</param>
/// <returns>The whole string, composed.</returns>
Inline String String_Compose(const String str)
{
	return str != NULL ? String_ComposeRange(str, 0, ((const struct StringInt *)str)->length) : (String )str;
}

/// <summary>
/// Normalize a string, so that its combining characters are in canonical order.
/// </summary>
/// <param name="str">The string you would like to normalize.</param>
/// <returns>The whole string, normalized.</returns>
Inline String String_Normalize(const String str)
{
	return str != NULL ? String_NormalizeRange(str, 0, ((const struct StringInt *)str)->length) : (String )str;
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
	return str != NULL ? String_ConvertUtf8ToCodePageRange(str, 0, ((const struct StringInt *)str)->length, utf8ToCodePageTables, numTables) : (String )str;
}

/// <summary>
/// Convert a string from a specific code-page encoding to standard UTF-8.
/// </summary>
/// <param name="str">The string you would like to convert to UTF-8.</param>
/// <param name="codePageToUtf8Table">A 256-entry table describing the resulting Unicode code point for each byte in the string.</param>
/// <returns>The whole string, converted to UTF-8.</returns>
Inline String String_ConvertCodePageToUtf8(const String str, const UInt16 *codePageToUtf8Table)
{
	return str != NULL ? String_ConvertCodePageToUtf8Range(str, 0, ((const struct StringInt *)str)->length, codePageToUtf8Table) : (String )str;
}

/// <summary>
/// Convert a string from UTF-8 encoding to that described by the given well-known legacy code page.
/// </summary>
/// <param name="str">The string you would like to convert to a code page.</param>
/// <param name="legacyCodePageID">The ID of the known legacy code-page.  If the code page is unknown, this will return the empty string.</param>
/// <returns>The whole string, converted to that code page.</returns>
Inline String String_ConvertUtf8ToKnownCodePage(const String str, Int legacyCodePageID)
{
	return str != NULL ? String_ConvertUtf8ToKnownCodePageRange(str, 0, ((const struct StringInt *)str)->length, legacyCodePageID) : (String )str;
}

/// <summary>
/// Convert a string from a specific well-known legacy code-page encoding to standard UTF-8.
/// </summary>
/// <param name="str">The string you would like to convert to UTF-8.</param>
/// <param name="legacyCodePageID">The ID of the known legacy code-page.  If the code page is unknown, this will return the empty string.</param>
/// <returns>The whole string, converted to UTF-8.</returns>
Inline String String_ConvertKnownCodePageToUtf8(const String str, Int legacyCodePageID)
{
	return str != NULL ? String_ConvertKnownCodePageToUtf8Range(str, 0, ((const struct StringInt *)str)->length, legacyCodePageID) : (String )str;
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
	return String_CompareRangeI(a, 0, ((const struct StringInt *)a)->length, b, 0, ((const struct StringInt *)b)->length, &usedSlowConversion);
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

#endif