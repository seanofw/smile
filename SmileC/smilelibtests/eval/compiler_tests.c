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

#include "../stdafx.h"

#include <smile/env/env.h>
#include <smile/eval/bytecode.h>
#include <smile/eval/opcode.h>
#include <smile/eval/compiler.h>
#include <smile/parsing/parser.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/smilepair.h>

STATIC_STRING(TestFilename, "test.sm");

TEST_SUITE(CompilerTests)

static SmileObject Parse(const char *text)
{
	String source;
	Lexer lexer;
	Parser parser;
	ParseScope scope;
	SmileObject expr;

	Smile_ResetEnvironment();

	source = String_FromC(text);

	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	parser = Parser_Create();
	scope = ParseScope_CreateRoot();

	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"), PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"), PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gc"), PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "gd"), PARSEDECL_GLOBAL, NULL, NULL);
	ParseScope_Declare(scope, SymbolTable_GetSymbolC(Smile_SymbolTable, "ge"), PARSEDECL_GLOBAL, NULL, NULL);

	expr = Parser_Parse(parser, lexer, scope);

	if (SMILE_KIND(parser->firstMessage) == SMILE_KIND_NULL) {
		String stringified = SmileObject_Stringify((SmileObject)expr);
		return expr;
	}
	else {
		ParseError error = (ParseMessage)(parser->firstMessage->a);
		String errorMessage = String_Format("%S: line %d: %S",
			error->position != NULL ? error->position->filename : String_Empty,
			error->position != NULL ? error->position->line : 0,
			error->message
		);
		FAIL_TEST(String_ToC(errorMessage));
		return NullObject;
	}
}

START_TEST(CanCompileNull)
{
	SmileObject expr = Parse("[]");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	const char *expectedResult =
		"\tLdNull\n";

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileByte)
{
	SmileObject expr = Parse("123x");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	const char *expectedResult =
		"\tLd8 123\n";

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt16)
{
	SmileObject expr = Parse("123s");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	const char *expectedResult =
		"\tLd16 123\n";

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt32)
{
	SmileObject expr = Parse("123t");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	const char *expectedResult =
		"\tLd32 123\n";

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt64)
{
	SmileObject expr = Parse("123");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	const char *expectedResult =
		"\tLd64 123\n";

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileBasicArithmetic)
{
	SmileObject expr = Parse("123 + 456");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 123\n"
		"\tLd64 456\n"
		"\tBinary %d\t; +\n",
		Smile_KnownSymbols.plus
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileMildlyInterestingArithmetic)
{
	SmileObject expr = Parse("(123 + -456) * 50");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 123\n"
		"\tLd64 456\n"
		"\tUnary %d\t; -\n"
		"\tBinary %d\t; +\n"
		"\tLd64 50\n"
		"\tBinary %d\t; *\n",
		Smile_KnownSymbols.minus,
		Smile_KnownSymbols.plus,
		Smile_KnownSymbols.star
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
// Read/write of global variables, properties, and members.

START_TEST(CanCompileGlobalReadsAndWrites)
{
	SmileObject expr = Parse("ga = gb");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLdX %d\t; gb\n"
		"\tStX %d\t; ga\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileReadsFromProperties)
{
	SmileObject expr = Parse("ga = gb.foo");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLdX %d\t; gb\n"
		"\tLdProp %d\t; foo\n"
		"\tStX %d\t; ga\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "foo"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileWritesToProperties)
{
	SmileObject expr = Parse("ga.foo = gb");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLdX %d\t; ga\n"
		"\tLdX %d\t; gb\n"
		"\tStProp %d\t; foo\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "foo")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileReadsFromMembers)
{
	SmileObject expr = Parse("ga = gb:10");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLdX %d\t; gb\n"
		"\tLd64 10\n"
		"\tLdMember\n"
		"\tStX %d\t; ga\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileWritesToMembers)
{
	SmileObject expr = Parse("ga:10 = gb");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLdX %d\t; ga\n"
		"\tLd64 10\n"
		"\tLdX %d\t; gb\n"
		"\tStMember\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "ga"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------
// Read/write of global variables, properties, and members.

START_TEST(CanCompileScopeVariableReads)
{
	SmileObject expr = Parse("{ a = gb }");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLdX %d\t; gb\n"
		"\tStLoc0 0\t; a\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, "gb"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "a")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->localSize == 1);
}
END_TEST

START_TEST(CanCompileNestedScopeVariableReads)
{
	SmileObject expr = Parse("{ var b = 10 { var a = b, c = a + b } }");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 10\n"
		"\tStpLoc0 0\t; b\n"
		"\tLdLoc0 0\t; b\n"
		"\tStpLoc0 1\t; a\n"
		"\tLdLoc0 1\t; a\n"
		"\tLdLoc0 0\t; b\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 2\t; c\n",
		Smile_KnownSymbols.plus
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->localSize == 3);
}
END_TEST

START_TEST(NestedScopesVariablesDontOverlap)
{
	SmileObject expr = Parse("{ var b = 10 { var a = b, c = a + b } { var d = b * 20 } }");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 10\n"
		"\tStpLoc0 0\t; b\n"
		"\tLdLoc0 0\t; b\n"
		"\tStpLoc0 1\t; a\n"
		"\tLdLoc0 1\t; a\n"
		"\tLdLoc0 0\t; b\n"
		"\tBinary %d\t; +\n"
		"\tStpLoc0 2\t; c\n"
		"\tLdLoc0 0\t; b\n"
		"\tLd64 20\n"
		"\tBinary %d\t; *\n"
		"\tStLoc0 3\t; d\n",
		Smile_KnownSymbols.plus,
		Smile_KnownSymbols.star
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));

	ASSERT(globalFunction->localSize == 4);
}
END_TEST

//-------------------------------------------------------------------------------------------------

START_TEST(CanCompileSimpleConditionals)
{
	SmileObject expr = Parse("[$if 1 < 10 `then-side `else-side]");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 1\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBf >L6\n"
		"\tLdSym %d\t; then-side\n"
		"\tJmp >L8\n"
		"L6:\n"
		"\tLdSym %d\t; else-side\n"
		"L8:\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "then-side"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "else-side")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

START_TEST(CanCompileConditionalsAllTheWay)
{
	SmileObject expr = Parse(
		"#syntax STMT: [if [EXPR x] then [STMT y]] => [$if x y]\n"
		"#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [$if x y z]\n"
		"\n"
		"if 1 < 10 then\n"
		"\t`then-side\n"
		"else\n"
		"\t`else-side\n"
	);

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 1\n"
		"\tLd64 10\n"
		"\tBinary %d\t; <\n"
		"\tBf >L6\n"
		"\tLdSym %d\t; then-side\n"
		"\tJmp >L8\n"
		"L6:\n"
		"\tLdSym %d\t; else-side\n"
		"L8:\n",
		Smile_KnownSymbols.lt,
		SymbolTable_GetSymbolC(Smile_SymbolTable, "then-side"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "else-side")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

//-------------------------------------------------------------------------------------------------

START_TEST(CanCompileAWhileLoopThatComputesLogarithms)
{
	SmileObject expr = Parse(
		"#syntax STMT: [while [EXPR x] do [STMT y]] => [$while [] x y]\n"
		"\n"
		"n = 12345678\n"
		"log = 0\n"
		"while n do {\n"
		"\tn >>>= 1\n"
		"\tlog += 1\n"
		"}\n"
	);

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr);
	String result;

	String expectedResult = String_Format(
		"\tLd64 12345678\n"
		"\tStpLoc0 0\t; n\n"
		"\tLd64 0\n"
		"\tStpLoc0 1\t; log\n"
		"\tLdNull\n"
		"\tJmp >L16\n"
		"L6:\n"
		"\tPop1\n"
		"\tLdLoc0 0\t; n\n"
		"\tLd64 1\n"
		"\tBinary %d\t; >>>\n"
		"\tStpLoc0 0\t; n\n"
		"\tLdLoc0 1\t; log\n"
		"\tLd64 1\n"
		"\tBinary %d\t; +\n"
		"\tStLoc0 1\t; log\n"
		"L16:\n"
		"\tLdLoc0 0\t; n\n"
		"\tBt L6\n",
		SymbolTable_GetSymbolC(Smile_SymbolTable, ">>>"),
		SymbolTable_GetSymbolC(Smile_SymbolTable, "+")
	);

	Compiler_EndFunction(compiler);

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, globalFunction, compiler->compiledTables);

	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

#include "compiler_tests.generated.inc"
