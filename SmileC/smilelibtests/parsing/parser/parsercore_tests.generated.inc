// This file was auto-generated.  Do not edit!
//
// SourceHash: db02edbb23cf95975c42512966a87a16

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

