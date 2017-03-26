// This file was auto-generated.  Do not edit!
//
// SourceHash: 1d1a1e3b90253182fd16667032d31e3d

START_TEST_SUITE(ParserCoreTests)
{
	EmptyInputResultsInEmptyParse,
	CanParseASequenceOfTerms,
	ParenthesesHaveNoMeaningInASequenceOfTerms,
	ParenthesesShouldOnlyAllowOneContainedElement,
	CanParseAndExpr,
	CanParseOrExpr,
	CanParseAMixOfAndAndOrAndNot,
	CanParseAMixOfAndAndOrAndNotWithParentheses,
	CanParseComparisons,
	CanParseSpecialComparisons,
	CanParsePlusAndMinus,
	CanParseStarAndSlash,
	PlusAndMinusHaveLowerPrecedenceThanStarAndSlash,
	PlusAndMinusHaveGreaterPrecedenceThanComparisons,
	CanParseArbitraryBinaryOperators,
	CanParseArbitraryPrefixOperators,
	SpecialBinaryOperatorsCanBeArbitraryPrefixOperators,
	CanParseAMixOfBinaryAndPrefixOperators,
	CanParseAMixOfSpecialBinaryAndArbitraryPrefixOperators,
	BinaryOperatorsDontWrapLines,
	BinaryOperatorsCanWrapLinesInParentheses,
	BinaryOperatorWrappingPropagatesIntoFunctions1,
	BinaryOperatorWrappingPropagatesIntoFunctions2,
	CanParseTheDotOperator,
	CanParseTheColonOperator,
	CanParseTheRangeOperator,
	CanParseTheSpecialDoubleHashOperator,
}
END_TEST_SUITE(ParserCoreTests)

