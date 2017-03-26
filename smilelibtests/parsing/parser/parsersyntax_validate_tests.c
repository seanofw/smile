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
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilesyntax.h>
#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parsesyntax.h>

#include "testhelpers.h"

TEST_SUITE(ParserSyntaxValidateTests)

START_TEST(CannotAddRulesToUnknownRootClasses)
{
	Parser parser = Parser_Create();
	SmileObject result = Parser_ParseFromC(parser, ParseScope_CreateRoot(), "#syntax FLERK: [math [EXPR x] plus [EXPR y]] => 123");
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(StmtMustStartWithAKeyword)
{
	Parser parser;
	ParseScope parseScope;
	SmileObject result;

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
	SmileObject result;

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
	SmileObject result;
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
	SmileObject result;
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
	SmileObject result;
	Int expectedErrorCount;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	expectedErrorCount = 0;

	// Starting with a keyword is okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [math [BINARYEXPR x] flerk [BINARYEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with BINARYEXPR is okay, if not followed by a standard multiplication operator.
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [[BINARYEXPR x] flerk [BINARYEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with any nonterminal other than BINARYEXPR is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [[TERM x] flerk [TERM y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);

	// Starting with BINARY but following that with one of the multiplication operators is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [[BINARYEXPR x] * [BINARYEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	result = Parser_ParseFromC(parser, parseScope, "#syntax MULEXPR: [[BINARYEXPR x] / [BINARYEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(BinaryExprRequiresComplexSyntaxPatternRules)
{
	Parser parser;
	ParseScope parseScope;
	SmileObject result;
	Int expectedErrorCount;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	expectedErrorCount = 0;

	// Starting with a keyword is okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax BINARYEXPR: [math [COLONEXPR x] flerk [COLONEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with COLONEXPR is okay, if followed by a keyword.
	result = Parser_ParseFromC(parser, parseScope, "#syntax BINARYEXPR: [[COLONEXPR x] flerk [COLONEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with any nonterminal other than COLONEXPR is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax BINARYEXPR: [[TERM x] flerk [TERM y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);

	// Starting with COLONEXPR but not following that with a keyword is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax BINARYEXPR: [[COLONEXPR x] [COLONEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(PrefixExprMustStartWithAKeyword)
{
	Parser parser;
	ParseScope parseScope;
	SmileObject result;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();

	result = Parser_ParseFromC(parser, parseScope, "#syntax PREFIXEXPR: [math [TERM x] plus [TERM y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 0);

	result = Parser_ParseFromC(parser, parseScope, "#syntax PREFIXEXPR: [[TERM x] plus [TERM y]] => 123");
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);

	result = Parser_ParseFromC(parser, parseScope, "#syntax PREFIXEXPR: [math [TERM x] minus [TERM y]] => 123");
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);
	ASSERT(Parser_GetErrorCount(parser) == 1);
}
END_TEST

START_TEST(PostfixExprRequiresComplexSyntaxPatternRules)
{
	Parser parser;
	ParseScope parseScope;
	SmileObject result;
	Int expectedErrorCount;

	parser = Parser_Create();
	parseScope = ParseScope_CreateRoot();
	expectedErrorCount = 0;

	// Starting with a keyword is okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax POSTFIXEXPR: [math [CONSEXPR x] flerk [CONSEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with CONSEXPR is okay, if followed by a keyword.
	result = Parser_ParseFromC(parser, parseScope, "#syntax POSTFIXEXPR: [[CONSEXPR x] flerk [CONSEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == expectedErrorCount);
	ASSERT(SMILE_KIND(result) != SMILE_KIND_NULL);

	// Starting with any nonterminal other than CONSEXPR is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax POSTFIXEXPR: [[TERM x] flerk [TERM y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);

	// Starting with CONSEXPR but not following that with a keyword is not okay.
	result = Parser_ParseFromC(parser, parseScope, "#syntax POSTFIXEXPR: [[CONSEXPR x] [CONSEXPR y]] => 123");
	ASSERT(Parser_GetErrorCount(parser) == ++expectedErrorCount);
	ASSERT(SMILE_KIND(result) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(TermMustStartWithAKeyword)
{
	Parser parser;
	ParseScope parseScope;
	SmileObject result;

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
