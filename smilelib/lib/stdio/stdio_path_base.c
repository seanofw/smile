//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2017 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <smile/smiletypes/easyobject.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/stringbuilder.h>

#if (SMILE_OS & SMILE_OS_UNIX_FAMILY) == SMILE_OS_UNIX_FAMILY
#	include <limits.h>
#	include <stdlib.h>
#elif  (SMILE_OS & SMILE_OS_WINDOWS_FAMILY) == SMILE_OS_WINDOWS_FAMILY
#	include <stdlib.h>
#	include <string.h>
#else
#	error Unsupported OS for "stdio" library.
#endif

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_INTERNAL_FUNC void Stdio_Path_Init(SmileUserObject base);

static Byte _stringChecks[] = {
	0, 0,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
};

SMILE_EXTERNAL_FUNCTION(Clean)
{
	// This "cleans" a path by performing the following operations:
	//
	//   1. All '\' characters are turned into '/' characters.
	//
	//   2. All './' path chunks are deleted.
	//
	//   3. All '../' path chunks are combined with their parent and deleted.
	//
	//   4. All sequences of multiple '/' characters are turned into a single '/'.
	//
	//   5. Any trailing sequence of '/' characters are removed, unless they also start the string.
	//
	//   6. Rule #4 is *partially* ignored if the multiple '/' characters are found
	//       at the start of the path or immediately following /^[^/\\]{2,}:/ --- that is,
	//       if something like "\\server\foo\bar" or "http://foo/bar/baz" is found, the
	//       initial double-slashes will be kept as-is.  Note that this means that if
	//       "disk:\\\\foo\\\\bar" is found, the result will be the slightly-unexpected result
	//       of "disk://foo/bar", because this routine cannot distinguish between a URL scheme
	//       and an Amiga-style disk name.  (But this routine *does* correctly turn
	//       "c:\\\\foo\\\\bar" into "c:/foo/bar".)

	return SmileArg_From(NullObject);
}

/// <summary>
/// This attempts to (synchronously) resolve a (possibly-relative) path on the actual
/// OS filesystem by traversing directories and resolving soft-links.  The result will
/// either be an absolute path string to a (probably) real file/folder, or 'null' if no
/// such file/folder exists or is somehow not accessible.  On some OSes, this may
/// require specific user permissions in order to succeed.
///
/// Warning:  This should *NOT* be used to test for file existence!  If this succeeds,
/// it is *NOT* a guarantee that the file exists or is accessible:  It is merely a
/// valid pathname according to this OS's rules.  If you need to test for existence,
/// use the File.exists or Dir.exists methods, or actually open/read the file or
/// directory.
/// </summary>
SMILE_EXTERNAL_FUNCTION(Resolve)
{
	String result;
	String original;
	const Byte *pathText;
	Int pathLength;

	// Get the original path.
	original = (String)argv[1].obj;
	pathText = String_GetBytes(original);
	pathLength = String_Length(original);

	// '\0' is not a valid character in a filename on any real OS.
	if (String_ContainsNul(original))
		return SmileArg_From(NullObject);

#	if (SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY
	{
		const char *resolvedPath;
		int resolvedLength;

		// Use realpath() to do the hard work.
		resolvedPath = realpath((const char *)pathText, NULL);
		resolvedLength = StrLen(resolvedPath);
		result = String_Create((const Byte *)resolvedPath, resolvedLength);
		free(resolvedPath);
	}
#	elif  (SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY
	{
		wchar_t *originalPath16, *resultPath16;
		size_t resolvedLength;

		// Convert this to UTF-16, because that's what the Windows APIs need.
		originalPath16 = (wchar_t *)String_ToUtf16(original, NULL);

		// Use _wfullpath() to do the hard work.
		resultPath16 = _wfullpath(NULL, originalPath16, 0);
		resolvedLength = wcslen(resultPath16);
		result = String_FromUtf16((const UInt16 *)resultPath16, resolvedLength);
		free(resultPath16);
	}
#	else
#		error Unsupported OS for "stdio" library.
#	endif

	return SmileArg_From((SmileObject)result);
}

SMILE_EXTERNAL_FUNCTION(Join)
{
	Int i;
	DECLARE_INLINE_STRINGBUILDER(pathBuilder, 256);
	const Byte *text;
	Int length, pathLength;
	String piece, result;
	Bool isFirst;

	INIT_INLINE_STRINGBUILDER(pathBuilder);

	// Skip the 'Path' object itself, if it's the first argument.
	i = 0;
	if (argv[i].obj == param) {
		i++;
	}

	// This is a bit like a hybrid of the logic in String./ and the logic in List.join:
	// Like String./, we're smart about merging slashes together, and avoid duplicates
	// where possible.  Like List.join, we cast types (which is very helpful to the user),
	// and skip empty pieces.

	// Append everything else to the stringbuilder, stripping off
	// any initial or trailing '/' or '\' characters we find each time we append.
	isFirst = True;
	for (; i < argc; i++) {
		piece = SMILE_VCALL1(argv[i].obj, toString, argv[i].unboxed);
		text = String_GetBytes(piece);
		length = String_Length(piece);

		if (!isFirst) {
			// Strip initial slashes from every subsequent piece.
			while (length > 0 && (*text == '/' || *text == '\\'))
				text++, length--;
		}
		if (length <= 0) continue;	// Don't append empty pieces or pieces that are just slashes.

		// Strip trailing slashes from the end of the stringbuilder before appending the new piece.
		while ((pathLength = StringBuilder_GetLength(pathBuilder)) > 0
			&& (StringBuilder_At(pathBuilder, pathLength - 1) == '/' || StringBuilder_At(pathBuilder, pathLength - 1) == '\\')) {
			StringBuilder_SetLength(pathBuilder, pathLength - 1);
		}

		// Append a slash, and then this next piece.
		StringBuilder_AppendByte(pathBuilder, '/');
		StringBuilder_Append(pathBuilder, text, 0, length);
	}

	// Return the resulting combined path.
	result = StringBuilder_ToString(pathBuilder);
	return SmileArg_From((SmileObject)result);
}

SMILE_EXTERNAL_FUNCTION(Split)
{
	String path = (String)argv[1].obj;
	Int splitIndex = 0, startIndex = 0;
	SmileList head = NullList, tail = NullList;

	while ((splitIndex = String_IndexOfAnyChar(path, (const Byte *)"/\\", 2, startIndex)) >= 0) {
		if (splitIndex != startIndex) {
			LIST_APPEND(head, tail, String_Substring(path, startIndex, splitIndex - startIndex));
		}
		startIndex = splitIndex + 1;
	}

	return SmileArg_From((SmileObject)head);
}

SMILE_EXTERNAL_FUNCTION(IsAbsolute)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(IsRelative)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(GetExt)
{
	String path = (String)argv[1].obj;
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *extEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr > pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	if (ptr <= pathText)
		return SmileArg_From((SmileObject)String_Empty);

	// Rewind back to the nearest '/' or '\' or '.' before this spot.
	extEnd = ptr--;
	while (ptr >= pathText && !(*ptr == '.' || *ptr == '/' || *ptr == '\\')) ptr--;
	if (ptr < pathText)
		return SmileArg_From((SmileObject)String_Empty);

	// If we reached a '.', we have an extension.  If not, this has no extension.
	if (*ptr != '.')
		return SmileArg_From((SmileObject)String_Empty);

	// Extract and return the file extension.
	return SmileArg_From((SmileObject)String_Create(ptr, extEnd - ptr));
}

SMILE_EXTERNAL_FUNCTION(GetFilename)
{
	String path = (String)argv[1].obj;
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *nameEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	if (ptr < pathText)
		return SmileArg_From((SmileObject)String_Empty);

	// Rewind back to the nearest '/' or '\' before this spot.
	nameEnd = ptr;
	while (ptr > pathText && !(ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;

	// Extract and return the filename.
	return SmileArg_From((SmileObject)String_Create(ptr, nameEnd - ptr));
}

SMILE_EXTERNAL_FUNCTION(GetFilenameWithoutExt)
{
	String path = (String)argv[1].obj;
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *nameEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	if (ptr < pathText)
		return SmileArg_From((SmileObject)String_Empty);

	// Rewind back to the nearest '/' or '\' or '.' before this spot.
	nameEnd = ptr;
	while (ptr > pathText && !(ptr[-1] == '.' || ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;

	// If we reached the start of the string, or we hit a '/' or '\', take the whole thing, since there is no extension.
	if (ptr == pathText || ptr[-1] != '.')
		return SmileArg_From((SmileObject)String_Create(ptr, nameEnd - ptr));

	// We reached a '.', so we have an extension that starts here.  Mark the spot,
	// and then keep rewinding to the start of the name.
	nameEnd = --ptr;
	while (ptr > pathText && !(ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;

	// Extract and return the name, without its file extension.
	return SmileArg_From((SmileObject)String_Create(ptr, nameEnd - ptr));
}

SMILE_EXTERNAL_FUNCTION(GetDirname)
{
	String path = (String)argv[1].obj;
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *possibleDirEnd, *dirEndWithoutSlashes;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
	if (ptr < pathText)
		return SmileArg_From((SmileObject)String_Empty);

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
		return SmileArg_From((SmileObject)(possibleDirEnd > pathText
			? String_Create(pathText, possibleDirEnd - pathText)
			: String_Empty));
	}
	else if (ptr[-1] != ':') {
		// The preceding character is not a ':', so we can take the result as-is.
		return SmileArg_From((SmileObject)String_Create(pathText, ptr - pathText));
	}
	else {
		// The preceding character is a ':', so we have to see what comes before it.
		while (ptr > pathText && !(ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;
		if (ptr == pathText) {
			// Rewound to the beginning.  This means that we're probably looking at a "disk:"
			// form, and we should keep any slashes as part of the root.
			return SmileArg_From((SmileObject)String_Create(pathText, possibleDirEnd - pathText));
		}
		else {
			// Reached another slash, so this is just an embedded path component that happens
			// to end in a colon.  Take the result as it was.
			return SmileArg_From((SmileObject)String_Create(pathText, dirEndWithoutSlashes - pathText));
		}
	}
}

void Stdio_Path_Init(SmileUserObject base)
{
	SetupFunction("get-ext", GetExt, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("get-filename", GetFilename, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("get-filename-without-ext", GetFilenameWithoutExt, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("get-dirname", GetDirname, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("clean", Clean, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("resolve", Resolve, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("join", Join, (void *)base, "Path paths...", ARG_CHECK_MIN, 2, 0, 2, _stringChecks);
	SetupFunction("split", Split, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("absolute?", IsAbsolute, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("relative?", IsRelative, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
}
