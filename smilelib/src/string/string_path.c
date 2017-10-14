
#include <smile/string.h>

String Path_GetExt(String path)
{
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *extEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr > pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	if (ptr <= pathText)
		return String_Empty;

	// Rewind back to the nearest '/' or '\' or '.' before this spot.
	extEnd = ptr--;
	while (ptr >= pathText && !(*ptr == '.' || *ptr == '/' || *ptr == '\\')) ptr--;
	if (ptr < pathText)
		return String_Empty;

	// If we reached a '.', we have an extension.  If not, this has no extension.
	if (*ptr != '.')
		return String_Empty;

	// Extract and return the file extension.
	return String_Create(ptr, extEnd - ptr);
}

String Path_GetFilename(String path)
{
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *nameEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	if (ptr < pathText)
		return String_Empty;

	// Rewind back to the nearest '/' or '\' before this spot.
	nameEnd = ptr;
	while (ptr > pathText && !(ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;

	// Extract and return the filename.
	return String_Create(ptr, nameEnd - ptr);
}

String Path_GetFilenameWithoutExt(String path)
{
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *nameEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	if (ptr < pathText)
		return String_Empty;

	// Rewind back to the nearest '/' or '\' or '.' before this spot.
	nameEnd = ptr;
	while (ptr > pathText && !(ptr[-1] == '.' || ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;

	// If we reached the start of the string, or we hit a '/' or '\', take the whole thing, since there is no extension.
	if (ptr == pathText || ptr[-1] != '.')
		return String_Create(ptr, nameEnd - ptr);

	// We reached a '.', so we have an extension that starts here.  Mark the spot,
	// and then keep rewinding to the start of the name.
	nameEnd = --ptr;
	while (ptr > pathText && !(ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;

	// Extract and return the name, without its file extension.
	return String_Create(ptr, nameEnd - ptr);
}

String Path_GetDirname(String path)
{
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *possibleDirEnd, *dirEndWithoutSlashes;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	if (ptr < pathText)
		return String_Empty;

	// Rewind back to the nearest '/' or '\' before this spot.
	while (ptr > pathText && !(ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;

	// The filename starts here.  Strip off any '/' or '\' marks before it, since they're not
	// strictly needed for the directory part.
	possibleDirEnd = ptr;
	while (ptr > pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	dirEndWithoutSlashes = ptr;

	// The directory part ends here --- except if it doesn't.  We have to decide if we should
	// have kept those trailing slashes or not, because of the path rules for Windows, AmigaOS,
	// and anything else that supports an initial "disk:" prefix.  So if the result ends with
	// a ':', and there are no preceding '/' or '\' characters, then we probably should have
	// kept the trailing slashes above, so that instead of "disk:" we return "disk:/".  This
	// produces slightly odd results for URLs, in that "http://foo.com/bar" results first in
	// "http://foo.com" and then "http://" as the root.  This is a reasonable tradeoff in trying
	// to build a function that supports everything, and since we have dedicated URL-oriented
	// functions in another library, this is "good enough" for most needs.
	if (ptr == pathText) {
		// Ran out of characters, so there is no parent.  Take whatever initial slashes that
		// existed as the parent, or the empty string if there are no slashes.
		return possibleDirEnd > pathText
			? String_Create(pathText, possibleDirEnd - pathText)
			: String_Empty;
	}
	else if (ptr[-1] != ':') {
		// The preceding character is not a ':', so we can take the result as-is.
		return String_Create(pathText, ptr - pathText);
	}
	else {
		// The preceding character is a ':', so we have to see what comes before it.
		while (ptr > pathText && !(ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
		if (ptr == pathText) {
			// Rewound to the beginning.  This means that we're probably looking at a "disk:"
			// form, and we should keep any slashes as part of the root.
			return String_Create(pathText, possibleDirEnd - pathText);
		}
		else {
			// Reached another slash, so this is just an embedded path component that happens
			// to end in a colon.  Take the result as it was.
			return String_Create(pathText, dirEndWithoutSlashes - pathText);
		}
	}
}
