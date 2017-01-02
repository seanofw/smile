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
#include <smile/eval/eval.h>
#include <smile/parsing/parser.h>
#include <smile/smiletypes/text/smilestring.h>
#include <smile/smiletypes/numeric/smileinteger32.h>
#include <smile/smiletypes/smilepair.h>

STATIC_STRING(TestFilename, "test.sm");

TEST_SUITE(EvalTests)

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
	expr = Parser_Parse(parser, lexer, scope);

	return SMILE_KIND(parser->firstMessage) == SMILE_KIND_NULL ? expr : NullObject;
}

static CompiledTables Compile(const char *text)
{
	SmileObject expr;
	Compiler compiler;
	ClosureInfo globalClosureInfo;
	CompiledFunction globalFunction;

	expr = Parse(text);

	compiler = Compiler_Create();
	globalClosureInfo = ClosureInfo_Create(NULL, CLOSURE_KIND_GLOBAL);
	Compiler_SetGlobalClosureInfo(compiler, globalClosureInfo);
	globalFunction = Compiler_CompileGlobal(compiler, expr);

	return compiler->compiledTables;
}

START_TEST(CanEvalAConstantInteger)
{
	CompiledTables compiledTables = Compile("1");

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunction);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER32);
	ASSERT(((SmileInteger32)result->value)->value == 1);
}
END_TEST

START_TEST(CanEvalAConstantSymbol)
{
	CompiledTables compiledTables = Compile("`a");

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunction);

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

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunction);

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

	EvalResult result = Eval_Run(compiledTables, compiledTables->globalFunction);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER32);
	ASSERT(((SmileInteger32)result->value)->value == 123);
}

END_TEST

#include "eval_tests.generated.inc"
