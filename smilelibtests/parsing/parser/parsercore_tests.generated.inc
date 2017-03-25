// This file was auto-generated.  Do not edit!
//
// SourceHash: 7a61c0c9710b938531b4326b5b1aff16

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

