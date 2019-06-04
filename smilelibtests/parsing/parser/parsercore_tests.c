//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2019 Sean Werkema
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

#include "../../stdafx.h"

#include <smile/parsing/parser.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/env/env.h>

#include "testhelpers.h"

TEST_SUITE(ParserCoreTests)

//-------------------------------------------------------------------------------------------------
//  Primitive list tests.

START_TEST(EmptyInputResultsInEmptyParse)
{
	Lexer lexer = SetupLexer("");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullObject);
}
END_TEST

START_TEST(CanParseASequenceOfTerms)
{
	SmileList expectedResult = SmileList_CreateList(
		(SmileObject)SmileInteger64_Create(12),
		SmileInteger64_Create(12345),
		SmileInteger64_Create(45),
		SmileInteger64_Create(0x10),
		SmileInteger64_Create(0x2B),
		String_FromC("or not"),
		SmileInteger64_Create(0x2B),
		NULL
	);

	Lexer lexer = SetupLexer("12 12345 45 0x10 0x2B ''or not'' 0x2B");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(SMILE_KIND(result) == SMILE_KIND_LIST);
	ASSERT(RecursiveEquals(((SmileList)result)->d, (SmileObject)expectedResult));
}
END_TEST

START_TEST(ParenthesesHaveNoMeaningInASequenceOfTerms)
{
	SmileList expectedResult = SmileList_CreateList(
		(SmileObject)SmileInteger64_Create(12),
		SmileInteger64_Create(12345),
		SmileInteger64_Create(45),
		SmileInteger64_Create(0x10),
		SmileInteger64_Create(0x2B),
		String_FromC("or not"),
		SmileInteger64_Create(0x2B),
		NULL
	);

	Lexer lexer = SetupLexer("12 ((12345)) (45) 0x10 0x2B (''or not'') 0x2B");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(SMILE_KIND(result) == SMILE_KIND_LIST);
	ASSERT(RecursiveEquals(((SmileList)result)->d, (SmileObject)expectedResult));
}
END_TEST

START_TEST(ParenthesesShouldOnlyAllowOneContainedElement)
{
	Lexer lexer = SetupLexer("12 (12345 45 0x10) 0x2B (''or not'') 0x2B");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Core expression grammar tests.

START_TEST(CanParseAndExpr)
{
	Lexer lexer = SetupLexer("\t true and false and true and gronk\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError trueDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "true"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError falseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "false"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gronk"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$and true false true gronk]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseOrExpr)
{
	Lexer lexer = SetupLexer("\t true or false or true or gronk\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError trueDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "true"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError falseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "false"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gronk"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$or true false true gronk]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseAMixOfAndAndOrAndNot)
{
	Lexer lexer = SetupLexer("\t true or not false and true and foo or not not gronk\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError trueDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "true"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError falseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "false"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gronk"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError parseDeclError2 = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "foo"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$or true [$and [$not false] true foo] [$not [$not gronk]]]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseAMixOfAndAndOrAndNotWithParentheses)
{
	Lexer lexer = SetupLexer("\t (true or not false) and true and (foo or not not gronk)\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError trueDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "true"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError falseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "false"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gronk"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError parseDeclError2 = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "foo"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$and [$or true [$not false]] true [$or foo [$not [$not gronk]]]]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseComparisons)
{
	Lexer lexer = SetupLexer("\t 1 < 10 and 0 == 0 and 15 >= 8 and 23 > 7 and 99 < 100 and 1 != 2\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$and [(1 . <) 10] [(0 . ==) 0] [(15 . >=) 8] [(23 . >) 7] [(99 . <) 100] [(1 . !=) 2]]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseSpecialComparisons)
{
	Lexer lexer = SetupLexer("\t 1 !== 10 and 0 === 0 and 15 is Number\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	Symbol numberSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "Number");
	ParseError declError = ParseScope_DeclareHere(parseScope, numberSymbol, PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$and [$ne 1 10] [$eq 0 0] [$is 15 Number]]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParsePlusAndMinus)
{
	Lexer lexer = SetupLexer("\t 12 + 34 \n 56 - 78 + 90");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn [(12 . +) 34] [([(56 . -) 78] . +) 90] ]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseStarAndSlash)
{
	Lexer lexer = SetupLexer("\t 12 * 34 \n 56 / 78 * 90");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn [(12 . *) 34] [([(56 . /) 78] . *) 90] ]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(PlusAndMinusHaveLowerPrecedenceThanStarAndSlash)
{
	Lexer lexer = SetupLexer(
		"56 + 78 * 90 \n 56 * 78 + 90 \n"
		"56 - 78 * 90 \n 56 * 78 - 90 \n"
		"56 + 78 / 90 \n 56 / 78 + 90 \n"
		"56 - 78 / 90 \n 56 / 78 - 90 \n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn"
		" [(56 . +) [(78 . *) 90]]"
		" [([(56 . *) 78] . +) 90]"
		" [(56 . -) [(78 . *) 90]]"
		" [([(56 . *) 78] . -) 90]"
		" [(56 . +) [(78 . /) 90]]"
		" [([(56 . /) 78] . +) 90]"
		" [(56 . -) [(78 . /) 90]]"
		" [([(56 . /) 78] . -) 90]"
		"]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(PlusAndMinusHaveGreaterPrecedenceThanComparisons)
{
	Lexer lexer = SetupLexer(
		"56 + 78 < 90 * 10 \n"
		"56 - 55 == 90 / 90 \n"
		"56 - 55 != 90 * 10 \n"
		"1 + 2 * 3 > 3 + 2 * 1 \n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn"
		" [([(56 . +) 78] . <) [(90 . *) 10]]"
		" [([(56 . -) 55] . ==) [(90 . /) 90]]"
		" [([(56 . -) 55] . !=) [(90 . *) 10]]"
		" [([(1 . +) [(2 . *) 3]] . >) [(3 . +) [(2 . *) 1]]]"
		"]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseArbitraryBinaryOperators)
{
	Lexer lexer = SetupLexer("\t 12 plus 34 \n ''foo'' with ''bar'' \n 56 minus 78 minus 90");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn [(12 . plus) 34] [(''foo'' . with) ''bar''] [([(56 . minus) 78] . minus) 90] ]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseArbitraryPrefixOperators)
{
	Lexer lexer = SetupLexer("\t minus 34 \n count html-encode reverse ''foo'' \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn [(34 . minus)] [([([(''foo'' . reverse)] . html-encode)] . count)] ]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(SpecialBinaryOperatorsCanBeArbitraryPrefixOperators)
{
	Lexer lexer = SetupLexer("\t -34 \n + * / ''foo'' \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn [(34 . -)] [([([(''foo'' . /)] . *)] . +)] ]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseAMixOfBinaryAndPrefixOperators)
{
	Lexer lexer = SetupLexer("\t negative 34 times negative 97 plus 14 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[([ ([(34 . negative)] . times) [(97 . negative)] ] . plus) 14]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseAMixOfSpecialBinaryAndArbitraryPrefixOperators)
{
	Lexer lexer = SetupLexer("\t sin -314 * cos +314 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ ([([(314 . -)] . sin)] . *) [([(314 . +)] . cos)] ]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(UnaryOperatorsDontWrapLines)
{
	Lexer lexer = SetupLexer("\t sin -314 * cos +\n314 * tan 123 \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	Parser_Parse(parser, lexer, parseScope);

	ASSERT(parser->firstMessage != NullList);
}
END_TEST

START_TEST(UnaryOperatorsCanWrapLinesInParentheses)
{
	Lexer lexer = SetupLexer("\t (sin -314 * cos +\n314 * tan 123) \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse(
		"[([([([(314 . -)] . sin)] . *) [([(314 . +)] . cos)]] . *) [(123 . tan)]]\n"
	);

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(BinaryOperatorsDontWrapLines)
{
	Lexer lexer = SetupLexer("\t sin -314 * cos +314 \n * tan 123 \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn\n"
		"[ ([([(314 . -)] . sin)] . *) [([(314 . +)] . cos)] ]\n"
		"[ ([(123 . tan)] . *) ]\n"
		"]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(BinaryOperatorsCanWrapLinesInParentheses)
{
	Lexer lexer = SetupLexer("\t (sin -314 * cos +314 \n * tan 123) \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse(
		"[([([([(314 . -)] . sin)] . *) [([(314 . +)] . cos)]] . *) [(123 . tan)]]\n"
	);

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(BinaryOperatorWrappingPropagatesIntoFunctions1)
{
	Lexer lexer = SetupLexer("\t |x| sin -x * cos +x \n * tan 123 \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn\n"
		"[$fn [x] [ ([([(x . -)] . sin)] . *) [([(x . +)] . cos)] ]]\n"
		"[ ([(123 . tan)] . *) ]\n"
		"]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(BinaryOperatorWrappingPropagatesIntoFunctions2)
{
	Lexer lexer = SetupLexer("\t (|x| sin -x * cos +x \n * tan 123) \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse(
		"[$fn [x] [([([([(x . -)] . sin)] . *) [([(x . +)] . cos)]] . *) [(123 . tan)]]]\n"
	);

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseTheDotOperator)
{
	Lexer lexer = SetupLexer("\t [true.string] [Stdout.print true.string.args.count] \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	Symbol stdoutSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "Stdout");
	ParseError trueDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "true"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError declError = ParseScope_DeclareHere(parseScope, stdoutSymbol, PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn [(true . string)] [(Stdout . print) (((true . string) . args) . count)] ]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseTheColonOperator)
{
	Lexer lexer = SetupLexer("\t x:1 \n y:-5 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	Symbol xSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "x");
	Symbol ySymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "y");
	ParseError declError = ParseScope_DeclareHere(parseScope, xSymbol, PARSEDECL_VARIABLE, NULL, NULL);
	ParseError declError2 = ParseScope_DeclareHere(parseScope, ySymbol, PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn [$index x 1] [$index y [(5 . -)]] ]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseTheRangeOperator)
{
	Lexer lexer = SetupLexer("\t 1..10 \n -5..+5 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$progn [(1 . range-to) 10] [([(5 . -)] . range-to) [(5 . +)]] ]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParsePropertyLookupsInsideADynamicString)
{
	Lexer lexer = SetupLexer("var foo, bar\n"
		"\"{[foo bar.baz]}\"");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$scope\n"
		"\t[foo bar]\n"
		"\t[([(List.of) [foo (bar.baz)]].join)]\n"
		"]\n");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseTheTypeofOperator)
{
	Lexer lexer = SetupLexer("\t typeof 5 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$typeof 5]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseTheTypeofOperatorInASequenceOfOtherUnaryOperators)
{
	Lexer lexer = SetupLexer("\t string typeof -5 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[[$dot [$typeof [[$dot 5 -]]] string]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CannotSplitLinesAfterTheTypeofOperator)
{
	Lexer lexer = SetupLexer("\t typeof\n5 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(parser->firstMessage != NullList);
}
END_TEST

START_TEST(CanParseTheSpecialDoubleHashOperator)
{
	Lexer lexer = SetupLexer("\t 1 ## 2 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(1), (SmileObject)SmileInteger64_Create(2));

	ASSERT(RecursiveEquals(result, expectedResult));

	lexer = SetupLexer("\t 1 ## 2 ## 3 ## 4 \n");
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(1),
			(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(2),
				(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(3), (SmileObject)SmileInteger64_Create(4))));

	ASSERT(RecursiveEquals(result, expectedResult));

	lexer = SetupLexer("\t 1 ## 2 ## 3 ## 4 ## [] \n");
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(1),
			(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(2),
				(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(3),
					(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(4), NullObject))));

	ASSERT(RecursiveEquals(result, expectedResult));

	lexer = SetupLexer("\t 1 ## [] ## 3 ## [] ## 5 ## [] \n");
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(1),
			(SmileObject)SmileList_Cons(NullObject,
				(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(3),
					(SmileObject)SmileList_Cons(NullObject,
						(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(5), NullObject)))));

	ASSERT(RecursiveEquals(result, expectedResult));

	lexer = SetupLexer("\t 1 ## ([] ## 3) ## [] ## (5 ## []) \n");
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(1),
			(SmileObject)SmileList_Cons((SmileObject)SmileList_Cons(NullObject, (SmileObject)SmileInteger64_Create(3)),
				(SmileObject)SmileList_Cons(NullObject,
					(SmileObject)SmileList_Cons((SmileObject)SmileInteger64_Create(5), NullObject))));

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

#include "parsercore_tests.generated.inc"
