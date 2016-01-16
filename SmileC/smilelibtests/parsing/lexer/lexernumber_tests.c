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
//  Zero forms.

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

	lexer = Setup("  \t  .0f  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
	ASSERT(lexer->token->data.float64 == 0.0);

	lexer = Setup("  \t  .0F  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
	ASSERT(lexer->token->data.float64 == 0.0);

	lexer = Setup("  \t  .0hf  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
	ASSERT(lexer->token->data.float32 == 0.0f);

	lexer = Setup("  \t  .0HF  \r\n");
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

	lexer = Setup("  \t  .0  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL64);
	ASSERT(Real64_Eq(lexer->token->data.real64, Real64_Zero));

	lexer = Setup("  \t  .0l  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
	ASSERT(Real128_Eq(lexer->token->data.real128, Real128_Zero));

	lexer = Setup("  \t  .0L  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
	ASSERT(Real128_Eq(lexer->token->data.real128, Real128_Zero));

	lexer = Setup("  \t  .0h  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
	ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Zero));

	lexer = Setup("  \t  .0H  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
	ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Zero));
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Single-digit forms.

START_TEST(ShouldRecognizeSingleDigits)
{
	Lexer lexer;
	int i;

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  %c  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.i == (Int32)i);

		lexer = SetupString(String_Format("  \t  %cl  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)i);

		lexer = SetupString(String_Format("  \t  %cL  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)i);

		lexer = SetupString(String_Format("  \t  %ch  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.i == (Int16)i);

		lexer = SetupString(String_Format("  \t  %cH  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.i == (Int16)i);

		lexer = SetupString(String_Format("  \t  %cx  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.i == (Byte)i);

		lexer = SetupString(String_Format("  \t  %cX  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.i == (Byte)i);
	}
}
END_TEST

START_TEST(ShouldRecognizeSingleHexDigits)
{
	Lexer lexer;
	int i;

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  0x%c  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.i == (Int32)i);

		lexer = SetupString(String_Format("  \t  0x%cl  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)i);

		lexer = SetupString(String_Format("  \t  0x%cL  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)i);

		lexer = SetupString(String_Format("  \t  0x%ch  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.i == (Int16)i);

		lexer = SetupString(String_Format("  \t  0x%cH  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.i == (Int16)i);

		lexer = SetupString(String_Format("  \t  0x%cx  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.i == (Byte)i);

		lexer = SetupString(String_Format("  \t  0x%cX  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.i == (Byte)i);
	}
}
END_TEST

START_TEST(ShouldRecognizeSingleFloatDigits)
{
	Lexer lexer;
	int i;

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  %c.0f  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
		ASSERT(lexer->token->data.float64 == (Float64)i);

		lexer = SetupString(String_Format("  \t  %c.0F  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
		ASSERT(lexer->token->data.float64 == (Float64)i);

		lexer = SetupString(String_Format("  \t  %c.0hf  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i);

		lexer = SetupString(String_Format("  \t  %c.0HF  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i);
	}

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  0.%cf  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
		ASSERT(lexer->token->data.float64 == (Float64)i / 10.0);

		lexer = SetupString(String_Format("  \t  0.%cF  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
		ASSERT(lexer->token->data.float64 == (Float64)i / 10.0);

		lexer = SetupString(String_Format("  \t  0.%chf  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i / 10.0f);

		lexer = SetupString(String_Format("  \t  0.%cHF  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i / 10.0f);
	}

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  .%cf  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
		ASSERT(lexer->token->data.float64 == (Float64)i / 10.0);

		lexer = SetupString(String_Format("  \t  .%cF  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
		ASSERT(lexer->token->data.float64 == (Float64)i / 10.0);

		lexer = SetupString(String_Format("  \t  .%chf  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i / 10.0f);

		lexer = SetupString(String_Format("  \t  .%cHF  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i / 10.0f);
	}
}
END_TEST

START_TEST(ShouldRecognizeSingleRealDigits)
{
	Lexer lexer;
	int i;

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  %c.0  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL64);
		ASSERT(Real64_Eq(lexer->token->data.real64, Real64_FromInt32((Int32)i)));

		lexer = SetupString(String_Format("  \t  %c.0l  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
		ASSERT(Real128_Eq(lexer->token->data.real128, Real128_FromInt32((Int32)i)));

		lexer = SetupString(String_Format("  \t  %c.0L  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
		ASSERT(Real128_Eq(lexer->token->data.real128, Real128_FromInt32((Int32)i)));

		lexer = SetupString(String_Format("  \t  %c.0h  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_FromInt32((Int32)i)));

		lexer = SetupString(String_Format("  \t  %c.0H  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_FromInt32((Int32)i)));
	}

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  0.%c  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL64);
		ASSERT(Real64_Eq(lexer->token->data.real64, Real64_Div(Real64_FromInt32((Int32)i), Real64_Ten)));

		lexer = SetupString(String_Format("  \t  0.%cl  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
		ASSERT(Real128_Eq(lexer->token->data.real128, Real128_Div(Real128_FromInt32((Int32)i), Real128_Ten)));

		lexer = SetupString(String_Format("  \t  0.%cL  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
		ASSERT(Real128_Eq(lexer->token->data.real128, Real128_Div(Real128_FromInt32((Int32)i), Real128_Ten)));

		lexer = SetupString(String_Format("  \t  0.%ch  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Div(Real32_FromInt32((Int32)i), Real32_Ten)));

		lexer = SetupString(String_Format("  \t  0.%cH  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Div(Real32_FromInt32((Int32)i), Real32_Ten)));
	}

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  .%c  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL64);
		ASSERT(Real64_Eq(lexer->token->data.real64, Real64_Div(Real64_FromInt32((Int32)i), Real64_Ten)));

		lexer = SetupString(String_Format("  \t  .%cl  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
		ASSERT(Real128_Eq(lexer->token->data.real128, Real128_Div(Real128_FromInt32((Int32)i), Real128_Ten)));

		lexer = SetupString(String_Format("  \t  .%cL  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
		ASSERT(Real128_Eq(lexer->token->data.real128, Real128_Div(Real128_FromInt32((Int32)i), Real128_Ten)));

		lexer = SetupString(String_Format("  \t  .%ch  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Div(Real32_FromInt32((Int32)i), Real32_Ten)));

		lexer = SetupString(String_Format("  \t  .%cH  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Div(Real32_FromInt32((Int32)i), Real32_Ten)));
	}
}
END_TEST

#include "lexernumber_tests.generated.inc"
