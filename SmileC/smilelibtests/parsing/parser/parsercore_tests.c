//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2016 Sean Werkema
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
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/env/env.h>

#include "testhelpers.h"

TEST_SUITE(ParserCoreTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Primitive list tests.

START_TEST(EmptyInputResultsInEmptyParse)
{
	Lexer lexer = SetupLexer("");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullList);
}
END_TEST

START_TEST(CanParseASequenceOfTerms)
{
	SmileList expectedResult = SmileList_CreateList(
		(SmileObject)SmileInteger32_Create(12),
		SmileInteger32_Create(12345),
		SmileInteger32_Create(45),
		SmileInteger32_Create(0x10),
		SmileInteger32_Create(0x2B),
		SmileString_Create(String_FromC("or not")),
		SmileInteger32_Create(0x2B),
		NULL
	);

	Lexer lexer = SetupLexer("12 12345 45 0x10 0x2B ''or not'' 0x2B");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(ParenthesesHaveNoMeaningInASequenceOfTerms)
{
	SmileList expectedResult = SmileList_CreateList(
		(SmileObject)SmileInteger32_Create(12),
		SmileInteger32_Create(12345),
		SmileInteger32_Create(45),
		SmileInteger32_Create(0x10),
		SmileInteger32_Create(0x2B),
		SmileString_Create(String_FromC("or not")),
		SmileInteger32_Create(0x2B),
		NULL
	);

	Lexer lexer = SetupLexer("12 ((12345)) (45) 0x10 0x2B (''or not'') 0x2B");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(ParenthesesShouldOnlyAllowOneContainedElement)
{
	Lexer lexer = SetupLexer("12 (12345 45 0x10) 0x2B (''or not'') 0x2B");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

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
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gronk"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$and true false true gronk]");

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseOrExpr)
{
	Lexer lexer = SetupLexer("\t true or false or true or gronk\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gronk"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$or true false true gronk]");

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseAMixOfAndAndOrAndNot)
{
	Lexer lexer = SetupLexer("\t true or not false and true and foo or not not gronk\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gronk"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError parseDeclError2 = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "foo"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$or true [$and [$not false] true foo] [$not [$not gronk]]]");

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseAMixOfAndAndOrAndNotWithParentheses)
{
	Lexer lexer = SetupLexer("\t (true or not false) and true and (foo or not not gronk)\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gronk"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseError parseDeclError2 = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "foo"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$and [$or true [$not false]] true [$or foo [$not [$not gronk]]]]");

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseComparisons)
{
	Lexer lexer = SetupLexer("\t 1 < 10 and 0 == 0 and 15 >= 8 and 23 > 7 and 99 < 100 and 1 != 2\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$and [(1 . <) 10] [(0 . ==) 0] [(15 . >=) 8] [(23 . >) 7] [(99 . <) 100] [(1 . !=) 2]]");

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseSpecialComparisons)
{
	Lexer lexer = SetupLexer("\t 1 !== 10 and 0 === 0 and 15 is Number\n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	Symbol numberSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "Number");
	ParseError declError = ParseScope_DeclareHere(parseScope, numberSymbol, PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$and [$ne 1 10] [$eq 0 0] [$is 15 Number]]");

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParsePlusAndMinus)
{
	Lexer lexer = SetupLexer("\t 12 + 34 \n 56 - 78 + 90");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ [(12 . +) 34] [([(56 . -) 78] . +) 90] ]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseStarAndSlash)
{
	Lexer lexer = SetupLexer("\t 12 * 34 \n 56 / 78 * 90");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ [(12 . *) 34] [([(56 . /) 78] . *) 90] ]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
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
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("["
		" [(56 . +) [(78 . *) 90]]"
		" [([(56 . *) 78] . +) 90]"
		" [(56 . -) [(78 . *) 90]]"
		" [([(56 . *) 78] . -) 90]"
		" [(56 . +) [(78 . /) 90]]"
		" [([(56 . /) 78] . +) 90]"
		" [(56 . -) [(78 . /) 90]]"
		" [([(56 . /) 78] . -) 90]"
		"]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
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
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("["
		" [([(56 . +) 78] . <) [(90 . *) 10]]"
		" [([(56 . -) 55] . ==) [(90 . /) 90]]"
		" [([(56 . -) 55] . !=) [(90 . *) 10]]"
		" [([(1 . +) [(2 . *) 3]] . >) [(3 . +) [(2 . *) 1]]]"
		"]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseArbitraryBinaryOperators)
{
	Lexer lexer = SetupLexer("\t 12 plus 34 \n ''foo'' with ''bar'' \n 56 minus 78 minus 90");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ [(12 . plus) 34] [(''foo'' . with) ''bar''] [([(56 . minus) 78] . minus) 90] ]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseArbitraryPrefixOperators)
{
	Lexer lexer = SetupLexer("\t minus 34 \n count html-encode reverse ''foo'' \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ [(34 . minus)] [([([(''foo'' . reverse)] . html-encode)] . count)] ]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(SpecialBinaryOperatorsCanBeArbitraryPrefixOperators)
{
	Lexer lexer = SetupLexer("\t -34 \n + * / ''foo'' \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ [(34 . -)] [([([(''foo'' . /)] . *)] . +)] ]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseAMixOfBinaryAndPrefixOperators)
{
	Lexer lexer = SetupLexer("\t negative 34 times negative 97 plus 14 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[([ ([(34 . negative)] . times) [(97 . negative)] ] . plus) 14]");

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseAMixOfSpecialBinaryAndArbitraryPrefixOperators)
{
	Lexer lexer = SetupLexer("\t sin -314 * cos +314 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ ([([(314 . -)] . sin)] . *) [([(314 . +)] . cos)] ]");

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

START_TEST(BinaryOperatorsDontWrapLines)
{
	Lexer lexer = SetupLexer("\t sin -314 * cos +314 \n * tan 123 \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[\n"
		"[ ([([(314 . -)] . sin)] . *) [([(314 . +)] . cos)] ]\n"
		"[ ([(123 . tan)] . *) ]\n"
		"]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(BinaryOperatorsCanWrapLinesInParentheses)
{
	Lexer lexer = SetupLexer("\t (sin -314 * cos +314 \n * tan 123) \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[\n"
		"[([([([(314 . -)] . sin)] . *) [([(314 . +)] . cos)]] . *) [(123 . tan)]]\n"
		"]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(BinaryOperatorWrappingPropagatesIntoFunctions1)
{
	Lexer lexer = SetupLexer("\t |x| sin -x * cos +x \n * tan 123 \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[\n"
		"[$fn [x] [ ([([(x . -)] . sin)] . *) [([(x . +)] . cos)] ]]\n"
		"[ ([(123 . tan)] . *) ]\n"
		"]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(BinaryOperatorWrappingPropagatesIntoFunctions2)
{
	Lexer lexer = SetupLexer("\t (|x| sin -x * cos +x \n * tan 123) \t");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[\n"
		"[$fn [x] [([([([(x . -)] . sin)] . *) [([(x . +)] . cos)]] . *) [(123 . tan)]]]\n"
		"]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseTheDotOperator)
{
	Lexer lexer = SetupLexer("\t [true.string] [Stdout.print true.string.args.count] \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	Symbol stdoutSymbol = SymbolTable_GetSymbolC(Smile_SymbolTable, "Stdout");
	ParseError declError = ParseScope_DeclareHere(parseScope, stdoutSymbol, PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ [(true . string)] [(Stdout . print) (((true . string) . args) . count)] ]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
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
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ [(x . get-member) 1] [(y . get-member) [(5 . -)]] ]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseTheRangeOperator)
{
	Lexer lexer = SetupLexer("\t 1..10 \n -5..+5 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[ [(Range . of) 1 10] [(Range . of) [(5 . -)] [(5 . +)]] ]");

	ASSERT(RecursiveEquals((SmileObject)result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseTheSpecialDoubleHashOperator)
{
	Lexer lexer = SetupLexer("\t 1 ## 2 \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(1), (SmileObject)SmileInteger32_Create(2));

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));

	lexer = SetupLexer("\t 1 ## 2 ## 3 ## 4 \n");
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(1),
			(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(2),
				(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(3), (SmileObject)SmileInteger32_Create(4))));

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));

	lexer = SetupLexer("\t 1 ## 2 ## 3 ## 4 ## [] \n");
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(1),
			(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(2),
				(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(3),
					(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(4), NullObject))));

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));

	lexer = SetupLexer("\t 1 ## [] ## 3 ## [] ## 5 ## [] \n");
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(1),
			(SmileObject)SmileList_Cons(NullObject,
				(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(3),
					(SmileObject)SmileList_Cons(NullObject,
						(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(5), NullObject)))));

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));

	lexer = SetupLexer("\t 1 ## ([] ## 3) ## [] ## (5 ## []) \n");
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = Parser_Parse(parser, lexer, parseScope);

	expectedResult =
		(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(1),
			(SmileObject)SmileList_Cons((SmileObject)SmileList_Cons(NullObject, (SmileObject)SmileInteger32_Create(3)),
				(SmileObject)SmileList_Cons(NullObject,
					(SmileObject)SmileList_Cons((SmileObject)SmileInteger32_Create(5), NullObject))));

	ASSERT(RecursiveEquals((SmileObject)result->a, (SmileObject)expectedResult));
}
END_TEST

#include "parsercore_tests.generated.inc"
