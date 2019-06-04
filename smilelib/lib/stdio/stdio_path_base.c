//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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
		free((void *)resolvedPath);
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
	Int length = String_Length(path);

	while ((splitIndex = String_IndexOfAnyChar(path, (const Byte *)"/\\", 2, startIndex)) >= 0) {
		if (splitIndex != startIndex)
			LIST_APPEND(head, tail, String_Substring(path, startIndex, splitIndex - startIndex));
		startIndex = splitIndex + 1;
	}

	if (startIndex < length)
		LIST_APPEND(head, tail, String_Substring(path, startIndex, length - startIndex));

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
	return SmileArg_From((SmileObject)Path_GetExt((String)argv[1].obj));
}

SMILE_EXTERNAL_FUNCTION(GetFilename)
{
	return SmileArg_From((SmileObject)Path_GetFilename((String)argv[1].obj));
}

SMILE_EXTERNAL_FUNCTION(GetFilenameWithoutExt)
{
	return SmileArg_From((SmileObject)Path_GetFilenameWithoutExt((String)argv[1].obj));
}

SMILE_EXTERNAL_FUNCTION(GetDirname)
{
	return SmileArg_From((SmileObject)Path_GetDirname((String)argv[1].obj));
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
