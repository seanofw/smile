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

typedef struct CommandLineArgsStruct {
	String scriptName;	// script.sm
	String script;	// -e "script"
	Bool verbose;	// -v --verbose
	Bool quiet;	// -q --quiet
	Bool checkOnly;	// -c --check
	Bool wrapWithWhile;	// -n
	Bool printLineInLoop;	// -p
	Bool outputResult;	// -o
	Bool warningsAsErrors;	// --warnings-as-errors
	SmileList globalDefinitions, globalDefinitionsTail;	// -Dfoo=bar
	SmileList scriptArgs, scriptArgsTail;	// -- ...args...
} *CommandLineArgs;

STATIC_STRING(_commandLineArgument, "command-line argument");
STATIC_STRING(_commandLineScriptName, "script");

static void PrintHelp()
{
	printf(
		"Usage: smile [options] [--] program.sm ...\n"
		"\n"
		"Execution options:\n"
		"  -c --check     Check syntax and for warnings/errors, but do not run\n"
		"  -Dname=value   Define a global variable with the given constant value.\n"
		"  -e 'script'    One line of program (several -e's allowed; omit program.sm)\n"
		"  -n             Add \"while { line = get-line Stdin ... }\" around program\n"
		"  -o             Print program's resulting value to Stdout\n"
		"  -p             Like '-n', but also add \"Stdout print line\" in the loop\n"
		"\n"
		"Information options:\n"
		"  -h --help      Help (you're looking at it)\n"
		"  -q --quiet     Do not display any warning messages\n"
		"  -v --verbose   Display additional version and/or debugging information\n"
		"  --warnings-as-errors\n"
		"                 Treat any warnings found the same as errors, and abort\n"
		"\n"
		"Control options:\n"
		"  --             Treat subsequent arguments as program name/args.\n"
		"\n",
		MAJOR_VERSION,
		MINOR_VERSION,
		BUILDSTRING
	);
}

static Bool PrintParseMessages(CommandLineArgs options, Parser parser)
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
				if (options->verbose) {
					shouldPrint = True;
					prefix = "";
				}
				break;

			case PARSEMESSAGE_WARNING:
				if (options->warningsAsErrors) {
					shouldPrint = True;
					prefix = "warning: ";
					hasErrors = True;
				}
				else if (!options->quiet) {
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

static SmileObject ParseOneConstantValue(CommandLineArgs options, const char *text)
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
		Bool hasErrors = PrintParseMessages(options, parser);
		if (hasErrors) return NullObject;
	}

	return result;
}

static CommandLineArgs CommandLineArgs_Create(void)
{
	CommandLineArgs options = GC_MALLOC_STRUCT(struct CommandLineArgsStruct);

	options->scriptName = NULL;
	options->script = NULL;
	options->verbose = False;
	options->quiet = False;
	options->checkOnly = False;
	options->wrapWithWhile = False;
	options->printLineInLoop = False;
	options->outputResult = False;
	options->warningsAsErrors = False;

	options->globalDefinitions = options->globalDefinitionsTail = NullList;
	options->scriptArgs = options->scriptArgsTail = NullList;

	return options;
}

static CommandLineArgs ParseCommandLine(int argc, const char **argv)
{
	int i;
	Bool lastOption;
	CommandLineArgs options;
	StringBuilder scriptBuilder;

	options = CommandLineArgs_Create();
	scriptBuilder = NULL;
	lastOption = False;

	for (i = 1; i < argc; i++) {
	
		if (!lastOption) {
			if (argv[i][0] == '-') {
				if (argv[i][1] == '-') {
					switch (argv[i][2]) {
						case 'h':
							if (!strcmp(argv[i] + 2, "help")) {
								PrintHelp();
								return False;
							}
							else goto unknownArgument;
						case 'q':
							if (!strcmp(argv[i] + 2, "quiet")) {
								options->quiet = True;
							}
							else goto unknownArgument;
						case 'v':
							if (!strcmp(argv[i] + 2, "verbose")) {
								options->verbose = True;
							}
							break;
						case 'w':
							if (!strcmp(argv[i] + 2, "warnings-as-errors")) {
								options->warningsAsErrors = True;
							}
							break;
						case 'c':
							if (!strcmp(argv[i] + 2, "check")) {
								options->checkOnly = True;
							}
							break;
						case '\0':
							lastOption = True;
							break;
						unknownArgument:
							fprintf(stderr, "Invalid command-line argument \"%s\".\n", argv[i]);
							return NULL;
					}
				}
				else {
					const char *ptr = argv[i] + 1;
					Bool done = False;
					while (!done) {
						switch (*ptr++) {
							case 'h':
							case '?':
								PrintHelp();
								return False;
							case 'q':
								options->quiet = True;
								break;
							case 'n':
								options->wrapWithWhile = True;
								break;
							case 'o':
								options->outputResult = True;
								break;
							case 'p':
								options->printLineInLoop = True;
								break;
							case 'c':
								options->checkOnly = True;
								break;
							case 'v':
								options->verbose = True;
								break;
							case 'e':
								if (*ptr != '\0') {
									fprintf(stderr, "Invalid command-line argument \"-%s\".\n", ptr-1);
									return NULL;
								}
								if (++i >= argc) {
									fprintf(stderr, "Invalid command-line argument \"-%s\".\n", ptr - 1);
									return NULL;
								}
								if (scriptBuilder == NULL)
									scriptBuilder = StringBuilder_Create();
								StringBuilder_AppendC(scriptBuilder, argv[i], 0, StrLen(argv[i]));
								StringBuilder_AppendByte(scriptBuilder, '\n');
								break;
							case 'D':
								{
									char *equalPtr;
									String stringKey;
									SmileObject value;
									Symbol symbol;

									equalPtr = strchr(ptr, '=');
									if (equalPtr == NULL) {
										fprintf(stderr, "Invalid command-line argument \"-%s\".\n", ptr-1);
										return NULL;
									}

									stringKey = String_Create((Byte *)(argv[i] + 2), equalPtr - (argv[i] + 2));
									value = ParseOneConstantValue(options, equalPtr + 1);
									symbol = SymbolTable_GetSymbol(Smile_SymbolTable, stringKey);

									// Make a simple list like this: [[key1 value1] [key2 value2] [key3 value3] ...]
									LIST_APPEND(options->globalDefinitions, options->globalDefinitionsTail,
										LIST_CONS(SmileSymbol_Create(symbol), LIST_CONS(value, NullObject)));
							
									done = True;
								}
								break;
							case '\0':
								done = True;
								break;
							default:
								fprintf(stderr, "Invalid command-line argument \"-%c\".\n", ptr[-1]);
								break;
						}
					}
				}
			
				continue;
			}
		}

		if (options->scriptName == NULL)
			options->scriptName = String_FromC(argv[i]);
		else {
			LIST_APPEND(options->scriptArgs, options->scriptArgsTail, SmileString_CreateC(argv[i]));
		}
	}

	if (scriptBuilder != NULL) {
		options->script = StringBuilder_ToString(scriptBuilder);
	}

	if (options->script != NULL && options->scriptName != NULL) {
		fprintf(stderr, "Cannot use both a script name and \"-e\" on the command-line.\n");
		return NULL;
	}

	return options;
}

static String LoadFile(String filename)
{
	FILE *fp;
	StringBuilder stringBuilder;
	Byte *buffer;
	size_t readLength;

	if ((fp = fopen(String_ToC(filename), "rb")) == NULL) {
		fprintf(stderr, "smile: Cannot open \"%s\" for reading.", String_ToC(filename));
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

static SmileObject ParseAndEval(CommandLineArgs options, String string, String filename, Int line)
{
	Lexer lexer;
	Parser parser;
	ParseScope globalScope;
	SmileList result;
	SmileList list;

	globalScope = ParseScope_CreateRoot();

	for (list = options->globalDefinitions; SMILE_KIND(list) != SMILE_KIND_NULL; list = LIST_REST(list)) {
		ParseScope_Declare(globalScope, ((SmileSymbol)((SmileList)list->a)->a)->symbol, PARSEDECL_GLOBAL, NULL, NULL);
	}

	lexer = Lexer_Create(string, 0, String_Length(string), filename, line, 1);
	parser = Parser_Create();

	result = Parser_Parse(parser, lexer, globalScope);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(options, parser);
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
	CommandLineArgs options;

	Smile_Init();

	if ((options = ParseCommandLine(argc, argv)) == NULL)
		return -1;

	if (options->scriptName == NULL && options->script == NULL) {
		fprintf(stderr, "Smile v%d.%d / %s\n", MAJOR_VERSION, MINOR_VERSION, BUILDSTRING);
		return 0;
	}

	if (options->verbose) {
		printf("Smile v%d.%d / %s\n", MAJOR_VERSION, MINOR_VERSION, BUILDSTRING);
		printf("\nVerbose output: true\n");
		if (options->quiet)
			printf("Quiet: true\n");
		if (options->warningsAsErrors)
			printf("Warnings as errors: true\n");
		if (options->checkOnly)
			printf("Check only: true\n");
		if (options->outputResult)
			printf("Output result: true\n");
		if (options->wrapWithWhile)
			printf("Wrap with while loop: true\n");
		if (options->printLineInLoop)
			printf("Print line in loop: true\n");
		if (options->scriptName != NULL) {
			printf("Script name: \"%s\"\n", String_ToC(options->scriptName));
			if (options->scriptArgs != NullList)
				printf("Arguments: %s\n", SmileObject_StringifyToC((SmileObject)options->scriptArgs));
		}
		else {
			printf("Script text:\n%s", String_ToC(options->script));
		}
		if (options->globalDefinitions != NULL) {
			printf("Global variables: %s\n", SmileObject_StringifyToC((SmileObject)options->globalDefinitions));
		}
		printf("\n");
	}

	if (options->scriptName != NULL) {
		if ((script = LoadFile(options->scriptName)) == NULL)
			return -1;

		ParseAndEval(options, script, options->scriptName, 1);
	}
	else {
		ParseAndEval(options, options->script, _commandLineScriptName, 1);
	}

	Smile_End();

	return 0;
}
