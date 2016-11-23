// This file was auto-generated.  Do not edit!
//
// SourceHash: c6344aac9ed7790b99d968231af26ded

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

