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
#include <smile/smiletypes/text/smilechar.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/env/env.h>

#include "testhelpers.h"

TEST_SUITE(ParserStringTests)

STATIC_STRING(TestFilename, "test.sm");

START_TEST(CanParseASingleChar)
{
	Lexer lexer = SetupLexer("'a'");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(SMILE_KIND(result) == SMILE_KIND_CHAR);
	ASSERT(((SmileChar)result)->value == 'a');
}
END_TEST

START_TEST(CanParseAPseudoDynamicString)
{
	SmileObject expectedResult = (SmileObject)SmileString_Create(String_FromC("This is a test."));

	Lexer lexer = SetupLexer("  \"This is a test.\"  ");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

START_TEST(CanParseADynamicString)
{
	Lexer lexer;
	Parser parser;
	SmileObject expectedResult;
	ParseScope parseScope;
	SmileObject result;

	lexer = SetupLexer("  \"This {x}is a {y}test.\"  ");

	expectedResult = SimpleParse("[ ([ (List.of) ''This '' x ''is a '' y ''test.'' ].join) ]");

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	ParseScope_Declare(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);
	ParseScope_Declare(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "y"), PARSEDECL_VARIABLE, NULL, NULL);
	result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(result, expectedResult));
}
END_TEST

#include "parserstring_tests.generated.inc"
