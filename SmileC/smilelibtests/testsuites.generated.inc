// This file was auto-generated.  Do not edit!

EXTERN_TEST_SUITE(StringCoreTests);
EXTERN_TEST_SUITE(StringExtraTests);
EXTERN_TEST_SUITE(StringFormatTests);
EXTERN_TEST_SUITE(StringHtmlTests);

void RunAllTests()
{
	TestSuiteResults *results = CreateEmptyTestSuiteResults();

	RUN_TEST_SUITE(results, StringCoreTests);
	RUN_TEST_SUITE(results, StringExtraTests);
	RUN_TEST_SUITE(results, StringFormatTests);
	RUN_TEST_SUITE(results, StringHtmlTests);

	DisplayTestSuiteResults(results);
}

