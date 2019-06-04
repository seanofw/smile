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

#include <smile/parsing/parser.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilesyntax.h>

#include "testhelpers.h"

TEST_SUITE(ParserSyntaxTests)

//-------------------------------------------------------------------------------------------------
//  Syntax-object parsing tests.

START_TEST(CanParseSimpleSyntaxForms)
{
	Lexer lexer = SetupLexer("#syntax STMT: [a b c] => 123");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();

	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedResult = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		(SmileList)SimpleParse("[a b c]"),
		(SmileObject)SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SMILE_SPECIAL_SYMBOL__QUOTE),
			(SmileObject)SmileList_Cons(
				(SmileObject)SmileInteger64_Create(123),
				NullObject
			)
		),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseSyntaxFormsThatUseSyntaxForms)
{
	Lexer lexer = SetupLexer("#syntax STMT: [do magic] => `(Stdout print \"Hello, World.\")");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("Stdout")), PARSEDECL_VARIABLE, NULL, NULL);

	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedResult = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		(SmileList)SimpleParse("[do magic]"),
		(SmileObject)SimpleParse("[$quote [(Stdout.print) \"Hello, World.\"]]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseSyntaxFormsThatUseListForms)
{
	Lexer lexer = SetupLexer("#syntax STMT: [do magic] => [Stdout.print \"Hello, World.\"]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("Stdout")), PARSEDECL_VARIABLE, NULL, NULL);

	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedResult = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		(SmileList)SimpleParse("[do magic]"),
		(SmileObject)SimpleParse("[$quote [(Stdout.print) \"Hello, World.\"]]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(CanParseSyntaxFormsThatContainNonterminals)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR x]] => [(x).* (x)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedResult = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("magic"))),
			(SmileObject)SmileList_Cons(
				(SmileObject)SmileNonterminal_Create(
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
					0,
					0,
					0,
					NULL
				),
				NullObject
			)
		),
		(SmileObject)SimpleParse("[(List.of) [(List.of) [$quote $dot] x [$quote *]] x]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedResult));
}
END_TEST

START_TEST(NonterminalsShouldNotLeakIntoTheContainingScope)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR x]] => [x.* x]\n"
		"var y = x");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(TheContainingScopeShouldInfluenceTheReplacement)
{
	// In this first test, we parse a syntax form that references a function outside its
	// scope.  If the function 'f' is not visible while parsing the body of this syntax
	// declaration, then we'll get [[x.f]] instead of [f x] as output.
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR x]] => `([f x])");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "f"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "magic")),
			(SmileObject)SmileList_Cons(
				(SmileObject)SmileNonterminal_Create(
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
					0,
					0,
					0,
					NULL
				),
				NullObject
			)
		),
		(SmileObject)SimpleParse("[$quote [f x]]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

START_TEST(TheContainingScopeShouldInfluenceTheReplacement2)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR y]] => `(x op y)");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

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
					0,
					0,
					NULL
				),
				NullObject
			)
		),
		(SmileObject)SimpleParse("[$quote [(x . op) y]]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

START_TEST(TheContainingScopeShouldInfluenceTheReplacement3)
{
	Lexer lexer = SetupLexer("#syntax STMT: [magic [EXPR y]] => `(x op y)");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();

	// Notice how we specifically *don't* declare "x" in this test, like we did in
	// the otherwise-identical test above.  "x" should now be seen by the parser
	// as an operator, not as a variable.
	//
	//ParseError parseDeclError = ParseScope_DeclareHere(parseScope, SymbolTable_GetSymbolC(Smile_SymbolTable, "x"), PARSEDECL_VARIABLE, NULL, NULL);

	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_Cons(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "magic")),
			(SmileObject)SmileList_Cons(
				(SmileObject)SmileNonterminal_Create(
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
					SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
					0,
					0,
					0,
					NULL
				),
				NullObject
			)
		),
		(SmileObject)SimpleParse("[$quote [([(y . op)] . x)]]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

START_TEST(CanParseSyntaxWithEmbeddedCommas)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if [EXPR x], [EXPR y] then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_CreateList(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "if")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, ",")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "then")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("z")),
				0,
				0,
				0,
				NULL
			),
			NULL
		),
		(SmileObject)SimpleParse("[(List.of) [$quote if] x y z]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

START_TEST(CanParseSyntaxWithEmbeddedSemicolons)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if [EXPR x]; [EXPR y] then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_CreateList(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "if")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, ";")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "then")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("z")),
				0,
				0,
				0,
				NULL
			),
			NULL
		),
		(SmileObject)SimpleParse("[(List.of) [$quote if] x y z]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

START_TEST(CanParseSyntaxWithEmbeddedColons)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if [EXPR x]: [EXPR y] then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_CreateList(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "if")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, ":")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "then")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("z")),
				0,
				0,
				0,
				NULL
			),
			NULL
		),
		(SmileObject)SimpleParse("[(List.of) [$quote if] x y z]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

START_TEST(CanParseSyntaxWithEmbeddedParentheses)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if ([EXPR x]: [EXPR y]) then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_CreateList(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "if")),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "(")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, ":")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, ")")),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "then")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("z")),
				0,
				0,
				0,
				NULL
			),
			NULL
		),
		(SmileObject)SimpleParse("[(List.of) [$quote if] x y z]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

START_TEST(CannotParseSyntaxWithMismatchedParentheses1)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if ([EXPR x]: [EXPR y] then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullObject);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotParseSyntaxWithMismatchedParentheses2)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if [EXPR x]: [EXPR y]) then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullObject);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CanParseSyntaxWithEmbeddedCurlyBraces)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if {[EXPR x]: [EXPR y]} then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_CreateList(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "if")),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "{")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, ":")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "}")),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "then")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("z")),
				0,
				0,
				0,
				NULL
			),
			NULL
		),
		(SmileObject)SimpleParse("[(List.of) [$quote if] x y z]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

START_TEST(CannotParseSyntaxWithMismatchedCurlyBraces1)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if {[EXPR x]: [EXPR y] then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullObject);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(CannotParseSyntaxWithMismatchedCurlyBraces2)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if [EXPR x]: [EXPR y]} then [STMT z]] => [if (x) (y) (z)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	ASSERT(result == NullObject);
	ASSERT(Parser_GetErrorCount(parser) > 0);
}
END_TEST

START_TEST(RealWorldSyntaxExample)
{
	Lexer lexer = SetupLexer("#syntax STMT: [if [EXPR x] then [STMT y]] => [if (x) (y)]");
	Parser parser = Parser_Create();
	ParseScope parseScope = ParseScope_CreateRoot();
	SmileObject result = Parser_Parse(parser, lexer, parseScope);

	SmileSyntax expectedSyntax = SmileSyntax_Create(
		SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
		SmileList_CreateList(
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "if")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("EXPR")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("x")),
				0,
				0,
				0,
				NULL
			),
			(SmileObject)SmileSymbol_Create(SymbolTable_GetSymbolC(Smile_SymbolTable, "then")),
			(SmileObject)SmileNonterminal_Create(
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("STMT")),
				SymbolTable_GetSymbol(Smile_SymbolTable, String_FromC("y")),
				0,
				0,
				0,
				NULL
			),
			NULL
		),
		(SmileObject)SimpleParse("[(List.of) [$quote if] x y]"),
		NULL
	);

	ASSERT(SmileObject_DeepCompare(result, (SmileObject)expectedSyntax));
}
END_TEST

#include "parsersyntax_decl_tests.generated.inc"
