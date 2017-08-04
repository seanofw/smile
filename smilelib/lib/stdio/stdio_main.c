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
	"File.print = |file exprs...| {\n"
		"str = join exprs\n"
		"file write byte-array str\n"
	"}\n"
	"\n"
	"File.print-line = |file exprs...| {\n"
		"str = join exprs\n"
		"file write byte-array str\n"
		"file write-byte '\\n'\n"
	"}\n"
	/*"\n"
	"#syntax STMT [print [EXPR+ exprs ,]] => (Stdout.print ## exprs)\n"
	"\n"*/
);

LibraryInfo Stdio_Main(void)
{
	ClosureInfo globalClosureInfo;
	Closure globalClosure;
	ParseMessage *parseMessages;
	Int numParseMessages;
	SmileObject parseResult;
	SmileUserObject fileBase, dirBase, pathBase;

	STATIC_STRING(stdioName, "stdio");

	fileBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "File"));
	dirBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "Dir"));
	pathBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "Path"));

	Stdio_File_Init(fileBase);
	Stdio_Dir_Init(dirBase);
	Stdio_Path_Init(pathBase);

	globalClosureInfo = ClosureInfo_Create(NULL, CLOSURE_KIND_GLOBAL);
	globalClosure = Closure_CreateGlobal(globalClosureInfo, NULL);

	Closure_SetGlobalVariable(globalClosure, SymbolTable_GetSymbolC(Smile_SymbolTable, "File"), (SmileObject)fileBase);
	Closure_SetGlobalVariable(globalClosure, SymbolTable_GetSymbolC(Smile_SymbolTable, "Dir"), (SmileObject)dirBase);
	Closure_SetGlobalVariable(globalClosure, SymbolTable_GetSymbolC(Smile_SymbolTable, "Path"), (SmileObject)pathBase);
	Stdio_File_DeclareStdInOutErr(globalClosure, (SmileObject)fileBase);

	parseResult = Smile_ParseInScope(globalClosureInfo, _stdioBootstrap, _stdioFilename, &parseMessages, &numParseMessages);

	if (numParseMessages > 0) {
		DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
		String formattedError;
		Int i;

		INIT_INLINE_STRINGBUILDER(stringBuilder);

		StringBuilder_AppendFormat(stringBuilder, "Parse errors in library:\n");
		for (i = 0; i < numParseMessages; i++) {
			formattedError = SMILE_VCALL1(parseMessages[i], toString, (SmileUnboxedData) { 0 });
			StringBuilder_AppendByte(stringBuilder, '\t');
			StringBuilder_AppendString(stringBuilder, formattedError);
			StringBuilder_AppendByte(stringBuilder, '\n');
		}
		Smile_ThrowException(Smile_KnownSymbols.load_error, String_Format("Parse errors in library:"));
	}

	Smile_EvalInScope(globalClosureInfo, parseResult);

	return LibraryInfo_Create(stdioName, True, globalClosureInfo, NULL, 0);
}
