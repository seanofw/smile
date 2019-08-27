// This file was auto-generated.  Do not edit!

EXTERN_TEST_SUITE(ByteCodeTests);
EXTERN_TEST_SUITE(CompilerTests);
EXTERN_TEST_SUITE(EvalConstantTests);
EXTERN_TEST_SUITE(EvalCoreTests);
EXTERN_TEST_SUITE(EvalTests);
EXTERN_TEST_SUITE(HashTests);
EXTERN_TEST_SUITE(Int32DictTests);
EXTERN_TEST_SUITE(LexerCoreTests);
EXTERN_TEST_SUITE(LexerIdentifierTests);
EXTERN_TEST_SUITE(LexerLoanwordTests);
EXTERN_TEST_SUITE(LexerNumberTests);
EXTERN_TEST_SUITE(LexerPunctuationTests);
EXTERN_TEST_SUITE(LexerStringTests);
EXTERN_TEST_SUITE(LexerUnicodeTests);
EXTERN_TEST_SUITE(ParserClassicTests);
EXTERN_TEST_SUITE(ParserCoreTests);
EXTERN_TEST_SUITE(ParserFuncTests);
EXTERN_TEST_SUITE(ParserNewTests);
EXTERN_TEST_SUITE(ParserQuoteTests);
EXTERN_TEST_SUITE(ParserStmtTests);
EXTERN_TEST_SUITE(ParserStringTests);
EXTERN_TEST_SUITE(ParserSyntaxTableTests);
EXTERN_TEST_SUITE(ParserSyntaxTests);
EXTERN_TEST_SUITE(ParserSyntaxValidateTests);
EXTERN_TEST_SUITE(ParserSyntaxWalkTests);
EXTERN_TEST_SUITE(ParserTermTests);
EXTERN_TEST_SUITE(PointerSetTests);
EXTERN_TEST_SUITE(Real128Tests);
EXTERN_TEST_SUITE(Real32Tests);
EXTERN_TEST_SUITE(Real64Tests);
EXTERN_TEST_SUITE(StringCoreTests);
EXTERN_TEST_SUITE(StringDictTests);
EXTERN_TEST_SUITE(StringExtraTests);
EXTERN_TEST_SUITE(StringFormatTests);
EXTERN_TEST_SUITE(StringHtmlTests);
EXTERN_TEST_SUITE(StringIdentTests);
EXTERN_TEST_SUITE(StringIntDictTests);
EXTERN_TEST_SUITE(StringParseTests);
EXTERN_TEST_SUITE(StringUnicodeTests);
EXTERN_TEST_SUITE(SymbolTableTests);
EXTERN_TEST_SUITE(TimestampTests);

TestSuiteResults *RunAllTests()
{
	TestSuiteResults *results = CreateEmptyTestSuiteResults();

	RUN_TEST_SUITE(results, ByteCodeTests);
	RUN_TEST_SUITE(results, CompilerTests);
	RUN_TEST_SUITE(results, EvalConstantTests);
	RUN_TEST_SUITE(results, EvalCoreTests);
	RUN_TEST_SUITE(results, EvalTests);
	RUN_TEST_SUITE(results, HashTests);
	RUN_TEST_SUITE(results, Int32DictTests);
	RUN_TEST_SUITE(results, LexerCoreTests);
	RUN_TEST_SUITE(results, LexerIdentifierTests);
	RUN_TEST_SUITE(results, LexerLoanwordTests);
	RUN_TEST_SUITE(results, LexerNumberTests);
	RUN_TEST_SUITE(results, LexerPunctuationTests);
	RUN_TEST_SUITE(results, LexerStringTests);
	RUN_TEST_SUITE(results, LexerUnicodeTests);
	RUN_TEST_SUITE(results, ParserClassicTests);
	RUN_TEST_SUITE(results, ParserCoreTests);
	RUN_TEST_SUITE(results, ParserFuncTests);
	RUN_TEST_SUITE(results, ParserNewTests);
	RUN_TEST_SUITE(results, ParserQuoteTests);
	RUN_TEST_SUITE(results, ParserStmtTests);
	RUN_TEST_SUITE(results, ParserStringTests);
	RUN_TEST_SUITE(results, ParserSyntaxTableTests);
	RUN_TEST_SUITE(results, ParserSyntaxTests);
	RUN_TEST_SUITE(results, ParserSyntaxValidateTests);
	RUN_TEST_SUITE(results, ParserSyntaxWalkTests);
	RUN_TEST_SUITE(results, ParserTermTests);
	RUN_TEST_SUITE(results, PointerSetTests);
	RUN_TEST_SUITE(results, Real128Tests);
	RUN_TEST_SUITE(results, Real32Tests);
	RUN_TEST_SUITE(results, Real64Tests);
	RUN_TEST_SUITE(results, StringCoreTests);
	RUN_TEST_SUITE(results, StringDictTests);
	RUN_TEST_SUITE(results, StringExtraTests);
	RUN_TEST_SUITE(results, StringFormatTests);
	RUN_TEST_SUITE(results, StringHtmlTests);
	RUN_TEST_SUITE(results, StringIdentTests);
	RUN_TEST_SUITE(results, StringIntDictTests);
	RUN_TEST_SUITE(results, StringParseTests);
	RUN_TEST_SUITE(results, StringUnicodeTests);
	RUN_TEST_SUITE(results, SymbolTableTests);
	RUN_TEST_SUITE(results, TimestampTests);

	DisplayTestSuiteResults(results);

	return results;
}


const char *TestSuiteNames[] = {
	"ByteCodeTests",
	"CompilerTests",
	"EvalConstantTests",
	"EvalCoreTests",
	"EvalTests",
	"HashTests",
	"Int32DictTests",
	"LexerCoreTests",
	"LexerIdentifierTests",
	"LexerLoanwordTests",
	"LexerNumberTests",
	"LexerPunctuationTests",
	"LexerStringTests",
	"LexerUnicodeTests",
	"ParserClassicTests",
	"ParserCoreTests",
	"ParserFuncTests",
	"ParserNewTests",
	"ParserQuoteTests",
	"ParserStmtTests",
	"ParserStringTests",
	"ParserSyntaxTableTests",
	"ParserSyntaxTests",
	"ParserSyntaxValidateTests",
	"ParserSyntaxWalkTests",
	"ParserTermTests",
	"PointerSetTests",
	"Real128Tests",
	"Real32Tests",
	"Real64Tests",
	"StringCoreTests",
	"StringDictTests",
	"StringExtraTests",
	"StringFormatTests",
	"StringHtmlTests",
	"StringIdentTests",
	"StringIntDictTests",
	"StringParseTests",
	"StringUnicodeTests",
	"SymbolTableTests",
	"TimestampTests",
};


int NumTestSuites = 41;

