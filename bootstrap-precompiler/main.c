//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Bootstrap Precompiler)
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

#include "stdafx.h"

static void Error(const char *filename, Int line, const char *format, ...)
{
	va_list v;
	va_start(v, format);

	if (filename != NULL) {
		if (line > 0) {
			fprintf(stderr, "%s:%d: ", filename, (int)line);
		}
		else {
			fprintf(stderr, "%s: ", filename);
		}
	}

	vfprintf(stderr, format, v);
	fprintf(stderr, "\n");

	va_end(v);
}

static Bool PrintParseMessages(Parser parser)
{
	SmileList list;
	ParseMessage parseMessage;
	Bool hasErrors;
	Bool shouldPrint;
	const char *prefix;
	LexerPosition position;
	String message;

	hasErrors = False;

	for (list = parser->firstMessage; SMILE_KIND(list) != SMILE_KIND_NULL; list = LIST_REST(list)) {
		parseMessage = (ParseMessage)LIST_FIRST(list);

		shouldPrint = False;
		prefix = "";

		switch (parseMessage->messageKind) {
			case PARSEMESSAGE_INFO:
				break;

			case PARSEMESSAGE_WARNING:
				shouldPrint = True;
				prefix = "warning: ";
				hasErrors = True;
				break;

			case PARSEMESSAGE_ERROR:
				shouldPrint = True;
				prefix = "";
				hasErrors = True;
				break;
		}
	
		if (!shouldPrint) continue;
	
		position = parseMessage->position;
		if (position->filename != NULL) {
			if (position->line > 0) {
				// Have a filename and a line number.
				message = String_Format("%S:%d: %s%S\r\n", position->filename, position->line, prefix, parseMessage->message);
			}
			else {
				// Have a filename but no line number.
				message = String_Format("%S: %s%S\r\n", position->filename, prefix, parseMessage->message);
			}
		}
		else {
			// Have no filename.
			message = String_Format("smile: %s%S\r\n", prefix, parseMessage->message);
		}
		
		fwrite(String_GetBytes(message), 1, String_Length(message), stderr);
	}

	return hasErrors;
}

static String LoadFile(String filename)
{
	FILE *fp;
	StringBuilder stringBuilder;
	Byte *buffer;
	size_t readLength;

	const int ReadLength = 0x10000;	// Read 64K at a time.

	if ((fp = fopen(String_ToC(filename), "rb")) == NULL) {
		Error("smile", 0, "Cannot open \"%s\" for reading.", String_ToC(filename));
		return NULL;
	}

	stringBuilder = StringBuilder_Create();

	buffer = GC_MALLOC_ATOMIC(ReadLength);
	if (buffer == NULL)
		Smile_Abort_OutOfMemory();

	while ((readLength = fread(buffer, 1, ReadLength, fp)) > 0) {
		StringBuilder_Append(stringBuilder, buffer, 0, readLength);
	}

	fclose(fp);

	return StringBuilder_ToString(stringBuilder);
}

static SmileObject Parse(String text, String filename)
{
	Lexer lexer;
	Parser parser;
	ParseScope globalScope;
	ClosureInfo closureInfo;
	SmileObject expr;

	closureInfo = ClosureInfo_Create(NULL, CLOSURE_KIND_GLOBAL);

	Smile_SetGlobalClosureInfo(closureInfo);
	Smile_InitCommonGlobals(closureInfo);

	globalScope = ParseScope_CreateRoot();
	ParseScope_DeclareVariablesFromClosureInfo(globalScope, closureInfo);

	lexer = Lexer_Create(text, 0, String_Length(text), Path_Resolve(Path_GetCurrentDir(), filename), 1, 1);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();

	expr = Parser_Parse(parser, lexer, globalScope);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(parser);
		if (hasErrors) return NULL;
	}

	return expr;
}

/// <summary>
/// Make sure that the given expr is for a [$progn] that consists of a series of [$set] operations
/// (or discardable constant values like 'null').  Anything else is a structural error in the bootstrap
/// and is disallowed.
/// </summary>
static Bool CheckExpr(SmileObject expr, String filename)
{
	SmileList list;
	SmileObject childExpr;

	if (SMILE_KIND(expr) != SMILE_KIND_LIST
		|| SMILE_KIND((list = (SmileList)expr)->a) != SMILE_KIND_SYMBOL
		|| ((SmileSymbol)list->a)->symbol != SMILE_SPECIAL_SYMBOL__PROGN) {
		Error(String_ToC(filename), SMILE_VCALL(expr, getSourceLocation)->line,
			"Bootstrap script *must* consist of a [$progn] sequence of [$set] to known names or properites of known names;"
			" this script does not start with a [$progn] (did you accidentally create a global variable?).");
		return False;
	}

	for (; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list)) {
		childExpr = list->a;

		if (SMILE_KIND(childExpr) != SMILE_KIND_LIST
			|| SMILE_KIND((list = (SmileList)childExpr)->a) != SMILE_KIND_SYMBOL
			|| ((SmileSymbol)list->a)->symbol != SMILE_SPECIAL_SYMBOL__SET) {
			Error(String_ToC(filename), SMILE_VCALL(expr, getSourceLocation)->line,
				"Bootstrap script *must* consist of a [$progn] sequence of [$set] to known names or properites of known names;"
				" this line is not a [$set] expression (are you trying to perform disallowed computation in the bootstrap?)");
			return False;
		}
	}

	return True;
}

int main(int argc, const char **argv)
{
	int i;
	const char *arg;
	String filename, text;
	SmileObject expr;

	Smile_Init();

	for (i = 1; i < argc; i++) {
		arg = argv[i];
		filename = String_FromC(arg);
		text = LoadFile(filename);
		expr = Parse(text, filename);
		if (!CheckExpr(expr, filename))
			continue;
	}

	Smile_End();

	return 0;
}
