//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2017 Sean Werkema
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
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/env/env.h>

#include "testhelpers.h"

TEST_SUITE(ParserClassicTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Classic [$set] tests.

START_TEST(CanParseAClassicSetThatCreatesANewVariable)
{
	Lexer lexer = SetupLexer("[$set x 10]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x] [$set x 10]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicSetThatAssignsAPreexistingVariable)
{
	Lexer lexer = SetupLexer("[$scope [x y] [$set y 10]]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x y] [$set y 10]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicSetThatAssignsANontrivialExpression)
{
	Lexer lexer = SetupLexer("[$set x 5*10+2]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x] [$set x [([(5 . *) 10] . +) 2] ]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Classic [$scope] tests.

START_TEST(CanParseAClassicScopeThatDoesntDoAnything)
{
	Lexer lexer = SetupLexer("[$scope [x]]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicScopeThatJustResultsInNull)
{
	Lexer lexer = SetupLexer("[$scope [x] []]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x] []]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicScopeThatReferencesItsVariables)
{
	Lexer lexer = SetupLexer("[$scope [x y] x = 5 + y]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x y] [$set x [(5 . +) y]]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicScopeThatContainsManyExpressionsLikeAProgNDoes)
{
	Lexer lexer = SetupLexer("[$scope [x y] y = 10 x = 5 + y y = x]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x y] [$set y 10] [$set x [(5 . +) y]] [$set y x]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Classic [$new] tests.

START_TEST(CanParseAClassicNewThatContainsNothing)
{
	Lexer lexer = SetupLexer("[$new [] []]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$new [] []]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicNewThatReferencesABaseObject)
{
	Lexer lexer = SetupLexer("[$scope [Object] [$new Object []]]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [Object] [$new Object []]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicNewThatContainsMembersWithValues)
{
	Lexer lexer = SetupLexer("[$scope [Object] [$new Object [ [x 1] [y 2] [z 3] ]]]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [Object] [$new Object [ [x 1] [y 2] [z 3] ]]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicNewThatContainsMembersWithComplexExpressions)
{
	Lexer lexer = SetupLexer("[$scope [Object] [$new Object [ [x 1*10] [y 2*10] [z 3*10] ]]]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [Object] [$new Object [ [x [(1 . *) 10]] [y [(2 . *) 10]] [z [(3 . *) 10]] ]]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Classic [$quote] tests.

START_TEST(CanParseAClassicQuoteThatQuotesNull)
{
	Lexer lexer = SetupLexer("[$quote []]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$quote []]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(ClassicQuoteOfASymbolShouldNotIntroduceAVariable)
{
	Lexer lexer = SetupLexer("[$quote x]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$quote x]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(ClassicQuoteShouldHandleMessyLispStyleExpressions)
{
	Lexer lexer = SetupLexer("[$quote [x y z [5 10]] ]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$quote [x y z [5 10]] ]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(ClassicQuoteShouldHandleMessySyntaxExpressions)
{
	Lexer lexer = SetupLexer("var x [$quote (x + 10)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x] [$quote [(x . +) 10] ]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Classic [$fn] tests.

START_TEST(CanParseAClassicFunctionThatReturnsNull)
{
	Lexer lexer = SetupLexer("[$fn [] []]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$fn [] []]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicFunctionThatReturnsAComputation)
{
	Lexer lexer = SetupLexer("[$fn [] 5*10+1]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$fn [] [([(5 . *) 10] . +) 1]]");

	String stringResult = SmileObject_Stringify(result);
	String stringExpectedResult = SmileObject_Stringify(expectedResult);

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicFunctionThatTakesArguments)
{
	Lexer lexer = SetupLexer("[$fn [x y] x * y * 2]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$fn [x y] [([(x . *) y] . *) 2]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseAClassicFunctionThatCanHandleRestsAndDefaults)
{
	Lexer lexer = SetupLexer("[$fn [x [y default 10] [z rest]] x * y]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$fn [x [y default 10] [z rest]] [(x . *) y]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Classic [$till] tests.

START_TEST(CanParseATillLoopThatContainsNothing)
{
	Lexer lexer = SetupLexer("[$till [forever] []]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$till [forever] []]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseATillLoopThatContainsAnExpressionReferencingATillFlag)
{
	Lexer lexer = SetupLexer("x = 10\n"
		"[$till [done]\n"
		"  {\n"
		"    x -= 1\n"
		"    [$if [$not x] done]\n"
		"  }\n"
		"]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x]\n"
		"  [$set x 10]\n"
		"  [$till [done] [$progn\n"
		"    [$opset - x 1]\n"
		"    [$if [$not x] done]\n"
		"  ]]\n"
		"]");

	String stringResult = SmileObject_Stringify(result);
	String stringExpectedResult = SmileObject_Stringify(expectedResult);

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseATillLoopThatContainsReferencesToManyTillFlags)
{
	Lexer lexer = SetupLexer("x = 10\n"
		"[$till [found not-found]\n"
		"  {\n"
		"    x -= 1\n"
		"    [$if x == 7 found]\n"
		"    [$if x < 5 not-found]\n"
		"  }\n"
		"]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);
	SmileObject expectedResult = SimpleParse("[$scope [x]\n"
		"  [$set x 10]\n"
		"  [$till [found not-found] [$progn\n"
		"    [$opset - x 1]\n"
		"    [$if [(x . ==) 7] found]\n"
		"    [$if [(x . <) 5] not-found]\n"
		"  ]]\n"
		"]");

	String stringResult = SmileObject_Stringify(result);
	String stringExpectedResult = SmileObject_Stringify(expectedResult);

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST
#include "parserclassic_tests.generated.inc"
