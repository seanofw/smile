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
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilebool.h>

TEST_SUITE(EvalTests)

static UserFunctionInfo Compile(const char *text)
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

	lexer = Lexer_Create(source, 0, String_Length(source), GetTestScriptName(), 1, 1);
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

	return globalFunction;
}

START_TEST(CanEvalAConstantInteger)
{
	UserFunctionInfo globalFunctionInfo = Compile("1");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 1);
}
END_TEST

START_TEST(CanEvalAConstantSymbol)
{
	UserFunctionInfo globalFunctionInfo = Compile("`a");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_SYMBOL);
	ASSERT(((SmileSymbol)result->value)->symbol == Smile_KnownSymbols.a);
}
END_TEST

START_TEST(CanEvalLocalVariableAssignments)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"x = `a\n"
		"y = `b\n"
		"x"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_SYMBOL);
	ASSERT(((SmileSymbol)result->value)->symbol == Smile_KnownSymbols.a);
}
END_TEST

START_TEST(CanEvalIfThenElse)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => [$if (x) (y)]\n"
		"#syntax STMT: [my-if [EXPR x] then [STMT y] else [STMT z]] => [$if (x) (y) (z)]\n"
		"x = 1\n"
		"my-if x then y = 123\n"
		"else y = 456\n"
		"y\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 123);
}
END_TEST

START_TEST(CanEvalBinaryMethodCalls)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"x = 1 + 2\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 3);
}
END_TEST

START_TEST(CanEvalComplexPilesOfBinaryAndUnaryMethodCalls)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"x = (-3 + 2 * 5) * 7\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 49);
}
END_TEST

START_TEST(CanEvalSmileCodeThatComputesALogarithm)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"#syntax STMT: [my-while [EXPR x] do [STMT y]] => [$while [] (x) (y)]\n"
		"\n"
		"n = 12345678\n"
		"log = 0\n"
		"my-while n do {\n"
		"\tn >>>= 1\n"
		"\tlog += 1\n"
		"}\n"
		"log\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 24);
}
END_TEST

START_TEST(CanEvalATillLoopThatComputesAnExponent)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => [$if (x) (y)]\n"
		"\n"
		"var x = 1\n"
		"[$till [reached-eight-bits] {\n"
		"\tmy-if x > 0xFF then reached-eight-bits\n"
		"\tx <<= 1\n"
		"}]\n"
		"x\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 256);
}
END_TEST

START_TEST(CanEvalSmileCodeThatConvertsBetweenTypes)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"str = \"1234\"\n"
		"n = 0 parse str\n"
		"m = n + 0 parse \"1111\"\n"
		"result = string m\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT_STRING((String)result->value, "2345", 4);
}
END_TEST

START_TEST(CanEvalDirectCallsToNativeFunctions)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"n = 12345\n"
		"m = 11111\n"
		"f = Integer64.+\n"
		"sum = [f n m]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 23456);
}
END_TEST

START_TEST(CanEvalCallsToUserFunctions)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |x| x + 111\n"
		"n = 123\n"
		"m = [f n]\n"
	);

	String global = UserFunctionInfo_ToString(globalFunctionInfo);
	String f = UserFunctionInfo_ToString(globalFunctionInfo->byteCodeSegment->compiledTables->userFunctions[0]);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 234);
}
END_TEST

START_TEST(CanEvalRecursiveCallsToUserFunctions)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"#syntax STMT: [my-if [EXPR x] then [STMT y]] => [$if (x) (y)]\n"
		"#syntax STMT: [my-if [EXPR x] then [STMT y] else [STMT z]] => [$if (x) (y) (z)]\n"
		"\n"
		"factorial = |x|\n"
		"\tmy-if x <= 1 then x\n"
		"\telse x * [factorial x - 1]\n"
		"\n"
		"n = [factorial 10]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 3628800);
}
END_TEST

START_TEST(UserFunctionsCanInfluenceTheirParentScope)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"var x, y\n"
		"x = 10\n"
		"f = |z| y = x + z\n"
		"x = 5\n"
		"[f 30]\n"
		"y\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 35);
}
END_TEST

START_TEST(UserFunctionsCanHaveZeroParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"var x, y\n"
		"x = 10\n"
		"f = || x + 100\n"
		"x = 5\n"
		"[f]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 105);
}
END_TEST

START_TEST(UserFunctionsCanHaveTwoParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b| a + b\n"
		"[f 10 20]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 30);
}
END_TEST

START_TEST(UserFunctionsCanHaveThreeParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c| a + b * c\n"
		"[f 10 20 30]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 610);
}
END_TEST

START_TEST(UserFunctionsCanHaveFourParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c d| a * b + c * d\n"
		"[f 10 20 30 40]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 1400);
}
END_TEST

START_TEST(UserFunctionsCanHaveFiveParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c d e| a * b + c * d + e\n"
		"[f 10 20 30 40 50]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 1450);
}
END_TEST

START_TEST(UserFunctionsCanHaveSixParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c d e f| a * b + c * d + e * f\n"
		"[f 10 20 30 40 50 60]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 4400);
}
END_TEST

START_TEST(UserFunctionsCanHaveSevenParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c d e f g| a * b + c * d + e * f + g\n"
		"[f 10 20 30 40 50 60 70]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 4470);
}
END_TEST

START_TEST(UserFunctionsCanHaveEightParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c d e f g h| a * b + c * d + e * f + g * h\n"
		"[f 10 20 30 40 50 60 70 80]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 10000);
}
END_TEST

START_TEST(UserFunctionsCanHaveNineParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c d e f g h i| a * b + c * d + e * f + g * h + i\n"
		"[f 10 20 30 40 50 60 70 80 90]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 10090);
}
END_TEST

START_TEST(UserFunctionsCanHaveTenParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c d e f g h i j| a * b + c * d + e * f + g * h + i * j\n"
		"[f 10 20 30 40 50 60 70 80 90 100]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 19000);
}
END_TEST

START_TEST(UserFunctionsCanHaveRestParameters)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"f = |a b c rest...| rest join \" \"\n"
		"[f 10 20 30 40 50 60]\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT_STRING((String)result->value, "40 50 60", 8);
}
END_TEST

START_TEST(CanUseStateMachinesToIterateLists)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"y = 0\n"
		"`[1 2 3 4 5 6 7 8 9 10] each |x| y += x * x\n"
		"y\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 385);
}
END_TEST

START_TEST(CanUseStateMachinesToProjectLists)
{
	static Int64 expectedResult[] = { 1, 4, 9, 16, 25, 36, 49, 64, 81, 100 };
	Int i;
	SmileList list;
	
	UserFunctionInfo globalFunctionInfo = Compile(
		"`[1 2 3 4 5 6 7 8 9 10] map |x| x * x\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_LIST);

	for (i = 0, list = (SmileList)result->value; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list), i++) {
		ASSERT(SMILE_KIND(list->a) == SMILE_KIND_INTEGER64);
		ASSERT(((SmileInteger64)list->a)->value == expectedResult[i]);
	}
}
END_TEST

START_TEST(MapReturnsNullForAnEmptyList)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"`[] map |x| x + 1\n"
		);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(CanUseStateMachinesToFilterLists)
{
	static Int64 expectedResult[] = { 1, 2, 4, 5, 7, 8, 10 };
	Int i;
	SmileList list;

	UserFunctionInfo globalFunctionInfo = Compile(
		"`[1 2 3 4 5 6 7 8 9 10] where |x| x mod 3 != 0\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_LIST);

	for (i = 0, list = (SmileList)result->value; SMILE_KIND(list) == SMILE_KIND_LIST; list = LIST_REST(list), i++) {
		ASSERT(SMILE_KIND(list->a) == SMILE_KIND_INTEGER64);
		ASSERT(((SmileInteger64)list->a)->value == expectedResult[i]);
	}
}
END_TEST

START_TEST(WhereReturnsNullForAnEmptyList)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"`[] where |x| x mod 3 != 0\n"
	);

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(CanUseStateMachinesToTestAny)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] any? 15\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);

	globalFunctionInfo = Compile("`[1 2 4 8 15 16 32 64 128] any? 15\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);
}
END_TEST

START_TEST(CanUseStateMachinesToTestAnyWithPredicates)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] any? |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);

	globalFunctionInfo = Compile("`[1 2 4 8 15 16 32 64 128] any? |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);
}
END_TEST

START_TEST(AnyHasWeirdCornerCasesForEmptyLists)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[] any? |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);
}
END_TEST

START_TEST(AnyInUnaryFormAnswersWhetherTheListIsEmpty)
{
	UserFunctionInfo globalFunctionInfo = Compile("any? `[]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);

	globalFunctionInfo = Compile("any? `[1 2 3]\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);
}
END_TEST

START_TEST(EmptyAnswersWhetherTheListIsEmpty)
{
	UserFunctionInfo globalFunctionInfo = Compile("empty? `[]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);

	globalFunctionInfo = Compile("empty? `[1 2 3]\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);
}
END_TEST

START_TEST(NullAnswersWhetherTheListIsEmpty)
{
	UserFunctionInfo globalFunctionInfo = Compile("null? `[]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);

	globalFunctionInfo = Compile("null? `[1 2 3]\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);
}
END_TEST

START_TEST(CanUseStateMachinesToTestContains)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] contains? 15\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);

	globalFunctionInfo = Compile("`[1 2 4 8 15 16 32 64 128] contains? 15\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);
}
END_TEST

START_TEST(CanUseStateMachinesToTestContainsWithPredicates)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] contains? |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);

	globalFunctionInfo = Compile("`[1 2 4 8 15 16 32 64 128] contains? |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);
}
END_TEST

START_TEST(ContainsHasWeirdCornerCasesForEmptyLists)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[] contains? |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);
}
END_TEST

START_TEST(CanUseStateMachinesToTestAll)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] all? 15\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);

	globalFunctionInfo = Compile("`[15 15 15 15 15 15] all? 15\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);
}
END_TEST

START_TEST(CanUseStateMachinesToTestAllWithPredicates)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[3 6 9 10 11 12 13 14 15] all? |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.FalseObj);

	globalFunctionInfo = Compile("`[3 6 9 12 15 18 21] all? |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);
}
END_TEST

START_TEST(AllHasWeirdCornerCasesForEmptyLists)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[] all? |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.TrueObj);
}
END_TEST

START_TEST(CanUseStateMachinesToFindFirst)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] first 15\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);

	globalFunctionInfo = Compile("`[1 2 4 8 15 16 32 64 128] first 15\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 15);
}
END_TEST

START_TEST(CanUseStateMachinesToFindFirstWithPredicates)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] first |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);

	globalFunctionInfo = Compile("`[1 2 4 8 15 16 32 64 128] first |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 15);
}
END_TEST

START_TEST(FirstReturnsNullForEmptyLists)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[] first 15\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);

	globalFunctionInfo = Compile("`[] first |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);
}
END_TEST

START_TEST(FirstInUnaryFormReturnsTheFirstItem)
{
	UserFunctionInfo globalFunctionInfo = Compile("first `[]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);

	globalFunctionInfo = Compile("first `[8 16 32 64 128]\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 8);
}
END_TEST

START_TEST(CanUseStateMachinesToFindIndexOf)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] index-of 15\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);

	globalFunctionInfo = Compile("`[1 2 4 8 15 16 32 64 128] index-of 15\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 4);
}
END_TEST

START_TEST(CanUseStateMachinesToFindIndexOfWithPredicates)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] index-of |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);

	globalFunctionInfo = Compile("`[1 2 4 8 15 16 32 64 128] index-of |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 4);
}
END_TEST

START_TEST(IndexOfReturnsNullForEmptyLists)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[] index-of 15\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);

	globalFunctionInfo = Compile("`[] index-of |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == NullObject);
}
END_TEST

START_TEST(CanUseStateMachinesToCountItems)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 3 1 2 3 3 9 5 1 3] count 3\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 4);
}
END_TEST

START_TEST(CanUseStateMachinesToCountWithPredicates)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[1 2 4 8 16 32 64 128] count |x| x mod 3 == 0\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.ZeroInt64);

	globalFunctionInfo = Compile("`[1 2 3 4 5 8 15 16 32 60 64 90 93 128] count |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 5);
}
END_TEST

START_TEST(CountReturnsZeroForEmptyLists)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[] count 15\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.ZeroInt64);

	globalFunctionInfo = Compile("`[] count |x| x mod 3 == 0\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.ZeroInt64);
}
END_TEST

START_TEST(CountInUnaryFormCountsAllItems)
{
	UserFunctionInfo globalFunctionInfo = Compile("count `[]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(result->value == (SmileObject)Smile_KnownObjects.ZeroInt64);

	globalFunctionInfo = Compile("count `[9 8 7 6 5 4 3]\n");
	result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 7);
}
END_TEST

START_TEST(CanConcatenateStrings)
{
	UserFunctionInfo globalFunctionInfo = Compile("\"foo\" + \"bar\"\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT_STRING((String)result->value, "foobar", 6);
}
END_TEST

START_TEST(CanConcatenateManyStrings)
{
	UserFunctionInfo globalFunctionInfo = Compile("\"You\" + \" say\" + \" goodbye,\" + \" and\" + \" I\" + \" say\" + \" hello.\"\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT_STRING((String)result->value, "You say goodbye, and I say hello.", 33);
}
END_TEST

START_TEST(CanConcatenateManyStringsMoreEfficiently1)
{
	UserFunctionInfo globalFunctionInfo = Compile("[\"You\".+ \" say\" \" goodbye,\" \" and\" \" I\" \" say\" \" hello.\"]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT_STRING((String)result->value, "You say goodbye, and I say hello.", 33);
}
END_TEST

START_TEST(CanConcatenateManyStringsMoreEfficiently2)
{
	UserFunctionInfo globalFunctionInfo = Compile("[String.+ \"You\" \" say\" \" goodbye,\" \" and\" \" I\" \" say\" \" hello.\"]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT_STRING((String)result->value, "You say goodbye, and I say hello.", 33);
}
END_TEST

START_TEST(CanConcatenateManyStringsMoreEfficiently3)
{
	UserFunctionInfo globalFunctionInfo = Compile("concat = String.+\n"
		"[concat \"You\" \" say\" \" goodbye,\" \" and\" \" I\" \" say\" \" hello.\"]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT_STRING((String)result->value, "You say goodbye, and I say hello.", 33);
}
END_TEST

START_TEST(CanCallFunctionsWhileFillingInOptionalArguments)
{
	UserFunctionInfo globalFunctionInfo = Compile("f = |x y=3| x + y\n"
		"[f 2 5]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 7);
}
END_TEST

START_TEST(CanCallFunctionsWithoutFillingInOptionalArguments)
{
	UserFunctionInfo globalFunctionInfo = Compile("f = |x y=3| x + y\n"
		"[f 2]\n");
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 5);
}
END_TEST

START_TEST(CanEvalATillLoopThatEscapesANestedFunction)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"var list = `[1 2 3 4 5]\n"
		"till found-even do\n"
		"\tlist each |x|\n"
		"\t\tif even? x then found-even\n"
	);
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(CanEvalATillLoopThatEscapesANestedFunctionForTheRightReason)
{
	UserFunctionInfo globalFunctionInfo = Compile(
		"var list = `[1 2 3 4 5]\n"
		"var value = 0\n"
		"till found-even, not-found do {\n"
		"\tlist each |x| {\n"
		"\t\tif x > 3 and even? x then {\n"
		"\t\t\tvalue = x\n"
		"\t\t\tfound-even\n"
		"\t\t}\n"
		"\t}\n"
		"\tnot-found\n"
		"}\n"
		"when found-even { value }\n"
		"when not-found { -1 }\n"
	);
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 4);
}
END_TEST

START_TEST(CanEvalATillLoopThatEscapesANestedFunctionForTheRightReason2)
{
	// This code is exactly the same as that of the previous test, but with 'x > 3'
	// replaced with 'x > 5', so that the target object cannot be found.
	UserFunctionInfo globalFunctionInfo = Compile(
		"var list = `[1 2 3 4 5]\n"
		"var value = 0\n"
		"till found-even, not-found do {\n"
		"\tlist each |x| {\n"
		"\t\tif x > 5 and even? x then {\n"
		"\t\t\tvalue = x\n"
		"\t\t\tfound-even\n"
		"\t\t}\n"
		"\t}\n"
		"\tnot-found\n"
		"}\n"
		"when found-even { value }\n"
		"when not-found { -1 }\n"
	);
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == -1);
}
END_TEST

#include "eval_tests.generated.inc"
