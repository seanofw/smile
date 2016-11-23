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

TEST_SUITE(ByteCodeTests)

START_TEST(CanEmitNop)
{
	ByteCodeSegment segment = ByteCodeSegment_Create();
	CompiledTables compiledTables = CompiledTables_Create();
	String result;

	const char *expectedResult = "\tNop\n";

	ByteCodeSegment_Emit(segment, Op_Nop);

	result = ByteCodeSegment_ToString(segment, compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanEmitIntegerLoads)
{
	ByteCodeSegment segment = ByteCodeSegment_Create();
	CompiledTables compiledTables = CompiledTables_Create();
	String result;

	const char *expectedResult =
		"\tLd8 123\n"
		"\tLd16 12345\n"
		"\tLd32 12345678\n"
		"\tLd64 1234567890\n";

	ByteCodeSegment_Emit(segment, Op_Ld8)->u.byte = 123;
	ByteCodeSegment_Emit(segment, Op_Ld16)->u.int16 = 12345;
	ByteCodeSegment_Emit(segment, Op_Ld32)->u.int32 = 12345678;
	ByteCodeSegment_Emit(segment, Op_Ld64)->u.int64 = 1234567890;

	result = ByteCodeSegment_ToString(segment, compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanEmitBranches)
{
	ByteCodeSegment segment = ByteCodeSegment_Create();
	CompiledTables compiledTables = CompiledTables_Create();
	String result;

	const char *expectedResult =
		"\tLd32 123\n"
		"\tJmp >L5\n"
		"L2:\n"
		"\tLd32 1\n"
		"\tBinary `-\n"
		"L5:\n"
		"\tDup1\n"
		"\tBt L2\n";

	ByteCodeSegment_Emit(segment, Op_Ld32)->u.int32 = 123;
	ByteCodeSegment_Emit(segment, Op_Jmp)->u.index = +4;
	ByteCodeSegment_Emit(segment, Op_Label)->u.index = +5;
	ByteCodeSegment_Emit(segment, Op_Ld32)->u.int32 = 1;
	ByteCodeSegment_Emit(segment, Op_Met1)->u.symbol = Smile_KnownSymbols.minus;
	ByteCodeSegment_Emit(segment, Op_Label)->u.index = -4;
	ByteCodeSegment_Emit(segment, Op_Dup1);
	ByteCodeSegment_Emit(segment, Op_Bt)->u.index = -5;

	result = ByteCodeSegment_ToString(segment, compiledTables);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

#include "bytecode_tests.generated.inc"
