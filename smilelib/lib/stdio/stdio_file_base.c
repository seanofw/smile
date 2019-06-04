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
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/smilehandle.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/raw/smilebytearray.h>
#include <smile/stringbuilder.h>

#include "stdio_internal.h"

SMILE_IGNORE_UNUSED_VARIABLES

SMILE_INTERNAL_FUNC void Stdio_File_Init(SmileUserObject base);

Bool Stdio_Invoked;

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

	Symbol set, start, cur, current, end, seek_set, seek_cur, seek_end;
} *FileInfo;

static Byte _handleChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
};

static Byte _seekChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_SYMBOL,
};

static Byte _writeByteChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_BYTE,
};

static Byte _writeCharChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_CHAR,
};

static Byte _writeUniChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_UNBOXED_UNI,
};

static Byte _readWriteChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_BYTEARRAY,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
	SMILE_KIND_MASK, SMILE_KIND_INTEGER64,
};

void Stdio_File_UpdateLastError(Stdio_File file)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		file->lastErrorCode = GetLastError();
		file->lastErrorMessage = file->lastErrorCode ? Smile_Win32_GetErrorString(file->lastErrorCode) : String_Empty;
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		file->lastErrorCode = errno;
		file->lastErrorMessage = errno ? Smile_Unix_GetErrorString(errno) : String_Empty;
#	else
#		error Unsupported OS.
#	endif
}

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
		Symbol symbol;

		switch (SMILE_KIND(argv[i].obj)) {
			case SMILE_KIND_UNBOXED_SYMBOL:
				symbol = argv[i].unboxed.symbol;
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
				break;

			case SMILE_KIND_UNBOXED_INTEGER64:
				fileMode = (UInt32)argv[i].unboxed.i64;
				break;
			case SMILE_KIND_UNBOXED_INTEGER32:
				fileMode = (UInt32)argv[i].unboxed.i32;
				break;
			case SMILE_KIND_UNBOXED_INTEGER16:
				fileMode = (UInt32)argv[i].unboxed.i16;
				break;
			case SMILE_KIND_UNBOXED_BYTE:
				fileMode = (UInt32)argv[i].unboxed.i8;
				break;

			default:
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
	fileHandle = Stdio_File_CreateFromPath(fileInfo->fileBase, path, flags, fileMode);

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

SMILE_EXTERNAL_FUNCTION(IsEof)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.eof?");

	return SmileUnboxedBool_From(file->isEof);
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

	if (!handle->methods->end(handle, True)) {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_FromC("Cannot close a special file handle.");
		return SmileUnboxedSymbol_From(fileInfo->error);
	}
	else {
		Stdio_File_UpdateLastError(file);
		file->isOpen = False;
		return SmileUnboxedSymbol_From(fileInfo->closed);
	}
}

SMILE_EXTERNAL_FUNCTION(Tell)
{
	Int64 pos;
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.tell");

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		pos = _telli64(file->fd);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		pos = lseek64(file->fd, 0, SEEK_CUR);
#	else
#		error Unsupported OS.
#	endif

	Stdio_File_UpdateLastError(file);

	return SmileUnboxedInteger64_From(pos);
}

static void SeekForReal(Stdio_File file, Int64 offset, int whence)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		_lseeki64(file->fd, offset, whence);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		lseek64(file->fd, offset, whence);
#	else
#		error Unsupported OS.
#	endif

	file->isEof = False;

	Stdio_File_UpdateLastError(file);
}

SMILE_EXTERNAL_FUNCTION(Seek)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.seek");
	FileInfo fileInfo = (FileInfo)param;
	int whence = SEEK_SET;

	if (argc == 3) {
		Symbol symbol = argv[2].unboxed.symbol;
		if (symbol == fileInfo->start || symbol == fileInfo->set || symbol == fileInfo->seek_set)
			whence = SEEK_SET;
		else if (symbol == fileInfo->cur || symbol == fileInfo->current || symbol == fileInfo->seek_cur)
			whence = SEEK_CUR;
		else if (symbol == fileInfo->end || symbol == fileInfo->seek_end)
			whence = SEEK_END;
		else
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_Format("Third parameter to 'File.seek' must be a known 'whence' symbol, not '%S'.",
					SymbolTable_GetName(Smile_SymbolTable, symbol)));
	}

	SeekForReal(file, argv[1].unboxed.i64, whence);

	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(SeekSet)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.seek-set");
	SeekForReal(file, argv[1].unboxed.i64, SEEK_SET);
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(SeekCur)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.seek-cur");
	SeekForReal(file, argv[1].unboxed.i64, SEEK_CUR);
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(SeekEnd)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.seek-end");
	SeekForReal(file, argv[1].unboxed.i64, SEEK_END);
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(ReadByte)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.read-byte");
	Byte buffer[1];
	int count;

	if (file->mode & FILE_MODE_STD)
		Stdio_Invoked = True;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		count = _read(file->fd, buffer, 1);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		count = read(file->fd, buffer, 1);
#	else
#		error Unsupported OS.
#	endif

	if (count > 0) {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
		return SmileUnboxedByte_From(buffer[0]);
	}
	else if (count == 0) {
		file->isEof = True;
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
		return SmileArg_From(NullObject);
	}
	else {
		Stdio_File_UpdateLastError(file);
		return SmileUnboxedSymbol_From(((FileInfo)param)->error);
	}
}

static SmileArg WriteByteInternal(Stdio_File file, FileInfo fileInfo, Byte byte)
{
	Byte buffer[1];
	int count;

	if (file->mode & FILE_MODE_STD)
		Stdio_Invoked = True;

	buffer[0] = byte;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		count = _write(file->fd, buffer, 1);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		count = write(file->fd, buffer, 1);
#	else
#		error Unsupported OS.
#	endif

	if (count > 0) {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
		return SmileUnboxedInteger64_From(count);
	}
	else {
		Stdio_File_UpdateLastError(file);
		return SmileUnboxedSymbol_From(fileInfo->error);
	}
}

static SmileArg WriteBytesInternal(Stdio_File file, FileInfo fileInfo, const Byte *buffer, UInt32 length)
{
	int count;

	if (file->mode & FILE_MODE_STD)
		Stdio_Invoked = True;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		count = _write(file->fd, buffer, length);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		count = write(file->fd, buffer, (int)length);
#	else
#		error Unsupported OS.
#	endif

	if (count > 0) {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
		return SmileUnboxedInteger64_From(count);
	}
	else {
		Stdio_File_UpdateLastError(file);
		return SmileUnboxedSymbol_From(fileInfo->error);
	}
}

SMILE_EXTERNAL_FUNCTION(WriteByte)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.write-byte");
	return WriteByteInternal(file, (FileInfo)param, argv[1].unboxed.i8);
}

SMILE_EXTERNAL_FUNCTION(WriteChar)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.write-char");
	return WriteByteInternal(file, (FileInfo)param, argv[1].unboxed.ch);
}

SMILE_EXTERNAL_FUNCTION(WriteUni)
{
	Stdio_File file;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 16);

	INIT_INLINE_STRINGBUILDER(stringBuilder);
	StringBuilder_AppendUnicode(stringBuilder, argv[1].unboxed.uni);

	file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.write-uni");
	return WriteBytesInternal(file, (FileInfo)param,
		StringBuilder_GetBytes(stringBuilder), (UInt32)StringBuilder_GetLength(stringBuilder));
}

SMILE_EXTERNAL_FUNCTION(Read)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.read");
	Int64 start, length, count;
	SmileByteArray byteArray;
	Byte *buffer;

	if (file->mode & FILE_MODE_STD)
		Stdio_Invoked = True;

	switch (argc) {
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("File.read: Invalid number of arguments."));
		case 2:
			// Buffer only.
			byteArray = (SmileByteArray)argv[1].obj;
			start = 0;
			length = byteArray->length;
			break;
		case 3:
			// Buffer and length.
			byteArray = (SmileByteArray)argv[1].obj;
			start = 0;
			length = argv[2].unboxed.i64;
			if (length > byteArray->length)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_FromC("File.read: Cannot write ByteArray outside its boundary."));
			break;
		case 4:
			// Buffer and length.
			byteArray = (SmileByteArray)argv[1].obj;
			start = argv[2].unboxed.i64;
			length = argv[3].unboxed.i64;
			if (start > byteArray->length || length > byteArray->length - start)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_FromC("File.read: Cannot write ByteArray outside its boundary."));
			break;
	}

	if (length == 0)
		return SmileUnboxedInteger64_From(0);
	if (length < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FromC("File.read: Length to read cannot be a negative number."));

#	if SizeofPtrInt > 4
		buffer = byteArray->data + start;
#	else
		buffer = byteArray->data + (Int32)start;
#	endif

	// Read the data for real.
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		if (length > Int32Max) length = Int32Max;
		count = _read(file->fd, buffer, (Int32)length);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		if (length > SSIZE_MAX) length = SSIZE_MAX;
		count = read(file->fd, buffer, (Int32)length);
#	else
#		error Unsupported OS.
#	endif

	if (count < 0) {
		Stdio_File_UpdateLastError(file);
	}
	else if (count == 0) {
		file->isEof = True;
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
	}
	else if (count > 0) {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
	}

	return SmileUnboxedInteger64_From(count);
}

SMILE_EXTERNAL_FUNCTION(Write)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.write");
	Int64 start, length, count;
	SmileByteArray byteArray;
	Byte *buffer;

	if (file->mode & FILE_MODE_STD)
		Stdio_Invoked = True;

	switch (argc) {
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("File.write: Invalid number of arguments."));
		case 2:
			// Buffer only.
			byteArray = (SmileByteArray)argv[1].obj;
			start = 0;
			length = byteArray->length;
			break;
		case 3:
			// Buffer and length.
			byteArray = (SmileByteArray)argv[1].obj;
			start = 0;
			length = argv[2].unboxed.i64;
			if (length > byteArray->length)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_FromC("File.write: Cannot read ByteArray outside its boundary."));
			break;
		case 4:
			// Buffer and length.
			byteArray = (SmileByteArray)argv[1].obj;
			start = argv[2].unboxed.i64;
			length = argv[3].unboxed.i64;
			if (start > byteArray->length || length > byteArray->length - start)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_FromC("File.write: Cannot read ByteArray outside its boundary."));
			break;
	}

	if (length == 0)
		return SmileUnboxedInteger64_From(0);
	if (length < 0)
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_FromC("File.write: Length to write cannot be a negative number."));

#	if SizeofPtrInt > 4
		buffer = byteArray->data + start;
#	else
		buffer = byteArray->data + (Int32)start;
#	endif

	// Write the data for real.
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		if (length > Int32Max) length = Int32Max;
		count = _write(file->fd, buffer, (Int32)length);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		if (length > SSIZE_MAX) length = SSIZE_MAX;
		count = write(file->fd, buffer, (Int32)length);
#	else
#		error Unsupported OS.
#	endif

	if (count < 0) {
		Stdio_File_UpdateLastError(file);
	}
	else {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
	}

	return SmileUnboxedInteger64_From(count);
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

	fileInfo->set = SymbolTable_GetSymbolC(Smile_SymbolTable, "set");
	fileInfo->start = SymbolTable_GetSymbolC(Smile_SymbolTable, "start");
	fileInfo->cur = SymbolTable_GetSymbolC(Smile_SymbolTable, "cur");
	fileInfo->current = SymbolTable_GetSymbolC(Smile_SymbolTable, "current");
	fileInfo->end = SymbolTable_GetSymbolC(Smile_SymbolTable, "end");
	fileInfo->seek_set = SymbolTable_GetSymbolC(Smile_SymbolTable, "seek-set");
	fileInfo->seek_cur = SymbolTable_GetSymbolC(Smile_SymbolTable, "seek-cur");
	fileInfo->seek_end = SymbolTable_GetSymbolC(Smile_SymbolTable, "seek-end");

	SetupFunction("open", Open, (void *)fileInfo, "File path mode...", 0, 0, 0, 0, NULL);
	SetupFunction("open?", IsOpen, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("error?", IsError, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("get-error", GetError, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("get-error-code", GetErrorCode, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("clear-error", ClearError, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("close", Close, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupSynonym("close", "end");
	SetupFunction("tell", Tell, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("seek", Seek, (void *)fileInfo, "file pos whence", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 3, 3, _seekChecks);
	SetupFunction("seek-set", SeekSet, (void *)fileInfo, "file pos", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _seekChecks);
	SetupFunction("seek-cur", SeekCur, (void *)fileInfo, "file pos", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _seekChecks);
	SetupFunction("seek-end", SeekEnd, (void *)fileInfo, "file pos", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _seekChecks);
	SetupFunction("read-byte", ReadByte, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("write-byte", WriteByte, (void *)fileInfo, "file byte", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _writeByteChecks);
	SetupFunction("write-char", WriteChar, (void *)fileInfo, "file char", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _writeCharChecks);
	SetupFunction("write-uni", WriteUni, (void *)fileInfo, "file uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _writeUniChecks);
	SetupFunction("read", Read, (void *)fileInfo, "file buffer start size", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 4, 4, _readWriteChecks);
	SetupFunction("write", Write, (void *)fileInfo, "file buffer start size", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 4, 4, _readWriteChecks);
	SetupFunction("eof?", IsEof, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	//SetupFunction("flush", Flush, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupSynonym("write-byte", "print-byte");
	SetupSynonym("write-char", "print-char");
	SetupSynonym("write-uni", "print-uni");
}
