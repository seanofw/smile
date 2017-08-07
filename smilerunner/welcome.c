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

#include "style.h"

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

static void PrintSmileWelcomeUTF8(void)
{
	printf_styled(
		"\n"
		" \033[0;36;44m\xE2\x96\x88\033[0;34;46m\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85"
			"\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85"
			"\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85"
			"\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\xE2\x96\x85\033[0;36;44m\xE2\x96\x88\033[0;34m\xE2\x94\x90\n"

		" \033[0;36;44m\xE2\x96\x88 \033[0;33;44;1mSmile Programming Language \033[0;36;44m\xE2\x96\x88\033[0;34m\xE2\x94\x82\n"
		" \033[0;36;44m\xE2\x96\x88  \033[1mv%d.%d / %s  \033[0;36;44m\xE2\x96\x88\033[0;34m\xE2\x94\x82\n"

		" \033[0;36;44m\xE2\x96\x88\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83"
			"\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83"
			"\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83"
			"\xE2\x96\x83\xE2\x96\x83\xE2\x96\x83\xE2\x96\x88\033[0;34m\xE2\x94\x82\n"

		" \033[0;34;40m\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80"
			"\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80"
			"\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80"
			"\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x98\033[0m\n"

		"\033[0;37;1m       Welcome to Smile! \033[33;1m:-)\033[0m\n"
		"\n"
		" For help, type \"\033[0;1;36mhelp\033[0m\" and press Enter.\n"
		"\n",

		SMILE_MAJOR_VERSION, SMILE_MINOR_VERSION, BUILDSTRING);
}

static void PrintSmileWelcomeCodePage437(void)
{
	printf_styled(
		"\n"
		" \033[0;36;44m\xDB\033[0;34;46m\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC"
			"\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\033[0;36;44m\xDB\033[0;34m\xBF\n"

		" \033[0;36;44m\xDB \033[0;33;44;1mSmile Programming Language \033[0;36;44m\xDB\033[0;34m\xB3\n"
		" \033[0;36;44m\xDB  \033[1mv%d.%d / %s  \033[0;36;44m\xDB\033[0;34m\xB3\n"

		" \033[0;36;44m\xDB\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC\xDC"
			"\xDC\xDC\xDC\xDC\xDC\xDC\xDB\033[0;34m\xB3\n"
		" \033[0;34;40m\xC0\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4"
			"\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xD9\033[0m\n"

		"\033[0;37;1m       Welcome to Smile! \033[33;1m:-)\033[0m\n"
		"\n"
		" For help, type \"\033[0;1;36mhelp\033[0m\" and press Enter.\n"
		"\n",

		SMILE_MAJOR_VERSION, SMILE_MINOR_VERSION, BUILDSTRING);
}

static void PrintSmileWelcomeASCII(void)
{
	printf_styled(
		"\n"
		" \033[0;36;44m+----------------------------+\033[0m\n"
		" \033[0;36;44m| \033[0;33;44;1mSmile Programming Language \033[0;36;44m|\033[0m\n"
		" \033[0;36;44m|  \033[1mv%d.%d / %s  \033[0;36;44m|\033[0m\n"
		" \033[0;36;44m+----------------------------+\033[0m\n"
		"\n"
		"\033[0;37;1m       Welcome to Smile! \033[33;1m:-)\033[0m\n"
		"\n"
		" For help, type \"\033[0;1;36mhelp\033[0m\" and press Enter.\n"
		"\n",

		SMILE_MAJOR_VERSION, SMILE_MINOR_VERSION, BUILDSTRING);
}

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
static Bool WindowsSupportsCodePage437 = False;

static BOOL CodePageEnumProc(LPTSTR lpCodePageString)
{
	if (!wcscmp(lpCodePageString, L"437"))
		WindowsSupportsCodePage437 = True;
	return TRUE;
}
#endif

void PrintSmileWelcome(void)
{
#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY)
	// Only systems that declare UTF-8 support can do the pretty box-drawing characters.
	String lang = String_FromC(getenv("LANG"));
	if (String_EndsWithC(String_ToLower(lang), ".utf8") || String_EndsWithC(String_ToLower(lang), ".utf-8")) {
		PrintSmileWelcomeUTF8();
	}
	else {
		PrintSmileWelcomeASCII();
	}

#	elif ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_WINDOWS_FAMILY)
	// Windows can't do UTF-8 box-drawing characters, but it often CAN do IBM code page 437 box-drawing!
	EnumSystemCodePages(CodePageEnumProc, CP_INSTALLED);
	if (WindowsSupportsCodePage437) {
		SetConsoleOutputCP(437);
		PrintSmileWelcomeCodePage437();
		SetConsoleOutputCP(CP_UTF8);
	}
	else {
		PrintSmileWelcomeASCII();
	}

#	else
	PrintSmileWelcomeASCII();
#	endif
}
