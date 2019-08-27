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
#include <smile/smiletypes/numeric/smilereal64.h>
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smiletimestamp.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/raw/smilebytearray.h>
#include <smile/stringbuilder.h>

#include "stdio_internal.h"

SMILE_IGNORE_UNUSED_VARIABLES

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)

// To make Windows pretend to return mode bits like Unix, we define
// the Unix mode-bit constants here so we can reference them by name
// in Windows-land.

#	define S_IFMT   0170000		// Mask for file type bits
#	define S_IFSOCK 0140000		// File type: Socket
#	define S_IFLNK  0120000		// File type: Soft link
#	define S_IFREG  0100000		// File type: Regular file
#	define S_IFBLK  0060000		// File type: Block device
#	define S_IFDIR  0040000		// File type: Directory
#	define S_IFCHR  0020000		// File type: Character device
#	define S_IFIFO  0010000		// File type: FIFO device
#	define S_ISUID  0004000		// Set-user-ID flag
#	define S_ISGID  0002000		// Set-group-ID flag
#	define S_ISVTX  0001000		// Sticky bit (i.e., privileged deletion only)

#	define S_IRWXU  0000700		// Mask for owner mode bits
#	define S_IRUSR  0000400		// Read-by-owner
#	define S_IWUSR  0000200		// Write-by-owner
#	define S_IXUSR  0000100		// Execute-by-owner

#	define S_IRWXG  0000070		// Mask for group mode bits
#	define S_IRGRP  0000040		// Read-by-group
#	define S_IWGRP  0000020		// Write-by-group
#	define S_IXGRP  0000010		// Execute-by-group

#	define S_IRWXO  0000070		// Mask for others (world) mode bits
#	define S_IROTH  0000004		// Read-by-others (world)
#	define S_IWOTH  0000002		// Write-by-others (world)
#	define S_IXOTH  0000001		// Execute-by-others (world)

#endif

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

static Byte _handleStringChecks[] = {
	SMILE_KIND_MASK, SMILE_KIND_HANDLE,
	SMILE_KIND_MASK, SMILE_KIND_STRING,
};

/// <summary>
/// This weird little Boolean flag is set to true if and only if the stdio library
/// is used at least once to read or write data from stdin, stdout, or stderr.
/// It's consumed by the Smile REPL so that the REPL knows whether your last operation
/// likely emitted text output; and if it did, the REPL then avoids printing out
/// the result of the evaluation, since your operation has already printed something.
/// It's a very weird special-case one-off, and it should NOT be consumed by anything
/// else that isn't the REPL.
/// </summary>
Bool Stdio_Invoked;

//-------------------------------------------------------------------------------------------------
// Opening and creating files.

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
		// Nothing specified, so assume open-existing (read) or create-if-missing (write/append).
		flags |= ((flags & (FILE_MODE_WRITE | FILE_MODE_APPEND)) != 0)
			? FILE_MODE_CREATE_OR_OPEN
			: FILE_MODE_OPEN_ONLY;
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

//-------------------------------------------------------------------------------------------------
// File status tests.

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

//-------------------------------------------------------------------------------------------------
// Errors.

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

//-------------------------------------------------------------------------------------------------
// Flush and Close.

SMILE_EXTERNAL_FUNCTION(Flush)
{
	SmileHandle handle = (SmileHandle)argv[0].obj;
	FileInfo fileInfo = (FileInfo)param;
	Stdio_File file = GetFileFromHandle(handle, (FileInfo)param, "File.flush");
	Bool result;

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		result = !!FlushFileBuffers(file->handle);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		result = !fsync(file->fd);
#	else
#		error Unsupported OS.
#	endif

	if (!result)
		Stdio_File_UpdateLastError(file);

	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(Close)
{
	SmileHandle handle = (SmileHandle)argv[0].obj;
	FileInfo fileInfo = (FileInfo)param;
	Stdio_File file = GetFileFromHandle(handle, (FileInfo)param, "File.close");

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);

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

//-------------------------------------------------------------------------------------------------
// Seek and tell.

SMILE_EXTERNAL_FUNCTION(Tell)
{
	Int64 pos;
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.tell");

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);

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

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);

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
	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);
	SeekForReal(file, 0, SEEK_SET);
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(SeekSet)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.seek-set");
	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);
	SeekForReal(file, argv[1].unboxed.i64, SEEK_SET);
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(SeekCur)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.seek-cur");
	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);
	SeekForReal(file, argv[1].unboxed.i64, SEEK_CUR);
	return argv[0];
}

SMILE_EXTERNAL_FUNCTION(SeekEnd)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.seek-end");
	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);
	SeekForReal(file, argv[1].unboxed.i64, SEEK_END);
	return argv[0];
}

//-------------------------------------------------------------------------------------------------
// Read and write.

SMILE_EXTERNAL_FUNCTION(ReadByte)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.read-byte");
	Byte buffer[1];
	int count;

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);
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
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);
	}
}

static SmileArg WriteByteInternal(Stdio_File file, FileInfo fileInfo, Byte byte)
{
	Byte buffer[1];
	int count;

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);
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
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);
	}
}

static SmileArg WriteBytesInternal(Stdio_File file, FileInfo fileInfo, const Byte *buffer, UInt32 length)
{
	int count;

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);
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
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);
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

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);
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

		// Explicitly *not* the `error symbol, due to the way loops are often written.
		// Symbols are truthy, so if we returned a symbol, an expression like
		// 'while file read buffer do [something]' would continue forever if there
		// was an error.
		return SmileArg_From(NullObject);
	}

	if (count == 0)
		file->isEof = True;

	file->lastErrorCode = 0;
	file->lastErrorMessage = String_Empty;
	return SmileUnboxedInteger64_From(count);
}

SMILE_EXTERNAL_FUNCTION(Write)
{
	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.write");
	Int64 start, length, count;
	SmileByteArray byteArray;
	Byte *buffer;

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(((FileInfo)param)->ioSymbols->closed);
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

		// Explicitly *not* the `error symbol, for parallelism with the implementation
		// of read above.
		return SmileArg_From(NullObject);
	}
	else {
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
		return SmileUnboxedInteger64_From(count);
	}
}

//-------------------------------------------------------------------------------------------------
// Remove and rename.

SMILE_EXTERNAL_FUNCTION(Rename)
{
	String oldName = (String)argv[0].obj;
	String newName = (String)argv[1].obj;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		Int oldName16Length, newName16Length;
		UInt16 *oldName16 = Stdio_ToWindowsPath(oldName, &oldName16Length);
		UInt16 *newName16 = Stdio_ToWindowsPath(newName, &newName16Length);
		Bool result = !!MoveFileW(oldName16, newName16);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		Bool result = !rename(String_ToC(oldName), String_ToC(newName));
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Remove)
{
	String name = (String)argv[0].obj;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		Int name16Length;
		UInt16 *name16 = Stdio_ToWindowsPath(name, &name16Length);
		Bool result = !!DeleteFileW(name16);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		Bool result = !unlink(String_ToC(name));
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedBool_From(result);
}

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
	static struct StringDictStruct _pathExtTable = { 0 };

	static void Stdio_InitializePathExtTable(void)
	{
		static UInt16 pathExt[] = { 'P', 'A', 'T', 'H', 'E', 'X', 'T', 0 };
		UInt16 stackBuffer[256];
		Int32 pathExtBufferSize = 255;
		UInt16 *pathExtBuffer = stackBuffer;
		Int32 result;
		String pathExtVariable;
		String *pieces;
		Int i, numPieces;
		String semicolon;

		// We will shortly populate this, but by initializing it first, we make sure that
		// the caller knows when we've already parsed the PATHEXT environment variable.
		StringDict_Clear(&_pathExtTable);

		// First, ask Windows for the PATHEXT environment variable.
		//
		// A typical Windows PATHEXT environment variable looks like this:
		//     PATHEXT=.COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC
		//
		// We take that, split it on semicolons, and make a hash table out
		// of it so that we can quickly look up a file by its extension to see
		// if Windows thinks it's an executable.  This is needed to correctly
		// simulate the 'execute' mode bit on Windows for the 'File.get-mode'
		// method.
	retry:
		result = GetEnvironmentVariableW(pathExt, pathExtBuffer, pathExtBufferSize);
		if (result < 0)
			return;
		if (result > pathExtBufferSize) {
			pathExtBuffer = GC_MALLOC_RAW_ARRAY(Int16, result + 1);
			pathExtBufferSize = result;
			goto retry;
		}

		// Transform Windows's UTF-16-encoded buffer into a UTF-8-encoded String.
		pathExtBuffer[pathExtBufferSize] = '\0';
		pathExtVariable = String_FromUtf16(pathExtBuffer, WStrLen(pathExtBuffer));

		// Split the environment-variable String on semicolons.
		semicolon = String_FromC(";");
		numPieces = String_Split(String_CaseFold(pathExtVariable), semicolon, &pieces);

		// Spin over each of the extensions, and add them to the _pathExtTable
		// dictionary so that they can be rapidly tested against filenames.
		for (i = 0; i < numPieces; i++) {
			StringDict_Add(&_pathExtTable, pieces[i], NULL);
		}
	}

	static Bool Stdio_IsExecutable(String name)
	{
		String ext;

		if (_pathExtTable._opaque.mask == 0)
			Stdio_InitializePathExtTable();

		// Extract out the filename extension, and case-fold it for comparison.
		ext = String_CaseFold(Path_GetExt(name));

		// See if that's a known executable extension.
		return StringDict_ContainsKey(&_pathExtTable, ext);
	}
#endif

//-------------------------------------------------------------------------------------------------
// Getting and setting file modes / attributes.

SMILE_EXTERNAL_FUNCTION(GetMode)
{
	Int64 result;
	String name = (String)argv[0].obj;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		Int name16Length;
		UInt16 *name16 = Stdio_ToWindowsPath(name, &name16Length);
		WIN32_FILE_ATTRIBUTE_DATA attrData;
		if (!GetFileAttributesExW(name16, GetFileExInfoStandard, &attrData))
			return SmileUnboxedBool_From(False);

		result = 0;
		if (attrData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE)
			result |= S_IFBLK;
		else if (attrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			result |= S_IFDIR | 0111;	// Directories are effectively always executable.
		else if (attrData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			result |= S_IFLNK;			// Reparse points aren't *always* soft links... but they *usually* are.
		else {
			result |= S_IFREG;
			if (Stdio_IsExecutable(name))
				result |= 0111;			// Has an executable extension, so mark it with executable bits.
		}
		result |= (attrData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			? 0444 : 0644;				// Acceptably-equivalent mode-bit behavior.

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		struct stat statbuf;
		if (lstat(String_ToC(name), &statbuf))
			return SmileUnboxedBool_From(False);
		result = (Int64)statbuf.st_mode;
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(SetMode)
{
	Int64 mode = 0;
	String name = (String)argv[0].obj;
	Bool result;

	switch (SMILE_KIND(argv[1].obj)) {
		case SMILE_KIND_UNBOXED_BYTE:
			mode = (Int64)(UInt64)(UInt8)argv[1].unboxed.i8;
			break;
		case SMILE_KIND_UNBOXED_INTEGER16:
			mode = (Int64)(UInt64)(UInt16)argv[1].unboxed.i16;
			break;
		case SMILE_KIND_UNBOXED_INTEGER32:
			mode = (Int64)(UInt64)(UInt32)argv[1].unboxed.i32;
			break;
		case SMILE_KIND_UNBOXED_INTEGER64:
			mode = argv[1].unboxed.i64;
			break;
		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_FromC("Argument 2 to File.set-mode must be of an Integer type."));
			break;
	}

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		Int name16Length;
		UInt16 *name16 = Stdio_ToWindowsPath(name, &name16Length);
		UInt32 attr = ((mode & 0222) != 0 ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_READONLY);
		result = !!SetFileAttributesW(name16, attr);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		result = !chmod(String_ToC(name), mode);
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedBool_From(result);
}

//-------------------------------------------------------------------------------------------------
// Getting and setting file sizes.

SMILE_EXTERNAL_FUNCTION(GetSize)
{
	Int64 result;
	String name = (String)argv[0].obj;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		Int name16Length;
		UInt16 *name16;
		HANDLE handle;
		LARGE_INTEGER fileSize;

		name16 = Stdio_ToWindowsPath(name, &name16Length);
		handle = CreateFileW(name16, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle == NULL || handle == INVALID_HANDLE_VALUE)
			return SmileUnboxedBool_From(False);
		if (!GetFileSizeEx(handle, &fileSize)) {
			CloseHandle(handle);
			return SmileUnboxedBool_From(False);
		}
		CloseHandle(handle);

		result = fileSize.QuadPart;
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		struct stat statbuf;
		if (lstat(String_ToC(name), &statbuf))
			return SmileUnboxedBool_From(False);
		result = (Int64)statbuf.st_size;
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedInteger64_From(result);
}

SMILE_EXTERNAL_FUNCTION(SetSize)
{
	Bool result;
	String name = (String)argv[0].obj;
	Int64 length = argv[1].unboxed.i64;

	if (length < 0) length = 0;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		// On Windows, we have to open the file, seek to where we want the file to end,
		// and then call SetEndOfFile(), which will either shrink or expand the file.
		// Unfortunately, per Windows' documentation, if it expands, there's no guarantee
		// as to what will be inside the empty space, and if it wasn't created as a sparse
		// file, the expansion will likely take a while.  This makes for a much less pleasant
		// implementation on Windows than on Unix, but it's passable.

		HANDLE handle;
		Int name16Length;
		UInt16 *name16;
		LARGE_INTEGER distanceToMove;

		name16 = Stdio_ToWindowsPath(name, &name16Length);

		handle = CreateFileW(name16, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL);
		if (handle == NULL)
			return SmileUnboxedBool_From(False);

		distanceToMove.LowPart = (Int32)length;
		distanceToMove.HighPart = (Int32)((UInt64)length >> 32);
		if (!SetFilePointerEx(handle, distanceToMove, NULL, FILE_BEGIN)) {
			CloseHandle(handle);
			return SmileUnboxedBool_From(False);
		}

		result = !!SetEndOfFile(handle);

		CloseHandle(handle);

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		// Unix truncate() can both shrink and expand files, and when a file expands, the
		// filesystem may even allocate it automatically as sparse storage; either way,
		// the spec guarantees that the file will appear to have '\0' bytes at the end.
		// Pretty sweet.

		result = !truncate(String_ToC(name), (off_t)length);
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedBool_From(result);
}

SMILE_EXTERNAL_FUNCTION(Truncate)
{
	SmileArg argvNew[2];

	argvNew[0] = argv[0];
	argvNew[1] = SmileUnboxedInteger64_From(0);

	return SetSize(2, argvNew, param);
}

//-------------------------------------------------------------------------------------------------
// Getting and setting file times.

enum {
	CREATE_TIME = 1,
	MODIFY_TIME = 2,
	ACCESS_TIME = 3,
};

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
	Inline Bool GetUnixTime(const char *name, Int timeType, Int64 *seconds, Int32 *nanoseconds)
	{
		struct stat statBuf;

		if (lstat(name, &statBuf))
			return False;

#		if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
			switch (timeType) {
				case CREATE_TIME:
					*seconds = (Int64)statBuf.st_ctim.tv_sec;
					*nanoseconds = (Int32)statBuf.st_ctim.tv_nsec;
					break;
				default:
				case MODIFY_TIME:
					*seconds = (Int64)statBuf.st_mtim.tv_sec;
					*nanoseconds = (Int32)statBuf.st_mtim.tv_nsec;
					break;
				case ACCESS_TIME:
					*seconds = (Int64)statBuf.st_atim.tv_sec;
					*nanoseconds = (Int32)statBuf.st_atim.tv_nsec;
					break;
			}
#		else
			switch (timeType) {
				case CREATE_TIME:
					*seconds = (Int64)statBuf.st_ctime;
					break;
				default:
				case MODIFY_TIME:
					*seconds = (Int64)statBuf.st_mtime;
					break;
				case ACCESS_TIME:
					*seconds = (Int64)statBuf.st_atime;
					break;
			}
			*nanoseconds = 0;
#		endif

		return True;
	}
#endif

SMILE_EXTERNAL_FUNCTION(GetTime)
{
	SmileTimestamp result;
	String name = (String)argv[0].obj;
	PtrInt timeType = (PtrInt)param;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)

		Int name16Length;
		UInt16 *name16 = Stdio_ToWindowsPath(name, &name16Length);
		WIN32_FILE_ATTRIBUTE_DATA attrData;
		FILETIME *fileTime;

		if (!GetFileAttributesExW(name16, GetFileExInfoStandard, &attrData))
			return SmileUnboxedBool_From(False);

		switch (timeType) {
			case CREATE_TIME:
				fileTime = &attrData.ftCreationTime;
				break;
			default:
			case MODIFY_TIME:
				fileTime = &attrData.ftLastWriteTime;
				break;
			case ACCESS_TIME:
				fileTime = &attrData.ftLastAccessTime;
				break;
		}

		result = SmileTimestamp_FromWindows(((Int64)fileTime->dwHighDateTime << 32) | fileTime->dwLowDateTime);

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		Int64 seconds;
		Int32 nanoseconds;

		if (!GetUnixTime(String_ToC(name), (Int)timeType, &seconds, &nanoseconds))
			return SmileUnboxedBool_From(False);

		result = SmileTimestamp_FromUnix(seconds, nanoseconds);
#	else
#		error Unsupported OS.
#	endif

	return SmileArg_From((SmileObject)result);
}

SMILE_EXTERNAL_FUNCTION(SetTime)
{
	Bool result;
	String name = (String)argv[0].obj;
	SmileTimestamp timestamp = (SmileTimestamp)argv[1].obj;
	PtrInt timeType = (PtrInt)param;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)

		Int name16Length;
		UInt16 *name16;
		HANDLE handle;
		FILETIME time;
		UInt64 ticks;
		
		name16 = Stdio_ToWindowsPath(name, &name16Length);

		ticks = SmileTimestamp_ToWindows(timestamp);
		time.dwLowDateTime = (UInt32)ticks;
		time.dwHighDateTime = (UInt32)(ticks >> 32);

		handle = CreateFileW(name16, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, 0, NULL);
		if (handle == NULL)
			return SmileUnboxedBool_From(False);

		switch (timeType) {
			case CREATE_TIME:
				result = !!SetFileTime(handle, &time, NULL, NULL);
				break;
			default:
			case MODIFY_TIME:
				result = !!SetFileTime(handle, NULL, NULL, &time);
				break;
			case ACCESS_TIME:
				result = !!SetFileTime(handle, NULL, &time, NULL);
				break;
		}

		CloseHandle(handle);

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)

		const char *pathname = String_ToC(name);

#		if _POSIX_C_SOURCE >= 200809L
			// On newer Unixes, we can use utimensat() to update just the correct field,
			// and update it to nanosecond precision.  This is likely at least any system
			// from the last ten years, if not longer.
			struct timespec times[2];

			switch (timeType) {
				case CREATE_TIME:
					return SmileUnboxedBool_From(False);
				default:
				case MODIFY_TIME:
					times[0].tv_nsec = UTIME_OMIT;
					times[1].tv_sec = (time_t)SmileTimestamp_ToUnix(timestamp);
					times[1].tv_nsec = timestamp->nanos;
					result = !utimensat(AT_FDCWD, pathname, times, AT_SYMLINK_NOFOLLOW);
					break;
				case ACCESS_TIME:
					times[1].tv_nsec = UTIME_OMIT;
					times[0].tv_sec = (time_t)SmileTimestamp_ToUnix(timestamp);
					times[0].tv_nsec = timestamp->nanos;
					result = !utimensat(AT_FDCWD, pathname, times, AT_SYMLINK_NOFOLLOW);
					break;
			}
#		else
			// On older Unixes, we're stuck with the utimes() interface, which loses precision.
			// Really really old Unixes might only support utime(), which we don't bother supporting
			// because it's very limited and loses even *more* precision.
			Int64 aSeconds, mSeconds;
			Int32 aNanoseconds, mNanoseconds;
			struct timeval times[2];

			switch (timeType) {
				case CREATE_TIME:
					return SmileUnboxedBool_From(False);
				default:
				case MODIFY_TIME:
					if (!GetUnixTime(String_ToC(name), ACCESS_TIME, &aSeconds, &aNanoseconds))
						return SmileUnboxedBool_From(False);
					times[0].tv_sec = (time_t)aSeconds;
					times[0].tv_usec = (int)(aNanoseconds / 1000);
					times[1].tv_sec = (time_t)SmileTimestamp_ToUnix(timestamp);
					times[1].tv_usec = (int)(timestamp->nanos / 1000);
					result = !utimes(pathname, times);
					break;
				case ACCESS_TIME:
					if (!GetUnixTime(String_ToC(name), MODIFY_TIME, &mSeconds, &mNanoseconds))
						return SmileUnboxedBool_From(False);
					times[0].tv_sec = (time_t)SmileTimestamp_ToUnix(timestamp);
					times[0].tv_usec = (int)(timestamp->nanos / 1000);
					times[1].tv_sec = (time_t)mSeconds;
					times[1].tv_usec = (int)(mNanoseconds / 1000);
					result = !utimes(pathname, times);
					break;
			}
#		endif
#	else
#		error Unsupported OS.
#	endif

	return SmileUnboxedBool_From(result);
}

//-------------------------------------------------------------------------------------------------
// Read-line (and, nominally, write-line).
//
// TODO:  This is a very inefficient implementation.  It's logically correct, given
//    that Files are very "raw" objects, but since we don't attempt to buffer anything
//    in memory, and we don't want to seek to run afoul of blocking on unusual file types,
//    we simply implement read-line as a byte-by-byte search for a '\n' character in the
//    stream, which means that read-line here is a couple orders of magnitude slower
//    than, say, a typical implementation of fgets().
//
//    A smarter future implementation would detect types of input streams and perform
//    clever buffering and stuff like that.

Inline SmileArg ReadLineCommon(Stdio_File file, FileInfo fileInfo, Bool keepNewline)
{
	Bool gotNewline = False;
	Byte buffer[1];
	int count;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 1024);

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	if (!file->isOpen)
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);
	if (file->mode & FILE_MODE_STD)
		Stdio_Invoked = True;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
	retry:
		count = _read(file->fd, buffer, 1);
		if (count > 0) {
			StringBuilder_AppendByte(stringBuilder, buffer[0]);
			if (buffer[0] != '\n')
				goto retry;
			gotNewline = True;
		}
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
	retry:
		 count = read(file->fd, buffer, 1);
		 if (count > 0) {
			 StringBuilder_AppendByte(stringBuilder, buffer[0]);
			 if (buffer[0] != '\n')
				 goto retry;
			 gotNewline = True;
		}
#	else
#		error Unsupported OS.
#	endif

	if (!keepNewline && gotNewline) {
		// Strip off the trailing newline, since we read one.
		//
		// Also, if there's a preceding '\r', strip that too.  This allows newlines to be of
		// either of the forms '\r\n' or '\n' --- but not '\r' or '\r\n' --- to be recognized.
		// This is a break with the way Smile usually handles newlines, but making all four
		// forms be correctly recognized is basically impossible when you have no way to
		// buffer the input stream or "unget" previously-read bytes.
		Int length = StringBuilder_GetLength(stringBuilder);
		if (length > 1 && StringBuilder_At(stringBuilder, length - 2) == '\r')
			StringBuilder_SetLength(stringBuilder, length - 2);
		else
			StringBuilder_SetLength(stringBuilder, length - 1);
	}

	if (count >= 0) {
		if (StringBuilder_GetLength(stringBuilder) > 0) {
			file->lastErrorCode = 0;
			file->lastErrorMessage = String_Empty;
			return SmileArg_From((SmileObject)StringBuilder_ToString(stringBuilder));
		}
		else {
			file->isEof = True;
			file->lastErrorCode = 0;
			file->lastErrorMessage = String_Empty;
			return SmileArg_From(NullObject);
		}
	}
	else {
		Stdio_File_UpdateLastError(file);
		return SmileUnboxedSymbol_From(fileInfo->ioSymbols->closed);
	}
}

SMILE_EXTERNAL_FUNCTION(ReadRawLine)
{
	Stdio_File file;
	file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.read-raw-line");

	return ReadLineCommon(file, (FileInfo)param, True);
}

SMILE_EXTERNAL_FUNCTION(ReadLine)
{
	Stdio_File file;
	file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.read-line");

	return ReadLineCommon(file, (FileInfo)param, False);
}

SMILE_EXTERNAL_FUNCTION(WriteRawLine)
{
	SmileArg newArgv[2];

	newArgv[0] = argv[0];
	newArgv[1] = SmileArg_From((SmileObject)String_ToByteArray((String)argv[1].obj));

	return Write(2, newArgv, param);
}

SMILE_EXTERNAL_FUNCTION(WriteLine)
{
	SmileArg newArgv[2];
	SmileArg result;

	newArgv[0] = argv[0];
	newArgv[1] = SmileArg_From((SmileObject)String_ToByteArray((String)argv[1].obj));

	result = Write(2, newArgv, param);
	if (result.obj == NullObject) return result;	// Encountered an error.

	Stdio_File file = GetFileFromHandle((SmileHandle)argv[0].obj, (FileInfo)param, "File.write-line");
	return WriteByteInternal(file, (FileInfo)param, '\n');
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
	SetupFunction("flush", Flush, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupSynonym("write-byte", "print-byte");
	SetupSynonym("write-char", "print-char");
	SetupSynonym("write-uni", "print-uni");
	SetupFunction("read-line", ReadLine, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupFunction("read-raw-line", ReadRawLine, (void *)fileInfo, "file", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _handleChecks);
	SetupSynonym("read-raw-line", "get-line");
	SetupFunction("write-line", WriteLine, (void *)fileInfo, "file string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _handleStringChecks);
	SetupFunction("write-raw-line", WriteRawLine, (void *)fileInfo, "file string", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _handleStringChecks);

	SetupFunction("rename", Rename, (void *)fileInfo, "old-name new-name", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _stringChecks);
	SetupSynonym("rename", "move");
	SetupSynonym("rename", "mv");

	SetupFunction("remove", Remove, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupSynonym("remove", "delete");
	SetupSynonym("remove", "unlink");
	SetupSynonym("remove", "rm");

	SetupFunction("get-mode", GetMode, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupSynonym("get-mode", "mode");
	SetupFunction("set-mode", SetMode, (void *)fileInfo, "path mode...", ARG_CHECK_MIN | ARG_CHECK_TYPES, 1, 0, 2, _modeChecks);
	SetupSynonym("set-mode", "chmod");

	SetupFunction("get-size", GetSize, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _stringChecks);
	SetupSynonym("get-size", "size");
	SetupFunction("set-size", SetSize, (void *)fileInfo, "path size", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 2, 2, _seekChecks);
	SetupFunction("truncate", Truncate, (void *)fileInfo, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _seekChecks);
	SetupSynonym("truncate", "trunc");

	SetupFunction("get-create-time", GetTime, (void *)(PtrInt)CREATE_TIME, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _timeChecks);
	SetupSynonym("get-create-time", "create-time");
	SetupFunction("set-create-time", SetTime, (void *)(PtrInt)CREATE_TIME, "path time", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timeChecks);
	SetupFunction("get-modify-time", GetTime, (void *)(PtrInt)MODIFY_TIME, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _timeChecks);
	SetupSynonym("get-modify-time", "modify-time");
	SetupSynonym("get-modify-time", "get-time");
	SetupSynonym("get-modify-time", "time");
	SetupFunction("set-modify-time", SetTime, (void *)(PtrInt)MODIFY_TIME, "path time", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timeChecks);
	SetupSynonym("set-modify-time", "set-time");
	SetupFunction("get-access-time", GetTime, (void *)(PtrInt)ACCESS_TIME, "path", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 1, 1, 1, _timeChecks);
	SetupSynonym("get-access-time", "access-time");
	SetupFunction("set-access-time", SetTime, (void *)(PtrInt)ACCESS_TIME, "path time", ARG_CHECK_EXACT | ARG_CHECK_TYPES, 2, 2, 2, _timeChecks);
}
