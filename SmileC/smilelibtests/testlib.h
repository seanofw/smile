//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2015 Sean Werkema
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

#ifndef __SMILELIBTESTS_TESTLIB_H__
#define __SMILELIBTESTS_TESTLIB_H__

// These macros are similar to those of the 'check' library, but they don't attempt to do
// everything that 'check' does.  This is just enough functionality to be able to write
// reasonable in-process unit tests.

//-------------------------------------------------------------------------------------------------
//  Internal types

/// <summary>
/// The actual type of a test function, which is actually invisible to the test itself (the test
/// is wrapped in an inner function with a different type).
/// </summary>
/// <returns>1 if the test succeeded, or 0 if the test failed.</returns>
typedef int (*TestFunc)();

/// <summary>
/// The apparent type of a test function, which is actually the type of the internal function that
/// wraps the test.  Takes no parameters, returns no parameters.
/// </summary>
typedef void (*TestFuncInternal)();

//-------------------------------------------------------------------------------------------------
//  Public type declarations

/// <summary>
/// A collection of results from a given test suite.
/// </summary>
typedef struct {
	int numSuccesses;
	int numFailures;
} TestSuiteResults;

//-------------------------------------------------------------------------------------------------
//  External parts of the implementation

int RunTestInternal(const char *name, const char *file, int line, TestFuncInternal testFuncInternal);
int FailTestInternal(const char *message);
void AssertStringInternal(String str, const char *expectedString, Int expectedLength, const char *message);
TestSuiteResults *RunTestSuiteInternal(const char *name, TestFunc *funcs, int numFuncs);
void DisplayTestSuiteResults(TestSuiteResults *results);
void WaitForAnyKey(void);

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation (macros)

/// <summary>
/// Declare the beginning of a unit test with the given name (unique to this C source file).
/// </summary>
/// <param name="__name__">The name of the unit test.</param>
#define START_TEST(__name__) \
	static void __name__##Internal(); \
	static int __name__() \
	{ \
		return RunTestInternal(#__name__, __FILE__, __LINE__, __name__##Internal); \
	} \
	static void __name__##Internal() \
	{

/// <summary>
/// Assert an invariant during a unit test.
/// </summary>
/// <param name="__n__">The invariant to assert.  If this results in zero or NULL,
/// the test will be aborted as a failure.</param>
#define ASSERT(__n__) \
	((!(__n__)) ? FailTestInternal(#__n__) : 0)

/// <summary>
/// Assert that the given string is a valid string and matches the given text and length.
/// This will fail the current unit test if this string does not match.
/// </summary>
/// <param name="str">The string to validate.</param>
/// <param name="expectedString">The expected contents of that string.</param>
/// <param name="expectedLength">The expected length of that string.</param>
#define ASSERT_STRING(__str__, __expectedString__, __expectedLength__) \
	AssertStringInternal(__str__, __expectedString__, __expectedLength__, #__str__ " is not correct (line " TOSTRING_AT_COMPILE_TIME(__LINE__) "):")

/// <summary>
/// Immediately pass (as good) the current unit test.  This stops the test's
/// execution and skips any and all code remaining in it.
/// </summary>
#define PASS_TEST \
		return

/// <summary>
/// Immediately fail (as bad) the current unit test.  This stops the test's
/// execution and skips any and all code remaining in it.
/// </summary>
/// <param name="__message__" type="const char *">A message string to display to the user as to why the test failed.</param>
#define FAIL_TEST(__message__) \
		FailTestInternal(__message__)

/// <summary>
/// Declare the end the current unit test.
/// </summary>
#define END_TEST \
	}

/// <summary>
/// Insert suitable 'extern' declarations for the given named test suite.
/// </summary>
/// <param name="__name__">The name of the test suite to import.</param>
#define EXTERN_TEST_SUITE(__name__) \
	extern int __name__##Count; \
	extern TestFunc __name__[]

/// <summary>
/// Declare the name of the current test suite (i.e., the formal name of all the
/// tests found in this file).
/// </summary>
/// <param name="__name__">The name of this test suite.</param>
#define TEST_SUITE(__name__)

/// <summary>
/// Declare the start of the list of the tests in this test suite.  This is for
/// internal use and is automatically generated.
/// </summary>
/// <param name="__name__">The name of this test suite.</param>
#define START_TEST_SUITE(__name__) \
	TestFunc __name__[] = 

/// <summary>
/// Declare the end of the list of the tests in this test suite.  This is for
/// internal use and is automatically generated.
/// </summary>
/// <param name="__name__">The name of this test suite.</param>
#define END_TEST_SUITE(__name__) \
	; \
	int __name__##Count = sizeof(__name__) / sizeof(TestFunc);

/// <summary>
/// Run the given named test suite, adding its results to the given collection
/// of results.
/// </summary>
/// <param name="__name__">The name of the test suite to run.</param>
/// <param name="__results__" type="TestSuiteResults *">The collection of TestSuiteResults to which this test's results should be added.</param>
#define RUN_TEST_SUITE(__results__, __name__) \
	AggregateTestSuiteResults(__results__, RunTestSuiteInternal(#__name__, __name__, __name__##Count));

//-------------------------------------------------------------------------------------------------
//  Inline parts of the implementation (actual functions)

Inline void AggregateTestSuiteResults(TestSuiteResults *dest, TestSuiteResults *src)
{
	dest->numSuccesses += src->numSuccesses;
	dest->numFailures += src->numFailures;
}

Inline TestSuiteResults *CreateEmptyTestSuiteResults(void)
{
	TestSuiteResults *results = GC_MALLOC_STRUCT(TestSuiteResults);
	if (results == NULL) Smile_Abort_OutOfMemory();
	results->numFailures = results->numSuccesses = 0;
	return results;
}

#endif
