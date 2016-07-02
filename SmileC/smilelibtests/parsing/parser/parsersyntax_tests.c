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

START_TEST(CanParseSyntaxFormsThatUseListForms)
{
	Lexer lexer = SetupLexer("#syntax STMT: [do magic] => [Stdout.print \"Hello, World.\"]");
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

START_TEST(CanParseSyntaxFormsThatContainNonterminals)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR x]] => [x.* x]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedResult = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("magic"))),
			(SmileObject)SmileList_Cons(
				(SmileObject)SmileNonterminal_Create(
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
					0,
					0
				),
				NullObject
			)
		),
		(SmileObject)SimpleParse("[(x.*) x]"),
		NULL
	);

	ASSERT(result->d == NullObject);
	ASSERT(SmileSyntax_Equals((SmileSyntax)result->a, expectedResult));
}
END_TEST

START_TEST(NonterminalsShouldNotLeakIntoTheContainingScope)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR x]] => [x.* x]\n"
		"var y = x");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(TheContainingScopeShouldInfluenceTheReplacement)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR x]] => [f x]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "f"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "magic")),
			(SmileObject)SmileList_Cons(
				(SmileObject)SmileNonterminal_Create(
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
					0,
					0
				),
				NullObject
			)
		),
		(SmileObject)SimpleParse("[f x]"),
		NULL
	);

	ASSERT((SmileObject)result != NullObject);
	ASSERT(SmileSyntax_Equals((SmileSyntax)result->a, expectedSyntax));
}
END_TEST

START_TEST(TheContainingScopeShouldInfluenceTheReplacement2)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR y]] => x op y");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	// We expect "x op y" to become "[x.op y]" when "x" is declared in the parent
	// scope.  If "x" is not an accessible variable, then this should parse
	// instead as "[[y.op].x]".  In this test, we do it "x" declared, and in the
	// next test, we do it without "x" declared to make sure it really is behaving
	// as expected.
	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "magic")),
			(SmileObject)SmileList_Cons(
				(SmileObject)SmileNonterminal_Create(
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
					0,
					0
				),
				NullObject
			)
		),
		(SmileObject)SimpleParse("[(x . op) y]"),
		NULL
	);

	ASSERT((SmileObject)result != NullObject);
	ASSERT(SmileSyntax_Equals((SmileSyntax)result->a, expectedSyntax));
}
END_TEST

START_TEST(TheContainingScopeShouldInfluenceTheReplacement3)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR y]] => x op y");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();

	// Notice how we specifically *don't* declare "x" in this test, like we did in
	// the otherwise-identical test above.  "x" should now be seen by the parser
	// as an operator, not as a variable.
	//
	//ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);

	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "magic")),
			(SmileObject)SmileList_Cons(
				(SmileObject)SmileNonterminal_Create(
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
					0,
					0
				),
				NullObject
			)
		),
		(SmileObject)SimpleParse("[([(y . op)] . x)]"),
		NULL
	);

	ASSERT((SmileObject)result != NullObject);
	ASSERT(SmileSyntax_Equals((SmileSyntax)result->a, expectedSyntax));
}
END_TEST

START_TEST(RealWorldSyntaxExample)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if [EXPR x] then [STMT y]] => [\\if x y]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileList result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_CreateList(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "if")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
				0,
				0
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "then")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
				0,
				0
			),
			NULL
		),
		(SmileObject)SimpleParse("[if x y]"),
		NULL
	);

	ASSERT((SmileObject)result != NullObject);
	ASSERT(SmileSyntax_Equals((SmileSyntax)result->a, expectedSyntax));
}
END_TEST

#include "parsersyntax_tests.generated.inc"
