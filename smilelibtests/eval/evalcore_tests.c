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

TEST_SUITE(EvalCoreTests)

static UserFunctionInfo CreateRawGlobalCode(const ByteCode byteCodes, Int numByteCodes, Int stackSize, Int localSize)
{
	Compiler compiler;
	ClosureInfo globalClosureInfo, closureInfo;
	UserFunctionInfo userFunctionInfo;
	CompilerFunction compilerFunction;
	String errorMessage;

	Smile_ResetEnvironment();

	globalClosureInfo = ClosureInfo_Create(NULL, CLOSURE_KIND_GLOBAL);
	Smile_InitCommonGlobals(globalClosureInfo);
	Smile_SetGlobalClosureInfo(globalClosureInfo);

	compiler = Compiler_Create();

	userFunctionInfo = UserFunctionInfo_Create(NULL, NULL, NullList, NullObject, &errorMessage);
	compilerFunction = Compiler_BeginFunction(compiler, NullList, NullObject);
	compilerFunction->userFunctionInfo = userFunctionInfo;
	compiler->compiledTables->globalFunctionInfo = userFunctionInfo;

	compiler->currentFunction->currentSourceLocation = 0;

	compilerFunction->stackSize = stackSize;
	compilerFunction->localSize = compilerFunction->localMax = (Int32)localSize;

	userFunctionInfo->byteCodeSegment = ByteCodeSegment_CreateFromByteCodes(compiler->compiledTables, byteCodes, numByteCodes, True);

	closureInfo = Compiler_SetupClosureInfoForCompilerFunction(compiler, compilerFunction);
	MemCpy(&userFunctionInfo->closureInfo, closureInfo, sizeof(struct ClosureInfoStruct));

	Compiler_EndFunction(compiler);

	Compiler_AddUserFunctionInfo(compiler, userFunctionInfo);

	return userFunctionInfo;
}

//-----------------------------------------------------------------------------
// Constants.

START_TEST(CanEvalNop)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Nop },
		{ .opcode = Op_Nop },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Nop },
		{ .opcode = Op_Nop },
	};

	UserFunctionInfo globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 1, 0);
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 12345);
}
END_TEST

START_TEST(CanEvalDup1)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Ld64, .u.int64 = 42 },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Dup1 },
	};

	static struct ByteCodeStruct byteCode2[] = {
		{ .opcode = Op_Ld64, .u.int64 = 42 },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Dup1 },
		{ .opcode = Op_Pop1 },
	};

	static struct ByteCodeStruct byteCode3[] = {
		{ .opcode = Op_Ld64,.u.int64 = 42 },
		{ .opcode = Op_Ld64,.u.int64 = 12345 },
		{ .opcode = Op_Dup1 },
		{ .opcode = Op_Pop1 },
		{ .opcode = Op_Pop1 },
	};

	UserFunctionInfo globalFunctionInfo;
	EvalResult result;

	globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 3, 0);
	result = Eval_Run(globalFunctionInfo);
	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 12345);

	globalFunctionInfo = CreateRawGlobalCode(byteCode2, sizeof(byteCode2) / sizeof(struct ByteCodeStruct), 3, 0);
	result = Eval_Run(globalFunctionInfo);
	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 12345);

	globalFunctionInfo = CreateRawGlobalCode(byteCode3, sizeof(byteCode3) / sizeof(struct ByteCodeStruct), 3, 0);
	result = Eval_Run(globalFunctionInfo);
	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 42);
}
END_TEST

START_TEST(CanEvalDup2)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Ld64,.u.int64 = 42 },
		{ .opcode = Op_Ld64,.u.int64 = 12345 },
		{ .opcode = Op_Dup2 },
	};

	UserFunctionInfo globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 3, 0);
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 42);
}
END_TEST

START_TEST(CanEvalDupN)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Ld64, .u.int64 = 42 },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Ld64, .u.int64 = 3456 },
		{ .opcode = Op_Ld64, .u.int64 = 567 },
		{ .opcode = Op_Dup, .u.index = 4 },
	};

	UserFunctionInfo globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 5, 0);
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 42);
}
END_TEST

START_TEST(CanEvalPop2)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Ld64, .u.int64 = 42 },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Ld64, .u.int64 = 3456 },
		{ .opcode = Op_Ld64, .u.int64 = 567 },
		{ .opcode = Op_Pop2 },
	};

	UserFunctionInfo globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 5, 0);
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 12345);
}
END_TEST

START_TEST(CanEvalPopN)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Ld64, .u.int64 = 42 },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Ld64, .u.int64 = 3456 },
		{ .opcode = Op_Ld64, .u.int64 = 567 },
		{ .opcode = Op_Pop, .u.index = 3 },
	};

	UserFunctionInfo globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 5, 0);
	EvalResult result = Eval_Run(globalFunctionInfo);

	ASSERT(result->evalResultKind == EVAL_RESULT_VALUE);
	ASSERT(SMILE_KIND(result->value) == SMILE_KIND_INTEGER64);
	ASSERT(((SmileInteger64)result->value)->value == 42);
}
END_TEST

START_TEST(CanEvalRep1)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Ld64, .u.int64 = 42 },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Ld64, .u.int64 = 3456 },
		{ .opcode = Op_Ld64, .u.int64 = 567 },
		{ .opcode = Op_Rep1 },
		{ .opcode = Op_Brk },
	};

	UserFunctionInfo globalFunctionInfo;
	Closure closure;

	globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 5, 0);
	Eval_Run(globalFunctionInfo);

	Eval_GetCurrentBreakpointInfo(&closure, NULL, NULL, NULL);

	ASSERT(SMILE_KIND(closure->variables[0].obj) == SMILE_KIND_UNBOXED_INTEGER64);
	ASSERT(closure->variables[0].unboxed.i64 == 42);

	ASSERT(SMILE_KIND(closure->variables[1].obj) == SMILE_KIND_UNBOXED_INTEGER64);
	ASSERT(closure->variables[1].unboxed.i64 == 12345);

	ASSERT(SMILE_KIND(closure->variables[2].obj) == SMILE_KIND_UNBOXED_INTEGER64);
	ASSERT(closure->variables[2].unboxed.i64 == 567);

	ASSERT(closure->stackTop == closure->variables + 3);
}
END_TEST

START_TEST(CanEvalRep2)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Ld64, .u.int64 = 42 },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Ld64, .u.int64 = 3456 },
		{ .opcode = Op_Ld64, .u.int64 = 567 },
		{ .opcode = Op_Rep2 },
		{ .opcode = Op_Brk },
	};

	UserFunctionInfo globalFunctionInfo;
	Closure closure;

	globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 5, 0);
	Eval_Run(globalFunctionInfo);

	Eval_GetCurrentBreakpointInfo(&closure, NULL, NULL, NULL);

	ASSERT(SMILE_KIND(closure->variables[0].obj) == SMILE_KIND_UNBOXED_INTEGER64);
	ASSERT(closure->variables[0].unboxed.i64 == 42);

	ASSERT(SMILE_KIND(closure->variables[1].obj) == SMILE_KIND_UNBOXED_INTEGER64);
	ASSERT(closure->variables[1].unboxed.i64 == 567);

	ASSERT(closure->stackTop == closure->variables + 2);
}
END_TEST

START_TEST(CanEvalRepN)
{
	static struct ByteCodeStruct byteCode[] = {
		{ .opcode = Op_Ld64, .u.int64 = 42 },
		{ .opcode = Op_Ld64, .u.int64 = 12345 },
		{ .opcode = Op_Ld64, .u.int64 = 3456 },
		{ .opcode = Op_Ld64, .u.int64 = 567 },
		{ .opcode = Op_Ld64, .u.int64 = 999 },
		{ .opcode = Op_Rep, .u.index = 3 },
		{ .opcode = Op_Brk },
	};

	UserFunctionInfo globalFunctionInfo;
	Closure closure;

	globalFunctionInfo = CreateRawGlobalCode(byteCode, sizeof(byteCode) / sizeof(struct ByteCodeStruct), 5, 0);
	Eval_Run(globalFunctionInfo);

	Eval_GetCurrentBreakpointInfo(&closure, NULL, NULL, NULL);

	ASSERT(SMILE_KIND(closure->variables[0].obj) == SMILE_KIND_UNBOXED_INTEGER64);
	ASSERT(closure->variables[0].unboxed.i64 == 42);

	ASSERT(SMILE_KIND(closure->variables[1].obj) == SMILE_KIND_UNBOXED_INTEGER64);
	ASSERT(closure->variables[1].unboxed.i64 == 999);

	ASSERT(closure->stackTop == closure->variables + 2);
}
END_TEST

#include "evalcore_tests.generated.inc"
