
#ifndef __SMILE_PARSING_IDENTKIND_H__
#define __SMILE_PARSING_IDENTKIND_H__

// Identifier flags structure:
//
//  0x000F: Kind of character.
//  0xFFF0: Character set, if this is a letter.

// Identifier character kinds:
//
//   0: Disallowed in identifiers.
//   1: Letter-name-style start character.
//   2: Letter-name-style middle/trailing character.
//   3: (illegal)
//   4: Punctuative-name-style character.
//   5: (illegal)
//   6: Either an letter-name-style middle/trailing character, or a punctuative-name-style character.
//   7: (illegal)
//   8..15: (reserved)

#define IDENTKIND_STARTLETTER		(1 << 0)
#define IDENTKIND_MIDDLELETTER		(1 << 1)
#define IDENTKIND_PUNCTUATION		(1 << 2)

// Letter character sets:
#define IDENTKIND_CHARSET_MASK		(0xFFF << 4)
#define IDENTKIND_CHARSET_ANY		(0)
#define IDENTKIND_CHARSET_LATIN		(1 << 4)
#define IDENTKIND_CHARSET_GREEK		(2 << 4)
#define IDENTKIND_CHARSET_CYRILLIC	(3 << 4)
#define IDENTKIND_CHARSET_ARMENIAN	(4 << 4)
#define IDENTKIND_CHARSET_HEBREW	(5 << 4)

// ...More to come, as people who speak those languages provide suitable identifier rules for them...

extern const UInt16 *SmileIdentifierTable[];
extern const UInt32 SmileIdentifierTableLength;

/// <summary>
/// Determine what kind of identifier the given character is.
/// </summary>
/// <param name="ch">The character to look up.</param>
/// <returns>The identifier-type of that character.</returns>
Inline UInt SmileIdentifierKind(UInt ch)
{
	UInt codePage = ch >> 8;
	if (codePage >= SmileIdentifierTableLength)
		return 0;
	return SmileIdentifierTable[codePage][ch & 0xFF];
}

#endif
