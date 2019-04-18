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

extern TestSuiteResults *RunAllTests();

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

#if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY) && (SMILE_COMPILER == SMILE_COMPILER_GCC)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
	unsigned long     uc_flags;
	struct ucontext   *uc_link;
	stack_t           uc_stack;
	struct sigcontext uc_mcontext;
	sigset_t          uc_sigmask;
} sig_ucontext_t;

void SegFaultHandler(int sig_num, siginfo_t * info, void * ucontext)
{
	void *             array[50];
	void *             caller_address;
	char **            messages;
	int                size, i;
	sig_ucontext_t *   uc;

	uc = (sig_ucontext_t *)ucontext;

	/* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
	caller_address = (void *)uc->uc_mcontext.eip; // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
	caller_address = (void *)uc->uc_mcontext.rip; // RIP: x86_64 specific
#else
#error Unsupported architecture. // TODO: Add support for other arch.
#endif

	fprintf(stderr, "signal %d (%s), address is %p from %p\n",
		sig_num, strsignal(sig_num), info->si_addr,
		(void *)caller_address);

	size = backtrace(array, 50);

	/* overwrite sigaction with caller's address */
	array[1] = caller_address;

	messages = backtrace_symbols(array, size);

	/* skip first stack frame (points here) */
	for (i = 1; i < size && messages != NULL; ++i) {
		fprintf(stderr, "at (%d): %s\n", i, messages[i]);
	}

	free(messages);

	exit(1);
}

#endif

int main(int argc, const char **argv)
{
	TestSuiteResults *results;

#	if ((SMILE_OS & SMILE_OS_FAMILY) == SMILE_OS_UNIX_FAMILY) && (SMILE_COMPILER == SMILE_COMPILER_GCC)
	{
		struct sigaction sigact;

		sigact.sa_sigaction = SegFaultHandler;
		sigact.sa_flags = SA_RESTART | SA_SIGINFO;

		if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0) {
			fprintf(stderr, "Unable to change signal handler for SEGV: (%s)\n", strsignal(SIGSEGV));
			exit(-2);
		}
	}
#	endif

	if (!ParseCommandLine(argc, argv))
		return -1;

	if (!QuietMode)
		printf(" Initializing the Smile runtime environment.\n");

	Smile_Init();

	if (!QuietMode)
		printf(" Running all unit tests...\n\n");

	results = RunAllTests();

	if (InteractiveMode) {
		printf(" Press any key to exit the test runner. ");
		fflush(stdout);
		WaitForAnyKey();
	}

	Smile_End();

	return (results->numFailures > 0);
}

#include "testsuites.generated.inc"
