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
#include <smile/env/env.h>

#include "testhelpers.h"

TEST_SUITE(ParserQuoteTests)

//-------------------------------------------------------------------------------------------------
//  Function-parsing tests.

START_TEST(CanParseAQuotedSymbol)
{
	Lexer lexer = SetupLexer("`x");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote x]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAQuotedList)
{
	Lexer lexer = SetupLexer("`[x y z]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [x y z]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAQuotedListWithPairsInIt)
{
	Lexer lexer = SetupLexer("`[x.y a.b c.d]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [(x . y) (a . b) (c . d)]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAQuotedListWithNestedLists)
{
	Lexer lexer = SetupLexer("`[x y [a b c] [p q [r s]] z]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [x y [a b c] [p q [r s]] z]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAQuotedExpression)
{
	Lexer lexer = SetupLexer("`(1 + 2 * 3)");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [(1 . +) [(2 . *) 3]]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAListTemplateForm)
{
	Lexer lexer = SetupLexer("`[x y z (1 + 2 * 3)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[(List . of) [$quote x] [$quote y] [$quote z] [(1 . +) [(2 . *) 3]]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAQuoteFormContainingQuotedContent)
{
	Lexer lexer = SetupLexer("`[x y z `(1 + 2 * 3)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [x y z [(1 . +) [(2 . *) 3]]]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseATemplateFormContainingQuotedContent)
{
	Lexer lexer = SetupLexer("`[x (a) z `(1 + 2 * 3)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result, expectedForm;

	ParseScope_DeclareHereC(parseScope, "a", PARSEDECL_VARIABLE, NULL, NULL);
	result = Parser_Parse(parser, lexer, parseScope);

	expectedForm = SimpleParse("[(List . of) [$quote x] a [$quote z] [$quote [(1 . +) [(2 . *) 3]]]]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseASplicingTemplateForm)
{
	Lexer lexer = SetupLexer("`[x @(a) z @(b)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result, expectedForm;

	ParseScope_DeclareHereC(parseScope, "a", PARSEDECL_VARIABLE, NULL, NULL);
	ParseScope_DeclareHereC(parseScope, "b", PARSEDECL_VARIABLE, NULL, NULL);
	result = Parser_Parse(parser, lexer, parseScope);

	expectedForm = SimpleParse("[(List . combine) [$quote [x]] a [$quote [z]] b]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseANestedSplicingTemplateForm)
{
	Lexer lexer = SetupLexer("`[x y z [c @(a) d]]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result, expectedForm;

	ParseScope_DeclareHereC(parseScope, "a", PARSEDECL_VARIABLE, NULL, NULL);
	result = Parser_Parse(parser, lexer, parseScope);

	expectedForm = SimpleParse(
		"[(List . of) [$quote x] [$quote y] [$quote z]"
			"[(List . combine) [$quote [c]] a [$quote [d]] ]"
		"]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAListTemplateFormUsingCurlyBraces)
{
	Lexer lexer = SetupLexer("`[x y z { 1 + 2 * 3 }]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[(List . of) [$quote x] [$quote y] [$quote z] [(1 . +) [(2 . *) 3]] ]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAPairTemplateForm)
{
	Lexer lexer = SetupLexer("`[(1 + 2 * 3).- x]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse(
"[ \
	(List . of) \
	[(List . of) [$quote $dot] [(1 . +) [(2 . *) 3]] [$quote -]] \
	[$quote x] \
]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(CanParseAStringTemplateForm)
{
	Lexer lexer = SetupLexer("`[x y z \"{x} is awesome.\"]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError xError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[(List . of) [$quote x] [$quote y] [$quote z] [([(List . of) x \" is awesome.\"] . join)]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

START_TEST(ParserIsntFooledByADynamicStringWithoutInserts)
{
	Lexer lexer = SetupLexer("`[x y z \"x is awesome.\"]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError xError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$quote [x y z \"x is awesome.\"]]");

	ASSERT(RecursiveEquals(result, expectedForm));
}
END_TEST

#include "parserquote_tests.generated.inc"
