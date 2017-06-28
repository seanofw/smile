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

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_INTERNAL_FUNC void Stdio_Path_Init(SmileUserObject base);

static Byte _stringChecks[] = {
	0, 0,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
};

SMILE_EXTERNAL_FUNCTION(Clean)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(Resolve)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(Join)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(Split)
{
	String path = (String)argv[0].obj;
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
	String path = (String)argv[0].obj;
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *extEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (*ptr == '/' || *ptr == '\\')) ptr--;
	if (ptr < pathText)
		return SmileArg_From((SmileObject)String_Empty);

	// Rewind back to the nearest '/' or '\' or '.' before this spot.
	extEnd = ptr + 1;
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
	String path = (String)argv[0].obj;
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *nameEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (*ptr == '/' || *ptr == '\\')) ptr--;
	if (ptr < pathText)
		return SmileArg_From((SmileObject)String_Empty);

	// Rewind back to the nearest '/' or '\' before this spot.
	nameEnd = ++ptr;
	while (ptr > pathText && !(ptr[-1] == '/' || ptr[-1] == '\\')) ptr--;

	// Extract and return the filename.
	return SmileArg_From((SmileObject)String_Create(ptr, nameEnd - ptr));
}

SMILE_EXTERNAL_FUNCTION(GetFilenameWithoutExt)
{
	String path = (String)argv[0].obj;
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *nameEnd;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (*ptr == '/' || *ptr == '\\')) ptr--;
	if (ptr < pathText)
		return SmileArg_From((SmileObject)String_Empty);

	// Rewind back to the nearest '/' or '\' or '.' before this spot.
	nameEnd = ++ptr;
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
	String path = (String)argv[0].obj;
	const Byte *pathText = String_GetBytes(path);
	Int pathLength = String_Length(path);
	const Byte *pathEnd = pathText + pathLength;
	const Byte *ptr;
	const Byte *possibleDirEnd, *dirEndWithoutSlashes;

	// Start at the end, and rewind to just before any trailing '/' or '\' marks.
	ptr = pathEnd;
	while (ptr >= pathText && (*ptr == '/' || *ptr == '\\')) ptr--;
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
		// Ran out of characters, so there is no parent.
		return SmileArg_From((SmileObject)String_Empty);
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
	SetupFunction("join", Join, NULL, "Path paths...", ARG_CHECK_MIN, 2, 0, 2, _stringChecks);
	SetupFunction("split", Split, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("absolute?", IsAbsolute, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
	SetupFunction("relative?", IsRelative, NULL, "Path path", ARG_CHECK_EXACT, 2, 2, 2, _stringChecks);
}
