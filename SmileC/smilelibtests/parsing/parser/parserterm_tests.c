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
#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/smilepair.h>
#include <smile/env/env.h>

#include "testhelpers.h"

TEST_SUITE(ParserTermTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Primitive term tests.

START_TEST(CanParseASingleByte)
{
	Lexer lexer = SetupLexer("127x");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(SMILE_KIND(result) == SMILE_KIND_BYTE);
	ASSERT(((SmileByte)result)->value == 127);
}
END_TEST

START_TEST(CanParseASingleInteger16)
{
	Lexer lexer = SetupLexer("16383h");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(SMILE_KIND(result) == SMILE_KIND_INTEGER16);
	ASSERT(((SmileInteger16)result)->value == 16383);
}
END_TEST

START_TEST(CanParseASingleInteger32)
{
	Lexer lexer = SetupLexer("12345");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(SMILE_KIND(result) == SMILE_KIND_INTEGER32);
	ASSERT(((SmileInteger32)result)->value == 12345);
}
END_TEST

START_TEST(CanParseASingleInteger64)
{
	Lexer lexer = SetupLexer("12345L");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(SMILE_KIND(result) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result)->value == 12345LL);
}
END_TEST

#include "parserterm_tests.generated.inc"
