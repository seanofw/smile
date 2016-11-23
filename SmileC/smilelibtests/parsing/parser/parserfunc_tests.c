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

TEST_SUITE(ParserFuncTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Function-parsing tests.

START_TEST(CanParseAFunction)
{
	Lexer lexer = SetupLexer("|x| x");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$fn [x] x]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAFunctionOfNoArguments)
{
	Lexer lexer = SetupLexer("|| 123");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$fn [] 123]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAFunctionWithMultipleArguments)
{
	Lexer lexer = SetupLexer("|w x y z| [w x y z]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$fn [w x y z] [w x y z]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(CanParseAFunctionWithOptionalCommas)
{
	Lexer lexer = SetupLexer("|w, x, y, z| [w x y z]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedForm = SimpleParse("[$fn [w x y z] [w x y z]]");

	ASSERT(result != NULL && result != NullList);
	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(RecursiveEquals(result->a, expectedForm));
}
END_TEST

START_TEST(AFunctionWithBrokenCommasShouldFail)
{
	Lexer lexer = SetupLexer("|w, x, y,| [w x y]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(AFunctionWithBrokenCommasShouldFail2)
{
	Lexer lexer = SetupLexer("|, x, y| [x y]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(AFunctionWithBrokenCommasShouldFail3)
{
	Lexer lexer = SetupLexer("|,| 123");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

#include "parserfunc_tests.generated.inc"
