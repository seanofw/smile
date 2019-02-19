#ifndef __SMILE_REGEX_H__
#define __SMILE_REGEX_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_DICT_STRINGDICT_H__
#include <smile/dict/stringdict.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

//-------------------------------------------------------------------------------------------------
//  Object declarations

/// <summary>
/// This represents a (possibly-)compiled regular expression.
/// </summary>
struct RegexStruct {
	Int32 cacheId;					// The ID of the last-cached compiled version of this regular expression.
	String pattern;					// The regex's pattern text, verbatim.
	String flags;					// The flags/option characters, which may be empty for an invalid regex.
};

/// <summary>
/// A range of a regex match: Where it starts in the input string, and how long it is.
/// </summary>
typedef struct RegexMatchRangeStruct {
	Int start;
	Int length;
} *RegexMatchRange;

/// <summary>
/// This is the result of an invocation of Regex_Match(), and is primarily a collection of
/// captures from the last match.
/// </summary>
struct RegexMatchStruct {
	Bool isMatch;								// Matched or not
	String input;								// The matched input string
	String errorMessage;						// Any error message resulting from a failed match
	Int numIndexedCaptures;						// Number of captures in the indexedCaptures array
	Int maxIndexedCaptures;						// Maximum size of the indexedCaptures array
	struct StringIntDictStruct namedCaptures;	// Named captures using (?<name>...) syntax; 'name' --> indexedCapture
	struct RegexMatchRangeStruct indexedCaptures[1];	// 0 is the whole match; 1..n are (capture groups)
};

//-------------------------------------------------------------------------------------------------
//  Object declarations

SMILE_API_FUNC Regex Regex_Create(String pattern, String flags, String *errorMessage);
SMILE_API_FUNC Bool Regex_Test(Regex regex, String input, Int startOffset);
SMILE_API_FUNC RegexMatch Regex_Match(Regex regex, String input, Int startOffset);
SMILE_API_FUNC Int Regex_Replace(Regex regex, String input, String replacement, Int startOffset, Int limit);
SMILE_API_FUNC Int Regex_Split(Regex regex, String input, String **pieces, Bool includeEmpty, Int limit);
SMILE_API_FUNC String Regex_ToString(Regex regex);

//-------------------------------------------------------------------------------------------------
//  Object declarations

/// <summary>
/// Determine if two regexes are the same (same pattern, and same flags).
/// </summary>
Inline Bool Regex_Equal(Regex a, Regex b)
{
	return (a->cacheId == b->cacheId)
		|| (String_Equals(a->pattern, b->pattern) && String_Equals(a->flags, b->flags));
}

/// <summary>
/// Determine if this is a valid regex, or if it's broken/not-compile-able.
/// </summary>
Inline Bool Regex_IsValid(Regex regex)
{
	return regex->cacheId != 0;
}

#endif