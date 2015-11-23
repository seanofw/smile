// This file was auto-generated.  Do not edit!

EXTERN_TEST_SUITE(Int32DictTests);
EXTERN_TEST_SUITE(Real128Tests);
EXTERN_TEST_SUITE(Real32Tests);
EXTERN_TEST_SUITE(Real64Tests);
EXTERN_TEST_SUITE(StringCoreTests);
EXTERN_TEST_SUITE(StringDictTests);
EXTERN_TEST_SUITE(StringExtraTests);
EXTERN_TEST_SUITE(StringFormatTests);
EXTERN_TEST_SUITE(StringHtmlTests);
EXTERN_TEST_SUITE(StringIntDictTests);
EXTERN_TEST_SUITE(StringParseTests);
EXTERN_TEST_SUITE(StringUnicodeTests);
EXTERN_TEST_SUITE(SymbolTableTests);
EXTERN_TEST_SUITE(_LexerCoreTests);

void RunAllTests()
{
	TestSuiteResults *results = CreateEmptyTestSuiteResults();

	RUN_TEST_SUITE(results, Int32DictTests);
	RUN_TEST_SUITE(results, Real128Tests);
	RUN_TEST_SUITE(results, Real32Tests);
	RUN_TEST_SUITE(results, Real64Tests);
	RUN_TEST_SUITE(results, StringCoreTests);
	RUN_TEST_SUITE(results, StringDictTests);
	RUN_TEST_SUITE(results, StringExtraTests);
	RUN_TEST_SUITE(results, StringFormatTests);
	RUN_TEST_SUITE(results, StringHtmlTests);
	RUN_TEST_SUITE(results, StringIntDictTests);
	RUN_TEST_SUITE(results, StringParseTests);
	RUN_TEST_SUITE(results, StringUnicodeTests);
	RUN_TEST_SUITE(results, SymbolTableTests);
	RUN_TEST_SUITE(results, _LexerCoreTests);

	DisplayTestSuiteResults(results);
}

