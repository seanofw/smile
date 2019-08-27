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

#include <stdio.h>
#include <smile/smiletypes/easyobject.h>
#include <smile/smiletypes/smilefunction.h>
#include <smile/smiletypes/base.h>
#include <smile/smiletypes/smilehandle.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/internal/staticstring.h>

#include "stdio_internal.h"

STATIC_STRING(_stdinName, "<stdin>");
STATIC_STRING(_stdoutName, "<stdout>");
STATIC_STRING(_stderrName, "<stderr>");

static Bool Stdio_File_ToBool(SmileHandle handle, SmileUnboxedData unboxedData)
{
	Stdio_File file = (Stdio_File)handle->ptr;

	UNUSED(unboxedData);

	return file->isOpen;
}

/// <summary>
/// This will be invoked to convert the file to a string.
/// </summary>
static String Stdio_File_ToString(SmileHandle handle, SmileUnboxedData unboxedData)
{
	Stdio_File file = (Stdio_File)handle->ptr;
	UNUSED(unboxedData);
	return file->path;
}

/// <summary>
/// Get a property of the file.
/// </summary>
static SmileObject Stdio_File_GetProperty(SmileHandle handle, Symbol symbol)
{
	Stdio_File file = (Stdio_File)handle->ptr;
	IoSymbols ioSymbols = file->ioSymbols;

	if (symbol == ioSymbols->name)
		return (SmileObject)file->path;
	else if (symbol == ioSymbols->readable)
		return (file->mode &FILE_MODE_READ) ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
	else if (symbol == ioSymbols->writable)
		return (file->mode & FILE_MODE_WRITE) ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
	else if (symbol == ioSymbols->appendable)
		return (file->mode & FILE_MODE_APPEND) ? (SmileObject)Smile_KnownObjects.TrueObj : (SmileObject)Smile_KnownObjects.FalseObj;
	else
		return SMILE_VCALL1(handle->base, getProperty, symbol);
}

/// <summary>
/// Ask a file if it has the given property.
/// </summary>
static Bool Stdio_File_HasProperty(SmileHandle handle, Symbol symbol)
{
	Stdio_File file = (Stdio_File)handle->ptr;
	IoSymbols ioSymbols = file->ioSymbols;

	return (symbol == ioSymbols->name || symbol == ioSymbols->readable
		|| symbol == ioSymbols->writable || symbol == ioSymbols->appendable);
}

/// <summary>
/// Get a list of all property names of a file.
/// </summary>
static SmileList Stdio_File_GetPropertyNames(SmileHandle handle)
{
	Stdio_File file = (Stdio_File)handle->ptr;
	IoSymbols ioSymbols = file->ioSymbols;

	return SmileList_CreateList(
		(SmileObject)SmileSymbol_Create(ioSymbols->name),
		(SmileObject)SmileSymbol_Create(ioSymbols->readable),
		(SmileObject)SmileSymbol_Create(ioSymbols->writable),
		(SmileObject)SmileSymbol_Create(ioSymbols->appendable),
		NULL
	);
}

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)

	static Bool Stdio_File_Win32End(SmileHandle handle, Bool userInvoked)
	{
		Stdio_File file = (Stdio_File)handle->ptr;

		UNUSED(userInvoked);

		if (file->mode & FILE_MODE_STD) return False;

		if (file->fd > 0) {
			_close(file->fd);
			file->fd = 0;
		}

		return True;
	}

	static struct SmileHandleMethodsStruct Stdio_File_Methods = {
		.end = Stdio_File_Win32End,
		.toBool = Stdio_File_ToBool,
		.toString = Stdio_File_ToString,
		.getProperty = Stdio_File_GetProperty,
		.hasProperty = Stdio_File_HasProperty,
		.getPropertyNames = Stdio_File_GetPropertyNames,
	};

	/// <summary>
	/// Construct a real C FILE* for the given Win32 HANDLE and the mode in which it was opened.
	/// </summary>
	static Int32 GetFileDescriptorFromWin32Handle(HANDLE handle, UInt32 mode)
	{
		Int32 fd;

		if (mode & FILE_MODE_STD) {
			if (handle == GetStdHandle(STD_INPUT_HANDLE)) return 0;
			if (handle == GetStdHandle(STD_OUTPUT_HANDLE)) return 1;
			if (handle == GetStdHandle(STD_ERROR_HANDLE)) return 2;
		}

		// Get a C-style file descriptor for the handle.
		fd = _open_osfhandle((intptr_t)handle,
			  ((mode & FILE_MODE_APPEND) ? _O_APPEND : 0)
			| ((mode & FILE_MODE_WRITE) ? 0 : _O_RDONLY));

		return fd;
	}

	SMILE_INTERNAL_FUNC SmileHandle Stdio_File_CreateFromWin32Handle(SmileObject base, String name, HANDLE win32Handle, UInt32 mode, IoSymbols ioSymbols)
	{
		SmileHandle handle;
		Stdio_File file;

		// Make the Stdio_File object first.
		file = GC_MALLOC_STRUCT(struct Stdio_FileStruct);
		if (file == NULL)
			Smile_Abort_OutOfMemory();

		// Make a SmileHandle that wraps the Stdio_File.
		handle = SmileHandle_Create(base, &Stdio_File_Methods, SymbolTable_GetSymbolC(Smile_SymbolTable, "File"), file);

		// Fill in the Stdio_File with real data.
		file->path = name;
		file->mode = mode;
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
		file->isOpen = True;
		file->fd = GetFileDescriptorFromWin32Handle(win32Handle, mode);
		file->ioSymbols = ioSymbols;
		file->handle = win32Handle;

		return handle;
	}

	void Stdio_File_DeclareStdInOutErr(ExternalVar *vars, Int *numVars, SmileObject fileBase, IoSymbols ioSymbols)
	{
		SmileHandle stdinHandle = Stdio_File_CreateFromWin32Handle((SmileObject)fileBase, _stdinName, GetStdHandle(STD_INPUT_HANDLE), FILE_MODE_READ | FILE_MODE_STD, ioSymbols);
		SmileHandle stdoutHandle = Stdio_File_CreateFromWin32Handle((SmileObject)fileBase, _stdoutName, GetStdHandle(STD_OUTPUT_HANDLE), FILE_MODE_WRITE | FILE_MODE_APPEND | FILE_MODE_STD, ioSymbols);
		SmileHandle stderrHandle = Stdio_File_CreateFromWin32Handle((SmileObject)fileBase, _stderrName, GetStdHandle(STD_ERROR_HANDLE), FILE_MODE_WRITE | FILE_MODE_APPEND | FILE_MODE_STD, ioSymbols);

		vars[*numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "stdin");
		vars[(*numVars)++].obj = (SmileObject)stdinHandle;
		vars[*numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "stdout");
		vars[(*numVars)++].obj = (SmileObject)stdoutHandle;
		vars[*numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "stderr");
		vars[(*numVars)++].obj = (SmileObject)stderrHandle;
	}

	SmileHandle Stdio_File_CreateFromPath(SmileObject base, String path, UInt32 mode, UInt32 newFileMode, IoSymbols ioSymbols)
	{
		HANDLE win32Handle;
		wchar_t *path16;
		Int length;
		UInt32 desiredAccess, shareMode, creationDisposition, flagsAndAttributes;
		Stdio_File file;
		SmileHandle handle;

		UNUSED(newFileMode);

		// Convert the path to UTF-16, because that's what the Windows APIs need.
		path16 = (wchar_t *)Stdio_ToWindowsPath(path, &length);

		// Compute out the various mode and attribute bits Windows needs from our mode mask.
		desiredAccess = 0;
		shareMode = 0;
		creationDisposition = 0;
		flagsAndAttributes = 0;

		if (mode & FILE_MODE_READ) desiredAccess |= GENERIC_READ;
		if (mode & (FILE_MODE_WRITE | FILE_MODE_APPEND)) desiredAccess |= GENERIC_WRITE;

		switch (mode & (FILE_MODE_OPEN_MASK | FILE_MODE_TRUNCATE)) {
			case FILE_MODE_OPEN_ONLY:
				creationDisposition = OPEN_EXISTING;
				break;
			case FILE_MODE_CREATE_ONLY:
			case FILE_MODE_CREATE_ONLY | FILE_MODE_TRUNCATE:
				creationDisposition = CREATE_NEW;
				break;
			case FILE_MODE_CREATE_OR_OPEN:
				creationDisposition = OPEN_ALWAYS;
				break;
			case FILE_MODE_OPEN_ONLY | FILE_MODE_TRUNCATE:
				creationDisposition = TRUNCATE_EXISTING;
				break;
			case FILE_MODE_CREATE_OR_OPEN | FILE_MODE_TRUNCATE:
				creationDisposition = CREATE_ALWAYS;
				break;
			default:
				handle = Stdio_File_CreateFromWin32Handle(base, path, NULL, mode, ioSymbols);
				file = (Stdio_File)handle->ptr;
				file->isOpen = False;
				file->lastErrorCode = (UInt32)~0;
				file->lastErrorMessage = String_FromC("Invalid mode flags.");
				return handle;
		}

		flagsAndAttributes = FILE_ATTRIBUTE_NORMAL;

		// Open the file for real.
		win32Handle = CreateFileW((LPCWSTR)path16, desiredAccess, shareMode, NULL, creationDisposition, flagsAndAttributes, NULL);

		// Create a wrapper object around it, even if it's not open.
		handle = Stdio_File_CreateFromWin32Handle(base, path, win32Handle, mode, ioSymbols);
		file = (Stdio_File)handle->ptr;

		// Record any errors.
		if (win32Handle == NULL || win32Handle == INVALID_HANDLE_VALUE) {
			file->isOpen = False;
			Stdio_File_UpdateLastError(file);
		}

		return handle;
	}

#elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)

#	include <unistd.h>
#	include <fcntl.h>
#	include <sys/types.h>
#	include <sys/stat.h>

	static Bool Stdio_File_UnixEnd(SmileHandle handle, Bool userInvoked)
	{
		Stdio_File file = (Stdio_File)handle->ptr;

		UNUSED(userInvoked);

		if (file->mode & FILE_MODE_STD) return False;

		if (file->fd != 0) {
			close(file->fd);
			file->fd = 0;
		}

		return True;
	}

	static struct SmileHandleMethodsStruct Stdio_File_Methods = {
		.end = Stdio_File_UnixEnd,
		.toBool = Stdio_File_ToBool,
		.toString = Stdio_File_ToString,
		.getProperty = Stdio_File_GetProperty,
		.hasProperty = Stdio_File_HasProperty,
		.getPropertyNames = Stdio_File_GetPropertyNames,
	};

	SMILE_INTERNAL_FUNC SmileHandle Stdio_File_CreateFromUnixFD(SmileObject base, String name, Int32 fd, UInt32 mode, IoSymbols ioSymbols)
	{
		SmileHandle handle;
		Stdio_File file;
		
		// Make the Stdio_File object first.
		file = GC_MALLOC_STRUCT(struct Stdio_FileStruct);
		if (file == NULL)
			Smile_Abort_OutOfMemory();

		handle = SmileHandle_Create(base, &Stdio_File_Methods, SymbolTable_GetSymbolC(Smile_SymbolTable, "File"), file);

		// Fill in the Stdio_File with real data.
		file->path = name;
		file->mode = mode;
		file->lastErrorCode = 0;
		file->lastErrorMessage = String_Empty;
		file->isOpen = True;
		file->isEof = False;
		file->fd = fd;
		file->ioSymbols = ioSymbols;

		return handle;
	}

	SmileHandle Stdio_File_CreateFromPath(SmileObject base, String path, UInt32 mode, UInt32 newFileMode, IoSymbols ioSymbols)
	{
		Stdio_File file;
		SmileHandle handle;
		int openFlags;
		int fd;

		// Unix wants forward slashes in the path, not backslashes.
		if (String_IndexOfChar(path, '\\', 0) >= 0) {
			path = String_ReplaceChar(path, '/', '\\');
		}

		// Figure out whether we're creating or opening.
		switch (mode & (FILE_MODE_OPEN_MASK | FILE_MODE_TRUNCATE)) {
			case FILE_MODE_OPEN_ONLY:
				openFlags = 0;
				break;
			case FILE_MODE_CREATE_ONLY:
			case FILE_MODE_CREATE_ONLY | FILE_MODE_TRUNCATE:
				openFlags = O_CREAT | O_EXCL;
				break;
			case FILE_MODE_CREATE_OR_OPEN:
				openFlags = O_CREAT;
				break;
			case FILE_MODE_OPEN_ONLY | FILE_MODE_TRUNCATE:
				openFlags = O_TRUNC;
				break;
			case FILE_MODE_CREATE_OR_OPEN | FILE_MODE_TRUNCATE:
				openFlags = O_CREAT | O_TRUNC;
				break;
			default:
				handle = Stdio_File_CreateFromUnixFD(base, path, -1, mode, ioSymbols);
				file = (Stdio_File)handle->ptr;
				file->isOpen = False;
				file->lastErrorCode = (UInt32)~0;
				file->lastErrorMessage = String_FromC("Invalid mode flags.");
				return handle;
		}

		// Figure out whether this is read, write, or append I/O.
		switch (mode & (FILE_MODE_READ | FILE_MODE_WRITE | FILE_MODE_APPEND)) {
			case 0:
			case FILE_MODE_READ:
				openFlags |= O_RDONLY;
				break;
			case FILE_MODE_WRITE:
				openFlags |= O_WRONLY;
				break;
			case FILE_MODE_READ | FILE_MODE_WRITE:
				openFlags |= O_RDWR;
				break;
			case FILE_MODE_APPEND:
			case FILE_MODE_APPEND | FILE_MODE_WRITE:
				openFlags |= O_WRONLY | O_APPEND;
				break;
			case FILE_MODE_APPEND | FILE_MODE_READ:
			case FILE_MODE_APPEND | FILE_MODE_READ | FILE_MODE_WRITE:
				openFlags |= O_RDWR | O_APPEND;
				break;
		}

		// Do it.
		fd = open(String_ToC(path), openFlags, newFileMode);

		// Create a wrapper object around it, even if it's not open.
		handle = Stdio_File_CreateFromUnixFD(base, path, fd, mode, ioSymbols);
		file = (Stdio_File)handle->ptr;

		// Record any errors.
		if (fd < 0) {
			file->isOpen = False;
			Stdio_File_UpdateLastError(file);
		}

		return handle;
	}

	void Stdio_File_DeclareStdInOutErr(ExternalVar *vars, Int *numVars, SmileObject fileBase, IoSymbols ioSymbols)
	{
		SmileHandle stdinHandle = Stdio_File_CreateFromUnixFD((SmileObject)fileBase, _stdinName, STDIN_FILENO, FILE_MODE_READ | FILE_MODE_STD, ioSymbols);
		SmileHandle stdoutHandle = Stdio_File_CreateFromUnixFD((SmileObject)fileBase, _stdoutName, STDOUT_FILENO, FILE_MODE_WRITE | FILE_MODE_APPEND | FILE_MODE_STD, ioSymbols);
		SmileHandle stderrHandle = Stdio_File_CreateFromUnixFD((SmileObject)fileBase, _stderrName, STDERR_FILENO, FILE_MODE_WRITE | FILE_MODE_APPEND | FILE_MODE_STD, ioSymbols);

		vars[*numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "stdin");
		vars[(*numVars)++].obj = (SmileObject)stdinHandle;
		vars[*numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "stdout");
		vars[(*numVars)++].obj = (SmileObject)stdoutHandle;
		vars[*numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "stderr");
		vars[(*numVars)++].obj = (SmileObject)stdoutHandle;
	}

#else
#	error Unsupported OS.
#endif

