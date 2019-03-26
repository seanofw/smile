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

typedef struct RegexCacheNodeStruct *RegexCacheNode;

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

/// <summary>
/// This struct holds the state for an interruptible regex-replace state machine.
/// </summary>
typedef struct RegexReplaceStateStruct {
	Regex regex;
	String input;
	Int startOffset;
	Int limit;

	RegexMatch match;
	Int length;
	const Byte *start, *end;
	struct RegexCacheNodeStruct *node;
	int matchOffset;
	Int matchStart, matchLength;

	StringBuilder stringBuilder;
} *RegexReplaceState;

//-------------------------------------------------------------------------------------------------
//  Object declarations

SMILE_API_FUNC Regex Regex_Create(String pattern, String flags, String *errorMessage);
SMILE_API_FUNC Bool Regex_Test(Regex regex, String input, Int startOffset);
SMILE_API_FUNC RegexMatch Regex_Match(Regex regex, String input, Int startOffset);
SMILE_API_FUNC RegexMatch Regex_MatchHere(Regex regex, String input, Int startOffset);
SMILE_API_FUNC Int Regex_Count(Regex regex, String input, Int startOffset, Int limit);
SMILE_API_FUNC String Regex_Replace(Regex regex, String input, String replacement, Int startOffset, Int limit);
SMILE_API_FUNC Int Regex_Split(Regex regex, String input, String **pieces, Bool includeEmpty, Int limit);
SMILE_API_FUNC String Regex_ToString(Regex regex);
SMILE_API_FUNC Int Regex_GetCaptureNames(Regex regex, String **names);
SMILE_API_FUNC Int Regex_GetCaptureCount(Regex regex);

SMILE_API_FUNC Regex Regex_WithEndAnchor(Regex regex);
SMILE_API_FUNC Regex Regex_WithStartAnchor(Regex regex);
SMILE_API_FUNC Regex Regex_AsCaseInsensitive(Regex regex);

SMILE_API_FUNC RegexReplaceState Regex_BeginReplace(Regex regex, String input, Int startOffset, Int limit);
SMILE_API_FUNC Bool Regex_ReplaceLoopTop(RegexReplaceState state);
SMILE_API_FUNC void Regex_ReplaceLoopBottom(RegexReplaceState state, String replacement);
SMILE_API_FUNC String Regex_EndReplace(RegexReplaceState state);

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
/// Calculate a reasonable hash code for this regex so it can be used in lookup tables.
/// </summary>
Inline UInt32 Regex_Hash(Regex regex)
{
	return (String_Hash(regex->pattern) * 29) + String_Hash(regex->flags);
}

/// <summary>
/// Determine if this is a valid regex, or if it's broken/not-compile-able.
/// </summary>
Inline Bool Regex_IsValid(Regex regex)
{
	return regex->cacheId != 0;
}

/// <summary>
/// Get the text of a numbered capture.
/// </summary>
Inline String RegexMatch_GetCapture(RegexMatch match, Int index)
{
	RegexMatchRange range;
	return index >= 0 && index < match->numIndexedCaptures
		? String_Substring(match->input, (range = &match->indexedCaptures[index])->start, range->length)
		: NULL;
}

/// <summary>
/// Get the text of a named capture.
/// </summary>
Inline String RegexMatch_GetNamedCapture(RegexMatch match, String name)
{
	Int index;
	return StringIntDict_TryGetValue(&match->namedCaptures, name, &index)
		? RegexMatch_GetCapture(match, index)
		: NULL;
}

#endif