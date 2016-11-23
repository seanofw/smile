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

static SmileList Parse(const char *text)
{
	String source;
	Lexer lexer;
	Parser parser;
	ParseScope scope;
	SmileList expr;

	Smile_ResetEnvironment();

	source = String_FromC(text);

	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	parser = Parser_Create();
	scope = ParseScope_CreateRoot();
	expr = Parser_Parse(parser, lexer, scope);

	return SMILE_KIND(parser->firstMessage) == SMILE_KIND_NULL ? expr : NullList;
}

START_TEST(CanCompileNull)
{
	SmileList expr = Parse("[]");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr->a);
	String result;

	const char *expectedResult =
		"\tLdNull\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileByte)
{
	SmileList expr = Parse("123x");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr->a);
	String result;

	const char *expectedResult =
		"\tLd8 123\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt16)
{
	SmileList expr = Parse("123h");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr->a);
	String result;

	const char *expectedResult =
		"\tLd16 123\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt32)
{
	SmileList expr = Parse("123");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr->a);
	String result;

	const char *expectedResult =
		"\tLd32 123\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileInt64)
{
	SmileList expr = Parse("123L");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr->a);
	String result;

	const char *expectedResult =
		"\tLd64 123\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileBasicArithmetic)
{
	SmileList expr = Parse("123 + 456");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr->a);
	String result;

	const char *expectedResult =
		"\tLd32 123\n"
		"\tLd32 456\n"
		"\tBinary `+\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileMildlyInterestingArithmetic)
{
	SmileList expr = Parse("(123 + -456) * 50");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr->a);
	String result;

	const char *expectedResult =
		"\tLd32 123\n"
		"\tLd32 456\n"
		"\tUnary `-\n"
		"\tBinary `+\n"
		"\tLd32 50\n"
		"\tBinary `*\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileSimpleConditionals)
{
	SmileList expr = Parse("[$if 1 < 10 `then-side `else-side]");

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	Int offset = Compiler_CompileExpr(compiler, expr->a);
	String result;

	const char *expectedResult =
		"\tLd32 1\n"
		"\tLd32 10\n"
		"\tBinary `<\n"
		"\tBf >L6\n"
		"\tLdSym `then-side\n"
		"\tJmp >L8\n"
		"L6:\n"
		"\tLdSym `else-side\n"
		"L8:\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanCompileConditionalsAllTheWay)
{
	SmileList exprs = Parse(
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
	Int offset = Compiler_CompileExprs(compiler, exprs);
	String result;

	const char *expectedResult =
		"\tLdObj @0\n"
		"\tPop1\n"
		"\tLdObj @1\n"
		"\tPop1\n"
		"\tLd32 1\n"
		"\tLd32 10\n"
		"\tBinary `<\n"
		"\tBf >L10\n"
		"\tLdSym `then-side\n"
		"\tJmp >L12\n"
		"L10:\n"
		"\tLdSym `else-side\n"
		"L12:\n";

	result = ByteCodeSegment_ToString(globalFunction->byteCodeSegment, compiler->compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

#include "compiler_tests.generated.inc"
