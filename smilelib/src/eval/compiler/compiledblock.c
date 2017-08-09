//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
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

#include <smile/eval/compiledblock.h>

/// <summary>
/// Construct a new, detached IntermediateInstruction with the given opcode.
/// Its initial values will all be NULL/default.
/// </summary>
/// <returns>A new IntermediateInstruction, with the given opcode in it.</returns>
IntermediateInstruction IntermediateInstruction_Create(Int opcode)
{
	IntermediateInstruction instruction = GC_MALLOC_STRUCT(struct IntermediateInstructionStruct);

	instruction->next = NULL;
	instruction->prev = NULL;

	instruction->opcode = (UInt32)opcode;
	instruction->sourceLocation = 0;
	instruction->instructionAddress = 0;

	instruction->u.int64 = 0;
	instruction->p.branchTarget = NULL;

	return instruction;
}

/// <summary>
/// Construct a new, empty CompiledBlock. Its initial values will all be NULL/default.
/// </summary>
/// <returns>A new, empty CompiledBlock.</returns>
CompiledBlock CompiledBlock_Create(void)
{
	CompiledBlock compiledBlock = GC_MALLOC_STRUCT(struct CompiledBlockStruct);

	compiledBlock->parent = NULL;
	compiledBlock->finalStackDelta = 0;
	compiledBlock->maxStackDepth = 0;
	compiledBlock->first = NULL;
	compiledBlock->last = NULL;
	compiledBlock->numInstructions = 0;
	compiledBlock->blockFlags = 0;

	return compiledBlock;
}

/// <summary>
/// Construct a new, empty CompiledBlock with its ERROR flag set to represent an un-compile-able code chunk.
/// </summary>
/// <returns>An error'ed CompiledBlock.</returns>
CompiledBlock CompiledBlock_CreateError(void)
{
	CompiledBlock compiledBlock = CompiledBlock_Create();
	compiledBlock->blockFlags |= BLOCK_FLAG_ERROR;
	return compiledBlock;
}

/// <summary>
/// Attach a new instruction to the given basic block after the provided instruction.
/// Note that this does NOT correct the maxStackDepth or finalStackDelta fields in the
/// CompiledBlock for its inclusion; if those are wrong after the new instruction is added,
/// it's the caller's responsibility to update them.
/// </summary>
/// <param name="compiledBlock">The CompiledBlock to attach the instruction to.</param>
/// <param name="insertAfterThis">The instruction to insert the new instruction after.
/// This instruction must already be attached to the CompiledBlock.  If this is NULL,
/// the new instruction will be inserted as the new first instruction in the block.</param>
/// <param name="newInstruction">The instruction to attach.  This must not be NULL, and
/// it must not already be attached to anything.</param>
void CompiledBlock_AttachInstruction(CompiledBlock compiledBlock, IntermediateInstruction insertAfterThis,
	IntermediateInstruction newInstruction)
{
	// Basic sanity checks.
	SMILE_ASSERT(compiledBlock != NULL);
	SMILE_ASSERT(newInstruction != NULL);

	// Cannot attach an instruction that has already been attached!
	SMILE_ASSERT(newInstruction->next == NULL && newInstruction->prev == NULL
		&& compiledBlock->first != newInstruction && compiledBlock->last != newInstruction);

	if (insertAfterThis == NULL) {

		// Insert at the head.
		newInstruction->next = compiledBlock->first;
		newInstruction->prev = NULL;
		compiledBlock->first = newInstruction;
		if (compiledBlock->last == NULL)
			compiledBlock->last = newInstruction;
	}
	else {

		// Insert after the given instruction, which may be the tail.
		newInstruction->next = insertAfterThis->next;
		newInstruction->prev = insertAfterThis;
		if (compiledBlock->last == insertAfterThis)
			compiledBlock->last = newInstruction;
		else
			insertAfterThis->next->prev = newInstruction;
		insertAfterThis->next = newInstruction;
	}

	// One more now.
	compiledBlock->numInstructions++;
}

/// <summary>
/// Detach the given instruction from the given CompiledBlock.  Note that this does NOT correct
/// the maxStackDepth or finalStackDelta fields in the CompiledBlock for its removal; if those
/// are wrong after its removal, it's the caller's responsibility to update them.
/// </summary>
/// <param name="compiledBlock">The CompiledBlock to detach the instruction from.</param>
/// <param name="instruction">The instruction to detach.  This must not be NULL.</param>
void CompiledBlock_DetachInstruction(CompiledBlock compiledBlock, IntermediateInstruction instruction)
{
	// Basic sanity checks.
	SMILE_ASSERT(compiledBlock != NULL);
	SMILE_ASSERT(instruction != NULL);

	// Detach the forward pointer.
	if (instruction->next != NULL) {
		instruction->next->prev = instruction->prev;
	}
	else {
		compiledBlock->last = instruction->prev;
	}

	// Detach the backward pointer.
	if (instruction->prev != NULL) {
		instruction->prev->next = instruction->next;
	}
	else {
		compiledBlock->first = instruction->next;
	}

	// One fewer now.
	compiledBlock->numInstructions--;
}

/// <summary>
/// Join two basic blocks together in sequence.  This destroys the original two basic
/// blocks, and results in a new combined block that contains all of the instructions
/// of the originals.  Note that the combined block's instruction addresses will be wrong,
/// if they were computed before this.  This operation *does* correctly update both
/// maxStackDepth and finalStackDelta of the resulting block to reflect the combined
/// behavior of the two blocks.
/// </summary>
/// <param name="firstBlock">The first block to combine.</param>
/// <param name="secondBlock">The second block to combine.</param>
/// <returns>A block that is the combination of the first two.  The two original block
/// pointers must not be used after this call, as they are no longer valid.</param>
CompiledBlock CompiledBlock_Combine(CompiledBlock firstBlock, CompiledBlock secondBlock)
{
	// Handle degenerate cases:  If one of the blocks has no instructions, just keep the
	// other block.  (If both have no instructions, we just return the empty 'firstBlock'.)
	if (secondBlock->first == NULL)
		return firstBlock;
	if (firstBlock->first == NULL)
		return secondBlock;

	// Hook the ends of the linked lists together.
	firstBlock->last->next = secondBlock->first;
	secondBlock->first->prev = firstBlock->last;

	// Move the endpoints from secondBlock to firstBlock.
	firstBlock->last = secondBlock->last;

	// Update the instruction counts.
	firstBlock->numInstructions += secondBlock->numInstructions;

	// Update the max depth.
	if (secondBlock->maxStackDepth > firstBlock->maxStackDepth)
		firstBlock->maxStackDepth = secondBlock->maxStackDepth;

	// Update the final count.
	firstBlock->finalStackDelta += secondBlock->finalStackDelta;

	// Zorch any meaningful values in secondBlock to keep it from being reused.
	secondBlock->first = secondBlock->last = NULL;
	secondBlock->numInstructions = 0;
	secondBlock->maxStackDepth = 0;
	secondBlock->finalStackDelta = 0;

	return firstBlock;
}

/// <summary>
/// Recursively walk through any child blocks of this block and combine their instructions into
/// this block.  This, in total, runs in O(n) time.
/// </summary>
void CompiledBlock_Flatten(CompiledBlock compiledBlock)
{
	IntermediateInstruction instr, next;
	CompiledBlock childBlock;

	// Spin over the instructions, flattening any that need it.
	for (instr = compiledBlock->first; instr != NULL; instr = next) {
		next = instr->next;

		// Only do flattening on child-block pseudo-instructions.
		if (instr->opcode != Op_Block)
			continue;

		// First, flatten the child block's own content (which is O(n) for its contents).
		childBlock = instr->p.childBlock;
		CompiledBlock_Flatten(childBlock);
		
		// The rest of the flattening takes O(1) time.

		if (childBlock->first == NULL) {
			// Degenerate case:  The child has no instructions, so just detach its
			// parent instruction and toss it.
			CompiledBlock_DetachInstruction(compiledBlock, instr);
			continue;
		}

		// Attach the child block's lists in place of its parent instruction.
		if (instr->prev != NULL) {
			instr->prev->next = childBlock->first;
		}
		else {
			compiledBlock->first = childBlock->first;
		}
		childBlock->first->prev = instr->prev;

		if (instr->next != NULL) {
			instr->next->prev = childBlock->last;
		}
		else {
			compiledBlock->last = childBlock->last;
		}
		childBlock->last->next = instr->next;

		// Reset the child block's pointers, just in case somebody tries to use it again.
		childBlock->first = childBlock->last = NULL;
		childBlock->numInstructions = 0;
		childBlock->parent = NULL;
	}
}

/// <summary>
/// Walk through all instructions of this block and any child blocks, and assign them
/// suitable addresses, and correct any branches to point to the proper relative target address.
/// </summary>
/// <param name="compiledBlock">The basic block to which address resolution should be applied.</param>
/// <param name="startAddress">The initial starting address for the block.</param>
/// <returns>The first instruction address *after* the basic block.</returns>
Int CompiledBlock_CalculateAddresses(CompiledBlock compiledBlock, Int startAddress, Bool includePseudoOps)
{
	IntermediateInstruction instr;

	// Step 1.  Spin over the instructions, assigning addresses, and recursing into
	// any child blocks to assign their addresses too.
	for (instr = compiledBlock->first; instr != NULL; instr = instr->next) {

		instr->instructionAddress = (UInt32)startAddress;

		if (instr->opcode == Op_Block) {
			// Child blocks need to be recursed into.  Note that this completely and
			// fully resolves their addresses, both instruction addresses and branch targets.
			if (includePseudoOps) startAddress++;
			startAddress = CompiledBlock_CalculateAddresses(instr->p.childBlock, startAddress, includePseudoOps);
			if (includePseudoOps) startAddress++;
		}
		else if (instr->opcode < Op_Pseudo || includePseudoOps) {
			// Every instruction gets an address except for pseudo-ops, which get none.
			startAddress++;
		}
	}

	return startAddress;
}

/// <summary>
/// Walk through all instructions of this block and any child blocks, and update any
/// branches to have real, appropriate target addresses.
/// </summary>
/// <param name="compiledBlock">The basic block to which branch-target resolution should be applied.</param>
void CompiledBlock_ResolveBranches(CompiledBlock compiledBlock)
{
	IntermediateInstruction instr;

	// Step 2.  Spin over the instructions again, and anywhere we find a branch, we
	// resolve it to point to its target label, relative to its own address.  Note that
	// we do *not* need to do this recursively, since the previous step will already
	// have performed that recursion for us in any child blocks.
	for (instr = compiledBlock->first; instr != NULL; instr = instr->next) {

		switch (instr->opcode) {
			case Op_Jmp:
			case Op_Bt:
			case Op_Bf:
				// Branches' indexes get resolved to their label's relative address.
				// We also have an extra safety check here against a NULL branch target,
				// which should never happen, but we don't want to crash if it does.
				instr->u.index = instr->p.branchTarget != NULL
					? ((Int)instr->p.branchTarget->instructionAddress - (Int)instr->instructionAddress) : 0;
				break;

			case Op_NewTill:
				// Till loops need to have all of their branch indexes filled in.
				// TODO: FIXME: DO THIS.
				break;

			case Op_Block:
				// Recurse into child blocks.
				CompiledBlock_ResolveBranches(instr->p.childBlock);
				break;
		}
	}
}

/// <summary>
/// Pack the entire given CompiledBlock down into a finished ByteCodeSegment.
/// </summary>
void CompiledBlock_AppendToByteCodeSegment(CompiledBlock compiledBlock, ByteCodeSegment segment, Bool includePseudoOps)
{
	IntermediateInstruction instr;
	ByteCode byteCode;

	ByteCodeSegment_More(segment, compiledBlock->numInstructions);

	for (instr = compiledBlock->first; instr != NULL; instr = instr->next) {

		if (instr->opcode == Op_Block) {
			if (includePseudoOps) {
				byteCode = &segment->byteCodes[segment->numByteCodes++];
				byteCode->opcode = (Byte)instr->opcode;
				byteCode->sourceLocation = instr->sourceLocation;
				byteCode->u.int64 = instr->u.int64;
			}

			// Child blocks need to be recursed into.
			CompiledBlock_AppendToByteCodeSegment(instr->p.childBlock, segment, includePseudoOps);

			if (includePseudoOps) {
				byteCode = &segment->byteCodes[segment->numByteCodes++];
				byteCode->opcode = (Byte)Op_EndBlock;
				byteCode->sourceLocation = instr->sourceLocation;
				byteCode->u.int64 = instr->u.int64;
			}
		}
		else if (instr->opcode != Op_Label || includePseudoOps) {
			// Everything else that's not a pseudo-op needs to be copied into the segment.
			byteCode = &segment->byteCodes[segment->numByteCodes++];
			byteCode->opcode = (Byte)instr->opcode;
			byteCode->sourceLocation = instr->sourceLocation;
			byteCode->u.int64 = instr->u.int64;
		}
	}
}

/// <summary>
/// Append a child block to the given block.  This does correctly update the stack information
/// and block flags for the inclusion of the child block.  If the child block is empty, this
/// does not alter the parent.
/// <summary>
/// <param name="parentBlock">The parent block to which the new child will be attached.</param>
/// <param name="newChild">The child block that is being attached.</param>
/// <returns>An Op_Block instruction for the child in the parent, or NULL if the child was empty and
/// was not attached to the parent.</returns>
IntermediateInstruction CompiledBlock_AppendChild(CompiledBlock parentBlock, CompiledBlock newChild)
{
	IntermediateInstruction instruction;

	if (newChild->first == NULL) return NULL;

	instruction = IntermediateInstruction_Create(Op_Block);
	instruction->p.childBlock = newChild;
	newChild->parent = parentBlock;
	CompiledBlock_AttachInstruction(parentBlock, parentBlock->last, instruction);

	if (parentBlock->finalStackDelta + newChild->maxStackDepth > parentBlock->maxStackDepth)
		parentBlock->maxStackDepth = parentBlock->finalStackDelta + newChild->maxStackDepth;
	parentBlock->finalStackDelta += newChild->finalStackDelta;

	if (newChild->blockFlags & BLOCK_FLAG_ERROR)
		parentBlock->blockFlags |= BLOCK_FLAG_ERROR;

	return instruction;
}

/// <summary>
/// Emit a single new instruction at the end of the given block, updating the stack
/// for its delta.
/// </summary>
IntermediateInstruction CompiledBlock_Emit(CompiledBlock compiledBlock, Int opcode, Int stackDelta, Int sourceLocation)
{
	IntermediateInstruction instruction = IntermediateInstruction_Create(opcode);
	instruction->sourceLocation = (UInt32)sourceLocation;

	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, instruction);

	compiledBlock->finalStackDelta += stackDelta;
	if (compiledBlock->finalStackDelta > compiledBlock->maxStackDepth)
		compiledBlock->maxStackDepth = compiledBlock->finalStackDelta;

	return instruction;
}

/// <summary>
/// Finish the entire given CompiledBlock, assigning it real addresses, resolving its branches,
/// and transforming it into an executable ByteCodeSegment.
/// </summary>
ByteCodeSegment CompiledBlock_Finish(CompiledBlock compiledBlock, struct CompiledTablesStruct *compiledTables, Bool includePseudoOps)
{
	ByteCodeSegment segment = ByteCodeSegment_Create(compiledTables);

	CompiledBlock_CalculateAddresses(compiledBlock, 0, includePseudoOps);
	CompiledBlock_ResolveBranches(compiledBlock);
	CompiledBlock_AppendToByteCodeSegment(compiledBlock, segment, includePseudoOps);

	return segment;
}

/// <summary>
/// Convert the contents of this compiled block to a string.  Note that this
/// *does* alter the block slightly, by computing the actual instruction addresses
/// for it, but other than resolving addresses, the block's contents are unchanged.
/// This is primarily only useful for debugging.
/// </summary>
String CompiledBlock_Stringify(CompiledBlock compiledBlock, struct CompiledTablesStruct *compiledTables)
{
	ByteCodeSegment segment = CompiledBlock_Finish(compiledBlock, compiledTables, True);
	return ByteCodeSegment_Stringify(segment);
}
