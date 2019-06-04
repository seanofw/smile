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

#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parsesyntax.h>

#include "testhelpers.h"

TEST_SUITE(ParserSyntaxWalkTests)

START_TEST(CanReplaceSimpleTerminalForms)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo] => 123\n"
		"4 + 5\n"
		"foo\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanReplaceMultiTerminalForms)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo bar baz] => 123\n"
		"4 + 5\n"
		"foo bar baz\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanReplaceFormsWithAKnownNonterminal)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR x] baz] => 123\n"
		"4 + 5\n"
		"foo 999 baz\n"
		"6 + 7\n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(SubstitutionWorksWithAKnownNonterminal)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR x] baz] => `[123 . + (x)]\n"
		"4 + 5\n"
		"foo 999 baz\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(123 . +) 999]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(SubstitutionOfComplexContentWorksWithAKnownNonterminal)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR x] baz] => `[123 . + (x)]\n"
		"4 + 5\n"
		"foo 8 * 9 / 10 baz\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(123 . +) [([(8 . *) 9] . /) 10]]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(IfThenTest)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => [$if (x) (y)]\n"
		"4 + 5\n"
		"my-if 1 < 2 then 10\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[$if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(IfThenElseTest)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [my-if [EXPR x] then [STMT y] else [STMT z]] => [$if (x) (y) (z)]\n"
		"4 + 5\n"
		"my-if 1 < 2 then 10 else 20\n"
		"6 + 7\n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[$if [(1 . <) 2] 10 20]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(IfThenElseTestWithBothIfThenRules)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => [$if (x) (y)]\n"
		"#syntax STMT: [my-if [EXPR x] then [STMT y] else [STMT z]] => [$if (x) (y) (z)]\n"
		"4 + 5\n"
		"my-if 1 < 2 then 10\n"
		"my-if 3 < 4 then 30 else 40\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[$if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_SIXTH(result), SimpleParse("[$if [(3 . <) 4] 30 40]")));
	ASSERT(RecursiveEquals(LIST_SEVENTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(IfThenElseTestWithNestedConditionals)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => [$if (x) (y)]\n"
		"#syntax STMT: [my-if [EXPR x] then [STMT y] else [STMT z]] => [$if (x) (y) (z)]\n"
		"4 + 5\n"
		"my-if 1 < 2 then\n"
		"  my-if 5 < 6 then 50\n"
		"  else 60\n"
		"else my-if 3 < 4 then\n"
		"  my-if 7 < 8 then 70\n"
		"  else 80\n"
		"else 40\n"
		"6 + 7\n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[$if [(1 . <) 2] [$if [(5 . <) 6] 50 60] [$if [(3 . <) 4] [$if [(7 . <) 8] 70 80] 40]]")));
	ASSERT(RecursiveEquals(LIST_SIXTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CStyleIfThenElseTest)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [my-if ( [EXPR x] ) [STMT y]] => [$if (x) (y)]\n"
		"#syntax STMT: [my-if ( [EXPR x] ) [STMT y] else [STMT z]] => [$if (x) (y) (z)]\n"
		"4 + 5\n"
		"my-if (1 < 2) 10\n"
		"my-if (3 < 4) 30 else 40\n"
		"6 + 7\n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[$if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_SIXTH(result), SimpleParse("[$if [(3 . <) 4] 30 40]")));
	ASSERT(RecursiveEquals(LIST_SEVENTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(SimpleCustomDslTest)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [fronk { [FOO-FRONKS x] }] => `[fronk [$quote (x)]]\n"
		"#syntax FOO-FRONKS: [[FOO-GROOP x] [FOO-FRONKS y]] => `[(x) @(y)]\n"
		"#syntax FOO-FRONKS: [[FOO-GROOP x]] => `[(x)]\n"
		"#syntax FOO-GROOP: [qux] => `qux\n"
		"#syntax FOO-GROOP: [xuq] => `xuq\n"
		"1 + 2\n"
		"fronk { qux xuq xuq qux }\n"
		"3 + 4\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SEVENTH(result), SimpleParse("[(1 . +) 2]")));
	ASSERT(RecursiveEquals(LIST_EIGHTH(result), SimpleParse("[fronk [$quote [qux xuq xuq qux]]]")));
	ASSERT(RecursiveEquals(LIST_NINTH(result), SimpleParse("[(3 . +) 4]")));
}
END_TEST

START_TEST(CustomDslsValidateContentThroughTheirSyntaxRules)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [fronk { [FOO-FRONKS x] }] => `[fronk [$quote (x)]]\n"
		"#syntax FOO-FRONKS: [[FOO-GROOP x] [FOO-FRONKS y]] => `[(x) @(y)]\n"
		"#syntax FOO-FRONKS: [[FOO-GROOP x]] => `[(x)]\n"
		"#syntax FOO-GROOP: [qux] => `qux\n"
		"#syntax FOO-GROOP: [xuq] => `xuq\n"
		"1 + 2\n"
		"fronk { qux blarg xuq qux }\n"
		"3 + 4\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	Parser_Parse(parser, lexer, parseScope);

	ASSERT(parser->firstMessage != NullList);
}
END_TEST

START_TEST(CanExtendStmtWithKeywordRoots)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [my-if ( [EXPR x] ) [STMT y]] => [$if (x) (y)]\n"
		"4 + 5\n"
		"my-if (1 < 2) 10\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[$if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanExtendExprWithKeywordRoots)
{
	Lexer lexer = SetupLexer(
		"#syntax EXPR: [my-if ( [EXPR x] ) [STMT y]] => [$if (x) (y)]\n"
		"4 + 5\n"
		"x = my-if (1 < 2) 10\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError declError = ParseScope_Declare(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_GLOBAL, NULL, NULL);
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[$set x [$if [(1 . <) 2] 10]]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanCreateBasicPrintStatementDynamically)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [print [EXPR+ exprs ,]] => `[Stdout.print [[List.of @@exprs].join]]\n"
		"\n"
		"print \"Hello, World.\"\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError declError = ParseScope_Declare(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "Stdout"), PARSEDECL_GLOBAL, NULL, NULL);
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(Stdout.print) [([(List.of) \"Hello, World.\"].join)]]")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
// Keyword tests

START_TEST(CanDeclareKeywords)
{
	Lexer lexer = SetupLexer(
		"keyword my-if\n"
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => `[$if @x @y]\n"
		"#syntax STMT: [my-if [EXPR x] then [STMT y] else [STMT z]] => `[$if @x @y @z]\n"
		"4 + 5\n"
		"my-if 1 < 2 then 10\n"
		"my-if 3 < 4 then 30 else 40\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	String foo = SmileObject_Stringify((SmileObject)result);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[$if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_SIXTH(result), SimpleParse("[$if [(3 . <) 4] 30 40]")));
	ASSERT(RecursiveEquals(LIST_SEVENTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(DeclaringKeywordsChangesParsingBehavior)
{
	// Without the keyword declaration, this is a series of operators.
	Lexer lexer = SetupLexer(
		"my-if 4 my-then 5 my-else 6\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[([([(4 . my-if)] . my-then) 5] . my-else) 6]");

	ASSERT(parser->firstMessage == NullList);
	ASSERT(RecursiveEquals((SmileObject)result, expectedResult));

	// With the keyword declaration, this is a syntax error, because 'my-if' and 'my-then' and 'my-else'
	// cannot be used as unary or binary operators.
	lexer = SetupLexer(
		"keyword my-if, my-then, my-else\n"
		"my-if 4 my-then 5 my-else 6\n"
	);
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(parser->firstMessage != NullList);

	// And now the kicker:  *With* the keyword declaration, *and* a syntax rule, this is allowed again,
	// because 'my-if' and 'my-then' and 'my-else' are still valid for use as syntax keywords.
	lexer = SetupLexer(
		"#syntax STMT: [my-if [EXPR x] my-then [STMT y] my-else [STMT z]] => `[$if @x @y @z]\n"
		"keyword my-if, my-then, my-else\n"
		"my-if 4 my-then 5 my-else 6\n"
	);
	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	String foo = SmileObject_Stringify((SmileObject)result);

	expectedResult = SimpleParse("[$if 4 5 6]");

	ASSERT(parser->firstMessage == NullList);
	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
// Speculative syntax parsing tests

START_TEST(CanCollectExprStarSequences)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR* exprs] { }] => 123\n"
		"4 + 5\n"
		"foo 3 1 4 1 5 9 { }\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanCollectEmptyExprStarSequences)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR* exprs] { }] => 123\n"
		"4 + 5\n"
		"foo { }\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanCollectOneExprStarSequences)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR* exprs] { }] => 123\n"
		"4 + 5\n"
		"foo 3 { }\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanCollectExprPlusSequences)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR+ exprs] { }] => 123\n"
		"4 + 5\n"
		"foo 3 1 4 1 5 9 { }\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanCollectOneExprPlusSequences)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR+ exprs] { }] => 123\n"
		"4 + 5\n"
		"foo 3 { }\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanCollectNamePlusSequences)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [test [NAME+ names] { }] => 123\n"
		"4 + 5\n"
		"test that the program works { }\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanCollectOneNamePlusSequences)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [test [NAME+ names] { }] => 123\n"
		"4 + 5\n"
		"test mycode { }\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanTransformUnitTestSyntax)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [test [NAME+ names] { [EXPR* exprs] }] => `[declare-test [$quote @names] [$fn [] @@exprs]]\n"
		"\n"
		"test basic addition {\n"
		"\t4 + 5\n"
		"\t6 + 7\n"
		"}\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[declare-test [$quote [basic addition]] [$fn [] [(4 . +) 5] [(6 . +) 7]]]");
	SmileObject actualResult = LIST_THIRD(result);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(actualResult, expectedResult));
}
END_TEST

#include "parsersyntax_walk_tests.generated.inc"
