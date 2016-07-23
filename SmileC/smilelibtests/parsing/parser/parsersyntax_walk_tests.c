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

TEST_SUITE(ParserSyntaxWalkTests)

STATIC_STRING(TestFilename, "test.sm");

START_TEST(CanReplaceSimpleTerminalForms)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo] => 123\n"
		"4 + 5\n"
		"foo\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanReplaceMultiTerminalForms)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo bar baz] => 123\n"
		"4 + 5\n"
		"foo bar baz\n"
		"6 + 7\n"
	);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

START_TEST(CanReplaceFormsWithAKnownNonterminal)
{
	Lexer lexer = SetupLexer(
		"#syntax STMT: [foo [EXPR x] baz] => 123\n"
		"4 + 5\n"
		"foo 999 baz\n"
		"6 + 7\n"
		);
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(RecursiveEquals(LIST_SECOND(result), SimpleParse("[(4 . +) 5]")));
	ASSERT(RecursiveEquals(LIST_THIRD(result), SimpleParse("123")));
	ASSERT(RecursiveEquals(LIST_FOURTH(result), SimpleParse("[(6 . +) 7]")));
}
END_TEST

#include "parsersyntax_walk_tests.generated.inc"
