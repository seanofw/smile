//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Command-Line Runner)
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

#include "stdafx.h"
#include "buildnum.h"

#include "style.h"

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

typedef struct CommandLineArgsStruct {
	String scriptName;			// script.sm
	String script;				// -e "script"
	Bool verbose;				// -v --verbose
	Bool quiet;					// -q --quiet
	Bool checkOnly;				// -c --check
	Bool showRawForm;			// -r --raw
	Bool wrapWithWhile;			// -n
	Bool printLineInLoop;		// -p
	Bool outputResult;			// -o
	Bool warningsAsErrors;		// --warnings-as-errors
	SmileList globalDefinitions, globalDefinitionsTail;		// -Dfoo=bar
	SmileList scriptArgs, scriptArgsTail;					// -- ...args...
} *CommandLineArgs;

void ReplMain(void);
void PrintSmileWelcome(void);

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
	printf_styled("\033[0;37m- ");
	vprintf_styled(format, v);
	printf_styled("\033[0m\n");
	va_end(v);
}

static void Error(const char *filename, Int line, const char *format, ...)
{
	va_list v;
	va_start(v, format);

	fprintf_styled(stderr, "\033[0;31;1m");

	if (filename != NULL) {
		if (line > 0) {
			fprintf_styled(stderr, "%s:%d: ", filename, (int)line);
		}
		else {
			fprintf_styled(stderr, "%s: ", filename);
		}
	}

	fprintf_styled(stderr, "\033[33;1m");

	vfprintf_styled(stderr, format, v);
	fprintf_styled(stderr, "\033[0m\n");

	va_end(v);
}

static void PrintHelp()
{
	printf_styled(
		"\n"
		"\033[0;37;1mUsage: \033[33msmile \033[0;36m[\033[1moptions\033[0;36m] [\033[1m--\033[0;36m] \033[1;37mprogram.sm \033[0;36m...\n"
		"\n"
		"\033[0;37;1mExecution options:\033[0;37m\n"
		"  \033[0;1;36m-c --check     \033[0;37mCheck syntax and for warnings/errors, but do not run\n"
		"  \033[0;1;36m-r --raw       \033[0;37mLike '-c', but print out the resulting 'raw' form of the code\n"
		"  \033[0;1;36m-D\033[0;36mname=value   \033[0;37mDefine a global variable with the given constant value\n"
		"  \033[0;1;36m-e \033[0;36m'script'    \033[0;37mOne line of program (several -e's allowed; omit program.sm)\n"
		"  \033[0;1;36m-n             \033[0;37mWrap script with \"till done { line = get-line Stdin ... }\"\n"
		"  \033[0;1;36m-o             \033[0;37mPrint program's resulting value to Stdout\n"
		"  \033[0;1;36m-p             \033[0;37mLike '-n', but also add \"Stdout print line\" in the loop\n"
		"\n"
		"\033[0;37;1mInformation options:\033[0;37m\n"
		"  \033[0;1;36m-h --help      \033[0;37mHelp (you're looking at it)\n"
		"  \033[0;1;36m-q --quiet     \033[0;37mDo not display any warning messages\n"
		"  \033[0;1;36m-v --verbose   \033[0;37mDisplay additional version and/or debugging information\n"
		"  \033[0;1;36m--warnings-as-errors\n"
		"                 \033[0;37mTreat any warnings found the same as errors, and abort\n"
		"\n"
		"\033[0;37;1mControl options:\033[0;37m\n"
		"  \033[0;1;36m--             \033[0;37mTreat subsequent arguments as program name/args\n"
		"\n"
	);
}

static Bool PrintParseMessage(CommandLineArgs options, ParseMessage parseMessage)
{
	Bool hasErrors = False;
	Bool shouldPrint = False;
	const char *prefix = "";
	LexerPosition position;
	String message;

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

	if (shouldPrint) {
		position = parseMessage->position;
		if (position != NULL && position->filename != NULL) {
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

static Bool PrintParseMessages(CommandLineArgs options, Parser parser)
{
	SmileList list;
	ParseMessage parseMessage;
	Bool hasErrors = False;

	for (list = parser->firstMessage; SMILE_KIND(list) != SMILE_KIND_NULL; list = LIST_REST(list)) {
		parseMessage = (ParseMessage)LIST_FIRST(list);
		hasErrors |= PrintParseMessage(options, parseMessage);
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
	options->showRawForm = False;
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
							break;
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
						case 'r':
							if (!strcmp(argv[i] + 2, "raw")) {
								options->showRawForm = True;
							}
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
							case 'r':
								options->showRawForm = True;
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
			LIST_APPEND(options->scriptArgs, options->scriptArgsTail, argv[i]);
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
	SmileObject expr;
	String stringified;

	closureInfo = SetupGlobalClosureInfo(options);
	globalScope = ParseScope_CreateRoot();
	ParseScope_DeclareVariablesFromClosureInfo(globalScope, closureInfo);

	lexer = Lexer_Create(string, 0, String_Length(string), Path_Resolve(Path_GetCurrentDir(), filename), line, 1);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();

	if (options->verbose) {
		Verbose("Parsing \"%s\".", String_ToC(filename));
	}
	expr = Parser_Parse(parser, lexer, globalScope);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(options, parser);
		if (hasErrors) return 1;
	}

	if (options->showRawForm) {
		stringified = SmileObject_Stringify(expr);
		fwrite(String_GetBytes(stringified), 1, String_Length(stringified), stdout);
		printf("\n");
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

	lexer = Lexer_Create(string, 0, String_Length(string), Path_Resolve(Path_GetCurrentDir(), filename), line, 1);
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
				String exceptionMessage = (String)SMILE_VCALL1(evalResult->exception, getProperty, Smile_KnownSymbols.message);
				SmileSymbol exceptionKindWrapped = (SmileSymbol)SMILE_VCALL1(evalResult->exception, getProperty, Smile_KnownSymbols.kind);
				String displayMessage, stackTraceMessage;
				Symbol exceptionKind;
				SmileObject stackTrace;

				if (SMILE_KIND(exceptionMessage) != SMILE_KIND_STRING)
					exceptionMessage = String_Empty;
				if (SMILE_KIND(exceptionKindWrapped) != SMILE_KIND_SYMBOL)
					exceptionKind = SymbolTable_GetSymbolC(Smile_SymbolTable, "unknown-error");
				else exceptionKind = exceptionKindWrapped->symbol;

				displayMessage = String_Format("\033[0;31;1m!Exception thrown (%S)%s\033[0;33;1m%S\033[0m\n",
					SymbolTable_GetName(Smile_SymbolTable, exceptionKind),
					!String_IsNullOrEmpty(exceptionMessage) ? ": " : "",
					exceptionMessage);
				fwrite_styled(String_ToC(displayMessage), 1, String_Length(displayMessage), stderr);
				fflush(stderr);

				stackTrace = SMILE_VCALL1(evalResult->exception, getProperty, Smile_KnownSymbols.stack_trace);
				stackTraceMessage = Smile_FormatStackTrace((SmileList)stackTrace);
				fwrite_styled(String_ToC(stackTraceMessage), 1, String_Length(stackTraceMessage), stderr);
				fflush(stderr);

				*result = NullObject;
			}
			return EVAL_RESULT_EXCEPTION;

		case EVAL_RESULT_BREAK:
			{
				Closure closure;
				CompiledTables compiledTables;
				ByteCodeSegment segment;
				String message;
				ByteCode byteCode;

				Eval_GetCurrentBreakpointInfo(&closure, &compiledTables, &segment, &byteCode);

				if (byteCode->sourceLocation > 0 && byteCode->sourceLocation < compiledTables->numSourceLocations) {
					CompiledSourceLocation sourceLocation = &compiledTables->sourcelocations[byteCode->sourceLocation];
					message = String_Format("%S: Stopped at breakpoint in \"%S\", line %d.\r\n",
						filename, sourceLocation->filename, sourceLocation->line);
				}
				else {
					message = String_Format("%S: Stopped at breakpoint.\r\n", filename);
				}
				fwrite(String_GetBytes(message), 1, String_Length(message), stderr);
				fflush(stderr);
				*result = NullObject;
			}
			return EVAL_RESULT_BREAK;
		
		case EVAL_RESULT_PARSEERRORS:
			// We can get "parse errors" from the compiler, not just from the parser.
			{
				Int i;
				Bool hasErrors = False;
				for (i = 0; i < evalResult->numMessages; i++) {
					hasErrors |= PrintParseMessage(options, evalResult->parseMessages[i]);
				}
				return hasErrors ? EVAL_RESULT_PARSEERRORS : 0;
			}

		default:
			*result = evalResult->value;
			return 0;
	}
}

static int SmileMain(int argc, const char **argv)
{
	String script = NULL;
	String scriptName = NULL;
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
			printf("Smile v%d.%d / %s\n", SMILE_MAJOR_VERSION, SMILE_MINOR_VERSION, BUILDSTRING);
			return 0;
		}
		PrintSmileWelcome();
		ReplMain();
		return 0;
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
		Verbose("Smile v%d.%d / %s", SMILE_MAJOR_VERSION, SMILE_MINOR_VERSION, BUILDSTRING);
		Verbose("");
		Verbose("Verbose output: true");
		if (options->quiet)
			Verbose("Quiet: true");
		if (options->warningsAsErrors)
			Verbose("Warnings as errors: true");
		if (options->checkOnly)
			Verbose("Check only: true");
		if (options->showRawForm)
			Verbose("Show raw form: true");
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
			indentedScript = String_Replace(script != NULL ? script : String_Empty, String_Newline, String_FromC("\n    "));
			fwrite(String_GetBytes(indentedScript), 1, String_Length(indentedScript), stdout);
			puts("\n");
		}
		if (options->globalDefinitions != NullList) {
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
	if (options->checkOnly || options->showRawForm) {
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

int main(int argc, const char **argv)
{
	UInt32 oldConsoleCP, oldConsoleOutputCP;
	int result;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		oldConsoleCP = GetConsoleCP();
		oldConsoleOutputCP = GetConsoleOutputCP();
		SetConsoleCP(CP_UTF8);
		SetConsoleOutputCP(CP_UTF8);
#	endif

	result = SmileMain(argc, argv);

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		SetConsoleCP(oldConsoleCP);
		SetConsoleOutputCP(oldConsoleOutputCP);
#	endif

	return result;
}
