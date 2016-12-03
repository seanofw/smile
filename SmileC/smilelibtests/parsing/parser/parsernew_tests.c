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

TEST_SUITE(ParserNewTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  New-expression tests.

START_TEST(CanParseEmptyNew)
{
	Lexer lexer = SetupLexer("\t new { } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$new Object []]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseInheritedNew)
{
	Lexer lexer = SetupLexer("\t new SomeNamespace.SomeClass.SomeNestedClass { } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError parseError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "SomeNamespace"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$new ((SomeNamespace . SomeClass) . SomeNestedClass) []]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseNewWithMembers)
{
	Lexer lexer = SetupLexer("\t new { x:10 y:20 z:5 + 7 } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$new Object [[x 10] [y 20] [z [(5 . +) 7]]]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(NewWithMembersWithColonsIsAnError)
{
	Lexer lexer = SetupLexer("\t new { x:10 y:''Foo'':2 z:20 } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullObject);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(NewWithMembersWithNestedColonsIsNotAnError)
{
	Lexer lexer = SetupLexer("\t new { x:10 y:(''Foo'':2) z:20 } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$new Object [ [x 10] [y [(''Foo''.get-member) 2]] [z 20] ]]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(NewWithMembersSupportsNestedFunctions)
{
	Lexer lexer = SetupLexer("\t new { x:|x| x + 1 y:20 } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$new Object [ [x [$fn [x] [(x . +) 1] ]] [y 20] ]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(NewWithMembersDisallowsColonsInNestedFunctions)
{
	Lexer lexer = SetupLexer("\t new { x:|x| x:1 y:20 } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullObject);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(NewWithMembersAllowsColonsInNestedFunctionsIfWrapped)
{
	Lexer lexer = SetupLexer("\t new { x:|x| (x:1) y:20 } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$new Object [ [x [$fn [x] [(x . get-member) 1] ]] [y 20] ]]");

	ASSERT(RecursiveEquals(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(NewWithMembersAllowsColonsInNestedFunctionsIfWrapped2)
{
	Lexer lexer = SetupLexer("\t new { x:|x| { x:1 } y:20 } \n");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileObject expectedResult = SimpleParse("[$new Object [ [x [$fn [x] [(x . get-member) 1] ]] [y 20] ]]");

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

#include "parsernew_tests.generated.inc"
