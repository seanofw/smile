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
#include <smile/env/env.h>

#include "testhelpers.h"

TEST_SUITE(ParserQuoteTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Function-parsing tests.

START_TEST(CanParseAQuotedSymbol)
{
	Lexer lexer = SetupLexer("`x");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote x]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAQuotedList)
{
	Lexer lexer = SetupLexer("`[x y z]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [x y z]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAQuotedListWithPairsInIt)
{
	Lexer lexer = SetupLexer("`[x.y a.b c.d]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [(x . y) (a . b) (c . d)]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAQuotedListWithNestedLists)
{
	Lexer lexer = SetupLexer("`[x y [a b c] [p q [r s]] z]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [x y [a b c] [p q [r s]] z]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAQuotedExpression)
{
	Lexer lexer = SetupLexer("`(1 + 2 * 3)");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [(1 . +) [(2 . *) 3]]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAListTemplateForm)
{
	Lexer lexer = SetupLexer("`[x y z (1 + 2 * 3)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[(List . of) [$quote x] [$quote y] [$quote z] [(1 . +) [(2 . *) 3]]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAListTemplateFormUsingCurlyBraces)
{
	Lexer lexer = SetupLexer("`[x y z { 1 + 2 * 3 }]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[(List . of) [$quote x] [$quote y] [$quote z] [$progn [(1 . +) [(2 . *) 3]]] ]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAPairTemplateForm)
{
	Lexer lexer = SetupLexer("`[(1 + 2 * 3).- x]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[(List . of) [(Pair . of) [(1 . +) [(2 . *) 3]] -] [$quote x]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAStringTemplateForm)
{
	Lexer lexer = SetupLexer("`[x y z \"{x} is awesome.\"]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError xError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[(List . of) [$quote x] [$quote y] [$quote z] [([(List . of) x \" is awesome.\"] . join)]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(ParserIsntFooledByADynamicStringWithoutInserts)
{
	Lexer lexer = SetupLexer("`[x y z \"x is awesome.\"]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError xError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [x y z \"x is awesome.\"]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

#include "parserquote_tests.generated.inc"
