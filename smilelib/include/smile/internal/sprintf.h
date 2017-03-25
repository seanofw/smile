
#ifndef __SMILE_INTERNAL_SPRINTF_H__
#define __SMILE_INTERNAL_SPRINTF_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_STRINGBUILDER_H__
#include <smile/stringbuilder.h>
#endif

struct SPrintfConfigStruct;

// Type sizes:  "%h..." or "%l..." or neither.
const int TypeSizeNormal = 0;
const int TypeSizeHalf = 1;
const int TypeSizeLong = 2;

// Number prefixes:  "% ..." or "%+..." or neither.
const int NumberPrefixNone = 0;
const int NumberPrefixSpace = 1;
const int NumberPrefixPlus = 2;

// Possible modes:  "%c", "%d", "%s", ...
const int ModeCharacter = 0;
const int ModeString = 1;
const int ModeSignedInteger = 2;
const int ModeUnsignedInteger = 3;
const int ModeOctal = 4;
const int ModeHexLower = 5;
const int ModeHexUpper = 6;
const int ModeFloatScientificLower = 7;
const int ModeFloatScientificUpper = 8;
const int ModeFloatDecimal = 9;
const int ModeFloatShortestLower = 10;
const int ModeFloatShortestUpper = 11;
const int ModePointer = 12;
const int ModeStar = 13;

// A function that knows how to format the provided value to the given StringBuilder using
// the given configuration.
typedef void (*Formatter)(StringBuilder dest, struct SPrintfConfigStruct *config, void *value);

// A function that knows how to get the next value in a sequence of values.
typedef void *(*ValueProvider)(int mode, int typeSize);

// Resulting decoded configuration of a "%..." insert in a format string.
struct SPrintfConfigStruct {
	Int typeSizeKind;
	Int mode;
	Formatter formatter;
	Int numberPrefixKind;
	Int width;
	Int precision;
	Bool haveWidth;
	Bool havePrecision;
	Bool leftJustify;
	Bool includeTypeIdentifier;
	Bool zeroJustify;
};
typedef struct SPrintfConfigStruct SPrintfConfig;

#endif
