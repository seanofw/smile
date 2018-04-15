
#include <smile/string.h>
#include <smile/internal/staticstring.h>
#include <smile/stringbuilder.h>

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
#	define WIN32_LEAN_AND_MEAN
#	pragma warning(push)
#	pragma warning(disable: 4255)
#	include <windows.h>
#	pragma warning(pop)
#elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
#   include <unistd.h>
#   include <errno.h>
#else
#	error Unsupported OS.
#endif

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

Bool Path_IsAbsolute(String path)
{
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *dirEndWithSlashes, *dirEndWithoutSlashes;

	ptr = pathText;
	while (ptr < pathEnd && (*ptr == '/' || *ptr == '\\')) ptr++;
	if (ptr >= pathEnd)
		return False;

	dirEndWithoutSlashes = ptr;
	dirEndWithSlashes = ptr;
	while (dirEndWithSlashes < pathEnd && (*dirEndWithSlashes == '/' || *dirEndWithSlashes == '\\'))
		dirEndWithSlashes++;

	if (ptr == pathText)
		return dirEndWithSlashes > pathText;
	else if (ptr[-1] != ':')
		return False;
	else
		return True;
}

String Path_GetRoot(String path)
{
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *dirEndWithSlashes, *dirEndWithoutSlashes;

	// Start at the beginning, and skim forward to the first '/' or '\' character.
	ptr = pathText;
	while (ptr < pathEnd && (*ptr == '/' || *ptr == '\\')) ptr++;
	if (ptr >= pathEnd)
		return String_Empty;	// No 'disk:' or initial '/', so we have no root.

	// Collect any slashes.
	dirEndWithoutSlashes = ptr;
	dirEndWithSlashes = ptr;
	while (dirEndWithSlashes < pathEnd && (*dirEndWithSlashes == '/' || *dirEndWithSlashes == '\\'))
		dirEndWithSlashes++;

	if (ptr == pathText) {
		// Collected no characters, so there is no root text.  Take whatever initial slashes that
		// existed as the parent, or the empty string if there are no slashes.
		return dirEndWithSlashes > pathText
			? String_Create(pathText, dirEndWithSlashes - pathText)
			: String_Empty;
	}
	else if (ptr[-1] != ':') {
		// The last character of the root text is not a ':', so there is no root; this is
		// just a relative fragment.
		return String_Empty;
	}
	else {
		// Got a colon, so this means that we're probably looking at a "disk:"
		// form, and we should keep any slashes after it as part of the root.
		return String_Create(pathText, dirEndWithSlashes - pathText);
	}
}

String Path_Resolve(String currentDirectory, String relativePath)
{
	STATIC_STRING(slash, "/");
	STATIC_STRING(backslash, "\\");
	String *pieces;
	String piece, result;
	Int numPieces, i;
	Int length;
	const Byte *bytes;
	String concatBuffer[3];

	if (String_IsNullOrEmpty(relativePath))
		return currentDirectory;

	result = currentDirectory != NULL ? currentDirectory : String_Empty;

	if (Path_IsAbsolute(relativePath)) {
		result = Path_GetRoot(relativePath);
		relativePath = String_SubstringAt(relativePath, String_Length(result));
	}

	numPieces = String_Split(String_ReplaceChar(relativePath, '\\', '/'), slash, &pieces);

	concatBuffer[1] = slash;

	for (i = 0; i < numPieces; i++) {
		piece = pieces[i];

		bytes = String_GetBytes(piece);
		length = String_Length(piece);

		if (length == 1 && bytes[0] == '.') {
			// Relative directory to here, so just skip this piece.
			continue;
		}
		else if (length == 2 && bytes[0] == '.' && bytes[1] == '.') {
			// Parent directory, so strip back the current path by one.
			result = Path_GetDirname(result);
		}
		else {
			// This is a normal filename or directory name, so append this piece.
			if (String_EndsWith(result, slash) || String_EndsWith(result, backslash)) {
				result = String_Concat(result, piece);
			}
			else {
				concatBuffer[0] = result;
				concatBuffer[2] = piece;
				result = String_ConcatMany(concatBuffer, 3);
			}
		}
	}

	return result;
}

String Path_GetCurrentDir(void)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)

		UInt16 *buffer;
		UInt16 inline_buffer[256];
		Int max = 256;
		Int dirLength;

		buffer = inline_buffer;
		for (;;) {
			dirLength = (Int)GetCurrentDirectoryW(max - 1, buffer);
			if (dirLength == 0)
				Smile_Abort_FatalError("Cannot get current working directory.");
			else if (dirLength > max - 2) {
				max = dirLength + 1;
				buffer = GC_MALLOC_RAW_ARRAY(UInt16, max);
				if (buffer == NULL)
					Smile_Abort_OutOfMemory();
				continue;
			}
			else break;
		}

		return String_ReplaceChar(String_FromUtf16(buffer, (Int)dirLength), '\\', '/');

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)

		char *buffer;
		char inline_buffer[256];
		Int max = 256;

		buffer = inline_buffer;

		while (getcwd(buffer, max - 1) == NULL) {
			if (errno != ERANGE)
				Smile_Abort_FatalError("Cannot get current working directory.");

			max *= 2;
			buffer = GC_MALLOC_RAW_ARRAY(Byte, max);
			if (buffer == NULL)
				Smile_Abort_OutOfMemory();
		}

		return String_FromC(buffer);

#	else
#		error Unsupported OS.
#	endif
}

Bool Path_SetCurrentDir(String path)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)

		Int length;
		UInt16 *buffer = String_ToUtf16(String_ReplaceChar(path, '/', '\\'), &length);

		return SetCurrentDirectoryW(buffer) ? True : False;

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)

		const char *bytes = String_ToC(path);
		return chdir(bytes) == 0;

#	else
#		error Unsupported OS.
#	endif
}
