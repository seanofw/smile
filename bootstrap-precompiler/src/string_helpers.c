
#include "stdafx.h"
#include "string_helpers.h"

//-----------------------------------------------------------------------------
// Smile Symbols --> C Identifiers

static Byte _hexChars[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};

String StringToCIdentifier(String text)
{
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	const Byte *src = String_GetBytes(text);
	Int len = String_Length(text);
	const Byte *end = src + len;
	const Byte *start;
	Byte ch;
	Byte tempbuf[3];

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	for (; src < end; src++) {

		// Consume characters that are valid C identifier characters, except for '_'.
		start = src;
		while (src < end && (
			(ch = *src) >= 'a' && ch <= 'z'
			|| ch >= 'A' && ch <= 'Z'
			|| ch >= '0' && ch <= '9'
			)) {
			src++;
		}

		// Copy those characters verbatim to the output.
		if (src > start)
			StringBuilder_Append(stringBuilder, start, 0, src - start);

		// If we encountered a non-C-ident character, or '_', emit a '_XX' where 'XX' is two hex bytes.
		if (src < end) {
			ch = *src;
			tempbuf[0] = '_';
			tempbuf[1] = _hexChars[(ch >> 4) & 0xF];
			tempbuf[2] = _hexChars[(ch) & 0xF];
			StringBuilder_Append(stringBuilder, tempbuf, 0, 3);
		}
	}

	return StringBuilder_ToString(stringBuilder);
}

String SymbolToCIdentifier(Symbol symbol)
{
	String symbolString = SymbolTable_GetName(Smile_SymbolTable, symbol);
	return StringToCIdentifier(symbolString);
}
