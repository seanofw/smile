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
#include <setjmp.h>

#ifdef _WIN32
	#include <conio.h>
	#include "ansicolor-w32.h"
#else
	#include <termios.h>
#endif

#include "testlib.h"

const char **RequestedTests = NULL;
int NumRequestedTests = 0;
Bool QuietMode = False;
Bool InteractiveMode = False;

/// <summary>
/// This jump-buffer is used to abort failed tests by unwinding the stack to the point
/// before the test ran.
/// </summary>
static jmp_buf TestJmpBuf;

/// <summary>
/// This exists to temporarily hold the test-failure message across the long-jump that
/// unwinds the stack.
/// </summary>
static const char *TestFailureMessage = NULL;

/// <summary>
/// This exists to temporarily hold the test-failure file across the long-jump that
/// unwinds the stack.
/// </summary>
static const char *TestFailureFile = NULL;

/// <summary>
/// This exists to temporarily hold the test-failure line across the long-jump that
/// unwinds the stack.
/// </summary>
static int TestFailureLine = 0;

/// <summary>
/// Assert that the given string is a valid string and matches the given text and length.
/// This will fail the current unit test if this string does not match.
/// </summary>
/// <param name="str">The string to validate.</param>
/// <param name="expectedString">The expected contents of that string.</param>
/// <param name="expectedLength">The expected length of that string.</param>
/// <param name="message">A message to display to the user to explain why the test failed.</param>
/// <returns>Non-NULL if there was an error, or a string error message (in a static buffer).</returns>
const char *TestStringInternal(String str, const char *expectedString, Int expectedLength, const char *message)
{
	static char buffer[1024];

	if (str == NULL) {
		sprintf(buffer, "%s: actual string is a NULL pointer", message);
		return buffer;
	}

	if (String_Length(str) != expectedLength) {
		sprintf(buffer, "%s: actual string length is %d", message, (int)String_Length(str));
		return buffer;
	}

	if (String_GetBytes(str)[expectedLength] != '\0') {
		sprintf(buffer, "%s: actual string is missing '\\0' after end", message);
		return buffer;
	}

	if (expectedLength > 0) {
		if (MemCmp(String_GetBytes(str), expectedString, expectedLength)) {
			sprintf(buffer, "%s: actual string bytes do not match", message);
			return buffer;
		}
	}
	else {
		if (str != String_Empty) {
			sprintf(buffer, "%s: actual string is not the String_Empty singleton", message);
			return buffer;
		}
	}

	return NULL;
}

/// <summary>
/// Print out an "OK" message to show the test succeeded.
/// </summary>
/// <param name="totalTicks">The total number of ticks this test executed in.</param>
static void PrintTestSuccess(UInt64 totalTicks)
{
	double usec;
	double timing;
	int precision;
	const char *units;

	// Compute the nicest way to print out the timing metrics.
	usec = (double)Smile_TicksToMicroseconds(totalTicks);
	if (usec >= 1000000.0) {
		precision = usec > 100000000.0 ? 0 : usec > 10000000.0 ? 1 : 2;
		timing = usec / 1000000.0;
		units = "s";
	}
	else if (usec >= 1000.0) {
		precision = usec > 100000.0 ? 0 : usec > 10000.0 ? 1 : 2;
		timing = usec / 1000.0;
		units = "ms";
	}
	else {
		precision = 0;
		timing = usec;
		units = "us";
	}

	// Print the message.
	printf("\x1B[0;1;32mOK\x1B[0m \x1B[36m(%.*f %s)\x1B[0m\n", precision, timing, units);

	// Flush it to the output, which ensures that it gets seen even if the next test fails.
	fflush(stdout);
}

/// <summary>
/// Print a big red failure message for this test.
/// </summary>
/// <param name="message">The message describing what failed in this test.</param>
/// <param name="file">The file in which the failure occurred.</param>
/// <param name="line">The line on which the failure occurred in that file.</param>
static void PrintTestFailure(const char *message, const char *file, int line)
{
	printf(
		"\x1B[0;1;37;41m FAILED \x1B[0m\n"
		"    \x1B[1;31m________________________________________________________________\x1B[0;1;31m\n"
		"\n"
		"    \x1B[31mFailed assertion was:\n"
		"      \x1B[33m%s\x1B[0m\n"
		"\n"
		"\x1B[0;1;31m    at %s%s: %d\n"
		"    \x1B[1;31m________________________________________________________________\x1B[0m\n"
		"\n",
		message,
		strlen(file) > 60 ? "..." : "",
		strlen(file) > 60 ? file + strlen(file) - 60 : file,
		line
	);

	// Flush it to the output, which ensures that it gets seen even if the next test also fails.
	fflush(stdout);
}

static void AddTimingResult(TestSuiteResults *results, const char *name, UInt64 ticks)
{
	Int i;
	const Int numSlowestTests = sizeof(results->slowestTests) / sizeof(TestResult);

	// If this was faster than the fastest of the slow tests, get rid of it.
	if (ticks < results->slowestTests[numSlowestTests - 1].duration)
		return;

	// Add it to the set, in descending sorted order.
	for (i = numSlowestTests - 2; i >= 0; i--) {
		if (ticks > results->slowestTests[i].duration) {
			results->slowestTests[i + 1].name = results->slowestTests[i].name;
			results->slowestTests[i + 1].suiteName = results->slowestTests[i].suiteName;
			results->slowestTests[i + 1].duration = results->slowestTests[i].duration;
		}
		else {
			results->slowestTests[i + 1].name = name;
			results->slowestTests[i + 1].suiteName = results->name;
			results->slowestTests[i + 1].duration = ticks;
			break;
		}
	}
	if (i < 0) {
		results->slowestTests[0].name = name;
		results->slowestTests[0].suiteName = results->name;
		results->slowestTests[0].duration = ticks;
	}
}

/// <summary>
/// Given a new set of test results, merge its slowest tests in with the existing set of slowest
/// tests to get a combined set of the "most" slowest tests.
/// </summary>
void MergeSlowTests(TestResult *dest, const TestResult *src)
{
	TestResult temp[20];
	TestResult *src2 = temp;
	Int i;

	MemCpy(temp, dest, sizeof(TestResult) * 20);

	for (i = 0; i < 20; i++) {
		if (src2->duration > src->duration) {
			dest->duration = src2->duration;
			dest->name = src2->name;
			dest->suiteName = src2->suiteName;
			dest++;
			src2++;
		}
		else {
			dest->duration = src->duration;
			dest->name = src->name;
			dest->suiteName = src->suiteName;
			dest++;
			src++;
		}
	}
}

/// <summary>
/// Run the given test.
/// </summary>
/// <param name="str">The string to validate.</param>
/// <param name="expectedString">The expected contents of that string.</param>
/// <param name="expectedLength">The expected length of that string.</param>
/// <param name="message">A message to display to the user to explain why the test failed.</param>
int RunTestInternal(TestSuiteResults *results, const char *name, const char *file, int line, TestFuncInternal testFuncInternal)
{
	UInt64 startTicks, endTicks;

	if (!QuietMode) {
		// First, print the name of the test we're about to run.
		if (strlen(name) > 58) {
			printf("  %.15s...%.40s: ", name, name + strlen(name) - 40);
		}
		else {
			printf("  %s: ", name);
		}

		// Flush it to the output, which ensures that it gets seen even if this test fails.
		fflush(stdout);
	}

	// Set a marker so we can get back to this if-statement if the test fails.
	if (setjmp(TestJmpBuf)) {

		if (QuietMode)
			printf("%s: ", name);

		// Test failed, so print the message, and return that it failed.
		PrintTestFailure(TestFailureMessage,
			TestFailureFile != NULL ? TestFailureFile : file,
			TestFailureFile != NULL ? TestFailureLine : line);

		if (InteractiveMode) {
			printf("Press any key to continue testing...");
			fflush(stdout);
			WaitForAnyKey();
			printf("\n");
		}

		return 0;
	}

	// Run the test, with timing metrics.
	startTicks = Smile_GetTicks();
	testFuncInternal();
	endTicks = Smile_GetTicks();

	// Test succeeded, so print "OK", and return that it succeeded.
	if (!QuietMode)
		PrintTestSuccess(endTicks - startTicks);

	AddTimingResult(results, name, endTicks - startTicks);

	return 1;
}

/// <summary>
/// Cause the current test to fail, aborting all successive execution.
/// </summary>
/// <param name="message">The message to display to the user explaining why this test failed.</param>
/// <returns>Never returns.</returns>
int FailTestInternal(const char *message)
{
	return FailTestWithLineInternal(message, NULL, 0);
}

/// <summary>
/// Cause the current test to fail, aborting all successive execution.
/// </summary>
/// <param name="message">The message to display to the user explaining why this test failed.</param>
/// <param name="file">The file in which the failure occurred.</param>
/// <param name="line">The line at which the failure occurred.</param>
/// <returns>Never returns.</returns>
int FailTestWithLineInternal(const char *message, const char *file, int line)
{
	char *safeMessage;

	// Save the message so the test runner can find it (and save it in case it gets stomped on).
	safeMessage = GC_MALLOC_ATOMIC(strlen(message) + 1);
	if (safeMessage == NULL)
		Smile_Abort_OutOfMemory();
	strcpy(safeMessage, message);
	TestFailureMessage = safeMessage;
	TestFailureFile = file;
	TestFailureLine = line;

	// Long-jump back to abort the test.  We pass '1', which is what will be returned by the
	// original setjmp() call.
	longjmp(TestJmpBuf, 1);

	return 0;	// Never reached.
}

/// <summary>
/// Determine if two strings match, case-insensitive.  This isn't intended for
/// super-fast or flexible needs, but it works.
/// </summary>
/// <param name="a">The first string to compare.</param>
/// <param name="b">The second string to compare.</param>
/// <returns>True if the strings match, false if they do not.</returns>
static Bool CompareStringsInsensitive(const char *a, const char *b)
{
	char ac, bc;

	while (*a) {
		ac = *a++;
		bc = *b++;

		if (ac >= 'A' && ac <= 'Z') ac += 'a' - 'A';
		if (bc >= 'A' && bc <= 'Z') bc += 'a' - 'A';

		if (ac != bc) return False;
	}

	return !*b;
}

/// <summary>
/// Given an ASCII character, perform simplistic case-folding on it by converting
/// uppercase characters to their lowercase equivalent.
/// </summary>
/// <param name="ch">The character to case-fold.</param>
/// <returns>The case-folded character, the same if it was not uppercase, or the lowercase
/// equivalent if it was uppercase.<returns>
Inline Byte CaseFold(Byte ch)
{
	if (ch >= 'A' && ch <= 'Z')
		return ch + 'a' - 'A';
	else
		return ch;
}

/// <summary>
/// Determine if a string matches a pattern containing wildcard characters, case-insensitive.
/// </summary>
/// <param name="pattern">The pattern to compare against.</param>
/// <param name="patternEnd">The end of the pattern to compare against (one past the last character; the
/// address of the '\0' if you're using NUL-terminated strings).</param>
/// <param name="text">The text string to test.</param>
/// <param name="textEnd">The end of the text string to test (one past the last character; the
/// address of the '\0' if you're using NUL-terminated strings).</param>
/// <returns>True if the string matches the pattern, false if it does not.</returns>
static Bool WildcardMatch(const Byte *pattern, const Byte *patternEnd, const Byte *text, const Byte *textEnd)
{
	Byte patternChar, textChar, nextPatternChar;

	while (pattern < patternEnd) {
		switch (patternChar = CaseFold(*pattern++)) {
		case '?':
			// If we ran out of characters, this is a fail.
			if (text == textEnd)
				return False;
			textChar = CaseFold(*text++);
			break;

		case '*':
			// Consume trailing '*' and '?' characters, since they don't mean much (except '?',
			// which adds mandatory filler space).
			while (pattern < patternEnd && ((patternChar = CaseFold(*pattern)) == '?' || patternChar == '*')) {
				if (patternChar == '?' && text == textEnd)
					return False;
				pattern++;
				text++;
			}

			// If we ran out of characters in the pattern, then this is a successful match,
			// since this star can consume everything after it in the text.
			if (pattern == patternEnd)
				return True;

			// Determine the next character in the text that we're searching for.
			nextPatternChar = patternChar;

			// Skim forward in the text looking for that next character, and then recursively
			// perform a pattern-match on the remainders of the pattern and text from there.
			// We use that next character to optimize the recursion, so that we don't recurse
			// if we know there won't be a match.
			while (text < textEnd) {
				textChar = CaseFold(*text);
				if (textChar == nextPatternChar && WildcardMatch(pattern, patternEnd, text, textEnd))
					return True;
				text++;
			}

			// None of the recursive searches matched, so this is a fail.
			return False;

		default:
			if (text == textEnd)
				return False;	// Ran out of characters.
			if (patternChar != CaseFold(*text++))
				return False;	// No match.
			break;
		}
	}

	return text == textEnd;
}

/// <summary>
/// Determine if this test suite is one we should be running or not.
/// </summary>
/// <param name="name">The name of a test suite.</param>
/// <returns>True if that test suite is in the set of requested tests, False if it should be skipped.</returns>
static Bool IsTestSuiteRequested(const char *name)
{
	int i;

	if (NumRequestedTests <= 0) return True;

	for (i = 0; i < NumRequestedTests; i++) {
		if (WildcardMatch(RequestedTests[i], RequestedTests[i] + strlen(RequestedTests[i]), name, name + strlen(name)))
			return True;
	}

	return False;
}

/// <summary>
/// Run all the tests in the given test suite, and return a new collection of results for it.
/// </summary>
/// <param name="name">The name of the test suite.</param>
/// <param name="funcs">An array of the functions in the test suite to be run.</param>
/// <param name="numFuncs">The number of functions to run.</param>
/// <returns>A new collection of results that describes the overall state of this test suite.</returns>
TestSuiteResults *RunTestSuiteInternal(const char *name, TestFunc *funcs, int numFuncs)
{
	int numSuccesses, numFailures, succeeded, i;
	TestSuiteResults *results;
	
	results = CreateEmptyTestSuiteResults();
	results->name = name;

	if (!IsTestSuiteRequested(name))
		return results;

	if (!QuietMode) {
		printf("\x1B[0;1;37m Test suite %s: \x1B[0m\n", name);
		fflush(stdout);
	}

	numSuccesses = 0, numFailures = 0;
	for (i = 0; i < numFuncs; i++, funcs++) {
		succeeded = (*funcs)(results);
		if (succeeded) numSuccesses++;
		else numFailures++;
	}

	if (!QuietMode)
		printf("\n");

	results->numFailures = numFailures;
	results->numSuccesses = numSuccesses;
	return results;
}

/// <summary>
/// Print a summary message for the given set of test-suite results.
/// </summary>
/// <param name="results">The results to print.</param>
void DisplayTestSuiteResults(TestSuiteResults *results)
{
	Int i;

	if (!QuietMode || results->numFailures > 0) {
		printf("\x1B[0;1;37m Test suite results:  \x1B[32m%d\x1B[37m tests succeeded,%s %d\x1B[37m tests failed. \x1B[0m\n\n",
			results->numSuccesses, results->numFailures > 0 ? " \x1B[1;33;41m" : "\x1B[1;32m", results->numFailures);
		fflush(stdout);
	}

	if (!QuietMode && results->numFailures == 0) {
		printf("\x1B[0;1;37m All tests pass.  Slowest tests were: \x1B[0m\n");
		for (i = 0; i < sizeof(results->slowestTests) / sizeof(TestResult); i++) {
			UInt64 time = Smile_TicksToMicroseconds(results->slowestTests[i].duration);
			printf("%6llu ms  %s.%s\n", (time + 500) / 1000, results->slowestTests[i].suiteName, results->slowestTests[i].name);
		}
		printf("\n");
	}
}

/// <summary>
/// Wait for any keystroke from the user.  Surprisingly complex and non-portable.
/// </summary>
void WaitForAnyKey(void)
{
	#ifdef _WIN32
		getch();
	#else
		struct termios oldInfo, info;

		// Get the current terminal behavior.
		tcgetattr(0, &oldInfo);

		// Disable canonical mode.
		tcgetattr(0, &info);
		info.c_lflag &= ~ICANON;
		info.c_cc[VMIN] = 1;
		info.c_cc[VTIME] = 0;
		tcsetattr(0, TCSANOW, &info);

		// Read one key.
		getchar();

		// Restore the terminal behavior to default.
		tcsetattr(0, TCSANOW, &oldInfo);
	#endif
}
