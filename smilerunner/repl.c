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
#include <stdlib.h>

#include <smile/parsing/internal/parserinternal.h>

#include "style.h"

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <io.h>
#	include "vendor/wineditline-2.202/include/editline/readline.h"
#elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
#	include <limits.h>
#	include <unistd.h>
#	include <errno.h>
#	include <readline/readline.h>
#endif

extern void ListFiles(String commandLine, Bool longMode, Int consoleWidth);
extern void ShowHelp(String commandLine);

static Int ProcessCommand(String input, Int lineNumber, ParseScope globalScope, ClosureInfo globalClosureInfo, SmileObject *result);

static ClosureInfo SetupGlobalClosureInfo()
{
	ClosureInfo closureInfo = ClosureInfo_Create(NULL, CLOSURE_KIND_GLOBAL);

	Smile_SetGlobalClosureInfo(closureInfo);
	Smile_InitCommonGlobals(closureInfo);

	return closureInfo;
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

			case PARSEMESSAGE_WARNING:
				shouldPrint = True;
				prefix = "\033[0;33;1m!Warning:\033[0;37;1m";
				break;

			case PARSEMESSAGE_ERROR:
				shouldPrint = True;
				prefix = "\033[0;31;1m?Error:\033[0;33;1m";
				hasErrors = True;
				break;

			default:
				prefix = "\033[0m";
				break;
		}

		if (!shouldPrint) continue;

		position = parseMessage->position;
		if (position->filename != NULL) {
			if (position->line > 0) {
				// Have a filename and a line number.
				message = String_Format("%s %S:%d: %S\033[0m\n", prefix, position->filename, position->line, parseMessage->message);
			}
			else {
				// Have a filename but no line number.
				message = String_Format("%s %S: %S\033[0m\n", prefix, position->filename, parseMessage->message);
			}
		}
		else {
			// Have no filename.
			message = String_Format("%s %S\033[0m\n", prefix, parseMessage->message);
		}

		fwrite_styled(String_ToC(message), 1, String_Length(message), stderr);
	}

	return hasErrors;
}

static Int ParseAndEval(String string, Int lineNumber, ParseScope globalScope, ClosureInfo globalClosureInfo, SmileObject *result)
{
	Lexer lexer;
	Parser parser;
	SmileList head, tail;
	SmileObject expr;
	EvalResult evalResult;

	lexer = Lexer_Create(string, 0, String_Length(string), NULL, lineNumber, 1, False);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();
	parser->lexer = lexer;
	parser->currentScope = globalScope;

	// Parse the input into a list of expressions.
	head = tail = NullList;
	Parser_ParseExprsOpt(parser, &head, &tail, BINARYLINEBREAKS_DISALLOWED | COMMAMODE_NORMAL | COLONMODE_MEMBERACCESS);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(parser);
		if (hasErrors) {
			*result = NullObject;
			return 1;
		}
	}

	// Turn the list of expressions into a [$progn ...].
	expr = (SmileObject)SmileList_ConsWithSource((SmileObject)Smile_KnownObjects._prognSymbol, (SmileObject)head, NULL);

	// Compile and eval the [$progn] expression.
	evalResult = Smile_EvalInScope(globalClosureInfo, expr);

	// Expose the current results as variables in the global scope.
	Smile_SetGlobalVariableC("$a", SMILE_KIND(head) == SMILE_KIND_LIST ? head->a : NullObject);
	Smile_SetGlobalVariableC("$p", expr);
	Smile_SetGlobalVariableC("$e", evalResult->exception);
	Smile_SetGlobalVariableC("$_", evalResult->value);

	// Handle errors or aborts.
	switch (evalResult->evalResultKind) {

		case EVAL_RESULT_PARSEERRORS:
			{
				Int i;
				for (i = 0; i < evalResult->numMessages; i++) {
					String message = String_Format("\033[0;31;1m?Error: \033[0;33;1m%S\033[0m\n", evalResult->parseMessages[i]->message);
					fwrite_styled(String_ToC(message), 1, String_Length(message), stderr);
				}
				fflush(stderr);
				*result = NullObject;
			}
			return 1;

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
			return 1;

		case EVAL_RESULT_BREAK:
			{
				String message = String_Format("\033[0;33;1mStopped at breakpoint.\033[0m\n");
				fwrite_styled(String_ToC(message), 1, String_Length(message), stderr);
				fflush(stderr);
				*result = NullObject;
			}
			return 2;

		default:
			*result = evalResult->value;
			return 0;
	}
}

static Int CalculateConsoleWidth()
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
		if (!GetConsoleScreenBufferInfo(stdoutHandle, &consoleScreenBufferInfo))
			return 80;

		return (Int)consoleScreenBufferInfo.dwSize.X;
#	else
		return 80;
#	endif
}

static String GetCurDir()
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		UInt16 buffer[MAX_PATH + 2];
		String result;

		if (!GetCurrentDirectoryW(MAX_PATH + 1, buffer)) {
			printf_styled("\033[0;31;1m?Error: \033[0;37;1mUnable to get current directory.\033[0m\n");
			return String_Empty;
		}

		result = String_ReplaceChar(String_FromUtf16(buffer, wcslen(buffer)), '\\', '/');
		return result;

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		char buffer[PATH_MAX + 1];
		String result;

		if (getcwd(buffer, PATH_MAX) == NULL) {
			printf_styled("\033[0;31;1m?Error: \033[0;37;1mUnable to get current directory.\033[0m\n");
			return String_Empty;
		}

		result = String_FromC(buffer);
		return result;

#	else
#		error Unsupported OS.
#	endif
}

static void SetCurDir(String path)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		UInt16 *path16 = String_ToUtf16(String_ReplaceChar(path, '/', '\\'), NULL);

		if (!SetCurrentDirectoryW(path16)) {
			printf_styled("\033[0;31;1m?Error: \033[0;33;1m%s\033[0m", String_ToC(Smile_Win32_GetErrorString(GetLastError())));
		}

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)

		const char *pathChars = String_ToC(path);

		if (chdir(pathChars)) {
			printf_styled("\033[0;31;1m?Error: \033[0;33;1m%s\033[0m\n", String_ToC(Smile_Unix_GetErrorString(errno)));
		}

#	else
#		error Unsupported OS.
#	endif
}

static void ClearScreen(void)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		COORD topLeftScreenCoordinate = { 0, 0 };
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		DWORD consoleSize;
		HANDLE stdoutHandle;
		DWORD numberOfCharsWritten;

		stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		GetConsoleScreenBufferInfo(stdoutHandle, &csbi);
		consoleSize = csbi.dwSize.X * csbi.dwSize.Y;

		FillConsoleOutputCharacterW(stdoutHandle, ' ', consoleSize, topLeftScreenCoordinate, &numberOfCharsWritten);
		FillConsoleOutputAttribute(stdoutHandle, csbi.wAttributes, consoleSize, topLeftScreenCoordinate, &numberOfCharsWritten);
		SetConsoleCursorPosition(stdoutHandle, topLeftScreenCoordinate);

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		printf_styled("\033[2J\033[0;0H");

#	else
#		error Unsupported OS.
#	endif
}

enum {
	UnknownCommand = 0,
	ProcessedCommand = +1,
	ExitCommand = -1,
};

void ReplMain()
{
	ClosureInfo globalClosureInfo;
	ParseScope globalScope;
	SmileObject result;
	String resultText;
	String input;
	char *line;
	Int commandType;
	Int lineNumber;

	globalClosureInfo = SetupGlobalClosureInfo();
	globalScope = ParseScope_CreateRoot();
	ParseScope_DeclareVariablesFromClosureInfo(globalScope, globalClosureInfo);

	ParseScope_DeclareHereC(globalScope, "$a", PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_DeclareHereC(globalScope, "$p", PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_DeclareHereC(globalScope, "$e", PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_DeclareHereC(globalScope, "$_", PARSEDECL_GLOBAL, NULL, NULL);

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		SetConsoleCtrlHandler(NULL, True);
#	endif

	lineNumber = 0;

	for (;;) {

		lineNumber++;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		printf_styled("\033[0;33;1m] \033[0;37;1m");
		line = readline("");
		printf_styled("\033[0m");
		fflush(stdout);
#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
		//line = readline("\1\033[0;33;1m\2] \1\033[0;37;1m\2");	// EditLine doesn't support color prompts (yet).
		printf_styled("\033[0;37;40;1m");
		line = readline("] ");
		printf_styled("\033[0m");
		fflush(stdout);
#	else
#		error Unsupported OS.
#	endif

		if (line == NULL) break;
		input = String_FromC(line);

		if (String_IsNullOrWhitespace(input)) continue;

		add_history(line);
		free(line);

		commandType = ProcessCommand(input, lineNumber, globalScope, globalClosureInfo, &result);
		if (commandType == ProcessedCommand) continue;
		if (commandType == ExitCommand) break;

		Stdio_Invoked = False;

		if (!ParseAndEval(input, lineNumber, globalScope, globalClosureInfo, &result))
		{
			if (!Stdio_Invoked) {
				resultText = SmileObject_Stringify(result);
				printf("%s\n", String_ToC(resultText));
			}
		}

		printf("\n");
	}

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		SetConsoleCtrlHandler(NULL, False);
#	else
		;
#	endif
}

static Int ProcessCommand(String input, Int lineNumber, ParseScope globalScope, ClosureInfo globalClosureInfo, SmileObject *result)
{
	Byte ch;

	switch (String_At(input, 0)) {

		case 'c':
			// Handle "cd" including various special DOSy forms of it like "cd.." .
			if (String_StartsWithC(input, "cd")
				&& (String_Length(input) == 2
					|| (ch = String_At(input, 2)) == ' ' || ch == '/' || ch == '\\' || ch == '.')) {
				String newDir = String_Trim(String_SubstringAt(input, 2));
				if (!String_IsNullOrEmpty(newDir)) {
					SetCurDir(newDir);
					printf("\n");
				}
				else {
					String curDir = GetCurDir();
					printf("%s\n\n", String_ToC(curDir));
				}
				return ProcessedCommand;
			}

			// "cls" and "clear".
			if (String_StartsWithC(input, "cls")
				&& (String_Length(input) == 3 || String_At(input, 3) == ' ')) {
				ClearScreen();
				return ProcessedCommand;
			}
			if (String_StartsWithC(input, "clear")
				&& (String_Length(input) == 5 || String_At(input, 5) == ' ')) {
				ClearScreen();
				return ProcessedCommand;
			}

			// "closure".
			if (String_StartsWithC(input, "closure")
				&& (String_Length(input) == 7 || String_At(input, 7) == ' ')) {
				printf("Error: The 'closure' command is not yet supported.\n\n");
				return ProcessedCommand;
			}

			// "closures".
			if (String_StartsWithC(input, "closures")
				&& (String_Length(input) == 8 || String_At(input, 8) == ' ')) {
				printf("Error: The 'closures' command is not yet supported.\n\n");
				return ProcessedCommand;
			}

			// "continue".
			if (String_StartsWithC(input, "continue")
				&& (String_Length(input) == 8 || String_At(input, 8) == ' ')) {
				printf("Error: The 'continue'/'go' command is not yet supported.\n\n");
				return ProcessedCommand;
			}
			break;

		case 'd':
			// Handle "dir", possibly with arguments.
			if (String_StartsWithC(input, "dir")
				&& (String_Length(input) == 3 || String_At(input, 3) == ' ')) {
				ListFiles(String_SubstringAt(input, 4), True, CalculateConsoleWidth());
				printf("\n");
				return ProcessedCommand;
			}
			break;

		case 'e':
			// "exit".
			if (String_StartsWithC(input, "exit")
				&& (String_Length(input) == 4 || String_At(input, 4) == ' ')) {
				printf("\n");
				return ExitCommand;
			}

			// "eval".
			if (String_StartsWithC(input, "eval")
				&& (String_Length(input) == 4 || String_At(input, 4) == ' ')) {
				String expr = String_SubstringAt(input, 5);
				if (!ParseAndEval(expr, lineNumber, globalScope, globalClosureInfo, result))
				{
					String resultText = SmileObject_Stringify(*result);
					printf("%s\n", String_ToC(resultText));
				}
				printf("\n");
				return ProcessedCommand;
			}
			break;

		case 'g':
			if (String_StartsWithC(input, "go")
				&& (String_Length(input) == 2 || String_At(input, 2) == ' ')) {
				printf("Error: The 'continue'/'go' command is not yet supported.\n\n");
				return ProcessedCommand;
			}
			break;

		case 'h':
			// "help".
			if (String_StartsWithC(input, "help")
				&& (String_Length(input) == 4 || String_At(input, 4) == ' ')) {
				ShowHelp(String_SubstringAt(input, 5));
				return ProcessedCommand;
			}
			break;

		case 'l':
			// Handle "ls", possibly with arguments.
			if (String_StartsWithC(input, "ls")
				&& (String_Length(input) == 2 || String_At(input, 2) == ' ')) {
				ListFiles(String_SubstringAt(input, 3), False, CalculateConsoleWidth());
				printf("\n");
				return ProcessedCommand;
			}
			if (String_StartsWithC(input, "loc")
				&& (String_Length(input) == 3 || String_At(input, 3) == ' ')) {
				printf("Error: The 'loc'/'location' command is not yet supported.\n\n");
				return ProcessedCommand;
			}
			if (String_StartsWithC(input, "location")
				&& (String_Length(input) == 8 || String_At(input, 8) == ' ')) {
				printf("Error: The 'loc'/'location' command is not yet supported.\n\n");
				return ProcessedCommand;
			}
			break;

		case 'p':
			// "pwd".
			if (String_StartsWithC(input, "pwd")
				&& (String_Length(input) == 3 || String_At(input, 3) == ' ')) {
				String curDir = GetCurDir();
				printf("%s\n\n", String_ToC(curDir));
				return ProcessedCommand;
			}
			break;

		case 'q':
			// "quit".
			if (String_StartsWithC(input, "quit")
				&& (String_Length(input) == 4 || String_At(input, 4) == ' ')) {
				printf("\n");
				return ExitCommand;
			}
			break;

		case 'r':
			// Handle "run", possibly with arguments.
			if (String_StartsWithC(input, "run")
				&& (String_Length(input) == 3 || String_At(input, 3) == ' ')) {
				printf("Error: The 'run' command is not yet supported.\n\n");
				return ProcessedCommand;
			}
			break;
	}

	return UnknownCommand;
}
