// This file was auto-generated.  Do not edit!
//
// SourceHash: e06d3f4266181de508b3413473075c76

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
	UnaryOperatorsDontWrapLines,
	UnaryOperatorsCanWrapLinesInParentheses,
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

