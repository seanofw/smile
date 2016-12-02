//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#include <smile/eval/bytecode.h>
#include <smile/eval/opcode.h>
#include <smile/eval/compiler.h>
#include <smile/string.h>
#include <smile/stringbuilder.h>

static String ByteCode_OperandsToString(ByteCode byteCode, Int address, struct CompiledTablesStruct *compiledTables);

/// <summary>
/// Create a new byte-code segment, with room for 'size' instructions initially.
/// </summary>
/// <param name="size">The number of initial empty instruction slots.</param>
ByteCodeSegment ByteCodeSegment_CreateWithSize(Int size)
{
	ByteCode byteCodes;
	ByteCodeSegment segment;

	segment = GC_MALLOC_STRUCT(struct ByteCodeSegmentStruct);
	if (segment == NULL)
		Smile_Abort_OutOfMemory();

	byteCodes = GC_MALLOC_RAW_ARRAY(struct ByteCodeStruct, size);
	if (byteCodes == NULL)
		Smile_Abort_OutOfMemory();

	segment->byteCodes = byteCodes;
	segment->numByteCodes = 0;
	segment->maxByteCodes = size;

	return segment;
}

/// <summary>
/// Grow the given byte-code segment until it has at least 'count' empty instruction slots
/// at the end.
/// </summary>
/// <param name="segment">The segment to possibly grow.</param>
/// <param name="count">How much space is needed for the new instruction(s) (often 1).</param>
void ByteCodeSegment_Grow(ByteCodeSegment segment, Int count)
{
	Int newMax;
	ByteCode newByteCodes;

	newMax = segment->maxByteCodes;
	while (segment->numByteCodes + count*2 >= newMax) {    // We want enough for 'count', and for at least one more construct of the same size.
		newMax *= 2;
	}

	newByteCodes = GC_MALLOC_RAW_ARRAY(struct ByteCodeStruct, newMax);
	if (newByteCodes == NULL)
		Smile_Abort_OutOfMemory();

	MemCpy(newByteCodes, segment->byteCodes, sizeof(struct ByteCodeStruct) * segment->numByteCodes);

	segment->byteCodes = newByteCodes;
	segment->maxByteCodes = newMax;
}

STATIC_STRING(_colonString, ":");

/// <summary>
/// Convert the given byte-code segment to a string that lists all its instructions,
/// in order.
/// </summary>
/// <param name="segment">The byte-code segment to dump.</param>
/// <param name="compiledTables">The compiled tables of objects and functions and strings
/// that this segment may reference.</param>
/// <returns>The byte-code segment's instructions, as a string.</returns>
String ByteCodeSegment_ToString(ByteCodeSegment segment, struct CompiledTablesStruct *compiledTables)
{
	Int i, end;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 256);
	String string;
	ByteCode byteCode;

	INIT_INLINE_STRINGBUILDER(stringBuilder);

	for (i = 0, end = segment->numByteCodes; i < end; i++) {
		byteCode = segment->byteCodes + i;
		string = ByteCode_ToString(byteCode, i, compiledTables);
		if (byteCode->opcode != Op_Label) {
			StringBuilder_AppendByte(stringBuilder, '\t');
		}
		StringBuilder_AppendString(stringBuilder, string);
		StringBuilder_AppendByte(stringBuilder, '\n');
	}

	string = StringBuilder_ToString(stringBuilder);
	return string;
}

/// <summary>
/// Convert the individual byte code to a string representation.
/// </summary>
/// <param name="byteCode">The byte code to dump.</param>
/// <param name="address">The address at which this byte code is stored within its segment.</param>
/// <param name="compiledTables">The compiled tables of objects and functions and strings
/// that this byte code may reference.</param>
/// <returns>The byte code's contents, as a string.</returns>
String ByteCode_ToString(ByteCode byteCode, Int address, struct CompiledTablesStruct *compiledTables)
{
	String opcode, operands;
	DECLARE_INLINE_STRINGBUILDER(stringBuilder, 64);

	if (byteCode->opcode == Op_Label)
		return String_Format("L%d:", address);
	
	opcode = Opcode_Names[byteCode->opcode];
	if (opcode == NULL) opcode = String_Format("Op%02X", byteCode->opcode);

	operands = ByteCode_OperandsToString(byteCode, address, compiledTables);
	if (operands == NULL)
		return opcode;

	INIT_INLINE_STRINGBUILDER(stringBuilder);
	StringBuilder_AppendString(stringBuilder, opcode);
	StringBuilder_AppendByte(stringBuilder, ' ');
	StringBuilder_AppendString(stringBuilder, operands);
	return StringBuilder_ToString(stringBuilder);
}

/// <summary>
/// Convert the given byte code's operands to a string representation.
/// </summary>
/// <param name="byteCode">The byte code whose operands you want to dump.</param>
/// <param name="address">The address at which this byte code is stored within its segment.</param>
/// <param name="compiledTables">The compiled tables of objects and functions and strings
/// that this byte code may reference.</param>
/// <returns>The byte code's operands, as a string.</returns>
static String ByteCode_OperandsToString(ByteCode byteCode, Int address, struct CompiledTablesStruct *compiledTables)
{
	Int opcode = byteCode->opcode;

	switch (opcode) {
		default:
			return NULL;

		// 00-0F
		case Op_Dup:
		case Op_Pop:
			return String_Format("%d", byteCode->u.int32);
		
		// 10-17
		case Op_LdBool:
			return String_Format("%s", byteCode->u.boolean ? "true" : "false");
		case Op_LdCh:
			return byteCode->u.ch < 32 || byteCode->u.uch > 126 ? String_Format("LdCh '\\x%02X'", (UInt32)byteCode->u.ch) : String_Format("LdCh '%c'", (UInt32)byteCode->u.ch);
		case Op_LdUCh:
			return byteCode->u.uch < 32 || byteCode->u.uch > 126 ? String_Format("LdUCh '\\x%X'", (UInt32)byteCode->u.uch) : String_Format("LdCh '%c'", (UInt32)byteCode->u.uch);
		case Op_LdStr:
			return String_Format("\"%S\"", String_AddCSlashes(compiledTables->strings[byteCode->u.index]));
		case Op_LdSym:
			return String_Format("`%S", String_AddCSlashes(SymbolTable_GetName(Smile_SymbolTable, byteCode->u.symbol)));
		case Op_LdObj:
			return String_Format("@%d", byteCode->u.index);
		
		// 18-1F
		case Op_Ld8:
			return String_Format("%u", (Int32)byteCode->u.byte);
		case Op_Ld16:
			return String_Format("%d", (Int32)byteCode->u.int16);
		case Op_Ld32:
			return String_Format("%d", byteCode->u.int32);
		case Op_Ld64:
			return String_Format("%ld", byteCode->u.int64);
		case Op_Ld128:
			return String_Format("@%u", (Int32)byteCode->u.index);
		
		// 20-27
		case Op_LdR16:
			return String_Format("%g", byteCode->u.real32);
		case Op_LdR32:
			return String_Format("%g", byteCode->u.real32);
		case Op_LdR64:
			return String_Format("%g", byteCode->u.real64);
		case Op_LdR128:
			return String_Format("@%u", (Int32)byteCode->u.index);

		// 28-2F
		case Op_LdF16:
			return String_Format("%g", byteCode->u.float32);
		case Op_LdF32:
			return String_Format("%g", byteCode->u.float32);
		case Op_LdF64:
			return String_Format("%g", byteCode->u.float64);
		case Op_LdF128:
			return String_Format("@%u", (Int32)byteCode->u.index);

		// 30-37
		case Op_LdLoc:
		case Op_StLoc:
		case Op_StpLoc:
		case Op_LdArg:
		case Op_StArg:
		case Op_StpArg:
			return String_Format("%d, %d", (Int32)byteCode->u.int32, (Int32)byteCode->u.int32);

		// 38-3F
		case Op_LdX:
		case Op_StX:
		case Op_StpX:
			return String_Format("`%S", SymbolTable_GetName(Smile_SymbolTable, byteCode->u.symbol));
		
		// 40-6F
		case Op_LdArg0: case Op_LdArg1: case Op_LdArg2: case Op_LdArg3:
		case Op_LdArg4: case Op_LdArg5: case Op_LdArg6: case Op_LdArg7:
		case Op_LdLoc0: case Op_LdLoc1: case Op_LdLoc2: case Op_LdLoc3:
		case Op_LdLoc4: case Op_LdLoc5: case Op_LdLoc6: case Op_LdLoc7:
		case Op_StArg0: case Op_StArg1: case Op_StArg2: case Op_StArg3:
		case Op_StArg4: case Op_StArg5: case Op_StArg6: case Op_StArg7:
		case Op_StLoc0: case Op_StLoc1: case Op_StLoc2: case Op_StLoc3:
		case Op_StLoc4: case Op_StLoc5: case Op_StLoc6: case Op_StLoc7:
		case Op_StpArg0: case Op_StpArg1: case Op_StpArg2: case Op_StpArg3:
		case Op_StpArg4: case Op_StpArg5: case Op_StpArg6: case Op_StpArg7:
		case Op_StpLoc0: case Op_StpLoc1: case Op_StpLoc2: case Op_StpLoc3:
		case Op_StpLoc4: case Op_StpLoc5: case Op_StpLoc6: case Op_StpLoc7:
			return String_Format("%d", (Int32)byteCode->u.int32);
		
		// 70-7F
		case Op_LdProp:
		case Op_StProp:
		case Op_StpProp:
			return String_Format("`%S", SymbolTable_GetName(Smile_SymbolTable, byteCode->u.symbol));
		
		// 80-8F
		case Op_Begin:
			return String_Format("%d, %d", (Int32)byteCode->u.int32, (Int32)byteCode->u.int32);
		case Op_Try:
		case Op_JmpEsc:
			return String_Format(byteCode->u.i2.a < 0 ? "L%d, %d" : ">L%d, %d", address + byteCode->u.i2.a, byteCode->u.i2.b);
		case Op_Esc:
			return String_Format(byteCode->u.index < 0 ? "L%d" : ">L%d", (Int32)(address + byteCode->u.index));
		
		// 90-AF
		case Op_Met0: case Op_Met1: case Op_Met2: case Op_Met3:
		case Op_Met4: case Op_Met5: case Op_Met6: case Op_Met7:
		case Op_TMet0: case Op_TMet1: case Op_TMet2: case Op_TMet3:
		case Op_TMet4: case Op_TMet5: case Op_TMet6: case Op_TMet7:
			return String_Format("`%S", SymbolTable_GetName(Smile_SymbolTable, (Symbol)byteCode->u.symbol));
		
		// B0-BF
		case Op_Jmp:
		case Op_Bt:
		case Op_Bf:
			return String_Format(byteCode->u.index < 0 ? "L%d" : ">L%d", (Int32)(address + byteCode->u.index));
		case Op_Met:
		case Op_TMet:
			return String_Format("`%S, %d", SymbolTable_GetName(Smile_SymbolTable, (Symbol)byteCode->u.i2.a), byteCode->u.i2.b);
		case Op_Call:
		case Op_TCall:
		case Op_CallEsc:
		case Op_LocalAlloc:
		case Op_LocalFree:
		case Op_Args:
			return String_Format("%d", byteCode->u.int32);
		
		// C0-CF
		case Op_NewFn:
			return String_Format("@%d", byteCode->u.int32);
		case Op_NewObj:
			return String_Format("%d", byteCode->u.int32);
		
		// E0-E7
		case Op_Add8:
		case Op_Sub8:
			return String_Format("%u", (Int32)byteCode->u.byte);
		case Op_Add16:
		case Op_Sub16:
			return String_Format("%d", (Int32)byteCode->u.int16);
		case Op_Add32:
		case Op_Sub32:
			return String_Format("%d", byteCode->u.int32);
		case Op_Add64:
		case Op_Sub64:
			return String_Format("%ld", byteCode->u.int64);
		
		// E8-EF
		case Op_AddR32:
		case Op_SubR32:
			return String_Format("%g", byteCode->u.real32);
		case Op_AddR64:
		case Op_SubR64:
			return String_Format("%g", byteCode->u.real64);
		case Op_AddF32:
		case Op_SubF32:
			return String_Format("%g", byteCode->u.float32);
		case Op_AddF64:
		case Op_SubF64:
			return String_Format("%g", byteCode->u.float64);

		// F0-FF
		case Op_Label:
			return String_Format("`%S", String_AddCSlashes(SymbolTable_GetName(Smile_SymbolTable, byteCode->u.symbol)));
	}
}

//-------------------------------------------------------------------------------------------------
// Opcode tables, for stringification.
//
// This is a whole lotta macro magic to make the "opnames.inc" file as
// dumb and simple as possible.  Visual Studio Intellisense really really
// doesn't like this wackiness, but it works really well.

#define BeginOps extern int __dummy__,
#define Op(__name__) __dummy2__; STATIC_STRING(_op_##__name__, #__name__); extern int __dummy__
#define EndOps __dummy2__;
#undef NULL
#define NULL __dummy2__; extern int __dummy__

BeginOps
#include "opnames.inc"
EndOps

#undef Op
#undef BeginOps
#undef EndOps
#undef NULL
#define NULL 0

#define Op(__name__) ((String)&_op_##__name__##Struct)

static String _opcode_Names[] = {
#include "opnames.inc"
};

String *Opcode_Names = _opcode_Names;

// End of opcode tables.
//-------------------------------------------------------------------------------------------------
