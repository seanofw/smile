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
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>

TEST_SUITE(ParserCoreTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Setup helpers.

static Lexer SetupString(String source)
{
	Lexer lexer;

	Smile_ResetEnvironment();

	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	return lexer;
}

static Lexer Setup(const char *string)
{
	return SetupString(String_FromC(string));
}

//-------------------------------------------------------------------------------------------------
//  Primitive term tests.

START_TEST(EmptyInputResultsInEmptyParse)
{
	Lexer lexer = Setup("");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullList);
}
END_TEST

START_TEST(CanParseASingleInteger32)
{
	Lexer lexer = Setup("12345");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result != NULL && result != NullList);

	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(SMILE_KIND(result->a) == SMILE_KIND_INTEGER32);
	ASSERT(((SmileInteger32)result->a)->value == 12345);
}
END_TEST

START_TEST(CanParseASingleInteger64)
{
	Lexer lexer = Setup("12345L");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result != NULL && result != NullList);

	ASSERT(result->a != NULL && result->a != NullObject);
	ASSERT(result->d == NullObject);

	ASSERT(SMILE_KIND(result->a) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->a)->value == 12345LL);
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

	Lexer lexer = Setup("12 12345 45 0x10 0x2B ''or not'' 0x2B");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);
	Bool equal;

	ASSERT(result != NULL && result != NullList);

	equal = SMILE_VCALL1(result, compareEqual, (SmileObject)expectedResult);
	ASSERT(equal);
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

	Lexer lexer = Setup("12 ((12345)) (45) 0x10 0x2B (''or not'') 0x2B");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);
	Bool equal;

	ASSERT(result != NULL && result != NullList);

	equal = SMILE_VCALL1(result, compareEqual, (SmileObject)expectedResult);
	ASSERT(equal);
}
END_TEST

START_TEST(CanParseAPseudoDynamicString)
{
	SmileList expectedResult = SmileList_CreateList(
		(SmileObject)SmileString_Create(String_FromC("This is a test.")),
		NULL
	);

	Lexer lexer = Setup("  \"This is a test.\"  ");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);
	Bool equal;

	ASSERT(result != NULL && result != NullList);

	equal = SMILE_VCALL1(result, compareEqual, (SmileObject)expectedResult);
	ASSERT(equal);
}
END_TEST

#include "parsercore_tests.generated.inc"
