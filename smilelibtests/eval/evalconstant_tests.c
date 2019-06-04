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

#include "../stdafx.h"

#include <smile/env/env.h>
#include <smile/eval/bytecode.h>
#include <smile/eval/opcode.h>
#include <smile/eval/compiler.h>
#include <smile/eval/eval.h>
#include <smile/parsing/parser.h>

#include <smile/smiletypes/numeric/smilebyte.h>
#include <smile/smiletypes/numeric/smileinteger16.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/numeric/smileinteger64.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/smilebool.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/string.h>

TEST_SUITE(EvalConstantTests)

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
	globalFunction = Compiler_CompileGlobalExpressionInGlobalScope(compiler, expr);

	return globalFunction;
}

//-----------------------------------------------------------------------------
// Constants.

START_TEST(CanEvalAConstantInteger64)
{
	UserFunctionInfo globalFunctionInfo = Compile("31415926535");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 31415926535LL);
}
END_TEST

START_TEST(CanEvalAConstantInteger32)
{
	UserFunctionInfo globalFunctionInfo = Compile("314159265t");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER32);
	ASSERT(((SmileInteger32)result->value)->value == 314159265);
}
END_TEST

START_TEST(CanEvalAConstantInteger16)
{
	UserFunctionInfo globalFunctionInfo = Compile("31415s");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER16);
	ASSERT(((SmileInteger16)result->value)->value == 31415);
}
END_TEST

START_TEST(CanEvalAConstantByte)
{
	UserFunctionInfo globalFunctionInfo = Compile("31x");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_BYTE);
	ASSERT(((SmileByte)result->value)->value == 31);
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

START_TEST(CanEvalAConstantString)
{
	UserFunctionInfo globalFunctionInfo = Compile("\"Hello World\"");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_STRING);
	ASSERT(String_EqualsC(((String)result->value), "Hello World"));
}
END_TEST

START_TEST(CanEvalAConstantFalse)
{
	UserFunctionInfo globalFunctionInfo = Compile("false");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_BOOL);
	ASSERT(((SmileBool)result->value)->value == False);
}
END_TEST

START_TEST(CanEvalAConstantTrue)
{
	UserFunctionInfo globalFunctionInfo = Compile("true");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_BOOL);
	ASSERT(((SmileBool)result->value)->value == True);
}
END_TEST

START_TEST(CanEvalAConstantNull)
{
	UserFunctionInfo globalFunctionInfo = Compile("null");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(CanEvalAConstantEmptyListAsNull)
{
	UserFunctionInfo globalFunctionInfo = Compile("[]");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_NULL);
}
END_TEST

START_TEST(AQuotedEmptyListIsAlsoNull)
{
	UserFunctionInfo globalFunctionInfo = Compile("`[]");

	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_NULL);
}
END_TEST

#include "evalconstant_tests.generated.inc"
