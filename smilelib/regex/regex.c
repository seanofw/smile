#include <smile/regex.h>
#include <smile/dict/int32dict.h>
#include <smile/dict/stringintdict.h>
#include <smile/dict/stringdict.h>
#include <smile/internal/staticstring.h>
#include <smile/stringbuilder.h>
#include <smile/array.h>

// The core of the Regex engine is implemented by Oniguruma, because it has
// great natural support for Unicode.
#define ONIG_EXTERN extern
#include "src/oniguruma.h"

static OnigSyntaxType OnigSyntaxSmile;

//-------------------------------------------------------------------------------------------------
//  Type declarations and definitions

// The cache will hold up to 256 recently-used regexes.
#define REGEX_CACHE_SIZE 256

/// <summary>
/// Every node in the regex cache will be of this form.
/// </summary>
struct RegexCacheNodeStruct {
	Int32 id;				// Unique non-repeating ID
	String pattern;			// The original regex pattern string
	String flags;			// Flags/options, like 'i' for case-insensitive
	String key;				// The cache key, which is exactly equal to "{flags}/{pattern}"
	Bool isValid;			// Whether this regex was compiled without errors
	String errorMessage;	// If this was compiled with errors, this is the error message
	OnigRegex onigRegex;	// The Oniguruma regex_t object itself
	Regex regex;			// A Smile Regex object that references this cache node

	struct RegexCacheNodeStruct *next, *prev;	// Next/previous in the LRU cache
};

//-------------------------------------------------------------------------------------------------
//  Static data for the regex cache.

// The next unique ID to use for a new regex cache entry.
static Int32 _regexNextId = 1;

// A dictionary that maps known keys ("{flags}/{pattern}") to non-deleted unique cache IDs.
// We keep this around because some Smile Regex objects may coincidentally match each other,
// so this will (should) cause their IDs (and thus cache nodes) to collapse into single
// instances.
static StringIntDict _keyToIdLookup;

// A dictionary that maps cache IDs to non-deleted RegexCacheNode instances.  Smile Regex
// objects reference cache entries by ID, which is a weak reference:  Cached entries may be
// evicted while Smile Regex objects still reference them, and the cached entries will be
// recreated the next time the Smile Regex object is used for pattern-matching.
static Int32Dict _idToCacheNodeLookup;

// A doubly-linked list that represents all non-deleted RegexCacheNodes.
static RegexCacheNode _regexCacheHead = NULL, _regexCacheTail = NULL;
static Int _regexCacheSize = 0;

//-------------------------------------------------------------------------------------------------
//  Internal mechanics.

Inline String Regex_ParseFlags(String flags, String *actualFlagsString, OnigEncoding *encoding, OnigOptionType *options)
{
	const Byte *flagsText, *flagsEnd, *flagsPtr;
	Byte actualFlags[16];
	Byte *actualFlagsDest;
	
	*options = ONIG_OPTION_NONE | ONIG_OPTION_CAPTURE_GROUP;
	*encoding = ONIG_ENCODING_UTF8;

	flagsText = String_GetBytes(flags);
	flagsEnd = flagsText + String_Length(flags);

	for (flagsPtr = flagsText; flagsPtr < flagsEnd; flagsPtr++) {
		switch (*flagsPtr) {
			case 'a':
				*options |= ONIG_OPTION_WORD_IS_ASCII
					| ONIG_OPTION_DIGIT_IS_ASCII
					| ONIG_OPTION_POSIX_IS_ASCII
					| ONIG_OPTION_SPACE_IS_ASCII;
				*encoding = ONIG_ENCODING_ASCII;
				break;
			case 'i':
				*options |= ONIG_OPTION_IGNORECASE;
				break;
			case 'm':
				*options |= ONIG_OPTION_MULTILINE;
				break;
			case 'n':
				*options &= ~ONIG_OPTION_CAPTURE_GROUP;
				*options |= ONIG_OPTION_DONT_CAPTURE_GROUP;
				break;
			case 's':
				*options |= ONIG_OPTION_SINGLELINE;
				break;
			case 'x':
				*options |= ONIG_OPTION_EXTEND;
				break;

			default:
				*actualFlagsString = NULL;
				*options = ONIG_OPTION_NONE | ONIG_OPTION_CAPTURE_GROUP;
				*encoding = ONIG_ENCODING_UTF8;
				return String_Format("Unknown/unsupported Regex option '%c'", *flagsPtr);
		}
	}

	// Rebuild the actual flags string, with the flags in canonical (alphabetic) order.
	actualFlagsDest = actualFlags;
	if (*encoding == ONIG_ENCODING_ASCII)
		*actualFlagsDest++ = 'a';
	if (*options & ONIG_OPTION_IGNORECASE)
		*actualFlagsDest++ = 'i';
	if (*options & ONIG_OPTION_MULTILINE)
		*actualFlagsDest++ = 'm';
	if (*options & ONIG_OPTION_DONT_CAPTURE_GROUP)
		*actualFlagsDest++ = 'n';
	if (*options & ONIG_OPTION_SINGLELINE)
		*actualFlagsDest++ = 's';
	if (*options & ONIG_OPTION_EXTEND)
		*actualFlagsDest++ = 'x';
	*actualFlagsString = String_Create(actualFlags, actualFlagsDest - actualFlags);

	return NULL;
}

/// <summary>
/// Compile the given regular expression pattern, using the given flags/options, and save the
/// resulting compiled form (along with any errors or warnings) inside the given cache node.
/// </summary>
/// <param name="pattern">The regular expression pattern to compile.</param>
/// <param name="flags">Flags for the pattern, which must match /^[aimnsx]*$/.  The options are:
///   <ul>
///     <li>a - "ASCII."  Match pattern and input as literal bytes, not as UTF-8-encoded Unicode code points.</li>
///     <li>i - "Insensitive."  Match the pattern to the text using case-insensitive comparisons.</li>
///     <li>m - "Multiline."  Like Perl, `^` and `$` match at newlines, not just at the beginning/end of the string.</li>
///     <li>n - "No numeric captures."  `(...)` is just for grouping; if you want to capture, use `(?&lt;name&gt;...)`.</li>
///     <li>s - "Single line."  Like Perl, `.` matches a newline with this flag enabled.</li>
///     <li>x - "eXtended."  Allow free-formatting and whitespace inside the regex pattern.</li>
///   </ul>
/// </param>
/// <returns>True if the regex was successfully compiled, false if there were errors in it.</returns>
static Bool Regex_Compile(String pattern, String flags, RegexCacheNode node)
{
	OnigRegex result = NULL;
	const Byte *patternText, *patternEnd;
	String actualFlagsString, errorMessage;
	OnigOptionType options;
	OnigEncoding encoding;
	OnigErrorInfo errorInfo;
	int onigResult;
	OnigUChar errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN + 1];

	patternText = String_GetBytes(pattern);
	patternEnd = patternText + String_Length(pattern);

	if ((errorMessage = Regex_ParseFlags(flags, &actualFlagsString, &encoding, &options)) != NULL) {
		goto fail;		// Heck yeah I wrote 'goto fail' in new code.  Wanna make something of it?
	}

	if ((onigResult = onig_new(&result, (const OnigUChar *)patternText, (const OnigUChar *)patternEnd,
		options, encoding, &OnigSyntaxSmile, &errorInfo)) != ONIG_NORMAL) {

		onig_error_code_to_str(errorBuf, onigResult, &errorInfo);
		errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN] = '\0';
		errorMessage = String_FromC(errorBuf);
		goto fail;
	}

	node->pattern = pattern;
	node->flags = actualFlagsString;
	node->isValid = True;
	node->errorMessage = NULL;
	node->onigRegex = result;
	return True;

fail:
	node->pattern = pattern;
	node->flags = actualFlagsString;
	node->isValid = False;
	node->errorMessage = errorMessage;
	node->onigRegex = NULL;
	return False;
}

/// <summary>
/// Detach the given node from the LRU cache list.
/// </summary>
/// <param name="node">The node to detach.</param>
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

/// <summary>
/// Attach the given node to the LRU cache list at its head.
/// </summary>
/// <param name="node">The node to attach.</param>
static void RegexCache_AttachAtHead(RegexCacheNode node)
{
	node->next = _regexCacheHead;
	node->prev = NULL;

	if (node->next != NULL)
		node->next->prev = node;
	else
		_regexCacheTail = node;

	_regexCacheHead = node;

	_regexCacheSize++;
}

/// <summary>
/// Bump the given node from its current position in the LRU cache list to the head of the cache.
/// </summary>
/// <param name="node">The node to bump.</param>
static void RegexCache_BumpNode(RegexCacheNode node)
{
	if (node == _regexCacheHead) return;

	RegexCache_Detach(node);
	RegexCache_AttachAtHead(node);
}

/// <summary>
/// Find a node in the cache by its unique ID.
/// </summary>
/// <param name="id">The ID to search for.</param>
/// <returns>The matching cache node, if any, or NULL if none matches.</returns>
static RegexCacheNode RegexCache_GetCacheNodeById(Int32 id)
{
	RegexCacheNode node = (RegexCacheNode)Int32Dict_GetValue(_idToCacheNodeLookup, id);
	return node;
}

/// <summary>
/// Find the ID of the a suitable cache node for this cache key, if any.
/// </summary>
/// <param name="id">The cache key to search for.</param>
/// <returns>The matching ID, if any, or 0 if none matches.</returns>
static Int32 RegexCache_GetIdByKey(String key)
{
	Int32 id = (Int32)StringIntDict_GetValue(_keyToIdLookup, key);
	return id;
}

/// <summary>
/// Add a new cache node for the given key, which must not already exist in the cache.
/// </summary>
/// <param name="key">The cache key to add a node for.</param>
/// <returns>A newly-created cache node, which will also be added to the various lookup
/// tables and cache LRU list.</returns>
static RegexCacheNode RegexCache_AddCacheNode(String key)
{
	Int32 newId;
	RegexCacheNode node;

	newId = _regexNextId++;

	node = GC_MALLOC_STRUCT(struct RegexCacheNodeStruct);
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
	StringIntDict_SetValue(_keyToIdLookup, key, newId);
	RegexCache_AttachAtHead(node);

	return node;
}

/// <summary>
/// Delete a cache node, removing it from the LRU cache list and both the key and ID lookup tables.
/// </summary>
/// <param name="node">The cache node to delete.</param>
static void RegexCache_DestroyCacheNode(RegexCacheNode node)
{
	RegexCache_Detach(node);

	StringIntDict_Remove(_keyToIdLookup, node->key);
	Int32Dict_Remove(_idToCacheNodeLookup, node->id);

	if (node->onigRegex != NULL) {
		onig_free(node->onigRegex);
	}

	// Zorch the node's data to avoid the possibility of false sharing (which may help the GC).
	node->flags = NULL;
	node->pattern = NULL;
	node->key = NULL;
	node->id = 0;
	node->next = NULL;
	node->prev = NULL;
	node->onigRegex = NULL;
	node->isValid = False;
	node->errorMessage = NULL;
	node->regex = NULL;
}

/// <summary>
/// Use the LRU list to evict old compiled regexes from the cache until we have at most `limit` regexes in it.
/// </summary>
/// <param name="limit">The maximum number of regexes that may exist within the cache.</param>
static void RegexCache_EvictOldest(Int limit)
{
	while (_regexCacheSize > limit) {
		RegexCache_DestroyCacheNode(_regexCacheTail);
	}
}

/// <summary>
/// Given a unique regex node ID, or a regex pattern+flags, go find or create a matching regex node,
/// evicting any old nodes that are likely to no longer be used.
/// </summary>
/// <param name="id">The ID to search for.  If this matches a node, that node will be returned,
/// regardless of whether pattern/flags match that node (they are presumed to match).</param>
/// <param name="pattern">The regex pattern to add to the cache.</param>
/// <param name="flags">The compile flags to use for that regex pattern.</param>
/// <returns>A regex cache node containing a compiled regex within it that matches the given ID and
/// pattern and flags.</returns>
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
		RegexCache_BumpNode(node);
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
		RegexCache_BumpNode(node);
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

/// <summary>
/// This struct is passed as 'arg' to the Regex_NameCallback() function during
/// a successful pattern match; it holds the 'region' that contains the captures from Oniguruma,
/// and a RegexMatch object into which those captures will be copied.
/// </summary>
typedef struct NameCallbackInfoStruct {
	OnigRegion *region;
	RegexMatch match;
} *NameCallbackInfo;

/// <summary>
/// During enumeration of the regex's matched names, this is invoked for each name to generate
/// the match data.  It uses a provided OnigRegion to locate the match data for each name, and
/// then adds the result into a provided RegexMatch dictionary.
/// </summary>
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

/// <summary>
/// Initialize the regex subsystem.  This must be called before using any of the regex
/// functions, but it is called automatically by Smile_Init().
/// </summary>
void Regex_Init(void)
{
	OnigEncoding encodings[1];
	encodings[0] = ONIG_ENCODING_UTF8;

	onig_initialize(encodings, 1);

	_keyToIdLookup = StringIntDict_Create();
	_idToCacheNodeLookup = Int32Dict_Create();
}

/// <summary>
/// End the regex subsystem.  This must be called to free any resources the regex subsystem
/// is still using.  It is called automatically by Smile_End().
/// </summary>
void Regex_End(void)
{
	_idToCacheNodeLookup = NULL;
	_keyToIdLookup = NULL;

	onig_end();
}

//-------------------------------------------------------------------------------------------------
//  Public interface for Regex creation.

/// <summary>
/// Construct a new regex that matches the old one, but that includes a start-anchor before it,
/// so that this regex is guaranteed to match only at the start of a string.
/// </summary>
/// <param name="regex">The original regex.</param>
/// <returns>A similar regex that is anchored to the start of the string.</returns>
Regex Regex_WithStartAnchor(Regex regex)
{
	RegexCacheNode node;
	STATIC_STRING(startAnchor, "\\A(");
	STATIC_STRING(endAnchor, ")");
	String pieces[3];

	if (String_StartsWith(regex->pattern, startAnchor))
		return regex;

	pieces[0] = startAnchor;
	pieces[1] = regex->pattern;
	pieces[2] = endAnchor;

	node = RegexCache_FindOrAdd(0, String_ConcatMany(pieces, 3), regex->flags);
	return node->regex;
}

/// <summary>
/// Construct a new regex that matches the old one, but that includes an end-anchor after it,
/// so that this regex is guaranteed to match only at the end of a string.
/// </summary>
/// <param name="regex">The original regex.</param>
/// <returns>A similar regex that is anchored to the end of the string.</returns>
Regex Regex_WithEndAnchor(Regex regex)
{
	RegexCacheNode node;
	STATIC_STRING(startAnchor, "(");
	STATIC_STRING(endAnchor, ")\\z");
	String pieces[3];

	if (String_EndsWith(regex->pattern, endAnchor))
		return regex;

	pieces[0] = startAnchor;
	pieces[1] = regex->pattern;
	pieces[2] = endAnchor;

	node = RegexCache_FindOrAdd(0, String_ConcatMany(pieces, 3), regex->flags);
	return node->regex;
}

/// <summary>
/// Construct a new regex that matches the old one, but that always performs a
/// case-insensitive match.
/// </summary>
/// <param name="regex">The original regex.</param>
/// <returns>A similar regex that is case-insensitive.</returns>
Regex Regex_AsCaseInsensitive(Regex regex)
{
	RegexCacheNode node;
	String flags = regex->flags;

	if (String_IndexOfChar(flags, 'i', 0) >= 0)
		return regex;

	node = RegexCache_FindOrAdd(0, regex->pattern, String_ConcatByte(flags, 'i'));
	return node->regex;
}

/// <summary>
/// Construct a new Regex object from a pattern and its compile flags.
/// <summary>
/// <param name="pattern">The regex pattern to compile.</param>
/// <param name="flags">The flags or options associated with that pattern, like "i" for a case-insensitive match.</param>
/// <param name="errorMessage">If non-NULL, this will be set to any error message generated by compiling the regex.</param>
/// <returns>The compiled, cached Regex object.</returns>
Regex Regex_Create(String pattern, String flags, String *errorMessage)
{
	RegexCacheNode node = RegexCache_FindOrAdd(0, pattern, flags);
	if (errorMessage != NULL)
		*errorMessage = node->errorMessage;
	return node->regex;
}

//-------------------------------------------------------------------------------------------------
//  Basic regex testing.

/// <summary>
/// Test an input string against a regex to see if they match.
/// <summary>
/// <param name="regex">A compiled regex pattern.</param>
/// <param name="input">The input string to test against the regex.</param>
/// <param name="startOffset">An optional starting character position within the string where the
/// search for a match will begin (typically 0).</param>
/// <returns>True if the string at or after the startOffset matches the regex; false if it does not.</returns>
Bool Regex_Test(Regex regex, String input, Int startOffset)
{
	const Byte *start = String_GetBytes(input);
	Int length = String_Length(input);
	const Byte *end = start + length;

	RegexCacheNode node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	if (startOffset < 0 || startOffset >= length)
		return False;

	int matchOffset = onig_search(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
		(const OnigUChar *)(start + startOffset), (const OnigUChar *)end, NULL, ONIG_OPTION_NONE);
	return matchOffset >= 0;
}

//-------------------------------------------------------------------------------------------------
//  Standard regex matching.

/// <summary>
/// Convert an Oniguruma error code to an error message.
/// <summary>
/// <param name="errorCode">An Oniguruma error code.</param>
/// <returns>A matching String for that error code.</returns>
static String Regex_OnigErrorToString(int errorCode)
{
	OnigUChar errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN + 1];

	onig_error_code_to_str(errorBuf, errorCode);
	errorBuf[ONIG_MAX_ERROR_MESSAGE_LEN] = '\0';

	return String_FromC(errorBuf);
}

/// <summary>
/// When a regex pattern match fails, possibly because of an error, this function generates a
/// RegexMatch object in a 'failed' state.
/// </summary>
/// <param name="input">The input text that failed to match the regex.</param>
/// <param name="errorMessage">An optional error message explaining why the input text failed to match.</param>
/// <returns>An otherwise-empty RegexMatch object in a 'failed' state, with the input text and error message inside it.</returns>
static RegexMatch Regex_CreateErrorMatch(String input, String errorMessage)
{
	RegexMatch match = GC_MALLOC_STRUCT(struct RegexMatchStruct);
	if (match == NULL)
		Smile_Abort_OutOfMemory();

	match->isMatch = False;
	match->input = input;
	match->numIndexedCaptures = 0;
	match->maxIndexedCaptures = 0;
	match->errorMessage = errorMessage;

	return match;
}

/// <summary>
/// When a regex pattern match succeeds, this function generates a RegexMatch object that contains
/// all of the captures from the match.
/// </summary>
/// <param name="input">The input text that successfully matched the regex.</param>
/// <param name="onigRegex">The Oniguruma regex that matched.</param>
/// <param name="onigRegion">An Oniguruma region object that contains all of the captures.</param>
/// <returns>A populated RegexMatch object in a 'succeeded' state, with the input text, capture array,
/// and capture dictionary inside it.</returns>
static RegexMatch Regex_CreateMatchFromRegion(String input, OnigRegex onigRegex, OnigRegion *onigRegion)
{
	RegexMatch match;
	struct NameCallbackInfoStruct nameCallbackInfo;
	int i, rangeStart, rangeEnd;
	RegexMatchRange range;

	// Allocate enough room for the match information, including its capture ranges, all at once.
	match = (RegexMatch)GC_MALLOC(sizeof(struct RegexMatchStruct)
		+ sizeof(struct RegexMatchRangeStruct) * (onigRegion->num_regs - 1));
	if (match == NULL)
		Smile_Abort_OutOfMemory();

	// We have data, so fill in the match with the basics, and allocate space for the capture info.
	// We return the capture info as simply ranges ((start,length) pairs), since we can't be certain
	// anyone will actually *want* the captured results.
	match->isMatch = True;
	match->input = input;
	match->numIndexedCaptures = onigRegion->num_regs;
	match->maxIndexedCaptures = onigRegion->num_regs;
	match->errorMessage = NULL;

	// Populate the indexed captures array.
	for (range = match->indexedCaptures, i = 0; i < onigRegion->num_regs; i++, range++) {
		rangeStart = onigRegion->beg[i];
		rangeEnd = onigRegion->end[i];
		range->start = rangeStart;
		range->length = rangeEnd - rangeStart;
	}

	// Populate the named captures dictionary, if there are named captures.
	if (onig_number_of_names(onigRegex) > 0) {

		// Make the dictionary for real, since we haven't done that yet.
		StringIntDict_ClearWithSize(&match->namedCaptures, 16);

		// Spin over the names and collect their data.
		nameCallbackInfo.match = match;
		nameCallbackInfo.region = onigRegion;
		onig_foreach_name(onigRegex, Regex_NameCallback, &nameCallbackInfo);
	}

	return match;
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
	const Byte *start, *end;
	OnigRegion *region;
	RegexCacheNode node;
	int matchOffset;
	Int length;
	RegexMatch match;

	// First, get actual pointers to the string content.
	start = String_GetBytes(input);
	length = String_Length(input);
	end = start + length;

	// Go make the real Oniguruma regex instance, if we need to, or find a suitable one
	// that exists in the cache.
	node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	// Make sure the start offset is sane.
	if (startOffset < 0 || startOffset >= length)
		return Regex_CreateErrorMatch(input,
			String_Format("Start offset at %ld for 'Regex.match' is outside string.", (Int64)startOffset));

	// Oniguruma needs somewhere to put its match information, so allocate that (off the malloc heap, not GC).
	region = onig_region_new();
	if (region == NULL)
		Smile_Abort_OutOfMemory();

	// Oniguruma.Go!();
	matchOffset = onig_search(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
		(const OnigUChar *)(start + startOffset), (const OnigUChar *)end, region, ONIG_OPTION_NONE);

	// Generate the resulting match object, which might be an error if things didn't work.
	match = (matchOffset < 0
		? Regex_CreateErrorMatch(input, matchOffset != ONIG_MISMATCH ? Regex_OnigErrorToString(matchOffset) : NULL)
		: Regex_CreateMatchFromRegion(input, node->onigRegex, region));

	onig_region_free(region, 1);
	return match;
}

//-------------------------------------------------------------------------------------------------
//  Repeated regex application: String-splitting and match-counting.

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

/// <summary>
/// Count the number of times the given regex matches the given input string.  This is a bit like
/// counting the number of items (minus one) returned by Regex_Split(), but faster.
/// </summary>
/// <param name="regex">The regular expression to match.</param>
/// <param name="input">The input string to match it against.</param>
/// <param name="startOffset">Where in the string to start the search.</param>
/// <param name="limit">A limit for how many times to match the input string.  For example, if
///   the split pattern is /,/ and the input is "1,2,3,4,5" and the limit is 2, this will return
///   2, not 4.  A limit of 0 means "no limit".</param>
/// <returns>The number of times the regex matches the given input string.</returns>
Int Regex_Count(Regex regex, String input, Int startOffset, Int limit)
{
	const Byte *start;
	const Byte *end;
	OnigRegion *region;
	RegexCacheNode node;
	int matchOffset;
	int lastOffset;
	Int length;
	Int count;

	// First, get pointers to the actual string data.
	start = String_GetBytes(input);
	length = String_Length(input);
	end = start + length;

	// Make sure the request was sane.
	if (startOffset < 0 || startOffset >= length) return 0;

	// Go make the real Oniguruma regex instance, if we need to, or find a suitable one
	// that exists in the cache.
	node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	// Oniguruma needs somewhere to record match positions.
	region = onig_region_new();
	if (region == NULL)
		Smile_Abort_OutOfMemory();

	// Ensure that nonsensical limits equal infinity (or close enough).
	if (limit <= 0) limit = -1;

	// Repeat searching for regex matches until we hit the end of the string or the limit.
	count = 0;
	for (lastOffset = (int)startOffset; lastOffset < length && limit--; lastOffset = region->end[0]) {

		// Search forward from the last offset to find the next pattern match.
		matchOffset = onig_search(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
			(const OnigUChar *)(start + lastOffset), (const OnigUChar *)end, region, ONIG_OPTION_NONE);

		// If there are no subsequent matches, we're done.
		if (matchOffset == ONIG_MISMATCH)
			break;

		// If the regex failed (i.e., it's broken or something), return nothing.
		if (matchOffset < 0) {
			onig_region_free(region, 1);
			return 0;
		}

		// If we moved forward, then add one to the count.
		if (matchOffset > lastOffset)
			count++;
	}

	onig_region_free(region, 1);

	return count;
}

//-------------------------------------------------------------------------------------------------
//  Regex replacement.

/// <summary>
/// These are the kinds of tokens that can be generated by the replacement-string parser.
/// </summary>
enum {
	REPLACE_TOKEN_LITERAL,			// Literal, plain, verbatim text.
	REPLACE_TOKEN_NUMERIC_CAPTURE,	// A numeric capture like '$0' or '$3' or '$*'.
	REPLACE_TOKEN_NAMED_CAPTURE,	// A named capture like '${foo}'.
	REPLACE_TOKEN_LAST_CAPTURE,		// The special "last" capture, like '$+'.
};

/// <summary>
/// Each token in the replacement string will be extracted as one of these objects, which
/// represents either literal text or a named/numbered capture group.
/// </summary>
typedef struct ReplacementTokenStruct {
	Int tokenKind;		// What kind of token this is, from the REPLACE_TOKEN_* enum above.
	String text;
	Int number;
} *ReplacementToken;

/// <summary>
/// Given a regex match, iterate through the parsed replacement array and substitute in
/// the match's captures, outputting the replaced text to the given StringBuilder.
/// </summary>
/// <param name="stringBuilder">The StringBuilder that will receive the replacement content.</param>
/// <param name="match">The result of the successful pattern match.</param>
/// <param name="replacementTokens">The sequence of replacement tokens generated by parsing the replacement text.</param>
/// <param name="numReplacementTokens">How many replacement tokens are in the replacementTokens sequence.</param>
static void ApplyReplacement(StringBuilder stringBuilder, RegexMatch match,
	ReplacementToken replacementTokens, Int numReplacementTokens)
{
	Int i;
	ReplacementToken replacementToken;
	Int number;
	RegexMatchRange range;

	for (i = 0; i < numReplacementTokens; i++) {
		replacementToken = &replacementTokens[i];

		switch (replacementToken->tokenKind) {
			case REPLACE_TOKEN_LITERAL:
				StringBuilder_AppendString(stringBuilder, replacementToken->text);
				break;
			case REPLACE_TOKEN_NUMERIC_CAPTURE:
				if (replacementToken->number <= match->numIndexedCaptures) {
					number = replacementToken->number;
					goto appendCapture;
				}
				break;
			case REPLACE_TOKEN_NAMED_CAPTURE:
				if (StringIntDict_TryGetValue(&match->namedCaptures, replacementToken->text, &number))
					goto appendCapture;
				break;
			case REPLACE_TOKEN_LAST_CAPTURE:
				if (match->numIndexedCaptures > 1) {
					number = match->numIndexedCaptures - 1;
					goto appendCapture;
				}
				break;
			appendCapture:
				range = &match->indexedCaptures[replacementToken->number];
				StringBuilder_AppendString(stringBuilder, String_Substring(match->input, range->start, range->length));
				break;
		}
	}
}

/// <summary>
/// Determine if the given character is a known C identifier character.  Used when determining
/// the extent of a bare named-capture replacement like '$foo'.
/// </summary>
Inline Bool IsCIdentChar(Byte ch)
{
	return ch >= 'a' && ch <= 'z'
		|| ch >= 'A' && ch <= 'Z'
		|| ch >= '0' && ch <= '9'
		|| ch == '_';
}

/// <summary>
/// Parse the given replacement string into a series of "replacement tokens," each describing
/// whether to emit literal text or a specific named or numeric capture group to the output.
/// </summary>
/// <param name="replacement">The replacement string to parse, which may include "replacement
/// substitutions" that start with a '$' or '\' and are followed by either a capture-group
/// number, a capture-group name, or the special '+' (last capture) or '&' (whole capture)
/// forms.  "\$" and "$$" can be used to emit a literal dollar-sign, and "\\" and "$\" can be
/// used to emit a literal backslash.</param>
/// <param name="replacementTokens">This will be assigned to an array of ReplacementToken
/// structs (inline in the array, not pointers to the structs).</param>
/// <returns>The number of ReplacementToken structs generated.</returns>
static Int ParseReplacement(String replacement, ReplacementToken *replacementTokens)
{
	struct ArrayStruct array;
	const Byte *start, *end, *src;
	const Byte *tokenStart;
	ReplacementToken token;
	Int number;

	Array_Init(&array, sizeof(struct ReplacementTokenStruct), 16, False);

	start = src = String_GetBytes(replacement);
	end = start + String_Length(replacement);

	tokenStart = start;

	while (src < end) {
		if (*src != '$' && *src != '\\') {
			src++;
			continue;
		}

		if (src > tokenStart) {
			token = (ReplacementToken)Array_Push(&array);
			token->tokenKind = REPLACE_TOKEN_LITERAL;
			token->text = String_Substring(replacement, tokenStart - start, src - tokenStart);
		}

		if (++src >= end) break;

		switch (*src) {
			case '$':
				token = (ReplacementToken)Array_Push(&array);
				token->tokenKind = REPLACE_TOKEN_LITERAL;
				token->text = String_Dollar;
				src++;
				break;

			case '\\':
				token = (ReplacementToken)Array_Push(&array);
				token->tokenKind = REPLACE_TOKEN_LITERAL;
				token->text = String_Backslash;
				src++;
				break;

			case '+':
				token = (ReplacementToken)Array_Push(&array);
				token->tokenKind = REPLACE_TOKEN_LAST_CAPTURE;
				src++;
				break;

			case '0':
			case '&':
				token = (ReplacementToken)Array_Push(&array);
				token->tokenKind = REPLACE_TOKEN_NUMERIC_CAPTURE;
				token->number = 0;
				src++;
				break;

			case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (src + 1 < end && src[1] >= '0' && src[1] <= '9') {
					number = (src[0] - '0') * 10 + (src[1] - '0');
					src += 2;
				}
				else {
					number = (src[0] - '0');
					src++;
				}
				token = (ReplacementToken)Array_Push(&array);
				token->tokenKind = REPLACE_TOKEN_NUMERIC_CAPTURE;
				token->number = number;
				break;

			case '{':
				src++;
				if (src < end && *src >= '0' && *src <= '9') {
					number = (*src - '0');
					while (src < end && *src >= '0' && *src <= '9') {
						number *= 10;
						number += (*src++ - '0');
					}
					if (src == end || *src != '}')
						break;
					src++;
					token = (ReplacementToken)Array_Push(&array);
					token->tokenKind = REPLACE_TOKEN_NUMERIC_CAPTURE;
					token->number = number;
				}
				else {
					tokenStart = src;
					while (src < end && IsCIdentChar(*src)) {
						src++;
					}
					if (src == end || *src != '}')
						break;
					token = (ReplacementToken)Array_Push(&array);
					token->tokenKind = REPLACE_TOKEN_NAMED_CAPTURE;
					token->text = String_Substring(replacement, tokenStart - start, src - tokenStart);
					src++;
				}
				break;

			default:
				break;
		}

		tokenStart = src;
	}

	if (src > tokenStart) {
		token = (ReplacementToken)Array_Push(&array);
		token->tokenKind = REPLACE_TOKEN_LITERAL;
		token->text = String_Substring(replacement, tokenStart - start, src - tokenStart);
	}

	*replacementTokens = (ReplacementToken)array.data;
	return array.length;
}

/// <summary>
/// Search through the 'input' string starting at 'startOffset' for matches of the given 'regex',
/// up to 'limit' matches, and replace each match found with the given replacement string, which
/// may contain capture-group substitutions like '$3' and '$foo', generating a new string that
/// contains all of the replaced text.
/// </summary>
/// <param name="regex">The compiled and cached regex to match against the input.</param>
/// <param name="input">The input string to search through.</param>
/// <param name="replacement">What content to use to replace the input text each time the regex matches.</param>
/// <param name="startOffset">A starting character position in the input at which to begin the search.</param>
/// <param name="limit">The maximum number of replacements to perform.  If limit <= 0, there will be no maximum.</param>
/// <returns>The string, with all regex matches within it replaced with the given replacement text.</returns>
String Regex_Replace(Regex regex, String input, String replacement, Int startOffset, Int limit)
{
	RegexMatch match;
	Int length, matchStart, matchLength;
	ReplacementToken replacementTokens;
	Int numReplacementTokens;
	const Byte *start, *end;
	RegexCacheNode node;
	int matchOffset;
	OnigRegion *region;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);

	start = String_GetBytes(input);
	length = String_Length(input);
	end = start + length;

	if (startOffset < 0 || startOffset >= length)
		return input;

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	numReplacementTokens = ParseReplacement(replacement, &replacementTokens);

	if (startOffset > 0) {
		StringBuilder_AppendSubstring(stringBuilder, input, 0, startOffset);
	}

	if (limit <= 0) limit = -1;

	// Go make the real Oniguruma regex instance, if we need to, or find a suitable one
	// that exists in the cache.
	node = RegexCache_FindOrAdd(regex->cacheId, regex->pattern, regex->flags);

	while (startOffset < length && limit--) {
		region = onig_region_new();
		if (region == NULL)
			Smile_Abort_OutOfMemory();

		matchOffset = onig_search(node->onigRegex, (const OnigUChar *)start, (const OnigUChar *)end,
			(const OnigUChar *)(start + startOffset), (const OnigUChar *)end, region, ONIG_OPTION_NONE);

		if (matchOffset < 0) {
			onig_region_free(region, 1);
			break;
		}

		match = Regex_CreateMatchFromRegion(input, node->onigRegex, region);
		onig_region_free(region, 1);

		matchStart = match->indexedCaptures[0].start;
		matchLength = match->indexedCaptures[0].length;

		if (matchStart > startOffset) {
			StringBuilder_AppendSubstring(stringBuilder, input, startOffset, matchStart - startOffset);
		}

		ApplyReplacement(stringBuilder, match, replacementTokens, numReplacementTokens);

		startOffset = matchStart + matchLength;
	}

	if (startOffset < length) {
		StringBuilder_AppendSubstring(stringBuilder, input, startOffset, length - startOffset);
	}

	return StringBuilder_ToString(stringBuilder);
}

//-------------------------------------------------------------------------------------------------
//  Regex replacement as an interruptible state-machine.

/// <summary>
/// Begin searching through the 'input' string starting at 'startOffset' for matches of the given
/// 'regex', up to 'limit' matches, and replace each match found with whatever string is provided
/// by a user function.
/// </summary>
/// <param name="regex">The compiled and cached regex to match against the input.</param>
/// <param name="input">The input string to search through.</param>
/// <param name="startOffset">A starting character position in the input at which to begin the search.</param>
/// <param name="limit">The maximum number of replacements to perform.  If limit <= 0, there will be no maximum.</param>
/// <returns>An object that holds the replacement machine's state for the rest of the replacement process.</returns>
RegexReplaceState Regex_BeginReplace(Regex regex, String input, Int startOffset, Int limit)
{
	RegexReplaceState state = GC_MALLOC_STRUCT(struct RegexReplaceStateStruct);

	state->regex = regex;
	state->input = input;
	state->startOffset = startOffset;
	state->limit = limit;

	state->start = String_GetBytes(state->input);
	state->length = String_Length(state->input);
	state->end = state->start + state->length;

	state->stringBuilder = StringBuilder_Create();

	if (state->startOffset < 0 || state->startOffset >= state->length) {
		state->startOffset = 0;
		state->limit = 0;
		state->node = NULL;
		return state;
	}

	if (state->startOffset > 0) {
		StringBuilder_AppendSubstring(state->stringBuilder, state->input, 0, state->startOffset);
	}

	if (state->limit <= 0) state->limit = -1;

	// Go make the real Oniguruma regex instance, if we need to, or find a suitable one
	// that exists in the cache.
	state->node = RegexCache_FindOrAdd(state->regex->cacheId, state->regex->pattern, state->regex->flags);

	return state;
}

/// <summary>
/// Run the top part of the replacement loop, before the user's replacement function is to be invoked.
/// This must leave each successive regex match in 'state->match'.
/// </summary>
/// <param name="state">The replacement machine's current state.</param>
/// <returns>True if the matching process should continue, or false if it should abort.</returns>
Bool Regex_ReplaceLoopTop(RegexReplaceState state)
{
	OnigRegion *region;

	if (state->startOffset >= state->length || !(state->limit--))
		return False;

	region = onig_region_new();
	if (region == NULL)
		Smile_Abort_OutOfMemory();

	state->matchOffset = onig_search(state->node->onigRegex, (const OnigUChar *)state->start, (const OnigUChar *)state->end,
		(const OnigUChar *)(state->start + state->startOffset), (const OnigUChar *)state->end, region, ONIG_OPTION_NONE);

	if (state->matchOffset < 0) {
		onig_region_free(region, 1);
		return False;
	}

	state->match = Regex_CreateMatchFromRegion(state->input, state->node->onigRegex, region);
	onig_region_free(region, 1);

	state->matchStart = state->match->indexedCaptures[0].start;
	state->matchLength = state->match->indexedCaptures[0].length;

	if (state->matchStart > state->startOffset) {
		StringBuilder_AppendSubstring(state->stringBuilder, state->input, state->startOffset, state->matchStart - state->startOffset);
	}

	return True;
}

/// <summary>
/// Run the bottom part of the replacement loop, after the user function's has been
/// invoked, which will append the user function's output to the internal StringBuilder
/// and then set up the machine for testing the next match.
/// </summary>
/// <param name="state">The replacement machine's current state.</param>
/// <param name="replacement">The text to substitute into the string (likely generated by a user function).</param>
void Regex_ReplaceLoopBottom(RegexReplaceState state, String replacement)
{
	StringBuilder_AppendString(state->stringBuilder, replacement);

	state->startOffset = state->matchStart + state->matchLength;
}

/// <summary>
/// When the machine is done, this completes the replacement process by finishing
/// the string being constructed in the StringBuilder, and then returns the whole
/// generated string.
/// </summary>
/// <param name="state">The replacement machine's final state.</param>
/// <returns>The fully-replaced string.</returns>
String Regex_EndReplace(RegexReplaceState state)
{
	if (state->startOffset < state->length) {
		StringBuilder_AppendSubstring(state->stringBuilder, state->input, state->startOffset, state->length - state->startOffset);
	}

	return StringBuilder_ToString(state->stringBuilder);
}

//-------------------------------------------------------------------------------------------------
//  Regex miscellaneous.

/// <summary>
/// Convert the given regex to its Smile loanword representation, including the initial '#' mark.
/// </summary>
/// <param name="regex">The Regex to convert to a string.</param>
/// <returns>The string-ified Regex.</returns>
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

//-------------------------------------------------------------------------------------------------
//  Syntax configuration.

/// <summary>
/// Oniguruma regex syntax configuration for Smile.  This attempts to mix the best of all the various
/// regex flavors, including features that are common (or should be), and excluding those that are
/// nonstandard (or rarely available, or weird hacks).  The result is similar to a fusion of Ruby's
/// and Perl 5's syntaxes, but with obscure forms and old backward-compatibility forms removed.
/// Notably, this leaves out the POSIX character forms like [[:alpha:]] and \p{alpha}, which are
/// just ridiculous and ugly and should not still exist.
/// </summary>
static OnigSyntaxType OnigSyntaxSmile = {
	(ONIG_SYN_OP_DOT_ANYCHAR
		| ONIG_SYN_OP_ASTERISK_ZERO_INF
		| ONIG_SYN_OP_PLUS_ONE_INF
		| ONIG_SYN_OP_QMARK_ZERO_ONE
		| ONIG_SYN_OP_BRACE_INTERVAL
		| ONIG_SYN_OP_VBAR_ALT
		| ONIG_SYN_OP_LPAREN_SUBEXP
		| ONIG_SYN_OP_ESC_AZ_BUF_ANCHOR
		| ONIG_SYN_OP_DECIMAL_BACKREF
		| ONIG_SYN_OP_BRACKET_CC
		| ONIG_SYN_OP_ESC_W_WORD
		| ONIG_SYN_OP_ESC_B_WORD_BOUND
		| ONIG_SYN_OP_ESC_S_WHITE_SPACE
		| ONIG_SYN_OP_ESC_D_DIGIT
		| ONIG_SYN_OP_LINE_ANCHOR
		| ONIG_SYN_OP_QMARK_NON_GREEDY
		| ONIG_SYN_OP_ESC_CONTROL_CHARS
		| ONIG_SYN_OP_ESC_C_CONTROL
		| ONIG_SYN_OP_ESC_OCTAL3
		| ONIG_SYN_OP_ESC_X_HEX2),
	(ONIG_SYN_OP2_QMARK_GROUP_EFFECT
		| ONIG_SYN_OP2_PLUS_POSSESSIVE_REPEAT
		| ONIG_SYN_OP2_PLUS_POSSESSIVE_INTERVAL
		| ONIG_SYN_OP2_QMARK_LT_NAMED_GROUP
		| ONIG_SYN_OP2_ESC_K_NAMED_BACKREF
		| ONIG_SYN_OP2_ESC_V_VTAB
		| ONIG_SYN_OP2_ESC_U_HEX4
		| ONIG_SYN_OP2_QMARK_LPAREN_IF_ELSE
		| ONIG_SYN_OP2_ESC_CAPITAL_R_GENERAL_NEWLINE
		| ONIG_SYN_OP2_ESC_CAPITAL_N_O_SUPER_DOT
		| ONIG_SYN_OP2_QMARK_TILDE_ABSENT_GROUP
		| ONIG_SYN_OP2_ESC_X_Y_GRAPHEME_CLUSTER),
	(ONIG_SYN_CONTEXT_INDEP_REPEAT_OPS
		| ONIG_SYN_CONTEXT_INVALID_REPEAT_OPS
		| ONIG_SYN_ALLOW_INVALID_INTERVAL
		| ONIG_SYN_ALLOW_INTERVAL_LOW_ABBREV
		| ONIG_SYN_DIFFERENT_LEN_ALT_LOOK_BEHIND
		| ONIG_SYN_ALLOW_MULTIPLEX_DEFINITION_NAME
		| ONIG_SYN_BACKSLASH_ESCAPE_IN_CC
		| ONIG_SYN_ALLOW_DOUBLE_RANGE_OP_IN_CC
		| ONIG_SYN_WARN_CC_OP_NOT_ESCAPED
		| ONIG_SYN_WARN_REDUNDANT_NESTED_REPEAT
		| ONIG_SYN_CONTEXT_INDEP_ANCHORS),
	(ONIG_OPTION_NONE),
	{
		(OnigCodePoint)'\\',						// escape
		(OnigCodePoint)ONIG_INEFFECTIVE_META_CHAR,	// anychar '.'
		(OnigCodePoint)ONIG_INEFFECTIVE_META_CHAR,	// anytime '*'
		(OnigCodePoint)ONIG_INEFFECTIVE_META_CHAR,	// zero or one time '?'
		(OnigCodePoint)ONIG_INEFFECTIVE_META_CHAR,	// one or more time '+'
		(OnigCodePoint)ONIG_INEFFECTIVE_META_CHAR,	// anychar anytime
	}
};
