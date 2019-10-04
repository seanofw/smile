//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter (Unit Tests)
//  Copyright 2004-2019 Sean Werkema
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

#include <smile/parsing/lexer.h>

TEST_SUITE(LexerPositionTests)

//-------------------------------------------------------------------------------------------------
//  Setup helper.

static Lexer SetupString(String source)
{
	Lexer lexer;

	Smile_ResetEnvironment();

	lexer = Lexer_Create(source, 0, String_Length(source), GetTestScriptName(), 1, 1, True);
	lexer->symbolTable = Smile_SymbolTable;

	return lexer;
}

static Lexer Setup(const char *string)
{
	return SetupString(String_FromC(string));
}

typedef struct PositionResultStruct {
	Int tokenKind;
	Int line;
	Int column;
	Int lineStart;
	Int length;
} *PositionResult;

static void VerifyPositions(Lexer lexer, PositionResult expectedResults, Int numExpectedResults)
{
	Int i;

	for (i = 0; i < numExpectedResults; i++) {
		PositionResult expected = expectedResults + i;
		Int actualToken = Lexer_Next(lexer);
		LexerPosition actualPos = Token_GetPosition(lexer->token);

		ASSERT(actualToken == expected->tokenKind);
		ASSERT(actualPos->line == expected->line);
		ASSERT(actualPos->column == expected->column);
		ASSERT(actualPos->lineStart == expected->lineStart);
		ASSERT(actualPos->length == expected->length);
	}
}

//-------------------------------------------------------------------------------------------------
//  Whitespace and comment tests.

START_TEST(CanMeasureWhitespaceAndNewlines)
{
	Lexer lexer = Setup("   \t\a\b   \r\n   \r   \n   \n\r   \f\v   ");

	static struct PositionResultStruct expectedResults[] = {
		// token				ln col  st len
		{ TOKEN_WHITESPACE,		 1,  1,  0,  9 },		// \t\a\b
		{ TOKEN_NEWLINE,		 1, 10,  0,  2 },		// \r\n
		{ TOKEN_WHITESPACE,		 2,  1, 11,  3 },
		{ TOKEN_NEWLINE,		 2,  4, 11,  1 },		// \r
		{ TOKEN_WHITESPACE,		 3,  1, 15,  3 },
		{ TOKEN_NEWLINE,		 3,  4, 15,  1 },		// \n
		{ TOKEN_WHITESPACE,		 4,  1, 19,  3 },
		{ TOKEN_NEWLINE,		 4,  4, 19,  2 },		// \n\r
		{ TOKEN_WHITESPACE,		 5,  1, 24,  8 },		// \f\v
		{ TOKEN_EOI,			 5,  9, 24,  0 },
	};

	VerifyPositions(lexer, expectedResults,
		sizeof(expectedResults) / sizeof(struct PositionResultStruct));
}
END_TEST

START_TEST(CanMeasureSingleLineComments)
{
	Lexer lexer = Setup("   // floob\r\n//greep   \r //spud  \n//poit   \n\r// narf ");

	static struct PositionResultStruct expectedResults[] = {
		// token					ln col  st len
		{ TOKEN_WHITESPACE,			 1,  1,  0,  3 },
		{ TOKEN_COMMENT_SINGLELINE,	 1,  4,  0,  8 },		// floob
		{ TOKEN_NEWLINE,			 1, 12,  0,  2 },		// \r\n
		{ TOKEN_COMMENT_SINGLELINE,	 2,  1, 13, 10 },		// greep
		{ TOKEN_NEWLINE,			 2, 11, 13,  1 },		// \r
		{ TOKEN_WHITESPACE,			 3,  1, 24,  1 },
		{ TOKEN_COMMENT_SINGLELINE,	 3,  2, 24,  8 },		// spud
		{ TOKEN_NEWLINE,			 3, 10, 24,  1 },		// \n
		{ TOKEN_COMMENT_SINGLELINE,	 4,  1, 34,  9 },		// poit
		{ TOKEN_NEWLINE,			 4, 10, 34,  2 },		// \n\r
		{ TOKEN_COMMENT_SINGLELINE,	 5,  1, 45,  8 },		// poit
		{ TOKEN_EOI,				 5,  9, 45,  0 },
	};

	VerifyPositions(lexer, expectedResults,
		sizeof(expectedResults) / sizeof(struct PositionResultStruct));
}
END_TEST

START_TEST(CanMeasureMultilineComments)
{
	Lexer lexer = Setup("   /* floob\r\n//greep   \r //spud  \n//poit   \n\r*/ narf ");

	static struct PositionResultStruct expectedResults[] = {
		// token					ln col  st len
		{ TOKEN_WHITESPACE,			 1,  1,  0,  3 },
		{ TOKEN_COMMENT_MULTILINE,	 1,  4,  0, 44 },
		{ TOKEN_WHITESPACE,			 5,  3, 45,  1 },
		{ TOKEN_ALPHANAME,			 5,  4, 45,  4 },
		{ TOKEN_WHITESPACE,			 5,  8, 45,  1 },
		{ TOKEN_EOI,				 5,  9, 45,  0 },
	};

	VerifyPositions(lexer, expectedResults,
		sizeof(expectedResults) / sizeof(struct PositionResultStruct));
}
END_TEST

START_TEST(CanMeasureIncludes)
{
	Lexer lexer = Setup("   #include \"stdio\"  ");

	static struct PositionResultStruct expectedResults[] = {
		// token					ln col  st len
		{ TOKEN_WHITESPACE,			 1,  1,  0,  3 },
		{ TOKEN_LOANWORD_INCLUDE,	 1,  4,  0,  8 },
		{ TOKEN_WHITESPACE,			 1, 12,  0,  1 },
		{ TOKEN_DYNSTRING,			 1, 13,  0,  7 },
		{ TOKEN_WHITESPACE,			 1, 20,  0,  2 },
		{ TOKEN_EOI,				 1, 22,  0,  0 },
	};

	VerifyPositions(lexer, expectedResults,
		sizeof(expectedResults) / sizeof(struct PositionResultStruct));
}
END_TEST

#include "lexerposition_tests.generated.inc"
