//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Command-Line Runner)
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

static const char *_whileWrapper =
	"#include \"stdio\"\n"
	"\n"
	"till done do {\n"
	"\tline = get-line Stdin\n"
	"\tif line === null then done\n"
	"%S\n"
	"%S\n"
	"}\n";

static void Verbose(const char *format, ...)
{
	va_list v;
	va_start(v, format);
	puts("- ");
	vfprintf(stdout, format, v);
	puts("\n");
	va_end(v);
}

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
	fputs("\n", stderr);

	va_end(v);
}

static void PrintHelp()
{
	printf(
		"\n"
		"Usage: smile [options] [--] program.sm ...\n"
		"\n"
		"Execution options:\n"
		"  -c --check     Check syntax and for warnings/errors, but do not run\n"
		"  -Dname=value   Define a global variable with the given constant value.\n"
		"  -e 'script'    One line of program (several -e's allowed; omit program.sm)\n"
		"  -n             Wrap script with \"till done { line = get-line Stdin ... }\"\n"
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
		"\n"
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
	lexer = Lexer_Create(string, 0, String_Length(string), String_FromC("command-line argument"), 1, 1);
	lexer->symbolTable = Smile_SymbolTable;
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
								return NULL;
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
							Error("smile", 0, "Invalid command-line argument \"%s\".\n", argv[i]);
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
								return NULL;
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
								options->printLineInLoop = options->wrapWithWhile = True;
								break;
							case 'c':
								options->checkOnly = True;
								break;
							case 'v':
								options->verbose = True;
								break;
							case 'e':
								if (*ptr != '\0') {
									Error("smile", 0, "Invalid command-line argument \"-%s\".\n", ptr-1);
									return NULL;
								}
								if (++i >= argc) {
									Error("smile", 0, "Invalid command-line argument \"-%s\".\n", ptr - 1);
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
										Error("smile", 0, "Invalid command-line argument \"-%s\".\n", ptr-1);
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
								Error("smile", 0, "Invalid command-line argument \"-%c\".\n", ptr[-1]);
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

	if (options->scriptName != NULL) {
		if (options->script != NULL) {
			Error("smile", 0, "Cannot use both a script name and \"-e\" on the command-line.\n");
			return NULL;
		}
		if (options->wrapWithWhile) {
			Error("smile", 0, "Cannot use both a script name and \"-n\" or \"-p\" on the command-line.\n");
			return NULL;
		}
	}

	return options;
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

static ClosureInfo SetupGlobalClosureInfo(CommandLineArgs options)
{
	SmileList list;
	ClosureInfo closureInfo = ClosureInfo_Create(NULL, CLOSURE_KIND_GLOBAL);

	Smile_SetGlobalClosureInfo(closureInfo);

	Smile_InitCommonGlobals(closureInfo);

	for (list = options->globalDefinitions; SMILE_KIND(list) != SMILE_KIND_NULL; list = LIST_REST(list)) {
		SmileSymbol symbol = (SmileSymbol)LIST_FIRST((SmileList)LIST_FIRST(list));
		SmileObject value = (SmileObject)LIST_SECOND((SmileList)LIST_FIRST(list));
		Smile_SetGlobalVariable(symbol->symbol, value);
	}

	return closureInfo;
}

static Int ParseOnly(CommandLineArgs options, String string, String filename, Int line)
{
	Lexer lexer;
	Parser parser;
	ParseScope globalScope;
	ClosureInfo closureInfo;

	closureInfo = SetupGlobalClosureInfo(options);
	globalScope = ParseScope_CreateRoot();
	ParseScope_DeclareVariablesFromClosureInfo(globalScope, closureInfo);

	lexer = Lexer_Create(string, 0, String_Length(string), filename, line, 1);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();

	if (options->verbose) {
		Verbose("Parsing \"%s\".", String_ToC(filename));
	}
	Parser_Parse(parser, lexer, globalScope);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(options, parser);
		if (hasErrors) return 1;
	}

	return 0;
}

static Int ParseAndEval(CommandLineArgs options, String string, String filename, Int line, SmileObject *result)
{
	Lexer lexer;
	Parser parser;
	ParseScope globalScope;
	ClosureInfo closureInfo;
	SmileObject parsedScript;
	EvalResult evalResult;

	closureInfo = SetupGlobalClosureInfo(options);
	globalScope = ParseScope_CreateRoot();
	ParseScope_DeclareVariablesFromClosureInfo(globalScope, closureInfo);

	lexer = Lexer_Create(string, 0, String_Length(string), filename, line, 1);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();

	if (options->verbose) {
		Verbose("Parsing \"%s\".", String_ToC(filename));
	}

	parsedScript = Parser_Parse(parser, lexer, globalScope);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(options, parser);
		if (hasErrors) {
			*result = NullObject;
			return 1;
		}
	}

	if (options->verbose) {
		Verbose("Evaluating parsed script.");
	}

	evalResult = Smile_EvalInScope(closureInfo, parsedScript);

	switch (evalResult->evalResultKind) {

		case EVAL_RESULT_EXCEPTION:
			{
				SmileArg unboxedException = SmileArg_Unbox(evalResult->exception);
				String stringified = SMILE_VCALL1(unboxedException.obj, toString, unboxedException.unboxed);
				String message = String_Format("%S: Uncaught exception thrown: %S\r\n", filename, stringified);
				fwrite(String_GetBytes(message), 1, String_Length(message), stderr);
				fflush(stderr);
				*result = NullObject;
			}
			return 1;

		case EVAL_RESULT_BREAK:
			{
				String message = String_Format("%S: Stopped at breakpoint.\r\n", filename);
				fwrite(String_GetBytes(message), 1, String_Length(message), stderr);
				fflush(stderr);
				*result = NullObject;
			}
			return 2;
		
		default:
			*result = evalResult->value;
			return 0;
	}
}

int main(int argc, const char **argv)
{
	String script;
	String scriptName;
	CommandLineArgs options;
	SmileObject result;
	Int exitCode;

	// Initialize the Smile runtime.
	Smile_Init();

	// Parse the command line.  This returns NULL if the user provided invalid options on
	// the command-line, or if they requested "--help".
	if ((options = ParseCommandLine(argc, argv)) == NULL)
		return -1;

	// If they gave us nothing to do, and requested "-v", then print the version number and
	// exit.  Otherwise, fall into the REPL.
	if (options->scriptName == NULL && options->script == NULL) {
		if (options->verbose) {
			printf("Smile v%d.%d / %s\n", MAJOR_VERSION, MINOR_VERSION, BUILDSTRING);
			return 0;
		}
		else {
			if (options->verbose) {
				Verbose("No script given, so entering REPL.");
			}
		
			printf(
				"+--------------------------------+\n"
				"  Smile Programming Language\n"
				"  v%d.%d / %s\n"
				"+--------------------------------+\n"
				"\n"
				"Welcome to Smile! :-)\n"
				"\n"
				"For help, type \"help\" and press Enter.\n"
				"\n",
				MAJOR_VERSION, MINOR_VERSION, BUILDSTRING);
		
			// TODO: FIXME: Create the REPL and invoke it here :-)
			printf("] (TODO: Add REPL here.)\n\n");

			return 0;
		}
	}

	// If they provided a script on the command-line itself using "-e", see if it needs to
	// have the "-p" or "-n" flags applied, and apply them to get the "full" script.
	if (options->script != NULL) {
		if (options->wrapWithWhile) {
			script = String_Format(_whileWrapper, options->script, options->printLineInLoop
				? String_FromC("Stdout print line") : String_Empty);
			scriptName = String_FromC("script");
		}
		else {
			script = options->script;
			scriptName = String_FromC("script");
		}
	}

	// If they requested verbose output, dump everything we know about what they've asked,
	// so they can be sure about what they asked us to do.
	if (options->verbose) {
		Verbose("Smile v%d.%d / %s", MAJOR_VERSION, MINOR_VERSION, BUILDSTRING);
		Verbose("");
		Verbose("Verbose output: true");
		if (options->quiet)
			Verbose("Quiet: true");
		if (options->warningsAsErrors)
			Verbose("Warnings as errors: true");
		if (options->checkOnly)
			Verbose("Check only: true");
		if (options->outputResult)
			Verbose("Output result: true");
		if (options->wrapWithWhile)
			Verbose("Wrap with while loop: true");
		if (options->printLineInLoop)
			Verbose("Print line in loop: true");
		if (options->scriptName != NULL) {
			Verbose("Script name: \"%s\"", String_ToC(options->scriptName));
			if (options->scriptArgs != NullList)
				Verbose("Arguments: %s", SmileObject_StringifyToC((SmileObject)options->scriptArgs));
		}
		else {
			String indentedScript;
			Verbose("Script text:");
			indentedScript = String_Replace(script, String_FromC("\n"), String_FromC("\n    "));
			fwrite(String_GetBytes(indentedScript), 1, String_Length(indentedScript), stdout);
			puts("\n");
		}
		if (options->globalDefinitions != NULL) {
			Verbose("Global variables: %s", SmileObject_StringifyToC((SmileObject)options->globalDefinitions));
		}
		Verbose("");
	}

	// If they're running an external ".sm" program, load that now.
	if (options->scriptName != NULL) {
		if (options->verbose) {
			Verbose("Loading \"%s\".", String_ToC(options->scriptName));
		}
		if ((script = LoadFile(options->scriptName)) == NULL)
			return -1;
		scriptName = options->scriptName;
	}

	// Now parse and evaluate the program!
	if (options->checkOnly) {
		exitCode = ParseOnly(options, script, scriptName, 1);
		result = NullObject;
	}
	else {
		exitCode = ParseAndEval(options, script, scriptName, 1, &result);
	}

	// If they requested the result to be outputted, do that now.
	if (options->outputResult) {
		// Convert the object to a string by a virtual call to its toString() method.
		SmileArg unboxedResult = SmileArg_Unbox(result);
		String stringResult = SMILE_VCALL1(unboxedResult.obj, toString, unboxedResult.unboxed);
		fwrite(String_GetBytes(stringResult), 1, String_Length(stringResult), stdout);
		fwrite("\n", 1, 1, stdout);
		fflush(stdout);
	}

	// And we're done.
	Smile_End();

	return (int)exitCode;
}
