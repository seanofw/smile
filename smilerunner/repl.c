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
#include <stdlib.h>

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include "vendor/wineditline-2.202/include/editline/readline.h"
#endif

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
				prefix = "! Warning:";
				break;

			case PARSEMESSAGE_ERROR:
				shouldPrint = True;
				prefix = "? Error:";
				hasErrors = True;
				break;
		}

		if (!shouldPrint) continue;

		position = parseMessage->position;
		if (position->filename != NULL) {
			if (position->line > 0) {
				// Have a filename and a line number.
				message = String_Format("%s %S:%d: %S\r\n", prefix, position->filename, position->line, parseMessage->message);
			}
			else {
				// Have a filename but no line number.
				message = String_Format("%s %S: %S\r\n", prefix, position->filename, parseMessage->message);
			}
		}
		else {
			// Have no filename.
			message = String_Format("%s %S\r\n", prefix, parseMessage->message);
		}

		fwrite(String_GetBytes(message), 1, String_Length(message), stderr);
	}

	return hasErrors;
}


static Int ParseAndEval(String string, ParseScope globalScope, ClosureInfo globalClosureInfo, SmileObject *result)
{
	Lexer lexer;
	Parser parser;
	SmileObject parsedScript;
	EvalResult evalResult;

	lexer = Lexer_Create(string, 0, String_Length(string), NULL, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;
	parser = Parser_Create();

	parsedScript = Parser_Parse(parser, lexer, globalScope);

	if (parser->firstMessage != NullList) {
		Bool hasErrors = PrintParseMessages(parser);
		if (hasErrors) {
			*result = NullObject;
			return 1;
		}
	}

	// If the script is wrapped with a [$scope], merge that scope's local variables into
	// the global scope so the user gets more-or-less what they expect:  Their statement's
	// declared variables will leak into the global parsing scope and global execution closure.
	if (SMILE_KIND(parsedScript) == SMILE_KIND_LIST
		&& SMILE_KIND(((SmileList)parsedScript)->a) == SMILE_KIND_SYMBOL
		&& ((SmileSymbol)((SmileList)parsedScript)->a)->symbol == SMILE_SPECIAL_SYMBOL__SCOPE) {

		SmileList scopeList = (SmileList)parsedScript;
		SmileList variableList = (SmileList)LIST_SECOND(scopeList);
		
		// Walk the variable list, and declare them in the global parsing scope and the global
		// execution closure.
		for (; SMILE_KIND(variableList) != SMILE_KIND_NULL; variableList = LIST_REST(variableList)) {
			SmileSymbol smileSymbol = (SmileSymbol)LIST_FIRST(variableList);
			ParseScope_DeclareHere(globalScope, smileSymbol->symbol, PARSEDECL_VARIABLE, NULL, NULL);
			Smile_SetGlobalVariable(smileSymbol->symbol, NullObject);
		}

		// Construct a [$progn] in place of the [$scope] for the rest of its arguments.
		parsedScript = (SmileObject)SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__PROGN),
			(SmileObject)LIST_REST(LIST_REST(scopeList))
		);
	}

	evalResult = Smile_EvalInScope(globalClosureInfo, parsedScript);

	switch (evalResult->evalResultKind) {

		case EVAL_RESULT_EXCEPTION:
			{
				SmileArg unboxedException = SmileArg_Unbox(evalResult->exception);
				String stringified = SMILE_VCALL1(unboxedException.obj, toString, unboxedException.unboxed);
				String message = String_Format("Uncaught exception thrown: %S\r\n", stringified);
				fwrite(String_GetBytes(message), 1, String_Length(message), stderr);
				fflush(stderr);
				*result = NullObject;
			}
			return 1;

		case EVAL_RESULT_BREAK:
			{
				String message = String_Format("Stopped at breakpoint.\r\n");
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

static String GetCurDir()
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		UInt16 buffer[MAX_PATH + 2];
		String result;

		if (!GetCurrentDirectoryW(MAX_PATH + 1, buffer)) {
			printf("Error: Unable to get current directory.\n");
			return String_Empty;
		}

		result = String_FromUtf16(buffer, wcslen(buffer));
		return result;
#	else
#		error Unsupported OS.
#	endif
}

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
	static String GetLastErrorString()
	{
		DWORD errorMessageID;
		LPWSTR messageBuffer;
		Int messageLength;
		String result;

		errorMessageID = GetLastError();
		if (!errorMessageID)
			return String_Empty;

		messageBuffer = NULL;
		messageLength = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorMessageID,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&messageBuffer,
			0,
			NULL
		);

		result = String_FromUtf16(messageBuffer, messageLength);

		LocalFree(messageBuffer);

		return result;
	}
#endif

static void SetCurDir(String path)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		UInt16 *path16 = String_ToUtf16(path, NULL);

		if (!SetCurrentDirectoryW(path16)) {
			printf("Error: %s", String_ToC(GetLastErrorString()));
		}
#	else
#		error Unsupported OS.
#	endif
}

static SmileList GetRawFileList(Bool includeAllFiles)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		WIN32_FIND_DATAW findData;
		HANDLE findFileHandle;
		SmileList head = NullList, tail = NullList;

		findFileHandle = FindFirstFileW(L"*", &findData);
		if (findFileHandle == NULL) {
			printf("Error: %s", String_ToC(GetLastErrorString()));
		}

		if (includeAllFiles || findData.cFileName[0] != '.') {
			LIST_APPEND(head, tail, String_FromUtf16(findData.cFileName, wcslen(findData.cFileName)));
		}

		while (FindNextFileW(findFileHandle, &findData)) {
			if (includeAllFiles || findData.cFileName[0] != '.') {
				LIST_APPEND(head, tail, String_FromUtf16(findData.cFileName, wcslen(findData.cFileName)));
			}
		}

		FindClose(findFileHandle);

		return head;
#	else
#		error Unsupported OS.
#	endif
}

static void ListFiles(void)
{
	SmileList fileList;

	fileList = GetRawFileList(False);

	for (; SMILE_KIND(fileList) != SMILE_KIND_NULL; fileList = LIST_REST(fileList)) {
		String fileName = (String)LIST_FIRST(fileList);
		printf("%s\n", String_ToC(fileName));
	}
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
#	else
#		error Unsupported OS.
#	endif
}

void ReplMain()
{
	ClosureInfo globalClosureInfo;
	ParseScope globalScope;
	SmileObject result;
	String resultText;

	String cdSpaceString = String_FromC("cd ");
	String cdSlashString = String_FromC("cd/");
	String cdBackslashString = String_FromC("cd\\");
	String cdDotString = String_FromC("cd.");
	String cdString = String_FromC("cd");
	String clearString = String_FromC("clear");
	String clsString = String_FromC("cls");
	String lsString = String_FromC("ls");
	String lsSpaceString = String_FromC("ls ");
	String pwdString = String_FromC("pwd");
	String exitString = String_FromC("exit");
	String quitString = String_FromC("quit");

	globalClosureInfo = SetupGlobalClosureInfo();
	globalScope = ParseScope_CreateRoot();
	ParseScope_DeclareVariablesFromClosureInfo(globalScope, globalClosureInfo);

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		SetConsoleCtrlHandler(NULL, True);
#	endif

	for (;;) {
		String input;
		char *line = readline("] ");
		if (line == NULL) break;
		input = String_FromC(line);

		input = String_Trim(input);
		if (String_Length(input) == 0) continue;

		add_history(line);
		free(line);

		switch (String_At(input, 0)) {
			case 'c':
				if (String_StartsWith(input, cdSpaceString)
					|| String_StartsWith(input, cdSlashString)
					|| String_StartsWith(input, cdBackslashString)
					|| String_StartsWith(input, cdDotString)
					|| String_Equals(input, cdString)) {
					String newDir = String_Trim(String_SubstringAt(input, 2));
					if (!String_IsNullOrEmpty(newDir)) {
						SetCurDir(newDir);
						printf("\n");
					}
					else {
						String curDir = GetCurDir();
						printf("%s\n\n", String_ToC(curDir));
					}
					continue;
				}
				if (String_Equals(input, clsString)
					|| String_Equals(input, clearString)) {
					ClearScreen();
					continue;
				}
				break;
			case 'l':
				if (String_StartsWith(input, lsSpaceString)
					|| String_Equals(input, lsString)) {
					ListFiles();
					printf("\n");
					continue;
				}
				break;
			case 'p':
				if (String_Equals(input, pwdString)) {
					String curDir = GetCurDir();
					printf("%s\n\n", String_ToC(curDir));
					continue;
				}
				break;
			case 'e':
				if (String_Equals(input, exitString)) {
					printf("\n");
					goto done;
				}
				break;
			case 'q':
				if (String_Equals(input, quitString)) {
					printf("\n");
					goto done;
				}
				break;
		}

		if (!ParseAndEval(input, globalScope, globalClosureInfo, &result))
		{
			resultText = SmileObject_Stringify(result);
			printf("%s\n", String_ToC(resultText));
		}

		printf("\n");
	}

done:
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
		SetConsoleCtrlHandler(NULL, False);
#	else
		;
#	endif
}
