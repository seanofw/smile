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

TEST_SUITE(ParserSyntaxValidateTests)

STATIC_STRING(TestFilename, "test.sm");

START_TEST(CannotAddRulesToUnknownRootClasses)
{
	Parser parser = Parser_Create();
	SmileList result = Parser_ParseFromC(parser, ParseScope_CreateRoot(), "#syntax FLERK: [math [EXPR x] plus [EXPR y]] => 123");
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(StmtMustStartWithAKeyword)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();

	result = Parser_ParseFromC(parser, parseScope, "#syntax STMT: [math [EXPR x] plus [EXPR y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	result = Parser_ParseFromC(parser, parseScope, "#syntax STMT: [[EXPR x] plus [EXPR y]] => 123");
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);

	result = Parser_ParseFromC(parser, parseScope, "#syntax STMT: [math [EXPR x] minus [EXPR y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);
}
END_TEST

START_TEST(ExprMustStartWithAKeyword)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();

	result = Parser_ParseFromC(parser, parseScope, "#syntax EXPR: [math [ADDEXPR x] plus [ADDEXPR y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	result = Parser_ParseFromC(parser, parseScope, "#syntax EXPR: [[ADDEXPR x] plus [ADDEXPR y]] => 123");
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);

	result = Parser_ParseFromC(parser, parseScope, "#syntax EXPR: [math [ADDEXPR x] minus [ADDEXPR y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);
}
END_TEST

START_TEST(CmpExprRequiresComplexSyntaxPatternRules)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;
	Int expectedErrorCount;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	expectedErrorCount = 0;

	// Starting with a keyword is okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [math [ADDEXPR x] flerk [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with ADDEXPR is okay, if not followed by a standard comparison operator.
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] flerk [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with any nonterminal other than ADDEXPR is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[MULEXPR x] flerk [MULEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);

	// Starting with ADDEXPR but following that with one of the standard comparisons is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] < [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] > [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] <= [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] >= [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] != [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] == [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] !== [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] === [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax CMPEXPR: [[ADDEXPR x] is [ADDEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(AddExprRequiresComplexSyntaxPatternRules)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;
	Int expectedErrorCount;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	expectedErrorCount = 0;

	// Starting with a keyword is okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax ADDEXPR: [math [MULEXPR x] flerk [MULEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with MULEXPR is okay, if not followed by a standard addition operator.
	result = Parser_ParseFromC(parser, parseScope, "#syntax ADDEXPR: [[MULEXPR x] flerk [MULEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with any nonterminal other than MULEXPR is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax ADDEXPR: [[TERM x] flerk [TERM y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);

	// Starting with MULEXPR but following that with one of the addition operators is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax ADDEXPR: [[MULEXPR x] + [MULEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax ADDEXPR: [[MULEXPR x] - [MULEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(MulExprRequiresComplexSyntaxPatternRules)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;
	Int expectedErrorCount;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	expectedErrorCount = 0;

	// Starting with a keyword is okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [math [BINARY x] flerk [BINARY y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with BINARY is okay, if not followed by a standard multiplication operator.
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [[BINARY x] flerk [BINARY y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with any nonterminal other than BINARY is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [[TERM x] flerk [TERM y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);

	// Starting with BINARY but following that with one of the multiplication operators is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [[BINARY x] * [BINARY y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [[BINARY x] / [BINARY y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(BinaryRequiresComplexSyntaxPatternRules)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;
	Int expectedErrorCount;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	expectedErrorCount = 0;

	// Starting with a keyword is okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax BINARY: [math [COLON x] flerk [COLON y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with COLON is okay, if followed by a keyword.
	result = Parser_ParseFromC(parser, parseScope, "#syntax BINARY: [[COLON x] flerk [COLON y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with any nonterminal other than COLON is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax BINARY: [[TERM x] flerk [TERM y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);

	// Starting with COLON but not following that with a keyword is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax BINARY: [[COLON x] [COLON y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(UnaryMustStartWithAKeyword)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();

	result = Parser_ParseFromC(parser, parseScope, "#syntax UNARY: [math [TERM x] plus [TERM y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	result = Parser_ParseFromC(parser, parseScope, "#syntax UNARY: [[TERM x] plus [TERM y]] => 123");
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);

	result = Parser_ParseFromC(parser, parseScope, "#syntax UNARY: [math [TERM x] minus [TERM y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);
}
END_TEST

START_TEST(PostfixRequiresComplexSyntaxPatternRules)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;
	Int expectedErrorCount;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	expectedErrorCount = 0;

	// Starting with a keyword is okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax POSTFIX: [math [DOUBLEHASH x] flerk [DOUBLEHASH y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with DOUBLEHASH is okay, if followed by a keyword.
	result = Parser_ParseFromC(parser, parseScope, "#syntax POSTFIX: [[DOUBLEHASH x] flerk [DOUBLEHASH y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with any nonterminal other than DOUBLEHASH is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax POSTFIX: [[TERM x] flerk [TERM y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);

	// Starting with DOUBLEHASH but not following that with a keyword is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax POSTFIX: [[DOUBLEHASH x] [DOUBLEHASH y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(TermMustStartWithAKeyword)
{
	Parser parser;
	ParseScope parseScope;
	SmileList result;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();

	result = Parser_ParseFromC(parser, parseScope, "#syntax TERM: [math [NAME x] plus [NAME y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	result = Parser_ParseFromC(parser, parseScope, "#syntax TERM: [[NAME x] plus [NAME y]] => 123");
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);

	result = Parser_ParseFromC(parser, parseScope, "#syntax TERM: [math [NAME x] minus [NAME y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);
}
END_TEST

#include "parsersyntax_validate_tests.generated.inc"
