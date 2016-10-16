//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Command-Line Runner)
//  Copyright 2004-2016 Sean Werkema
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
#include "buildnum.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1

static const char *_scriptName = NULL;
static Bool _verbose = False;
static Bool _quiet = False;
static Bool _warningsAsErrors = False;
static SmileList _globalDefinitions, _globalDefinitionsTail;

STATIC_STRING(_commandLineArgument, "command-line argument");

static void PrintHelp()
{
	printf(
		"Smile v%d.%d / %s\n"
		"\n"
		"Usage: smile [options] [script.sm]\n"
		"\n"
		"Options:\n"
		"  -h --help             You're looking at it.\n"
		"  -v --verbose          Display additional debugging information.\n"
		"  -q --quiet            Do not display any warning messages.\n"
		"  --warnings-as-errors  Treat any warnings the same as errors, and abort.\n"
		"  -Dname=value          Define a global variable with a constant value.\n"
		"\n",
		MAJOR_VERSION,
		MINOR_VERSION,
		BUILDSTRING
	);
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
	
		switch (parseMessage->messageKind) {
			case PARSEMESSAGE_INFO:
				if (_verbose) {
					shouldPrint = True;
					prefix = "";
				}
				break;

			case PARSEMESSAGE_WARNING:
				if (_warningsAsErrors) {
					shouldPrint = True;
					prefix = "warning: ";
					hasErrors = True;
				}
				else if (!_quiet) {
					shouldPrint = True;
					prefix = "warning: ";
				}
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

static SmileObject ParseOneConstantValue(const char *text)
{
	String string;
	Lexer lexer;
	Parser parser;
	SmileObject result;

	string = String_FromC(text);
	lexer = Lexer_Create(string, 0, String_Length(string), _commandLineArgument, 1, 1);
	parser = Parser_Create();

	result = Parser_ParseConstant(parser, lexer, ParseScope_CreateRoot());
	
	if (parser->firstMessage != NULL) {
		Bool hasErrors = PrintParseMessages(parser);
		if (hasErrors) return NullObject;
	}

	return result;
}

static Bool ParseCommandLine(int argc, const char **argv)
{
	int i;
	Array array = NULL;

	_globalDefinitions = _globalDefinitionsTail = NullList;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "-?")) {
				PrintHelp();
				return False;
			}
			else if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) {
				_quiet = True;
			}
			else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
				_verbose = True;
			}
			else if (!strcmp(argv[i], "--warnings-as-errors")) {
				_warningsAsErrors = True;
			}
			else if (argv[i][0] == '-' && argv[i][1] == 'D') {
				char *equalPtr;
				String stringKey;
				SmileObject value;
				Symbol symbol;
			
				equalPtr = strchr(argv[i], '=');
				if (equalPtr == NULL) {
					fprintf(stderr, "Invalid command-line argument \"%s\".\n", argv[i]);
					return False;
				}
			
				stringKey = String_Create((Byte *)(argv[i] + 2), equalPtr - (argv[i] + 2));
				value = ParseOneConstantValue(equalPtr + 1);
				symbol = SymbolTable_GetSymbol(Smile_SymbolTable, stringKey);

				// Make a simple list like this: [[key1 value1] [key2 value2] [key3 value3] ...]
				LIST_APPEND(_globalDefinitions, _globalDefinitionsTail,
					LIST_CONS(SmileSymbol_Create(symbol), LIST_CONS(value, NullObject)));
			}
			else {
				fprintf(stderr, "Unknown command-line argument \"%s\".\n", argv[i]);
			}
		}
		else {
			if (_scriptName == NULL)
				_scriptName = argv[i];
			else {
				fprintf(stderr, "Too many scripts specified on command-line: %s\n", argv[i]);
				return False;
			}
		}
	}

	return True;
}

static String LoadFile(const char *filename)
{
	FILE *fp;
	StringBuilder stringBuilder;
	Byte *buffer;
	size_t readLength;

	if ((fp = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "smile: Cannot open \"%s\" for reading.", filename);
		return NULL;
	}

	stringBuilder = StringBuilder_Create();

	buffer = GC_MALLOC_ATOMIC(0x10000);
	if (buffer == NULL)
		Smile_Abort_OutOfMemory();

	while ((readLength = fread(buffer, 1, 0x10000, fp)) > 0) {
		StringBuilder_Append(stringBuilder, buffer, 0, readLength);
	}

	fclose(fp);

	return StringBuilder_ToString(stringBuilder);
}

static SmileObject ParseAndEval(String string, String filename, Int line)
{
	Lexer lexer;
	Parser parser;
	ParseScope globalScope;
	SmileList result;
	SmileList list;

	globalScope = ParseScope_CreateRoot();

	for (list = _globalDefinitions; SMILE_KIND(list) != SMILE_KIND_NULL; list = LIST_REST(list)) {
		ParseScope_Declare(globalScope, ((SmileSymbol)((SmileList)list->a)->a)->symbol, PARSEDECL_GLOBAL, NULL, NULL);
	}

	lexer = Lexer_Create(string, 0, String_Length(string), filename, line, 1);
	parser = Parser_Create();

	result = Parser_Parse(parser, lexer, globalScope);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(parser);
		if (hasErrors) return NullObject;
	}

	{
		String output = SmileObject_Stringify((SmileObject)result);
		fwrite(String_GetBytes(output), 1, String_Length(output), stdout);
		printf("\r\n");
	}

	return NullObject;
}

int main(int argc, const char **argv)
{
	String script;

	Smile_Init();

	if (!ParseCommandLine(argc, argv))
		return -1;

	if (_scriptName == NULL) {
		fprintf(stderr, "Smile v%d.%d / %s\n", MAJOR_VERSION, MINOR_VERSION, BUILDSTRING);
		return 0;
	}

	if ((script = LoadFile(_scriptName)) == NULL)
		return -1;

	ParseAndEval(script, String_FromC(_scriptName), 1);

	Smile_End();

	return 0;
}
