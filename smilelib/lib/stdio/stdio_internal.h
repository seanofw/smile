#ifndef __SMILE_LIB_STDIO_INTERNAL_H__
#define __SMILE_LIB_STDIO_INTERNAL_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif

#ifndef __SMILE_SMILETYPES_SMILEOBJECT_H__
#include <smile/smiletypes/smileobject.h>
#endif

#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#include <stdio.h>

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)

#	define WIN32_LEAN_AND_MEAN
#	pragma warning(push)
#	pragma warning(disable: 4255)
#	include <windows.h>
#	include <fcntl.h>
#	include <io.h>
#	pragma warning(pop)

	typedef struct Stdio_FileStruct {
		String path;
		UInt32 mode;
		UInt32 lastErrorCode;
		String lastErrorMessage;
		Bool isOpen;
		Bool isEof;
		Int32 fd;

		HANDLE handle;
	} *Stdio_File;

	SMILE_INTERNAL_FUNC SmileHandle Stdio_File_CreateFromWin32Handle(SmileObject base, String name, HANDLE handle, UInt32 mode);

#elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)

#	include <sys/types.h>
#	include <unistd.h>
#	include <errno.h>
#	include <limits.h>

	typedef struct Stdio_FileStruct {
		String path;
		UInt32 mode;
		UInt32 lastErrorCode;
		String lastErrorMessage;
		Bool isOpen;
		Bool isEof;
		Int32 fd;
	} *Stdio_File;

	SMILE_INTERNAL_FUNC SmileHandle Stdio_File_CreateFromUnixFD(SmileObject base, String name, Int32 fd, UInt32 mode);

#else
#	error Unsupported OS.
#endif

typedef enum {
	FILE_MODE_READ = (1 << 0),
	FILE_MODE_WRITE = (1 << 1),
	FILE_MODE_APPEND = (1 << 2),	// Set write pointer to end of file before each write (requires FILE_MODE_WRITE)
	FILE_MODE_TRUNCATE = (1 << 3),	// At opening, delete any existing data (requires FILE_MODE_WRITE)

	FILE_MODE_OPEN_MASK = (0xF << 4),	// How to open the file
	FILE_MODE_CREATE_ONLY = (1 << 4),	// At opening, create it if it doesn't exist, fail if it does
	FILE_MODE_OPEN_ONLY = (2 << 4),	// At opening, fail if it doesn't exist, open if it does
	FILE_MODE_CREATE_OR_OPEN = (3 << 4),	// At opening, create if it doesn't exist, open if it does

	FILE_MODE_STD = (1 << 8),	// This file is one of the three specials: Stdin, Stdout, Stderr
} Stdio_FileMode;

SMILE_INTERNAL_FUNC void Stdio_File_DeclareStdInOutErr(ExternalVar *vars, Int *numVars, SmileObject fileBase);
SMILE_INTERNAL_FUNC SmileHandle Stdio_File_CreateFromPath(SmileObject base, String path, UInt32 openMode, UInt32 newFileMode);
SMILE_INTERNAL_FUNC void Stdio_File_UpdateLastError(Stdio_File file);

SMILE_INTERNAL_FUNC void Stdio_File_Init(SmileUserObject base);
SMILE_INTERNAL_FUNC void Stdio_Dir_Init(SmileUserObject base);
SMILE_INTERNAL_FUNC void Stdio_Path_Init(SmileUserObject base);

#endif