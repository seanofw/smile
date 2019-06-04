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

TEST_SUITE(ByteCodeTests)

START_TEST(CanEmitNop)
{
	CompiledTables compiledTables = CompiledTables_Create();
	ByteCodeSegment segment = ByteCodeSegment_Create(compiledTables);
	String result;

	const char *expectedResult = "0: \tNop\n";

	ByteCodeSegment_Emit(segment, Op_Nop, 0);

	result = ByteCodeSegment_ToString(segment, NULL);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanEmitIntegerLoads)
{
	CompiledTables compiledTables = CompiledTables_Create();
	ByteCodeSegment segment = ByteCodeSegment_Create(compiledTables);
	String result;

	const char *expectedResult =
		"0: \tLd8     123\n"
		"1: \tLd16    12345\n"
		"2: \tLd32    12345678\n"
		"3: \tLd64    1234567890\n";

	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Ld8, 0)].u.byte = 123;
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Ld16, 0)].u.int16 = 12345;
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Ld32, 0)].u.int32 = 12345678;
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Ld64, 0)].u.int64 = 1234567890;

	result = ByteCodeSegment_ToString(segment, NULL);
	ASSERT_STRING(result, expectedResult, StrLen(expectedResult));
}
END_TEST

START_TEST(CanEmitBranches)
{
	CompiledTables compiledTables = CompiledTables_Create();
	ByteCodeSegment segment = ByteCodeSegment_Create(compiledTables);
	String result;

	String expectedResult = String_Format(
		"0: \tLd32    123\n"
		"1: \tJmp     >L5\n"
		"2: L2:\n"
		"3: \tLd32    1\n"
		"4: \tBinary  `- (%d)\n"
		"5: L5:\n"
		"6: \tDup1\n"
		"7: \tBt      L2\n",
		Smile_KnownSymbols.minus
	);

	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Ld32, 0)].u.int32 = 123;
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Jmp, 0)].u.index = +4;
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Label, 0)].u.index = +5;
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Ld32, 0)].u.int32 = 1;
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Met1, 0)].u.symbol = Smile_KnownSymbols.minus;
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Label, 0)].u.index = -4;
	ByteCodeSegment_Emit(segment, Op_Dup1, 0);
	segment->byteCodes[ByteCodeSegment_Emit(segment, Op_Bt, 0)].u.index = -5;

	result = ByteCodeSegment_ToString(segment, NULL);
	ASSERT_STRING(result, String_ToC(expectedResult), String_Length(expectedResult));
}
END_TEST

#include "bytecode_tests.generated.inc"
