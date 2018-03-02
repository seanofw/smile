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
	/*"\n"
	"#syntax STMT [print [EXPR+ exprs ,]] => (Stdout.print ## exprs)\n"
	"\n"*/
);

ModuleInfo Stdio_Main(void)
{
	ParseMessage *parseMessages;
	Int numParseMessages;
	SmileObject expr;
	SmileUserObject fileBase, dirBase, pathBase;
	ParseScope moduleScope;
	ExternalVar vars[8];
	Int numVars;

	STATIC_STRING(stdioName, "stdio");

	fileBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "File"));
	dirBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "Dir"));
	pathBase = SmileUserObject_Create((SmileObject)Smile_KnownBases.Handle, SymbolTable_GetSymbolC(Smile_SymbolTable, "Path"));

	Stdio_File_Init(fileBase);
	Stdio_Dir_Init(dirBase);
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
