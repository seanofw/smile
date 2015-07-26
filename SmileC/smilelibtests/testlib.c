#include "stdafx.h"
#include <setjmp.h>

#include "testlib.h"

static jmp_buf TestJmpBuf;

int RunTestInternal(const char *name, const char *file, int line, TestFuncInternal testFuncInternal)
{
	printf("  %s: ", name);
	fflush(stdout);
	if (!setjmp(TestJmpBuf)) {
		testFuncInternal();
		printf("OK\n");
		fflush(stdout);
		return 1;
	}
	printf("    at %s: %d\n    ========\n", file, line);
	fflush(stdout);
	return 0;
}

int FailTestInternal(const char *message)
{
	printf("FAILED\n    ========\n    Failed assertion was:\n      %s\n", message);
	fflush(stdout);
	longjmp(TestJmpBuf, 1);
	return 0;	// Never reached.
}

TestSuiteResults *RunTestSuiteInternal(const char *name, TestFunc *funcs, int numFuncs)
{
	int numSuccesses, numFailures, succeeded, i;
	TestSuiteResults *results;

	printf("Test suite %s:\n", name);
	fflush(stdout);

	numSuccesses = 0, numFailures = 0;
	for (i = 0; i < numFuncs; i++, funcs++) {
		succeeded = (*funcs)();
		if (succeeded) numSuccesses++;
		else numFailures++;
	}

	printf("\n");

	results = GC_MALLOC_STRUCT(TestSuiteResults);
	if (results == NULL) Smile_Abort_OutOfMemory();
	results->numFailures = numFailures;
	results->numSuccesses = numSuccesses;
	return results;
}

void DisplayTestSuiteResults(TestSuiteResults *results)
{
	printf("Test suite results:  %d tests succeeded, %d tests failed.\n\n", results->numSuccesses, results->numFailures);
	fflush(stdout);
}
