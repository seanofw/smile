#include <smile/regex.h>
#include <smile/dict/int32dict.h>
#include <smile/dict/stringintdict.h>
#include <smile/dict/stringdict.h>
#include <smile/internal/staticstring.h>
#include <smile/array.h>
#include <malloc.h>

// The core of the Regex engine is implemented by Oniguruma, because it has
// great natural support for Unicode.
#define ONIG_EXTERN extern
#include "src/oniguruma.h"

//-------------------------------------------------------------------------------------------------
//  Type declarations and definitions

#define REGEX_CACHE_SIZE 256

typedef struct RegexCacheNodeStruct {
	OnigRegex onigRegex;
	Int32 id;
	String pattern;
	String flags;
	String key;
	Bool isValid;
	String errorMessage;
	Regex regex;
	struct RegexCacheNodeStruct *next, *prev;
} *RegexCacheNode;

//-------------------------------------------------------------------------------------------------
//  Static data for the regex cache.

static Int32 _regexNextId = 1;

static StringIntDict _keyToIdLookup;
static Int32Dict _idToCacheNodeLookup;

static RegexCacheNode _regexCacheHead = NULL, _regexCacheTail = NULL;
static Int _regexCacheSize = 0;

//-------------------------------------------------------------------------------------------------
//  Internal mechanics.

static Bool Regex_Compile(String pattern, String flags, RegexCacheNode node)
{
	OnigRegex result = NULL;
	const Byte *patternText, *patternEnd;
	const Byte *flagsText, *flagsEnd, *flagsPtr;
	Byte actualFlags[16];
	Byte *actualFlagsDest;
	String actualFlagsString;

	OnigOptionType options = ONIG_OPTION_NONE | ONIG_OPTION_CAPTURE_GROUP;
	OnigEncoding encoding;
	OnigErrorInfo errorInfo;
	int onigResult;

	encoding = ONIG_ENCODING_UTF8;

	patternText = String_GetBytes(pattern);
	patternEnd = patternText + String_Length(pattern);
	flagsText = String_GetBytes(flags);
	flagsEnd = flagsText + String_Length(flags);

	for (flagsPtr = flagsText; flagsPtr < flagsEnd; flagsPtr++) {
		switch (*flagsPtr) {
			case 'a':
				options |= ONIG_OPTION_WORD_IS_ASCII
					| ONIG_OPTION_DIGIT_IS_ASCII
					| ONIG_OPTION_POSIX_IS_ASCII
					| ONIG_OPTION_SPACE_IS_ASCII;
				encoding = ONIG_ENCODING_ASCII;
				break;
			case 'i':
				options |= ONIG_OPTION_IGNORECASE;
				break;
			case 'm':
				options |= ONIG_OPTION_MULTILINE;
				break;
			case 's':
				options |= ONIG_OPTION_SINGLELINE;
				break;
			case 'n':
				options &= ~ONIG_OPTION_CAPTURE_GROUP;
				options |= ONIG_OPTION_DONT_CAPTURE_GROUP;
				break;
			case 'x':
				options |= ONIG_OPTION_EXTEND;
				break;
			default:
				node->pattern = pattern;
				node->flags = String_Empty;
				node->isValid = False;
				node->errorMessage = String_Format("Unknown/unsupported Regex option '%c'", *flagsPtr);
				node->onigRegex = NULL;
				return False;
		}
	}

	// Rebuild the actual flags string.
	actualFlagsDest = actualFlags;
	if (encoding == ONIG_ENCODING_ASCII)
		*actualFlagsDest++ = 'a';
	if (options & ONIG_OPTION_IGNORECASE)
		*actualFlagsDest++ = 'i';
	if (options & ONIG_OPTION_MULTILINE)
		*actualFlagsDest++ = 'm';
	if (options & ONIG_OPTION_DONT_CAPTURE_GROUP)
		*actualFlagsDest++ = 'n';
	if (options & ONIG_OPTION_SINGLELINE)
		*actualFlagsDest++ = 's';
	if (options & ONIG_OPTION_EXTEND)
		*actualFlagsDest++ = 'x';
	actualFlagsString = String_Create(actualFlags, actualFlagsDest - actualFlags);

	onigResult = onig_new(&result, (const OnigUChar *)patternText, (const OnigUChar *)patternEnd,
		options, encoding, ONIG_SYNTAX_PERL_NG, &errorInfo);

	if (onigResult == ONIG_NORMAL) {
		node->pattern = pattern;
		node->flags = actualFlagsString;
		node->isValid = True;
		node->errorMessage = NULL;
		node->onigRegex = result;
	}
	else {
		OnigUChar errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN + 1];

		onig_error_code_to_str(errorBuf, onigResult, &errorInfo);
		errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN] = '\0';

		node->pattern = pattern;
		node->flags = actualFlagsString;
		node->isValid = False;
		node->errorMessage = String_FromC(errorBuf);
		node->onigRegex = NULL;
	}

	return node->isValid;
}

static void RegexCache_Detach(RegexCacheNode node)
{
	if (node->next != NULL)
		node->next->prev = node->prev;
	else
		_regexCacheTail = node->prev;

	if (node->prev != NULL)
		node->prev->next = node->next;
	else
		_regexCacheHead = node->next;

	_regexCacheSize--;
}

static void RegexCache_AttachAtHead(RegexCacheNode node)
{
	node->next = _regexCacheHead;
	node->prev = NULL;

	if (node->next != NULL)
		node->next->prev = node;
	else
		_regexCacheTail = node;

	_regexCacheSize++;
}

static RegexCacheNode RegexCache_GetCacheNodeById(Int32 id)
{
	RegexCacheNode node = (RegexCacheNode)Int32Dict_GetValue(_idToCacheNodeLookup, id);
	return node;
}

static Int32 RegexCache_GetIdByKey(String key)
{
	Int32 id = (Int32)StringIntDict_GetValue(_keyToIdLookup, key);
	return id;
}

static RegexCacheNode RegexCache_AddCacheNode(String key)
{
	Int32 newId;
	RegexCacheNode node;

	newId = _regexNextId++;

	node = (RegexCacheNode)malloc(sizeof(struct RegexCacheNodeStruct));
	if (node == NULL)
		Smile_Abort_OutOfMemory();
	node->flags = NULL;
	node->pattern = NULL;
	node->key = key;
	node->id = newId;
	node->next = NULL;
	node->prev = NULL;
	node->onigRegex = NULL;
	node->isValid = False;
	node->errorMessage = NULL;
	node->regex = NULL;

	Int32Dict_Add(_idToCacheNodeLookup, newId, node);
	StringIntDict_Add(_keyToIdLookup, key, newId);
	RegexCache_AttachAtHead(node);

	return node;
}

static void RegexCache_EvictOldest(Int limit)
{
	while (_regexCacheSize > limit) {
		RegexCacheNode deletableNode = _regexCacheTail;
		RegexCache_Detach(deletableNode);

		if (deletableNode->onigRegex != NULL) {
			onig_free(deletableNode->onigRegex);
			free(deletableNode);
		}
	}
}

static RegexCacheNode RegexCache_FindOrAdd(Int32 id, String pattern, String flags)
{
	Regex regex;
	RegexCacheNode node;
	String pieces[3];
	String key;
	STATIC_STRING(slash, "/");

	// First, just see if the ID gives us an exact match to a cached instance.
	node = RegexCache_GetCacheNodeById(id);
	if (node != NULL) {
		RegexCache_Detach(node);
		RegexCache_AttachAtHead(node);
		return node;
	}

	// Didn't find it by ID, so see if another instance coincidentally has the same pattern.
	pieces[0] = flags;
	pieces[1] = slash;
	pieces[2] = pattern;

	key = String_ConcatMany(pieces, 3);
	id = RegexCache_GetIdByKey(key);

	if (id != 0) {
		// Another instance coincidentally has the same pattern, so use it.
		node = RegexCache_GetCacheNodeById(id);
		RegexCache_Detach(node);
		RegexCache_AttachAtHead(node);
		return node;
	}

	// Nothing has that pattern, so we need to add this to the cache, and maybe
	// evict old regex instance(s).
	node = RegexCache_AddCacheNode(key);
	Regex_Compile(pattern, flags, node);

	regex = GC_MALLOC_STRUCT(struct RegexStruct);
	if (regex == NULL)
		Smile_Abort_OutOfMemory();
	regex->cacheId = node->id;
	regex->pattern = pattern;
	regex->flags = flags;
	node->regex = regex;

	RegexCache_EvictOldest(REGEX_CACHE_SIZE);

	return node;
}

typedef struct NameCallbackInfoStruct {
	OnigRegion *region;
	RegexMatch match;
} *NameCallbackInfo;

static int Regex_NameCallback(const UChar *name, const UChar *nameEnd, int nGroupNum, int *groupNums, regex_t *regex, void *arg)
{
	NameCallbackInfo info = (NameCallbackInfo)arg;
	RegexMatch match = info->match;
	OnigRegion *region = info->region;
	String nameString;
	Int ref;

	UNUSED(nGroupNum);
	UNUSED(groupNums);

	ref = (Int)onig_name_to_backref_number(regex, name, nameEnd, region);
	nameString = String_Create((const Byte *)name, nameEnd - name);

	// Apply proper "multi-match" semantics:  We keep the first instance of each name
	// that actually matched something; if a name didn't match anything, we don't keep
	// the empty match, but we also ensure that each name in the regex always maps to a key
	// in the output dictionary (empty string if nothing else matched).
	if (!StringIntDict_Add(&match->namedCaptures, nameString, ref)) {

		// Something already exists.  Was it meaningful?
		Int lastRef = StringIntDict_GetValue(&match->namedCaptures, nameString);
		Bool lastHadContent = (region->beg[lastRef] != region->end[lastRef]);
		Bool thisHasContent = (region->beg[ref] != region->end[ref]);

		// If it didn't have meaningful content before, but we have meaningful content now,
		// replace the old answer with a better one.
		if (!lastHadContent && thisHasContent)
			StringIntDict_SetValue(&match->namedCaptures, nameString, ref);
	}

	return 0;  // 0: Continue iterating through names
}

//-------------------------------------------------------------------------------------------------
//  Semi-public interface.

void Regex_Init(void)
{
	OnigEncoding encodings[1];
	encodings[0] = ONIG_ENCODING_UTF8;

	onig_initialize(encodings, 1);

	_keyToIdLookup = StringIntDict_Create();
	_idToCacheNodeLookup = Int32Dict_Create();
}

void Regex_End(void)
{
	_idToCacheNodeLookup = NULL;
	_keyToIdLookup = NULL;

	onig_end();
}

//-------------------------------------------------------------------------------------------------
//  Public interface.

Regex Regex_Create(String pattern, String flags, String *errorMessage)
{
	RegexCacheNode node = RegexCache_FindOrAdd(0, pattern, flags);
	if (errorMessage != NULL)
		*errorMessage = node->errorMessage;
	return node->regex;
}

Bool Regex_Test(Regex regex, String input, Int startOffset)
{
	const Byte *start = String_GetBytes(input);
	Int length = String_Length(input);
	const Byte *end = start + length;

	RegexCacheNode node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	if (startOffset < 0 || startOffset >= length)
		return False;

	int matchLength = onig_match(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
		(const OnigUChar *)(start + startOffset), NULL, ONIG_OPTION_NONE);
	return matchLength >= 0;
}

/// <summary>
/// Match the given regex against the given string, searching forward from the
/// starting offset if necessary.
/// </summary>
/// <param name="regex">The regular expression to match, which may include explicit captures.</param>
/// <param name="input">The input string to match against.</param>
/// <param name="startOffset">Where in the string to start the match (useful for repeated matches).</param>
/// <returns>A RegexMatch object that describes where the match occurred (if it matched), and what
///   content was captured (if anything).</param>
RegexMatch Regex_Match(Regex regex, String input, Int startOffset)
{
	const Byte *start;
	const Byte *end;
	OnigRegion *region;
	RegexCacheNode node;
	int matchOffset;
	RegexMatch match;
	Int length;
	int i, rangeStart, rangeEnd;
	RegexMatchRange range;
	struct NameCallbackInfoStruct nameCallbackInfo;
	OnigUChar errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN + 1];

	// First, get actual pointers to the string content.
	start = String_GetBytes(input);
	length = String_Length(input);
	end = start + length;

	// Go make the real Oniguruma regex instance, if we need to, or find a suitable one
	// that exists in the cache.
	node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	// Make sure the start offset is sane.
	if (startOffset < 0 || startOffset >= length) {
		match = GC_MALLOC_STRUCT(struct RegexMatchStruct);
		if (match == NULL)
			Smile_Abort_OutOfMemory();

		match->isMatch = False;
		match->input = input;
		match->numIndexedCaptures = 0;
		match->maxIndexedCaptures = 0;
		match->errorMessage = String_Format("Start offset at %ld for 'Regex.match' is outside string.", (Int64)startOffset);
		return match;
	}

	// Oniguruma needs somewhere to put its match information, so allocate that (off the malloc heap, not GC).
	region = onig_region_new();
	if (region == NULL)
		Smile_Abort_OutOfMemory();

	// Oniguruma.Go!();
	matchOffset = onig_search(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
		(const OnigUChar *)(start + startOffset), (const OnigUChar *)end, region, ONIG_OPTION_NONE);

	// If we failed to match, abort and return that fact, and possibly include an error message if
	// something is wrong with the regex.
	if (matchOffset < 0) {
		match = GC_MALLOC_STRUCT(struct RegexMatchStruct);
		if (match == NULL)
			Smile_Abort_OutOfMemory();

		match->isMatch = False;
		match->input = input;
		match->numIndexedCaptures = 0;
		match->maxIndexedCaptures = 0;

		if (matchOffset != ONIG_MISMATCH) {
			onig_error_code_to_str(errorBuf, matchOffset);
			errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN] = '\0';
			match->errorMessage = String_FromC(errorBuf);
		}

		onig_region_free(region, 1);
		return match;
	}

	// Allocate enough room for the match information, including its capture ranges, all at once.
	match = (RegexMatch)GC_MALLOC(sizeof(struct RegexMatchStruct) + sizeof(struct RegexMatchRangeStruct) * (region->num_regs - 1));
	if (match == NULL)
		Smile_Abort_OutOfMemory();

	// We have data, so fill in the match with the basics, and allocate space for the capture info.
	// We return the capture info as simply ranges ((start,length) pairs), since we can't be certain
	// anyone will actually *want* the captured results.
	match->isMatch = True;
	match->input = input;
	match->numIndexedCaptures = region->num_regs;
	match->maxIndexedCaptures = region->num_regs;
	match->errorMessage = NULL;

	// Populate the indexed captures array.
	for (range = match->indexedCaptures, i = 0; i < region->num_regs; i++, range++) {
		rangeStart = region->beg[i];
		rangeEnd = region->end[i];
		range->start = rangeStart;
		range->length = rangeEnd - rangeStart;
	}

	// Populate the named captures dictionary, if there are named captures.
	if (onig_number_of_names(node->onigRegex) > 0) {

		// Make the dictionary for real, since we haven't done that yet.
		StringIntDict_ClearWithSize(&match->namedCaptures, 16);

		// Spin over the names and collect their data.
		nameCallbackInfo.match = match;
		nameCallbackInfo.region = region;
		onig_foreach_name(node->onigRegex, Regex_NameCallback, &nameCallbackInfo);
	}

	onig_region_free(region, 1);

	return match;
}

/// <summary>
/// Split the given input string by the given regex, putting the split pieces into 'pieces',
/// and returning how many were produced.
/// </summary>
/// <param name="regex">The regular expression to split the string by, which may include
///   explicit captures that will be included in the output.</param>
/// <param name="input">The input string to split.</param>
/// <param name="pieces">This return parameter will be filled in with the resulting split pieces.</param>
/// <param name="includeEmpty">Whether to include empty string pieces between matching regexes or
///   to discard them.  For example, if the split pattern is /,/ and the input is "1,2,,4", the default
///   behavior is to return ["1" "2" "4"], but if this flag is true, the function will instead return
///   ["1" "2" "" "4"].</param>
/// <param name="limit">A limit for how many times to split the input string.  For example, if
///   the split pattern is /,/ and the input is "1,2,3,4,5" and the limit is 2, this will return
///   ["1" "2" "3,4,5"].  A limit of 0 means "no limit".</param>
/// <returns>The number of pieces generated in the pieces array.</returns>
Int Regex_Split(Regex regex, String input, String **pieces, Bool includeEmpty, Int limit)
{
	const Byte *start;
	const Byte *end;
	OnigRegion *region;
	RegexCacheNode node;
	int matchOffset;
	int lastOffset;
	Int length;
	struct ArrayStruct piecesArray;
	int i, rangeStart, rangeEnd;
	String capturePiece;

	// First, get pointers to the actual string data.
	start = String_GetBytes(input);
	length = String_Length(input);
	end = start + length;

	// Go make the real Oniguruma regex instance, if we need to, or find a suitable one
	// that exists in the cache.
	node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	// We'll be collecting any found string pieces in this automatically-resizing array (which we start
	// at room for 32 pieces, which is enough to avoid resizing for many scenarios where this method gets
	// used, but still small enough that at 128 (32-bit CPU) or 256 (64-bit CPU) bytes we're not wasting
	// a huge amount of memory).
	Array_Init(&piecesArray, sizeof(String), 32, False);

	// Oniguruma needs somewhere to record match positions.
	region = onig_region_new();
	if (region == NULL)
		Smile_Abort_OutOfMemory();

	// Ensure that nonsensical limits equal infinity (or close enough).
	if (limit <= 0) limit = -1;

	// Repeat searching for regex matches until we hit the end of the string or the limit.
	for (lastOffset = 0; lastOffset < length && limit--; lastOffset = region->end[0]) {

		// Search forward from the last offset to find the next pattern match.
		matchOffset = onig_search(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
			(const OnigUChar *)(start + lastOffset), (const OnigUChar *)end, region, ONIG_OPTION_NONE);

		// If there are no subsequent matches, we're done.
		if (matchOffset == ONIG_MISMATCH)
			break;

		// If the regex failed (i.e., it's broken or something), return nothing.
		if (matchOffset < 0) {
			*pieces = NULL;
			onig_region_free(region, 1);
			return 0;
		}

		// If we moved forward, or if they want to keep empty pieces, then
		// add the next string piece to the collection.
		if (matchOffset > lastOffset || includeEmpty) {
			capturePiece = String_Substring(input, lastOffset, matchOffset - lastOffset);
			*(String *)Array_Push(&piecesArray) = capturePiece;
		}

		// Add any explicit captures as string pieces too.
		for (i = 1; i < region->num_regs; i++) {
			rangeStart = region->beg[i];
			rangeEnd = region->end[i];
			capturePiece = String_Substring(input, rangeStart, rangeEnd - rangeStart);
			*(String *)Array_Push(&piecesArray) = capturePiece;
		}
	}

	onig_region_free(region, 1);

	// If there's anything left in the string, or if they want to keep empty pieces, then
	// add the last string piece to the collection.
	if (lastOffset < length || includeEmpty) {
		capturePiece = String_SubstringAt(input, lastOffset);
		*(String *)Array_Push(&piecesArray) = capturePiece;
	}

	// We have useful data split from the string, so return it.
	*pieces = (String *)piecesArray.data;
	return piecesArray.length;
}

Int Regex_Replace(Regex regex, String input, String replacement, Int startOffset, Int limit)
{
	UNUSED(regex);
	UNUSED(input);
	UNUSED(replacement);
	UNUSED(startOffset);
	UNUSED(limit);

	return 0;
}

String Regex_ToString(Regex regex)
{
	STATIC_STRING(hashSlash, "#/");
	STATIC_STRING(slash, "/");
	STATIC_STRING(escapedSlash, "\\/");

	String pieces[4];
	pieces[0] = hashSlash;
	pieces[1] = regex->pattern;
	pieces[2] = slash;
	pieces[3] = regex->flags;

	return String_ConcatMany(pieces, 4);
}
