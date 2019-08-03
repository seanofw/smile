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

Bool Stdio_Invoked;

typedef struct FileInfoStruct {
	SmileObject fileBase;
	IoSymbols ioSymbols;
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

static Byte _timeChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_STRING,
	0, 0,
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
	IoSymbols ioSymbols = fileInfo->ioSymbols;
	SmileHandle fileHandle;
	const char *methodName = "File.open";

	if (argv[i].obj == fileInfo->fileBase) i++;

	if (SMILE_KIND(argv[i].obj) != SMILE_KIND_STRING) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error,
			String_Format("First argument to '%s' must be a String path.", methodName));
	}
	path = (String)argv[i++].obj;

	for (; i < argc; i++) {
		Symbol symbol;

		switch (SMILE_KIND(argv[i].obj)) {
			case SMILE_KIND_UNBOXED_SYMBOL:
				symbol = argv[i].unboxed.symbol;
				if (symbol == ioSymbols->read || symbol == ioSymbols->reading || symbol == ioSymbols->read_only)
					flags |= FILE_MODE_READ;
				else if (symbol == ioSymbols->write || symbol == ioSymbols->writing || symbol == ioSymbols->write_only)
					flags |= FILE_MODE_WRITE;
				else if (symbol == ioSymbols->append || symbol == ioSymbols->appending || symbol == ioSymbols->append_only)
					flags |= FILE_MODE_APPEND;
				else if (symbol == ioSymbols->read_write)
					flags |= FILE_MODE_READ | FILE_MODE_WRITE;
				else if (symbol == ioSymbols->read_append)
					flags |= FILE_MODE_READ | FILE_MODE_APPEND;
				else if (symbol == ioSymbols->trunc || symbol == ioSymbols->truncate)
					flags |= FILE_MODE_TRUNCATE;
				else if (symbol == ioSymbols->create || symbol == ioSymbols->create_only)
					flags |= FILE_MODE_CREATE_ONLY;
				else if (symbol == ioSymbols->open || symbol == ioSymbols->open_only)
					flags |= FILE_MODE_OPEN_ONLY;
				else if (symbol == ioSymbols->create_or_open)
					flags |= FILE_MODE_CREATE_OR_OPEN;
				else
					Smile_ThrowException(Smile_KnownSymbols.native_method_error,
						String_Format("%s: Unknown mode '%S'", methodName, SymbolTable_GetName(Smile_SymbolTable, symbol)));
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
					String_Format("Arguments to '%s' must be access symbols like 'read' and 'write',"
						" or an Integer for 'rwx'-style file-mode bits.", methodName));
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
	if (handle->handleKind != fileInfo->ioSymbols->File)
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
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->error);
	}
	else {
		Stdio_File_UpdateLastError(file);
		file->isOpen = False;
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);
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
	IoSymbols ioSymbols = fileInfo->ioSymbols;
	int whence = SEEK_SET;

	if (argc == 3) {
		Symbol symbol = argv[2].unboxed.symbol;
		if (symbol == ioSymbols->start || symbol == ioSymbols->set || symbol == ioSymbols->seek_set)
			whence = SEEK_SET;
		else if (symbol == ioSymbols->cur || symbol == ioSymbols->current || symbol == ioSymbols->seek_cur)
			whence = SEEK_CUR;
		else if (symbol == ioSymbols->end || symbol == ioSymbols->seek_end)
			whence = SEEK_END;
		else
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_Format("Third parameter to 'File.seek' must be a known 'whence' symbol, not '%S'.",
					SymbolTable_GetName(Smile_SymbolTable, symbol)));
	}

	SeekForReal(file, argv[1].unboxed.i64, whence);

	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(Rewind)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.rewind");
	SeekForReal(file, 0, SEEK_SET);
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
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->error);
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
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->error);
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
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->error);
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
			if (!(byteArray->kind & SMILE_SECURITY_WRITABLE))
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_FromC("File.read: Cannot write to a read-only ByteArray."));
			start = 0;
			length = byteArray->length;
			break;
		case 3:
			// Buffer and length.
			byteArray = (SmileByteArray)argv[1].obj;
			if (!(byteArray->kind & SMILE_SECURITY_WRITABLE))
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_FromC("File.read: Cannot write to a read-only ByteArray."));
			start = 0;
			length = argv[2].unboxed.i64;
			if (length > byteArray->length)
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_FromC("File.read: Cannot write ByteArray outside its boundary."));
			break;
		case 4:
			// Buffer and length.
			byteArray = (SmileByteArray)argv[1].obj;
			if (!(byteArray->kind & SMILE_SECURITY_WRITABLE))
				Smile_ThrowException(Smile_KnownSymbols.native_method_error,
					String_FromC("File.read: Cannot write to a read-only ByteArray."));
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

//-------------------------------------------------------------------------------------------------

SMILE_EXTERNAL_FUNCTION(Rename)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(Remove)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(GetMode)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(SetMode)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(GetCreateTime)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(SetCreateTime)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(GetModifyTime)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(SetModifyTime)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(GetAccessTime)
{
	return SmileArg_From(NullObject);
}

SMILE_EXTERNAL_FUNCTION(SetAccessTime)
{
	return SmileArg_From(NullObject);
}

//-------------------------------------------------------------------------------------------------

void Stdio_File_Init(SmileUserObject base, IoSymbols ioSymbols)
{
	FileInfo fileInfo = GC_MALLOC_STRUCT(struct FileInfoStruct);
	if (fileInfo == NULL)
		Smile_Abort_OutOfMemory();

	fileInfo->fileBase = (SmileObject)base;
	fileInfo->ioSymbols = ioSymbols;

	SetupFunction("open", Open, (void *)fileInfo, "path mode...", 0, 0, 0, 0, NULL);
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
	SetupFunction("rewind", Rewind, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _seekChecks);
	SetupFunction("read-byte", ReadByte, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("write-byte", WriteByte, (void *)fileInfo, "file byte", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _writeByteChecks);
	SetupFunction("write-char", WriteChar, (void *)fileInfo, "file char", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _writeCharChecks);
	SetupFunction("write-uni", WriteUni, (void *)fileInfo, "file uni", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _writeUniChecks);
	SetupFunction("read", Read, (void *)fileInfo, "file buffer start size", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 4, 4, _readWriteChecks);
	SetupFunction("write", Write, (void *)fileInfo, "file buffer start size", ARG_CHECK_MIN | ARG_CHECK_MAX | ARG_CHECK_TYPES, 2, 4, 4, _readWriteChecks);
	SetupFunction("eof?", IsEof, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupSynonym("eof?", "eoi?");
	//SetupFunction("flush", Flush, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupSynonym("write-byte", "print-byte");
	SetupSynonym("write-char", "print-char");
	SetupSynonym("write-uni", "print-uni");

	SetupFunction("rename", Rename, (void *)fileInfo, "old-name new-name", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupSynonym("rename", "move");
	SetupSynonym("rename", "mv");

	SetupFunction("remove", Remove, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupSynonym("remove", "delete");
	SetupSynonym("remove", "unlink");
	SetupSynonym("remove", "rm");

	SetupFunction("get-mode", GetMode, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupFunction("set-mode", SetMode, (void *)fileInfo, "path mode...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 2, _modeChecks);
	SetupSynonym("set-mode", "chmod");

	SetupFunction("get-create-time", GetCreateTime, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _timeChecks);
	SetupFunction("set-create-time", SetCreateTime, (void *)fileInfo, "path time", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timeChecks);
	SetupFunction("get-modify-time", GetModifyTime, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _timeChecks);
	SetupFunction("set-modify-time", SetModifyTime, (void *)fileInfo, "path time", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timeChecks);
	SetupFunction("get-access-time", GetAccessTime, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _timeChecks);
	SetupFunction("set-access-time", SetAccessTime, (void *)fileInfo, "path time", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timeChecks);
}
