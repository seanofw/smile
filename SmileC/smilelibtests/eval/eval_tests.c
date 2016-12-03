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

TEST_SUITE(EvalTests)

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

static Compiler Compile(const char *text)
{
	SmileList exprs = Parse(text);

	Compiler compiler = Compiler_Create();
	CompiledFunction globalFunction = Compiler_CompileGlobal(compiler, exprs);
	
	return compiler;
}

START_TEST(CanEvalAConstant)
{
	Compiler compiler = Compile("1");

	String string = ByteCodeSegment_ToString(compiler->compiledTables->compiledFunctions[0]->byteCodeSegment, NULL, compiler->compiledTables);

}
END_TEST

#include "eval_tests.generated.inc"
