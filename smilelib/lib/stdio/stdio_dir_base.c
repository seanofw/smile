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
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilelist.h>

#include "stdio_internal.h"

SMILE_IGNORE_UNUSED_VARIABLES

typedef struct DirInfoStruct {
	SmileObject dirBase;
	IoSymbols ioSymbols;
} *DirInfo;

static Byte _stringChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
};

static Byte _modeChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	0, 0,
};

SMILE_EXTERNAL_FUNCTION(SetCurrent)
{
	String path = (String)argv[0].obj;
	Bool result;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		Int length;
		UInt16 *pathName = Stdio_ToWindowsPath(path, &length);
		result = !!SetCurrentDirectoryW(pathName);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		result = chdir(String_ToC(path)) == 0;
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(GetCurrent)
{
	String result;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		UInt16 inlineBuffer[256];
		UInt16 *buffer = inlineBuffer;
		UInt32 bufferSize = 256, returnedCount;

	retry:
		returnedCount = GetCurrentDirectoryW(bufferSize - 1, buffer);
		if (returnedCount == 0)
			return SmileArg_From(NullObject);
		if (returnedCount > bufferSize) {
			returnedCount = GetCurrentDirectoryW(0, NULL);
			bufferSize = returnedCount + 1;
			buffer = GC_MALLOC_ATOMIC(sizeof(UInt16) * bufferSize);
			goto retry;
		}

		result = Stdio_FromWindowsPath(buffer, returnedCount);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		char inlineBuffer[256];
		char *buffer = inlineBuffer, *returnedBuffer;
		size_t bufferSize = 256;

	retry:
		returnedBuffer = getcwd((char *)buffer, bufferSize - 1);
		if (returnedBuffer != NULL) {
			result = String_Create(buffer, StrLen(buffer));
		}
		else {
			bufferSize *= 2;
			buffer = GC_MALLOC_ATOMIC(bufferSize);
			goto retry;
		}
#	else
#		error Unsupported OS.
#	endif

	return SmileArg_From((SmileObject)result);
}

SMILE_EXTERNAL_FUNCTION(Create)
{
	String path = (String)argv[0].obj;
	Bool result;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		Int length;
		UInt16 *pathName = Stdio_ToWindowsPath(path, &length);
		result = !!CreateDirectoryW(pathName, NULL);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		UInt32 mode = argc > 1 ? Stdio_ParseModeArg(argv[1], "Dir.create") : 0755;
		result = mkdir(String_ToC(path), (mode_t)mode) == 0;
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Remove)
{
	String path = (String)argv[0].obj;
	Bool result;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		Int length;
		UInt16 *pathName = Stdio_ToWindowsPath(path, &length);
		result = !!RemoveDirectoryW(pathName);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		result = rmdir(String_ToC(path)) == 0;
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Read)
{
	String path = (String)argv[0].obj;
	SmileList head, tail;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		WIN32_FIND_DATAW findData;
		Int length;
		UInt16 *pathName;
		HANDLE handle;
		String paths[2];
		String fileName;

		paths[0] = path;
		paths[1] = String_FromC("*.*");
		pathName = Stdio_ToWindowsPath(String_SlashAppend(paths, 2), &length);

		LIST_INIT(head, tail);

		handle = FindFirstFileW(pathName, &findData);
		if (handle == INVALID_HANDLE_VALUE || handle == NULL)
			return SmileUnboxedBool_From(False);

		do {
			findData.cFileName[259] = '\0';
			fileName = String_FromUtf16(findData.cFileName, WStrLen(findData.cFileName));
			LIST_APPEND(head, tail, fileName);
		} while (FindNextFileW(handle, &findData));

		FindClose(handle);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		DIR *dir;
		struct dirent *entry;
		String fileName;

		LIST_INIT(head, tail);

		if ((dir = opendir(String_ToC(path))) == NULL)
			return SmileUnboxedBool_From(False);

		while ((entry = readdir(dir)) != NULL) {
			fileName = String_FromC(entry->d_name);
			LIST_APPEND(head, tail, fileName);
		}

		closedir(dir);
#	else
#		error Unsupported OS.
#	endif

	return SmileArg_From((SmileObject)head);
}

void Stdio_Dir_Init(SmileUserObject base, IoSymbols ioSymbols)
{
	DirInfo dirInfo = GC_MALLOC_STRUCT(struct DirInfoStruct);
	if (dirInfo == NULL)
		Smile_Abort_OutOfMemory();

	dirInfo->dirBase = (SmileObject)base;
	dirInfo->ioSymbols = ioSymbols;

	SetupFunction("set-current", SetCurrent, (void *)dirInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupSynonym("set-current", "chdir");
	SetupSynonym("set-current", "cd");
	SetupSynonym("set-current", "set");

	SetupFunction("get-current", GetCurrent, (void *)dirInfo, "", ARG_CHECK_EXACT, 0, 0, 0, NULL);
	SetupSynonym("get-current", "getcwd");
	SetupSynonym("get-current", "getpwd");
	SetupSynonym("get-current", "cwd");
	SetupSynonym("get-current", "pwd");
	SetupSynonym("get-current", "get");

	SetupFunction("create", Create, (void *)dirInfo, "path mode...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 2, _modeChecks);
	SetupSynonym("create", "mkdir");
	SetupSynonym("create", "md");

	SetupFunction("remove", Remove, (void *)dirInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupSynonym("remove", "rmdir");
	SetupSynonym("remove", "rd");
	SetupSynonym("remove", "delete");
	SetupSynonym("remove", "unlink");

	SetupFunction("read", Read, (void *)dirInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
}
