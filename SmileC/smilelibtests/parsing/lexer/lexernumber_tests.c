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

#include <smile/parsing/lexer.h>

TEST_SUITE(LexerNumberTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Setup helper.

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
//  Integer forms.

START_TEST(ShouldRecognizeZero)
{
	Lexer lexer = Setup("  \t  0  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
	ASSERT(lexer->token->data.i == 0);

	lexer = Setup("  \t  0l  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 0);

	lexer = Setup("  \t  0L  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 0);

	lexer = Setup("  \t  0h  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.i == 0);

	lexer = Setup("  \t  0H  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.i == 0);

	lexer = Setup("  \t  0x  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.i == 0);

	lexer = Setup("  \t  0X  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.i == 0);
}
END_TEST

START_TEST(ShouldRecognizeHexZero)
{
	Lexer lexer = Setup("  \t  0x0  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
	ASSERT(lexer->token->data.i == 0);

	lexer = Setup("  \t  0x0l  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 0);

	lexer = Setup("  \t  0x0L  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 0);

	lexer = Setup("  \t  0x0h  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.i == 0);

	lexer = Setup("  \t  0x0H  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.i == 0);

	lexer = Setup("  \t  0x0x  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.i == 0);

	lexer = Setup("  \t  0x0X  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.i == 0);
}
END_TEST

START_TEST(ShouldRecognizeFloatZero)
{
	Lexer lexer = Setup("  \t  0.0f  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
	ASSERT(lexer->token->data.float64 == 0.0);

	lexer = Setup("  \t  0.0F  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
	ASSERT(lexer->token->data.float64 == 0.0);

	lexer = Setup("  \t  0.0hf  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
	ASSERT(lexer->token->data.float32 == 0.0f);

	lexer = Setup("  \t  0.0HF  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
	ASSERT(lexer->token->data.float32 == 0.0f);
}
END_TEST

START_TEST(ShouldRecognizeRealZero)
{
	Lexer lexer = Setup("  \t  0.0  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL64);
	ASSERT(Real64_Eq(lexer->token->data.real64, Real64_Zero));

	lexer = Setup("  \t  0.0l  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
	ASSERT(Real128_Eq(lexer->token->data.real128, Real128_Zero));

	lexer = Setup("  \t  0.0L  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
	ASSERT(Real128_Eq(lexer->token->data.real128, Real128_Zero));

	lexer = Setup("  \t  0.0h  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
	ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Zero));

	lexer = Setup("  \t  0.0H  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
	ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Zero));
}
END_TEST

#include "lexernumber_tests.generated.inc"
