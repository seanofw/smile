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
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/smilesyntax.h>

#include "testhelpers.h"

TEST_SUITE(ParserSyntaxTests)

STATIC_STRING(TestFilename, "test.sm");

//-------------------------------------------------------------------------------------------------
//  Syntax-object parsing tests.

START_TEST(CanParseSimpleSyntaxForms)
{
	Lexer lexer = SetupLexer("#syntax STMT: [a b c] => 123");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();

	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedResult = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		(SmileList)SimpleParse("[a b c]"),
		(SmileObject)SmileInteger32_Create(123),
		NULL
	);

	ASSERT(result->d == NullObject);
	ASSERT(SmileSyntax_Equals((SmileSyntax)result->a, expectedResult));
}
END_TEST

START_TEST(CanParseSyntaxFormsThatUseSyntaxForms)
{
	Lexer lexer = SetupLexer("#syntax STMT: [do magic] => Stdout print \"Hello, World.\"");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("Stdout")), PARSEDECL_VARIABLE, NULL, NULL);

	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedResult = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		(SmileList)SimpleParse("[do magic]"),
		(SmileObject)SimpleParse("[ (Stdout.print) \"Hello, World.\" ]"),
		NULL
	);

	ASSERT(result->d == NullObject);
	ASSERT(SmileSyntax_Equals((SmileSyntax)result->a, expectedResult));
}
END_TEST

#include "parsersyntax_tests.generated.inc"
