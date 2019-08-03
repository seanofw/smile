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
#include <stdlib.h>

#include <smile/internal/types.h>
#include <smile/gc.h>
#include <smile/env/env.h>
#include <smile/env/symboltable.h>
#include <smile/env/knownsymbols.h>
#include <smile/env/knownobjects.h>
#include <smile/env/knownbases.h>
#include <smile/smiletypes/smileuserobject.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/internal/staticstring.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/parseinclude.h>
#include <smile/eval/eval.h>
#include <smile/stringbuilder.h>

#include "stdio_internal.h"

STATIC_STRING(_stdioFilename, "stdio");

STATIC_STRING(_stdioBootstrap,
	"File.print = |file exprs...| file print-list exprs\n"
	"\n"
	"File.print-line = |file exprs...| file print-list-line exprs\n"
	"\n"
	"File.print-list = |file list| {\n"
		"str = join list\n"
		"file write byte-array str\n"
	"}\n"
	"\n"
	"File.print-list-line = |file list| {\n"
		"file print-list list\n"
		"file write-char '\\n'\n"
	"}\n"
	"\n"
	"#syntax STMT: [print [EXPR+ exprs ,]] => `[Stdout.print @(exprs)]\n"
	"#syntax STMT: [print-line [EXPR+ exprs ,]] => `[Stdout.print-line @(exprs)]\n"
	"\n"
);

UInt32 Stdio_ParseModeArg(SmileArg arg, const char *methodName)
{
	switch (SMILE_KIND(arg.obj)) {
		case SMILE_KIND_UNBOXED_INTEGER64:
			return (UInt32)arg.unboxed.i64;
		case SMILE_KIND_UNBOXED_INTEGER32:
			return (UInt32)arg.unboxed.i32;
		case SMILE_KIND_UNBOXED_INTEGER16:
			return (UInt32)arg.unboxed.i16;
		case SMILE_KIND_UNBOXED_BYTE:
			return (UInt32)arg.unboxed.i8;

		default:
			Smile_ThrowException(Smile_KnownSymbols.native_method_error,
				String_Format("Argument to '%s' must be an integer containing 'rwx'-style file-mode bits.", methodName));
	}
}

/// <summary>
/// Convert the given path, represented a UTF-8-encoded String object, to a Windows-style
/// buffer of wide UTF-16 characters, transforming all '/' into Windows-style '\' separators.
/// </summary>
/// <param name="path">The input path to convert.</param>
/// <param name="length">This will be set to the length of the returned UTF-16 array.</param>
/// <returns>A newly-allocated array containing the UTF-16-equivalent version of the same path,
/// with all forward slashes turned into backslashes.</returns>
UInt16 *Stdio_ToWindowsPath(String path, Int *length)
{
	// Convert the string from UTF-8 to UTF-16.
	UInt16 *buffer = String_ToUtf16(path, length);
	UInt16 *end = buffer + *length;
	UInt16 *cur;

	// Windows wants backslashes in the path, not forward slashes.
	for (cur = buffer; cur < end; cur++) {
		if (*cur == '/')
			*cur = '\\';
	}

	return buffer;
}

/// <summary>
/// Convert the given path, represented as an Windows-style array of UTF-16 characters,
/// to a String path that uses UTF-8 encoding and all '/' instead of '\' separators.
/// </summary>
/// <param name="buffer">The start of the buffer of UTF-16 characters to convert.</param>
/// <param name="length">The number of UTF-16 "wide" characters in the input buffer.</param>
/// <returns>A UTF-8-encoded path, with all backslashes turned into forward slashes.</returns>
String Stdio_FromWindowsPath(UInt16 * buffer, Int length)
{
	// Convert from Windows's preferred UTF-16 to an equivalent UTF-8 encoding.
	String result = String_FromUtf16(buffer, length);

	// Windows uses backslashes in the path, so we convert them to forward slashes
	// for consistency across platforms.
	return String_ReplaceChar(result, '\\', '/');
}

static IoSymbols Stdio_CreateIoSymbols(void)
{
	IoSymbols ioSymbols = GC_MALLOC_STRUCT(struct IoSymbolsStruct);
	if (ioSymbols == NULL)
		Smile_Abort_OutOfMemory();

	ioSymbols->File = SymbolTable_GetSymbolC(Smile_SymbolTable, "File");
	ioSymbols->read = SymbolTable_GetSymbolC(Smile_SymbolTable, "read");
	ioSymbols->reading = SymbolTable_GetSymbolC(Smile_SymbolTable, "reading");
	ioSymbols->read_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "read-only");
	ioSymbols->write = SymbolTable_GetSymbolC(Smile_SymbolTable, "write");
	ioSymbols->writing = SymbolTable_GetSymbolC(Smile_SymbolTable, "writing");
	ioSymbols->write_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "write-only");
	ioSymbols->append = SymbolTable_GetSymbolC(Smile_SymbolTable, "append");
	ioSymbols->appending = SymbolTable_GetSymbolC(Smile_SymbolTable, "appending");
	ioSymbols->append_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "append-only");
	ioSymbols->read_write = SymbolTable_GetSymbolC(Smile_SymbolTable, "read-write");
	ioSymbols->read_append = SymbolTable_GetSymbolC(Smile_SymbolTable, "read-append");
	ioSymbols->trunc = SymbolTable_GetSymbolC(Smile_SymbolTable, "trunc");
	ioSymbols->truncate = SymbolTable_GetSymbolC(Smile_SymbolTable, "truncate");
	ioSymbols->create = SymbolTable_GetSymbolC(Smile_SymbolTable, "create");
	ioSymbols->open = SymbolTable_GetSymbolC(Smile_SymbolTable, "open");
	ioSymbols->create_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "create-only");
	ioSymbols->open_only = SymbolTable_GetSymbolC(Smile_SymbolTable, "open-only");
	ioSymbols->create_or_open = SymbolTable_GetSymbolC(Smile_SymbolTable, "create-or-open");

	ioSymbols->closed = SymbolTable_GetSymbolC(Smile_SymbolTable, "closed");
	ioSymbols->error = SymbolTable_GetSymbolC(Smile_SymbolTable, "error");

	ioSymbols->set = SymbolTable_GetSymbolC(Smile_SymbolTable, "set");
	ioSymbols->start = SymbolTable_GetSymbolC(Smile_SymbolTable, "start");
	ioSymbols->cur = SymbolTable_GetSymbolC(Smile_SymbolTable, "cur");
	ioSymbols->current = SymbolTable_GetSymbolC(Smile_SymbolTable, "current");
	ioSymbols->end = SymbolTable_GetSymbolC(Smile_SymbolTable, "end");
	ioSymbols->seek_set = SymbolTable_GetSymbolC(Smile_SymbolTable, "seek-set");
	ioSymbols->seek_cur = SymbolTable_GetSymbolC(Smile_SymbolTable, "seek-cur");
	ioSymbols->seek_end = SymbolTable_GetSymbolC(Smile_SymbolTable, "seek-end");

	return ioSymbols;
}

ModuleInfo Stdio_Main(void)
{
	ParseMessage *parseMessages;
	Int numParseMessages;
	SmileObject expr;
	SmileUserObject fileBase, dirBase, pathBase;
	ParseScope moduleScope;
	ExternalVar vars[8];
	Int numVars;
	IoSymbols ioSymbols = Stdio_CreateIoSymbols();

	STATIC_STRING(stdioName, "stdio");

	fileBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "File"));
	dirBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "Dir"));
	pathBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "Path"));

	Stdio_File_Init(fileBase, ioSymbols);
	Stdio_Dir_Init(dirBase, ioSymbols);
	Stdio_Path_Init(pathBase);

	numVars = 0;
	vars[numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "File");
	vars[numVars++].obj = (SmileObject)fileBase;
	vars[numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "Dir");
	vars[numVars++].obj = (SmileObject)dirBase;
	vars[numVars].symbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "Path");
	vars[numVars++].obj = (SmileObject)pathBase;
	Stdio_File_DeclareStdInOutErr(vars, &numVars, (SmileObject)fileBase);

	expr = Smile_ParseInScope(_stdioBootstrap, _stdioFilename, vars, numVars, &parseMessages, &numParseMessages, &moduleScope);

	return ModuleInfo_Create(stdioName, numParseMessages == 0, expr, moduleScope, parseMessages, numParseMessages);
}
