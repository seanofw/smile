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

#include "../stdafx.h"

#include <smile/env/env.h>
#include <smile/eval/bytecode.h>
#include <smile/eval/opcode.h>
#include <smile/eval/compiler.h>
#include <smile/eval/eval.h>
#include <smile/parsing/parser.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilepair.h>

STATIC_STRING(TestFilename, "test.sm");

TEST_SUITE(EvalTests)

static CompiledTables Compile(const char *text)
{
	String source;
	Lexer lexer;
	Parser parser;
	ParseScope globalScope;
	SmileObject expr;
	Compiler compiler;
	ClosureInfo globalClosureInfo;
	UserFunctionInfo globalFunction;

	Smile_ResetEnvironment();

	source = String_FromC(text);

	lexer = Lexer_Create(source, 0, String_Length(source), TestFilename, 1, 1);
	lexer->symbolTable = Smile_SymbolTable;

	globalClosureInfo = ClosureInfo_Create(NULL, CLOSURE_KIND_GLOBAL);
	Smile_InitCommonGlobals(globalClosureInfo);

	parser = Parser_Create();
	globalScope = ParseScope_CreateRoot();
	ParseScope_DeclareVariablesFromClosureInfo(globalScope, globalClosureInfo);
	expr = Parser_Parse(parser, lexer, globalScope);

	compiler = Compiler_Create();
	Compiler_SetGlobalClosureInfo(compiler, globalClosureInfo);
	globalFunction = Compiler_CompileGlobal(compiler, expr);

	return compiler->compiledTables;
}

START_TEST(CanEvalAConstantInteger)
{
	CompiledTables compiledTables = Compile("1");

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 1);
}
END_TEST

START_TEST(CanEvalAConstantSymbol)
{
	CompiledTables compiledTables = Compile("`a");

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_SYMBOL);
	ASSERT(((SmileSymbol)result->value)->symbol == Smile_KnownSymbols.a);
}
END_TEST

START_TEST(CanEvalLocalVariableAssignments)
{
	CompiledTables compiledTables = Compile(
		"x = `a\n"
		"y = `b\n"
		"x"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_SYMBOL);
	ASSERT(((SmileSymbol)result->value)->symbol == Smile_KnownSymbols.a);
}
END_TEST

START_TEST(CanEvalIfThenElse)
{
	CompiledTables compiledTables = Compile(
		"#syntax STMT: [if [EXPR x] then [STMT y]] => [$if x y]\n"
		"#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [$if x y z]\n"
		"x = 1\n"
		"if x then y = 123\n"
		"else y = 456\n"
		"y\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 123);
}
END_TEST

START_TEST(CanEvalBinaryMethodCalls)
{
	CompiledTables compiledTables = Compile(
		"x = 1 + 2\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 3);
}
END_TEST

START_TEST(CanEvalComplexPilesOfBinaryAndUnaryMethodCalls)
{
	CompiledTables compiledTables = Compile(
		"x = (-3 + 2 * 5) * 7\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 49);
}
END_TEST

START_TEST(CanEvalSmileCodeThatComputesALogarithm)
{
	CompiledTables compiledTables = Compile(
		"#syntax STMT: [while [EXPR x] do [STMT y]] => [$while [] x y]\n"
		"\n"
		"n = 12345678\n"
		"log = 0\n"
		"while n do {\n"
		"\tn >>>= 1\n"
		"\tlog += 1\n"
		"}\n"
		"log\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 24);
}
END_TEST

START_TEST(CanEvalSmileCodeThatConvertsBetweenTypes)
{
	CompiledTables compiledTables = Compile(
		"str = \"1234\"\n"
		"n = 0 parse str\n"
		"m = n + 0 parse \"1111\"\n"
		"result = string m\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT_STRING(SmileString_GetString((SmileString)result->value), "2345", 4);
}
END_TEST

START_TEST(CanEvalDirectCallsToNativeFunctions)
{
	CompiledTables compiledTables = Compile(
		"n = 12345\n"
		"m = 11111\n"
		"f = Integer64.+\n"
		"sum = [f n m]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 23456);
}
END_TEST

START_TEST(CanEvalCallsToUserFunctions)
{
	CompiledTables compiledTables = Compile(
		"f = |x| x + 111\n"
		"n = 123\n"
		"m = [f n]\n"
	);

	String global = ByteCodeSegment_ToString(compiledTables->globalFunctionInfo->byteCodeSegment, compiledTables->globalFunctionInfo, compiledTables);
	String f = ByteCodeSegment_ToString(compiledTables->userFunctions[0]->byteCodeSegment, compiledTables->userFunctions[0], compiledTables);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 234);
}
END_TEST

START_TEST(CanEvalRecursiveCallsToUserFunctions)
{
	CompiledTables compiledTables = Compile(
		"#syntax STMT: [if [EXPR x] then [STMT y]] => [$if x y]\n"
		"#syntax STMT: [if [EXPR x] then [STMT y] else [STMT z]] => [$if x y z]\n"
		"\n"
		"factorial = |x|\n"
		"\tif x <= 1 then x\n"
		"\telse x * [factorial x - 1]\n"
		"\n"
		"n = [factorial 10]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 3628800);
}
END_TEST

START_TEST(UserFunctionsCanInfluenceTheirParentScope)
{
	CompiledTables compiledTables = Compile(
		"var x, y\n"
		"x = 10\n"
		"f = |z| y = x + z\n"
		"x = 5\n"
		"[f 30]\n"
		"y\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 35);
}
END_TEST

START_TEST(UserFunctionsCanHaveZeroParameters)
{
	CompiledTables compiledTables = Compile(
		"var x, y\n"
		"x = 10\n"
		"f = || x + 100\n"
		"x = 5\n"
		"[f]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 105);
}
END_TEST

START_TEST(UserFunctionsCanHaveTwoParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b| a + b\n"
		"[f 10 20]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 30);
}
END_TEST

START_TEST(UserFunctionsCanHaveThreeParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b c| a + b * c\n"
		"[f 10 20 30]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 610);
}
END_TEST

START_TEST(UserFunctionsCanHaveFourParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b c d| a * b + c * d\n"
		"[f 10 20 30 40]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 1400);
}
END_TEST

START_TEST(UserFunctionsCanHaveFiveParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b c d e| a * b + c * d + e\n"
		"[f 10 20 30 40 50]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 1450);
}
END_TEST

START_TEST(UserFunctionsCanHaveSixParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b c d e f| a * b + c * d + e * f\n"
		"[f 10 20 30 40 50 60]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 4400);
}
END_TEST

START_TEST(UserFunctionsCanHaveSevenParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b c d e f g| a * b + c * d + e * f + g\n"
		"[f 10 20 30 40 50 60 70]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 4470);
}
END_TEST

START_TEST(UserFunctionsCanHaveEightParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b c d e f g h| a * b + c * d + e * f + g * h\n"
		"[f 10 20 30 40 50 60 70 80]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 10000);
}
END_TEST

START_TEST(UserFunctionsCanHaveNineParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b c d e f g h i| a * b + c * d + e * f + g * h + i\n"
		"[f 10 20 30 40 50 60 70 80 90]\n"
	);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 10090);
}
END_TEST

START_TEST(UserFunctionsCanHaveTenParameters)
{
	CompiledTables compiledTables = Compile(
		"f = |a b c d e f g h i j| a * b + c * d + e * f + g * h + i * j\n"
		"[f 10 20 30 40 50 60 70 80 90 100]\n"
	);

	String global = ByteCodeSegment_ToString(compiledTables->globalFunctionInfo->byteCodeSegment, compiledTables->globalFunctionInfo, compiledTables);
	String f = ByteCodeSegment_ToString(compiledTables->userFunctions[0]->byteCodeSegment, compiledTables->userFunctions[0], compiledTables);

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 19000);
}
END_TEST

#include "eval_tests.generated.inc"
