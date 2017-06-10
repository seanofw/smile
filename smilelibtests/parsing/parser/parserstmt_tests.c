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
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/env/env.h>

#include "testhelpers.h"

TEST_SUITE(ParserStmtTests)

//-------------------------------------------------------------------------------------------------
//  If tests.

START_TEST(CanParseIfThenStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"if 1 < 2 then 10\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseIfThenElseStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"if 1 < 2 then 10 else 20\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$if [(1 . <) 2] 10 20]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseIfThenElseWithBothRules)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"if 1 < 2 then 10\n"
		"if 3 < 4 then 30 else 40\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$if [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[$if [(3 . <) 4] 30 40]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseIfThenElseWithNestedConditionals)
{
	Lexer lexer = SetupLexer(
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
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$if [(1 . <) 2] [$if [(5 . <) 6] 50 60] [$if [(3 . <) 4] [$if [(7 . <) 8] 70 80] 40]]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Unless tests.

START_TEST(CanParseUnlessThenStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"unless 1 < 2 then 10\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$if [$not [(1 . <) 2]] 10]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseUnlessThenElseStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"unless 1 < 2 then 10 else 20\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$if [$not [(1 . <) 2]] 10 20]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseUnlessThenElseWithBothRules)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"unless 1 < 2 then 10\n"
		"unless 3 < 4 then 30 else 40\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$if [$not [(1 . <) 2]] 10]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[$if [$not [(3 . <) 4]] 30 40]")));
	ASSERT(RecursiveEquals(LIST_FIFTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseUnlessThenElseWithNestedConditionals)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"unless 1 < 2 then\n"
		"  unless 5 < 6 then 50\n"
		"  else 60\n"
		"else unless 3 < 4 then\n"
		"  unless 7 < 8 then 70\n"
		"  else 80\n"
		"else 40\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$if [$not [(1 . <) 2]] [$if [$not [(5 . <) 6]] 50 60] [$if [$not [(3 . <) 4]] [$if [$not [(7 . <) 8]] 70 80] 40]]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  While/until tests.

START_TEST(CanParseWhileDoStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"while 1 < 2 do 10\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$while [(1 . <) 2] 10]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseUntilDoStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"until 1 < 2 do 10\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$while [$not [(1 . <) 2]] 10]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Do-while/do-until tests.

START_TEST(CanParseDoWhileStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"do 10 while 1 < 2\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$while 10 [(1 . <) 2] null]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseDoUntilStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"do 10 until 1 < 2\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$while 10 [$not [(1 . <) 2]] null]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Try/catch tests.

START_TEST(CanParseTryCatchStatements)
{
	Lexer lexer = SetupLexer(
		"try {\n"
		"  4 + 5\n"
		"  6 + 7\n"
		"}\n"
		"catch |e| 10 + 20\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$catch")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[$progn [(4 . +) 5] [(6 . +) 7]]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$fn [e] [(10 . +) 20]]")));
}
END_TEST

START_TEST(CanParseTryCatchStatementsWithARawFunction)
{
	Lexer lexer = SetupLexer(
		"try {\n"
		"  4 + 5\n"
		"  6 + 7\n"
		"}\n"
		"catch [$fn [e] 10 + 20]\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$catch")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[$progn [(4 . +) 5] [(6 . +) 7]]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$fn [e] [(10 . +) 20]]")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Return tests.

START_TEST(CanParseReturnStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"return 10 + 20\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$return [(10 . +) 20]]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Till tests.

START_TEST(CanParseTillStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"till done do 10 + 20\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$till [done] [(10 . +) 20]]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseTillFlagsInsideTillStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"till done do {\n"
		"    10 + 20\n"
		"    if 1 then done\n"
		"    30 + 40\n"
		"}\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$till [done] [$progn [(10 . +) 20] [$if 1 done] [(30 . +) 40]]]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseMultipleTillFlagsInsideTillStatements)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"till found, not-found, abort do {\n"
		"    10 + 20\n"
		"    if 1 then found\n"
		"    if 2 then not-found\n"
		"    if 3 then abort\n"
		"    30 + 40\n"
		"}\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$till [found not-found abort] [$progn\n"
			"[(10 . +) 20]\n"
			"[$if 1 found]\n"
			"[$if 2 not-found]\n"
			"[$if 3 abort]\n"
			"[(30 . +) 40]\n"
		"]]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseAWhenClauseForATillStatement)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"till done do {\n"
		"    10 + 20\n"
		"    if 1 then done\n"
		"    30 + 40\n"
		"}\n"
		"when done {\n"
		"    50 + 60\n"
		"}\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$till\n"
			"[done]\n"
			"[$progn [(10 . +) 20] [$if 1 done] [(30 . +) 40]]\n"
			"[[done [(50 . +) 60]]]\n"
		"]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanParseMultipleWhenClausesForATillStatement)
{
	Lexer lexer = SetupLexer(
		"4 + 5\n"
		"till found, not-found, abort do {\n"
		"    10 + 20\n"
		"    if 1 then found\n"
		"    if 2 then not-found\n"
		"    if 3 then abort\n"
		"    30 + 40\n"
		"}\n"
		"when found {\n"
		"    50 + 60\n"
		"}\n"
		"when not-found {\n"
		"    70 + 80\n"
		"}\n"
		"when abort {\n"
		"    90 + 100\n"
		"}\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = (SmileList)Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_FIRST(result), SimpleParse("$progn")));
	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("[$till\n"
		"[found not-found abort]\n"
		"[$progn\n"
			"[(10 . +) 20]\n"
			"[$if 1 found]\n"
			"[$if 2 not-found]\n"
			"[$if 3 abort]\n"
			"[(30 . +) 40]\n"
		"]\n"
		"[\n"
			"[found [(50 . +) 60]]\n"
			"[not-found [(70 . +) 80]]\n"
			"[abort [(90 . +) 100]]\n"
		"]\n"
	"]")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

#include "parserstmt_tests.generated.inc"
