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

static UInt64 AToIDecimal(const char *str)
{
	UInt64 value = 0;
	char ch;

	while ((ch = *str++)) {
		if (ch == '_') continue;
		value = (value * 10) + (ch - '0');
	}

	return value;
}

static UInt64 AToIOctal(const char *str)
{
	UInt64 value = 0;
	char ch;

	while ((ch = *str++)) {
		if (ch == '_') continue;
		value = (value * 8) + (ch - '0');
	}

	return value;
}

static UInt64 AToIHex(const char *str)
{
	UInt64 value = 0;
	char ch;

	while ((ch = *str++)) {
		if (ch == '_') continue;
		if (ch >= 'a' && ch <= 'f')
			value = (value * 16) + (ch - 'a') + 10;
		else if (ch >= 'A' && ch <= 'F')
			value = (value * 16) + (ch - 'A') + 10;
		else
			value = (value * 16) + (ch - '0');
	}

	return value;
}

//-------------------------------------------------------------------------------------------------
//  Zero forms.

START_TEST(ShouldRecognizeZero)
{
	Lexer lexer = Setup("  \t  0  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 0);

	lexer = Setup("  \t  0t  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
	ASSERT(lexer->token->data.int32 == 0);

	lexer = Setup("  \t  0T  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
	ASSERT(lexer->token->data.int32 == 0);

	lexer = Setup("  \t  0s  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.int16 == 0);

	lexer = Setup("  \t  0S  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.int16 == 0);

	lexer = Setup("  \t  0x  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.byte == 0);

	lexer = Setup("  \t  0X  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.byte == 0);
}
END_TEST

START_TEST(ShouldRecognizeHexZero)
{
	Lexer lexer = Setup("  \t  0x0  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 0);

	lexer = Setup("  \t  0x0t  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
	ASSERT(lexer->token->data.int32 == 0);

	lexer = Setup("  \t  0x0T  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
	ASSERT(lexer->token->data.int32 == 0);

	lexer = Setup("  \t  0x0s  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.int16 == 0);

	lexer = Setup("  \t  0x0S  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.int16 == 0);

	lexer = Setup("  \t  0x0x  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.byte == 0);

	lexer = Setup("  \t  0x0X  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.byte == 0);
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

	lexer = Setup("  \t  0.0tf  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
	ASSERT(lexer->token->data.float32 == 0.0f);

	lexer = Setup("  \t  0.0TF  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
	ASSERT(lexer->token->data.float32 == 0.0f);

	lexer = Setup("  \t  .0f  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
	ASSERT(lexer->token->data.float64 == 0.0);

	lexer = Setup("  \t  .0F  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
	ASSERT(lexer->token->data.float64 == 0.0);

	lexer = Setup("  \t  .0tf  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
	ASSERT(lexer->token->data.float32 == 0.0f);

	lexer = Setup("  \t  .0TF  \r\n");
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

	lexer = Setup("  \t  0.0t  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
	ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Zero));

	lexer = Setup("  \t  0.0T  \r\n");
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

	lexer = Setup("  \t  .0t  \r\n");
	ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
	ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Zero));

	lexer = Setup("  \t  .0T  \r\n");
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
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)i);

		lexer = SetupString(String_Format("  \t  %ct  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)i);
	
		lexer = SetupString(String_Format("  \t  %cT  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)i);
	
		lexer = SetupString(String_Format("  \t  %cs  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)i);
	
		lexer = SetupString(String_Format("  \t  %cS  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)i);
	
		lexer = SetupString(String_Format("  \t  %cx  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)i);
	
		lexer = SetupString(String_Format("  \t  %cX  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)i);
	}
}
END_TEST

START_TEST(ShouldRecognizeSingleHexDigits)
{
	Lexer lexer;
	int i;

	for (i = 1; i <= 9; i++) {
		lexer = SetupString(String_Format("  \t  0x%c  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)i);
	
		lexer = SetupString(String_Format("  \t  0x%ct  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)i);
	
		lexer = SetupString(String_Format("  \t  0x%cT  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)i);
	
		lexer = SetupString(String_Format("  \t  0x%cs  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)i);
	
		lexer = SetupString(String_Format("  \t  0x%cS  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)i);
	
		lexer = SetupString(String_Format("  \t  0x%cx  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)i);
	
		lexer = SetupString(String_Format("  \t  0x%cX  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)i);
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

		lexer = SetupString(String_Format("  \t  %c.0tf  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i);
	
		lexer = SetupString(String_Format("  \t  %c.0TF  \r\n", '0' + i));
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

		lexer = SetupString(String_Format("  \t  0.%ctf  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i / 10.0f);
	
		lexer = SetupString(String_Format("  \t  0.%cTF  \r\n", '0' + i));
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

		lexer = SetupString(String_Format("  \t  .%ctf  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == (Float32)i / 10.0f);

		lexer = SetupString(String_Format("  \t  .%cTF  \r\n", '0' + i));
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

		lexer = SetupString(String_Format("  \t  %c.0t  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_FromInt32((Int32)i)));

		lexer = SetupString(String_Format("  \t  %c.0T  \r\n", '0' + i));
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

		lexer = SetupString(String_Format("  \t  0.%ct  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Div(Real32_FromInt32((Int32)i), Real32_Ten)));

		lexer = SetupString(String_Format("  \t  0.%cT  \r\n", '0' + i));
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

		lexer = SetupString(String_Format("  \t  .%ct  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Div(Real32_FromInt32((Int32)i), Real32_Ten)));

		lexer = SetupString(String_Format("  \t  .%cT  \r\n", '0' + i));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_Div(Real32_FromInt32((Int32)i), Real32_Ten)));
	}
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Multi-digit integer forms.

static const char *_8BitDecimalIntegers[] = {
	"10", "16", "64", "95", "127",
	"128", "160", "255",
};

static const char *_15BitDecimalIntegers[] = {
	"10", "16", "64", "95", "127",
	"128", "160", "255", "256", "314", "768",
	"1_000", "1_023", "1_024", "9_999", "24_576", "31_415",
};

static const char *_31BitDecimalIntegers[] = {
	"10", "16", "64", "95", "127",
	"128", "160", "255", "256", "314", "768",
	"1_000", "1_023", "1_024", "9_999", "24_576", "31_415",
	"65_535", "65_536", "100_000", "271_828", "314_159", "1_000_000", "2_718_282", "3_141_593",
	"10_000_000", "2_7182_818", "31_415_927", "100_000_000", "1_000_000_000", "2_147_483_647",
};

START_TEST(ShouldRecognizeDecimalIntegers)
{
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_8BitDecimalIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %sx  \r\n", _8BitDecimalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)AToIDecimal(_8BitDecimalIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  %sX  \r\n", _8BitDecimalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)AToIDecimal(_8BitDecimalIntegers[i]));
	}

	for (i = 0; i < sizeof(_15BitDecimalIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %ss  \r\n", _15BitDecimalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)AToIDecimal(_15BitDecimalIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  %sS  \r\n", _15BitDecimalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)AToIDecimal(_15BitDecimalIntegers[i]));
	}

	for (i = 0; i < sizeof(_31BitDecimalIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %st  \r\n", _31BitDecimalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)AToIDecimal(_31BitDecimalIntegers[i]));

		lexer = SetupString(String_Format("  \t  %sT  \r\n", _31BitDecimalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)AToIDecimal(_31BitDecimalIntegers[i]));
	}

	for (i = 0; i < sizeof(_31BitDecimalIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %s  \r\n", _31BitDecimalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)AToIDecimal(_31BitDecimalIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  %s1234567  \r\n", _31BitDecimalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)AToIDecimal(_31BitDecimalIntegers[i]) * 10000000ULL + 1234567ULL);
	}
}
END_TEST

static const char *_8BitOctalIntegers[] = {
	"1", "4", "7", "10", "16", "64", "177", "255", "314", "355", "377",
};

static const char *_15BitOctalIntegers[] = {
	"1", "4", "7", "10", "16", "64", "177", "255", "314", "355", "377",
	"1_000", "3_141", "3_777", "10_000", "24_576", "31_415", "37_777", "65_535",
	"65_536", "77_777",
};

static const char *_31BitOctalIntegers[] = {
	"1", "4", "7", "10", "16", "64", "177", "255", "314", "355", "377",
	"1_000", "3_141", "3_777", "10_000", "24_576", "31_415", "37_777", "65_535",
	"65_536", "77_777", "100_000", "271_525", "314_157", "1_000_000", "2_713_252", "3_141_573",
	"10_000_000", "27_152_515", "31_415_727", "100_000_000", "1_000_000_000", "2_147_473_647",
	"17_777_777_777",
};

START_TEST(ShouldRecognizeOctalIntegers)
{
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_8BitOctalIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  0%sx  \r\n", _8BitOctalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)AToIOctal(_8BitOctalIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  0%sX  \r\n", _8BitOctalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)AToIOctal(_8BitOctalIntegers[i]));
	}

	for (i = 0; i < sizeof(_15BitOctalIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  0%ss  \r\n", _15BitOctalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)AToIOctal(_15BitOctalIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  0%sS  \r\n", _15BitOctalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)AToIOctal(_15BitOctalIntegers[i]));
	}

	for (i = 0; i < sizeof(_31BitOctalIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  0%st  \r\n", _31BitOctalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)AToIOctal(_31BitOctalIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  0%sT  \r\n", _31BitOctalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)AToIOctal(_31BitOctalIntegers[i]));
	}

	for (i = 0; i < sizeof(_31BitOctalIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  0%s  \r\n", _31BitOctalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)AToIOctal(_31BitOctalIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  0%s1234567  \r\n", _31BitOctalIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)AToIOctal(_31BitOctalIntegers[i]) * 010000000 + 01234567);
	}
}
END_TEST

static const char *_8BitHexIntegers[] = {
	"1", "4", "7", "A", "F", "a", "f",
	"10", "1C", "5a", "64", "8B", "9D",
	"A5", "b0", "cc", "D9", "e0", "FF",
};

static const char *_15BitHexIntegers[] = {
	"1", "4", "7", "A", "F", "a", "f",
	"10", "1C", "5a", "64", "8B", "9D",
	"A5", "b0", "cc", "D9", "e0", "FF",
	"3D0", "10_00", "12_34", "5A_A5",
	"5A5A", "5_AAA", "678_9", "7F_FF",
};

static const char *_31BitHexIntegers[] = {
	"1", "4", "7", "A", "F", "a", "f",
	"10", "1C", "5a", "64", "8B", "9D",
	"A5", "b0", "cc", "D9", "e0", "FF",
	"3D0", "10_00", "12_34", "5A_A5",
	"5A5A", "5_AAA", "678_9", "7F_FF",
	"27182", "31415", "57AB9", "ABCDEF",
	"CCCCCC", "2_7182818", "3_1415926",
	"7FFF_FFFF",
};

START_TEST(ShouldRecognizeHexIntegers)
{
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_8BitHexIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  0x%sx  \r\n", _8BitHexIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)AToIHex(_8BitHexIntegers[i]));

		lexer = SetupString(String_Format("  \t  0x%sX  \r\n", _8BitHexIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
		ASSERT(lexer->token->data.byte == (Byte)AToIHex(_8BitHexIntegers[i]));
	}

	for (i = 0; i < sizeof(_15BitHexIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  0x%ss  \r\n", _15BitHexIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)AToIHex(_15BitHexIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  0x%sS  \r\n", _15BitHexIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
		ASSERT(lexer->token->data.int16 == (Int16)AToIHex(_15BitHexIntegers[i]));
	}

	for (i = 0; i < sizeof(_31BitHexIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  0x%st  \r\n", _31BitHexIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)AToIHex(_31BitHexIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  0x%sT  \r\n", _31BitHexIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER32);
		ASSERT(lexer->token->data.int32 == (Int32)AToIHex(_31BitHexIntegers[i]));
	}

	for (i = 0; i < sizeof(_31BitHexIntegers) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  0x%s  \r\n", _31BitHexIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)AToIHex(_31BitHexIntegers[i]));
	
		lexer = SetupString(String_Format("  \t  0x%s1234567  \r\n", _31BitHexIntegers[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
		ASSERT(lexer->token->data.int64 == (Int64)AToIHex(_31BitHexIntegers[i]) * 0x10000000 + 0x1234567);
	}
}
END_TEST

//-------------------------------------------------------------------------------------------------
//  Multi-digit real and float forms.

static const char *_testReal32Values[] = {
	"1.0", "1.",
	"125.0", "125.", ".125", "0.125", "125.125",
	"3.141_59", "31_415.8",
};

static const Float32 _testFloat32Values[] = {
	1.0f, 1.0f,
	125.0f, 125.0f, 0.125f, 0.125f, 125.125f,
	3.14159f, 31415.8f,
};

static const char *_testReal64Values[] = {
	"1.0", "1.",
	"125.0", "125.", ".125", "0.125", "125.125",
	"3.141_59", "31_415.8",
	"3.141_592_653_589", "3_141_592_653_589.0",
	"314_159.26_53_58_9",
};

static const Float64 _testFloat64Values[] = {
	1.0, 1.0,
	125.0, 125.0, 0.125, 0.125, 125.125,
	3.14159, 31415.8,
	3.141592653589, 3141592653589.0,
	314159.2653589,
};

START_TEST(ShouldRecognizeFloat32s)
{
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_testReal32Values) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %stf  \r\n", _testReal32Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == _testFloat32Values[i]);
	
		lexer = SetupString(String_Format("  \t  %sTF  \r\n", _testReal32Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT32);
		ASSERT(lexer->token->data.float32 == _testFloat32Values[i]);
	}
}
END_TEST

START_TEST(ShouldRecognizeFloat64s)
{
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_testReal64Values) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %sf  \r\n", _testReal64Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
		ASSERT(lexer->token->data.float64 == _testFloat64Values[i]);

		lexer = SetupString(String_Format("  \t  %sF  \r\n", _testReal64Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
		ASSERT(lexer->token->data.float64 == _testFloat64Values[i]);
	}
}
END_TEST

START_TEST(ShouldRecognizeReal32s)
{
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_testReal32Values) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %st  \r\n", _testReal32Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_FromFloat32(_testFloat32Values[i])));
	
		lexer = SetupString(String_Format("  \t  %sT  \r\n", _testReal32Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL32);
		ASSERT(Real32_Eq(lexer->token->data.real32, Real32_FromFloat32(_testFloat32Values[i])));
	}
}
END_TEST

START_TEST(ShouldRecognizeReal64s)
{
	Lexer lexer;
	int i;

	for (i = 0; i < sizeof(_testReal64Values) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %s  \r\n", _testReal64Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL64);
		ASSERT(Real64_Eq(lexer->token->data.real64, Real64_FromFloat64(_testFloat64Values[i])));
	}
}
END_TEST

START_TEST(ShouldRecognizeReal128s)
{
	Lexer lexer;
	int i;
	Real128 expectedValue;

	for (i = 0; i < sizeof(_testReal64Values) / sizeof(const char *); i++) {
		lexer = SetupString(String_Format("  \t  %sl  \r\n", _testReal64Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
		expectedValue = Real64_ToReal128(Real64_FromFloat64(_testFloat64Values[i]));
		ASSERT(Real128_Eq(lexer->token->data.real128, expectedValue));

		lexer = SetupString(String_Format("  \t  %sL  \r\n", _testReal64Values[i]));
		ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
		expectedValue = Real64_ToReal128(Real64_FromFloat64(_testFloat64Values[i]));
		ASSERT(Real128_Eq(lexer->token->data.real128, expectedValue));
	}
}
END_TEST

START_TEST(ShouldConsumeNumericSuffixes)
{
	Lexer lexer = SetupString(String_FromC("  \t  127x  64.0f  32.0L  16S  8  0x2BS  0777s  0x  0  \r\n"));

	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.byte == 127);

	ASSERT(Lexer_Next(lexer) == TOKEN_FLOAT64);
	ASSERT(lexer->token->data.float64 == 64);

	ASSERT(Lexer_Next(lexer) == TOKEN_REAL128);
	ASSERT(Real128_Eq(lexer->token->data.real128, Real128_FromInt32(32)));

	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.int16 == 16);

	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 8);

	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.int16 == 0x2B);

	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER16);
	ASSERT(lexer->token->data.int16 == 0777);

	ASSERT(Lexer_Next(lexer) == TOKEN_BYTE);
	ASSERT(lexer->token->data.byte == 0);

	ASSERT(Lexer_Next(lexer) == TOKEN_INTEGER64);
	ASSERT(lexer->token->data.int64 == 0);

	ASSERT(Lexer_Next(lexer) == TOKEN_EOI);
}
END_TEST

#include "lexernumber_tests.generated.inc"
