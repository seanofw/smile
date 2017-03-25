//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
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

extern void RunAllTests();

extern const char *TestSuiteNames[];
extern int NumTestSuites;

static void PrintHelp()
{
	int i;

	printf(
		"Usage: smilelibtests [options] test1 test2 test3\n"
		"\n"
		"Options:\n"
		"  -h --help         You're looking at it.\n"
		"  -q --quiet        Don't display successful tests; only show errors.\n"
		"  -i --interactive  Wait for user input when a test fails or after a run.\n"
		"\n"
		"Test fixture names:\n"
	);

	for (i = 0; i < NumTestSuites; i++) {
		printf("  %s\n", TestSuiteNames[i]);
	}

	printf(
		"\n"
		"Tests may be specified with a wildcard (for example, \"string*\").\n"
		"If no tests are explicitly named, all tests will be run.\n"
		"\n"
	);
}

static Bool ParseCommandLine(int argc, const char **argv)
{
	int i;
	Array array = NULL;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "-?")) {
				PrintHelp();
				return False;
			}
			else if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) {
				QuietMode = True;
			}
			else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--interactive")) {
				InteractiveMode = True;
			}
			else {
				fprintf(stderr, "Unknown command-line argument \"%s\".\n", argv[i]);
			}
		}
		else {
			if (array == NULL)
				array = Array_Create(sizeof(const char *), 16, False);
			*(const char **)Array_Push(array) = argv[i];
		}
	}

	if (array != NULL) {
		RequestedTests = (const char **)array->data;
		NumRequestedTests = (int)array->length;
	}
	else {
		RequestedTests = NULL;
		NumRequestedTests = 0;
	}

	return True;
}

int main(int argc, const char **argv)
{
	if (!ParseCommandLine(argc, argv))
		return -1;

	if (!QuietMode)
		printf(" Initializing the Smile runtime environment.\n");

	Smile_Init();

	if (!QuietMode)
		printf(" Running all unit tests...\n\n");

	RunAllTests();

	if (InteractiveMode) {
		printf(" Press any key to exit the test runner. ");
		WaitForAnyKey();
	}

	Smile_End();

	return 0;
}

#include "testsuites.generated.inc"
