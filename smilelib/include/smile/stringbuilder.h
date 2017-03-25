#ifndef __SMILE_STRINGBUILDER_H__
#define __SMILE_STRINGBUILDER_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_MEM_H__
#include <smile/mem.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#ifndef __SMILE_GC_H__
#include <smile/gc.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Internal types

/// <summary>
/// The internal implementation of a StringBuilder.
/// </summary>
struct StringBuilderInt {
	Byte *text;				// A pointer to the actual bytes of the string (nul-terminated).
	Int length;				// The perceived length of the text array (in bytes, not Unicode code points).
	Int max;				// The actual length of the text array.
};

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A StringBuilder, which is a tool for efficiently constructing strings by repeatedly
/// appending text to them.
/// </summary>
typedef struct StringBuilderStruct {
	struct StringBuilderInt _opaque;
} *StringBuilder;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

SMILE_API_FUNC void StringBuilder_InitWithSize(StringBuilder stringBuilder, Int initialSize);
SMILE_API_FUNC StringBuilder StringBuilder_CreateFromBytes(const Byte *text, Int start, Int length);

SMILE_API_FUNC void StringBuilder_Append(StringBuilder stringBuilder, const Byte *text, Int start, Int length);
SMILE_API_FUNC void StringBuilder_AppendByte(StringBuilder stringBuilder, Byte ch);
SMILE_API_FUNC void StringBuilder_AppendRepeat(StringBuilder stringBuilder, Byte ch, Int length);
SMILE_API_FUNC void StringBuilder_AppendUnicode(StringBuilder stringBuilder, UInt32 value);

SMILE_API_FUNC void StringBuilder_AppendFormat(StringBuilder stringBuilder, const char *format, ...);
SMILE_API_FUNC void StringBuilder_AppendFormatv(StringBuilder stringBuilder, const char *format, va_list v);
SMILE_API_FUNC void StringBuilder_AppendFormatString(StringBuilder stringBuilder, const String format, ...);
SMILE_API_FUNC void StringBuilder_AppendFormatStringv(StringBuilder stringBuilder, const String format, va_list v);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation

#define DECLARE_INLINE_STRINGBUILDER(__name__, __size__) \
	Byte __name__##TempBuf[(__size__)]; \
	struct StringBuilderInt __name__##Int; \
	StringBuilder __name__

#define INIT_INLINE_STRINGBUILDER(__name__) \
	(__name__##Int.text = __name__##TempBuf, \
	 __name__##Int.length = 0, \
	 __name__##Int.max = sizeof(__name__##TempBuf), \
	 __name__##TempBuf[0] = '\0', \
	 __name__ = (StringBuilder)&(__name__##Int))

Inline StringBuilder StringBuilder_CreateWithSize(Int initialSize)
{
	struct StringBuilderInt *sb = GC_MALLOC_STRUCT(struct StringBuilderInt);
	if (sb == NULL) Smile_Abort_OutOfMemory();
	StringBuilder_InitWithSize((StringBuilder)sb, initialSize);
	return (StringBuilder)sb;
}

Inline StringBuilder StringBuilder_Create(void)
{
	return StringBuilder_CreateWithSize(256);
}

Inline void StringBuilder_Init(StringBuilder stringBuilder)
{
	StringBuilder_InitWithSize(stringBuilder, 256);
}

Inline StringBuilder StringBuilder_CreateFromString(const String str)
{
	const struct StringInt *s = (const struct StringInt *)str;
	return StringBuilder_CreateFromBytes(s->text, 0, s->length);
}

Inline StringBuilder StringBuilder_CreateFromSubstring(const String str, Int start, Int length)
{
	const struct StringInt *s = (const struct StringInt *)str;
	return StringBuilder_CreateFromBytes(s->text, start, length);
}

Inline String StringBuilder_ToString(const StringBuilder stringBuilder)
{
	const struct StringBuilderInt *sb = (const struct StringBuilderInt *)stringBuilder;
	return String_Create(sb->text, sb->length);
}

Inline void StringBuilder_SetLength(StringBuilder stringBuilder, Int length)
{
	struct StringBuilderInt *sb = (struct StringBuilderInt *)stringBuilder;

	if (length < sb->length)
	{
		sb->length = length;
		sb->text[length] = '\0';
	}
	else if (length > sb->length)
	{
		StringBuilder_AppendRepeat(stringBuilder, '\0', length - sb->length);
	}
}

Inline const Byte *StringBuilder_GetBytes(const StringBuilder stringBuilder)
{
	return ((const struct StringBuilderInt *)stringBuilder)->text;
}

Inline Int StringBuilder_GetLength(const StringBuilder stringBuilder)
{
	return ((const struct StringBuilderInt *)stringBuilder)->length;
}

Inline void StringBuilder_Clear(StringBuilder stringBuilder)
{
	StringBuilder_SetLength(stringBuilder, 0);
}

#define StringBuilder_AppendC(__stringBuilder__, __text__, __start__, __length__) \
	(StringBuilder_Append((__stringBuilder__), ((const Byte *)(__text__)), (__start__), (__length__)))

Inline void StringBuilder_AppendString(StringBuilder stringBuilder, const String str)
{
	const struct StringInt *s = (const struct StringInt *)str;
	StringBuilder_Append(stringBuilder, s->text, 0, s->length);
}

Inline void StringBuilder_AppendSubstring(StringBuilder stringBuilder, const String str, Int start, Int length)
{
	const struct StringInt *s = (const struct StringInt *)str;
	StringBuilder_Append(stringBuilder, s->text, start, length);
}

#endif