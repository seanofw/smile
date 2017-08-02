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
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilehandle.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilesymbol.h>

#include "stdio_internal.h"

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_INTERNAL_FUNC void Stdio_File_Init(SmileUserObject base);

typedef struct FileInfoStruct {
	SmileObject fileBase;

	Symbol File;

	Symbol read, reading, read_only;
	Symbol write, writing, write_only;
	Symbol append, appending, append_only;
	Symbol read_write, read_append;
	Symbol trunc, truncate;
	Symbol create, open, create_only, open_only, create_or_open;

	Symbol closed;
	Symbol error;
} *FileInfo;

static Byte _handleChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
};

SMILE_EXTERNAL_FUNCTION(Open)
{
	Int i = 0;
	UInt32 flags = 0;
	String path;
	UInt32 fileMode = 0644;
	FileInfo fileInfo = (FileInfo)param;
	SmileHandle fileHandle;

	if (argv[i].obj == fileInfo->fileBase) i++;

	if (SMILE_KIND(argv[i].obj) != SMILE_KIND_STRING) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FromC("First argument to 'File.open' must be a String path."));
	}
	path = (String)argv[i++].obj;

	for (; i < argc; i++) {
		if (SMILE_KIND(argv[i].obj) == SMILE_KIND_UNBOXED_SYMBOL) {
			Symbol symbol = argv[i].unboxed.symbol;
			if (symbol == fileInfo->read || symbol == fileInfo->reading || symbol == fileInfo->read_only)
				flags |= FILE_MODE_READ;
			else if (symbol == fileInfo->write || symbol == fileInfo->writing || symbol == fileInfo->write_only)
				flags |= FILE_MODE_WRITE;
			else if (symbol == fileInfo->append || symbol == fileInfo->appending || symbol == fileInfo->append_only)
				flags |= FILE_MODE_APPEND;
			else if (symbol == fileInfo->read_write)
				flags |= FILE_MODE_READ | FILE_MODE_WRITE;
			else if (symbol == fileInfo->read_append)
				flags |= FILE_MODE_READ | FILE_MODE_APPEND;
			else if (symbol == fileInfo->trunc || symbol == fileInfo->truncate)
				flags |= FILE_MODE_TRUNCATE;
			else if (symbol == fileInfo->create || symbol == fileInfo->create_only)
				flags |= FILE_MODE_CREATE_ONLY;
			else if (symbol == fileInfo->open || symbol == fileInfo->open_only)
				flags |= FILE_MODE_OPEN_ONLY;
			else if (symbol == fileInfo->create_or_open)
				flags |= FILE_MODE_CREATE_OR_OPEN;
			else
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_Format("File.open: Unknown mode '%S'", SymbolTable_GetName(Smile_SymbolTable, symbol)));
		}
		else if (SMILE_KIND(argv[i].obj) == SMILE_KIND_UNBOXED_INTEGER64) {
			fileMode = (UInt32)argv[i].unboxed.i64;
		}
		else {
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("Arguments to 'File.open' must be access symbols like 'read' and 'write',"
					" or an Integer for 'rwx'-style file-mode bits."));
		}
	}

	if ((flags & (FILE_MODE_READ | FILE_MODE_WRITE | FILE_MODE_APPEND)) == 0) {
		// Nothing specified, so assume read.
		flags |= FILE_MODE_READ;
	}

	if ((flags & FILE_MODE_OPEN_MASK) == 0) {
		// Nothing specified, so assume open-existing.
		flags |= FILE_MODE_CREATE_ONLY;
	}

	// We're all set, so go do it!
	fileHandle = Stdio_File_CreateFromPath(fileInfo->fileBase, path, flags);

	// Right or wrong, return the file.
	return SmileArg_From((SmileObject)fileHandle);
}

static Stdio_File GetFileFromHandle(SmileHandle handle, FileInfo fileInfo, const char *functionName)
{
	if (handle->handleKind != fileInfo->File)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_Format("Argument to '%s' must be a 'File'.", functionName));

	return (Stdio_File)handle->ptr;
}

SMILE_EXTERNAL_FUNCTION(IsOpen)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.open?");

	return SmileUnboxedBool_From(file->isOpen);
}

SMILE_EXTERNAL_FUNCTION(IsError)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.error?");

	return SmileUnboxedBool_From(file->lastErrorCode != 0 || !String_IsNullOrEmpty(file->lastErrorMessage));
}

SMILE_EXTERNAL_FUNCTION(GetError)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.error");

	return SmileArg_From(!String_IsNullOrEmpty(file->lastErrorMessage) ? (SmileObject)file->lastErrorMessage : NullObject);
}

SMILE_EXTERNAL_FUNCTION(GetErrorCode)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.error-code");

	return SmileUnboxedInteger64_From(file->lastErrorCode);
}

SMILE_EXTERNAL_FUNCTION(ClearError)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.clear-error");

	file->lastErrorCode = 0;
	file->lastErrorMessage = String_Empty;

	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(Close)
{
	SmileHandle handle = (SmileHandle)argv[0].obj;
	FileInfo fileInfo = (FileInfo)param;
	Stdio_File file = GetFileFromHandle(handle, (FileInfo)param, "File.close");

	if (!handle->end(handle, True)) {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_FromC("Cannot close a special file handle.");
		return SmileUnboxedSymbol_From(fileInfo->error);
	}
	else {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
		file->isOpen = False;
		return SmileUnboxedSymbol_From(fileInfo->closed);
	}
}

void Stdio_File_Init(SmileUserObject base)
{
	FileInfo fileInfo = GC_MALLOC_STRUCT(struct FileInfoStruct);
	if (fileInfo == NULL)
		Smile_Abort_OutOfMemory();

	fileInfo->fileBase = (SmileObject)base;

	fileInfo->File = SymbolTable_GetSymbolC(Smile_SymbolTable, "File");
	fileInfo->read = SymbolTable_GetSymbolC(Smile_SymbolTable, "read");
	fileInfo->reading = SymbolTable_GetSymbolC(Smile_SymbolTable, "reading");
	fileInfo->read_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "read-only");
	fileInfo->write = SymbolTable_GetSymbolC(Smile_SymbolTable, "write");
	fileInfo->writing = SymbolTable_GetSymbolC(Smile_SymbolTable, "writing");
	fileInfo->write_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "write-only");
	fileInfo->append = SymbolTable_GetSymbolC(Smile_SymbolTable, "append");
	fileInfo->appending = SymbolTable_GetSymbolC(Smile_SymbolTable, "appending");
	fileInfo->append_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "append-only");
	fileInfo->read_write = SymbolTable_GetSymbolC(Smile_SymbolTable, "read-write");
	fileInfo->read_append = SymbolTable_GetSymbolC(Smile_SymbolTable, "read-append");
	fileInfo->trunc = SymbolTable_GetSymbolC(Smile_SymbolTable, "trunc");
	fileInfo->truncate = SymbolTable_GetSymbolC(Smile_SymbolTable, "truncate");
	fileInfo->create = SymbolTable_GetSymbolC(Smile_SymbolTable, "create");
	fileInfo->open = SymbolTable_GetSymbolC(Smile_SymbolTable, "open");
	fileInfo->create_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "create-only");
	fileInfo->open_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "open-only");
	fileInfo->create_or_open = SymbolTable_GetSymbolC(Smile_SymbolTable, "create-or-open");

	fileInfo->closed = SymbolTable_GetSymbolC(Smile_SymbolTable, "closed");
	fileInfo->error = SymbolTable_GetSymbolC(Smile_SymbolTable, "error");

	SetupFunction("open", Open, (void *)fileInfo, "File path mode...", 0, 0, 0, 0, NULL);
	SetupFunction("open?", IsOpen, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("error?", IsError, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("get-error", GetError, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("get-error-code", GetErrorCode, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("clear-error", ClearError, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("close", Close, (void *)fileInfo, "file", 0, 0, 0, 0, NULL);
	SetupSynonym("close", "end");
}
