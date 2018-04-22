
#ifndef __SMILE_EVAL_COMPILER_COMPILEDBLOCK_H__
#define __SMILE_EVAL_COMPILER_COMPILEDBLOCK_H__

#ifndef __SMILE_EVAL_BYTECODE_H__
#include <smile/eval/bytecode.h>
#endif

#ifndef __SMILE_EVAL_OPCODE_H__
#include <smile/eval/opcode.h>
#endif

//-------------------------------------------------------------------------------------------------
// Basic-block and intermediate-code types

/// <summary>
/// Miscellaneous flags describing the overall behavior of this block.
/// </summary>
typedef enum {
	BLOCK_FLAG_ESCAPE = (1 << 0),	// This block escapes on all code paths; any subsequent code is dead.
	BLOCK_FLAG_ERROR = (1 << 1),	// This block failed to compile with an error (and stack is likely invalid).
} BlockFlags;

/// <summary>
/// A basic block, which represents a chunk of instrucitons that have no branches into or
/// out of them, and that may only influence the stack by pushing zero or more items onto it.
/// </summary>
typedef struct CompiledBlockStruct {
	struct CompiledBlockStruct *parent;
	struct IntermediateInstructionStruct *first, *last;
	Int numInstructions;
	Int maxStackDepth, finalStackDelta;
	BlockFlags blockFlags;
} *CompiledBlock;

/// <summary>
/// A single intermediate-instruction in a basic block.  These use the same opcodes and
/// operands as real instructions, but they are constructed as a linked list for easy
/// insertion and deletion of instructions; and branches point at Op_Label pseudo-instructions;
/// and Op_Block pseudo-instructions may "include" a child CompiledBlock into this block.
/// </summary>
typedef struct IntermediateInstructionStruct {

	struct IntermediateInstructionStruct *prev, *next;	// Pointers to nearby instructions.

	UInt32 opcode : 8;				// The opcode for this instruction.
	UInt32 instructionAddress : 24;	// The offset this instruction is at (after address resolution).
	UInt32 sourceLocation;			// The index of the source location that generated this (for debugging).

	union {
		Int64 int64;
		Int32 int32;
		Int16 int16;
		Byte byte;
		Real64 real64;
		Real32 real32;
		Float32 float32;
		Float64 float64;
		Symbol symbol;
		Int index;
		Int delta;
		Byte ch;
		UInt32 uni;
		Bool boolean;
		struct {
			Int32 a, b;
		} i2;
	} u;

	union {
		CompiledBlock childBlock;
		struct IntermediateInstructionStruct *branchTarget;
	} p;

} *IntermediateInstruction;

//-------------------------------------------------------------------------------------------------
// Basic-block and intermediate-code functions

extern IntermediateInstruction IntermediateInstruction_Create(Int opcode);
extern CompiledBlock CompiledBlock_Create(void);
extern CompiledBlock CompiledBlock_CreateError(void);
extern void CompiledBlock_AttachInstruction(CompiledBlock compiledBlock, IntermediateInstruction insertAfterThis, IntermediateInstruction newInstruction);
extern void CompiledBlock_DetachInstruction(CompiledBlock compiledBlock, IntermediateInstruction instruction);
extern void CompiledBlock_Clear(CompiledBlock compiledBlock);
extern CompiledBlock CompiledBlock_Combine(CompiledBlock firstblock, CompiledBlock secondBlock);
extern void CompiledBlock_Flatten(CompiledBlock compiledBlock);
extern Int CompiledBlock_CalculateAddresses(CompiledBlock compiledBlock, Int startAddress, Bool includePseudoOps);
extern void CompiledBlock_ResolveBranches(CompiledBlock compiledBlock);
extern void CompiledBlock_AppendToByteCodeSegment(CompiledBlock compiledBlock, ByteCodeSegment segment, Bool includePseudoOps);
extern Int CompiledBlock_CountInstructions(CompiledBlock compiledBlock, Bool includePseudoOps);
extern IntermediateInstruction CompiledBlock_AppendChild(CompiledBlock parentBlock, CompiledBlock newChild);
extern IntermediateInstruction CompiledBlock_Emit(CompiledBlock compiledBlock, Int opcode, Int stackDelta, Int sourceLocation);
extern ByteCodeSegment CompiledBlock_Finish(CompiledBlock compiledBlock, struct CompiledTablesStruct *compiledTables, Bool includePseudoOps);
extern String CompiledBlock_Stringify(CompiledBlock compiledBlock, struct CompiledTablesStruct *compiledTables);

#endif
