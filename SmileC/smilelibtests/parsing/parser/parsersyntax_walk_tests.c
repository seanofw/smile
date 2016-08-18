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

#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parsesyntax.h>

#include "testhelpers.h"

TEST_SUITE(ParserSyntaxWalkTests)

STATIC_STRING(TestFilename, "test.sm");

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
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
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
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
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
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(SubstitutionWorksWithAKnownNonterminal)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR x] baz] => (123 + x)\n"
		"4 + 5\n"
		"foo 999 baz\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(123 . +) 999]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(SubstitutionOfComplexContentWorksWithAKnownNonterminal)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR x] baz] => (123 + x)\n"
		"4 + 5\n"
		"foo 8 * 9 / 10 baz\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(123 . +) [([(8 . *) 9] . /) 10]]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(IfThenTest)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [if [EXPR x] then [STMT y]] => [if x y]\n"
		"4 + 5\n"
		"if 1 < 2 then 10\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(IfThenElseTest)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [if x y z]\n"
		"4 + 5\n"
		"if 1 < 2 then 10 else 20\n"
		"6 + 7\n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[if [(1 . <) 2] 10 20]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(IfThenElseTestWithBothIfThenRules)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [if [EXPR x] then [STMT y]] => [if x y]\n"
		"#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [if x y z]\n"
		"4 + 5\n"
		"if 1 < 2 then 10\n"
		"if 3 < 4 then 30 else 40\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[if [(3 . <) 4] 30 40]")));
	ASSERT(RecursiveEquals(LIST_SIXTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(IfThenElseTestWithNestedConditionals)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [if [EXPR x] then [STMT y]] => [if x y]\n"
		"#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [if x y z]\n"
		"4 + 5\n"
		"if 1 < 2 then\n"
		"  if 5 < 6 then 50\n"
		"  else 60\n"
		"else if 3 < 4 then\n"
		"  if 7 < 8 then 70\n"
		"  else 80\n"
		"else 40\n"
		"6 + 7\n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[if [(1 . <) 2] [if [(5 . <) 6] 50 60] [if [(3 . <) 4] [if [(7 . <) 8] 70 80] 40]]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CStyleIfThenElseTest)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [if ( [EXPR x] ) [STMT y]] => [if x y]\n"
		"#syntax STMT: [if ( [EXPR x] ) [STMT y] else [STMT z]] => [if x y z]\n"
		"4 + 5\n"
		"if (1 < 2) 10\n"
		"if (3 < 4) 30 else 40\n"
		"6 + 7\n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[if [(3 . <) 4] 30 40]")));
	ASSERT(RecursiveEquals(LIST_SIXTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(SimpleCustomDslTest)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [fronk { [FRONKS x] }] => [fronk `x]\n"
		"#syntax FRONKS: [[GROOP x] [FRONKS y]] => [x y]\n"
		"#syntax FRONKS: [[GROOP x]] => [x]\n"
		"#syntax GROOP: [qux] => qux\n"
		"#syntax GROOP: [xuq] => xuq\n"
		"1 + 2\n"
		"fronk { qux xuq xuq qux }\n"
		"3 + 4\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SIXTH(result), SimpleParse("[(1 . +) 2]")));
	ASSERT(RecursiveEquals(LIST_SEVENTH(result), SimpleParse("[fronk [quote [qux [xuq [xuq [qux]]]]]]")));
	ASSERT(RecursiveEquals(LIST_EIGHTH(result), SimpleParse("[(3 . +) 4]")));
}
END_TEST

#include "parsersyntax_walk_tests.generated.inc"
