#include <smile/regex.h>
#include <smile/dict/int32dict.h>
#include <smile/dict/stringintdict.h>
#include <smile/dict/stringdict.h>
#include <smile/internal/staticstring.h>
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

	OnigOptionType options = ONIG_OPTION_NONE | ONIG_OPTION_SINGLELINE;
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
			case 'i':
				options |= ONIG_OPTION_IGNORECASE;
				break;
			case 'm':
				options |= ONIG_OPTION_MULTILINE | ONIG_OPTION_NEGATE_SINGLELINE;
				break;
			case 'a':
				options |= ONIG_OPTION_WORD_IS_ASCII
					| ONIG_OPTION_DIGIT_IS_ASCII
					| ONIG_OPTION_POSIX_IS_ASCII
					| ONIG_OPTION_SPACE_IS_ASCII;
				encoding = ONIG_ENCODING_ASCII;
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

Bool Regex_Test(Regex regex, String input)
{
	const Byte *start = String_GetBytes(input);
	const Byte *end = start + String_Length(input);

	RegexCacheNode node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	int matchLength = onig_match(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
		(const OnigUChar *)start, NULL, ONIG_OPTION_NONE);
	return matchLength >= 0;
}

RegexMatch Regex_Match(Regex regex, String input)
{
	const Byte *start;
	const Byte *end;
	OnigRegion *region;
	RegexCacheNode node;
	int matchOffset;
	RegexMatch match;

	start = String_GetBytes(input);
	end = start + String_Length(input);

	node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	region = onig_region_new();
	if (region == NULL)
		Smile_Abort_OutOfMemory();

	matchOffset = onig_search(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
		(const OnigUChar *)start, (const OnigUChar *)end, region, ONIG_OPTION_NONE);

	match = GC_MALLOC_STRUCT(struct RegexMatchStruct);
	if (match == NULL)
		Smile_Abort_OutOfMemory();

	if (matchOffset < 0) {
		match->isMatch = False;
		match->indexedCaptures = NULL;
		match->numIndexedCaptures = 0;
		match->namedCaptures = StringDict_Create();
		match->matchStart = 0;
		match->matchEnd = 0;

		if (matchOffset != ONIG_MISMATCH) {
			OnigUChar errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN + 1];

			onig_error_code_to_str(errorBuf, matchOffset);
			errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN] = '\0';

			match->errorMessage = String_FromC(errorBuf);
		}
	}
	else {
		match->isMatch = True;
		match->matchStart = matchOffset;
		match->matchEnd = matchOffset;
		match->indexedCaptures = NULL;
		match->numIndexedCaptures = 0;
		match->namedCaptures = StringDict_Create();
	}

	onig_region_free(region, 1);

	return match;
}

Int Regex_Replace(Regex regex, String input, String replacement, Int limit)
{
	UNUSED(regex);
	UNUSED(input);
	UNUSED(replacement);
	UNUSED(limit);

	return 0;
}

Int Regex_Split(Regex regex, String input, String **pieces)
{
	UNUSED(regex);
	UNUSED(input);
	UNUSED(pieces);

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
