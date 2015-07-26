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

typedef int (*TestFunc)();
typedef void (*TestFuncInternal)();

typedef struct {
	int numSuccesses;
	int numFailures;
} TestSuiteResults;

int RunTestInternal(const char *name, const char *file, int line, TestFuncInternal testFuncInternal);
int EndTestInternal();
int FailTestInternal(const char *message);
TestSuiteResults *RunTestSuiteInternal(const char *name, TestFunc *funcs, int numFuncs);
void DisplayTestSuiteResults(TestSuiteResults *results);

#define START_TEST(__name__) \
	static void __name__##Internal(); \
	static int __name__() \
	{ \
		return RunTestInternal(#__name__, __FILE__, __LINE__, __name__##Internal); \
	} \
	static void __name__##Internal() \
	{

#define ASSERT(__n__) \
	((!(__n__)) ? FailTestInternal(#__n__) : 0)

#define PASS_TEST \
		return

#define FAIL_TEST(__message__) \
		FailTestInternal(__message__)

#define END_TEST \
	}

#define EXTERN_TEST_SUITE(__name__) \
	extern int __name__##Count; \
	extern TestFunc __name__[]

#define TEST_SUITE(__name__)

#define START_TEST_SUITE(__name__) \
	TestFunc __name__[] = 
#define END_TEST_SUITE(__name__) \
	; \
	int __name__##Count = sizeof(__name__) / sizeof(TestFunc);

#define RUN_TEST_SUITE(__results__, __name__) \
	AggregateTestSuiteResults(__results__, RunTestSuiteInternal(#__name__, __name__, __name__##Count));

Inline void AssertString(String str, const char *expectedString, Int expectedLength)
{
	ASSERT(str != NULL);

	ASSERT(String_Length(str) == expectedLength);

	ASSERT(String_GetBytes(str)[expectedLength] == '\0');

	if (expectedLength > 0) {
		ASSERT(!MemCmp(String_GetBytes(str), expectedString, expectedLength));
	}
	else {
		ASSERT(str == String_Empty);
	}
}

Inline void AggregateTestSuiteResults(TestSuiteResults *dest, TestSuiteResults *src)
{
	dest->numSuccesses += src->numSuccesses;
	dest->numFailures += src->numFailures;
}

Inline TestSuiteResults *CreateEmptyTestSuiteResults()
{
	TestSuiteResults *results = GC_MALLOC_STRUCT(TestSuiteResults);
	if (results == NULL) Smile_Abort_OutOfMemory();
	results->numFailures = results->numSuccesses = 0;
	return results;
}

#endif
