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

#include <smile/eval/compiler.h>
#include <smile/eval/compiler_internal.h>
#include <smile/smiletypes/smilelist.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static Bool ValidateTill(Compiler compiler, SmileList args, SmileList *flags, SmileObject *body, SmileList *whens);
static Int CountFlags(Compiler compiler, SmileList flags);
static void DefineVariablesForFlags(Compiler compiler, SmileList flags, Int numFlags, CompileScope tillScope,
	TillContinuationInfo tillInfo, Int tillContinuationVariableIndex, IntermediateInstruction nullLabel);
static Int CompileWhens(Compiler compiler, IntermediateInstruction nullLabel, Int numFlags,
	CompiledBlock parentBlock, CompileScope tillScope,
	SmileList whens, IntermediateInstruction exitTarget,
	Int tillContinuationVariableIndex, CompileFlags compileFlags);
static CompiledBlock CompileNullCase(Compiler compiler, IntermediateInstruction nullLabel, Int numFlags, Int numWhens,
	Int tillContinuationVariableIndex, CompileFlags compileFlags);
static CompiledBlock GenerateNewTillBlock(Compiler compiler, Int numFlags, Int tillContinuationVariableIndex,
	TillContinuationInfo *tillInfo);
static CompiledBlock GenerateEndTillBlock(Compiler compiler, Int tillContinuationVariableIndex);
static Bool IsRealContinuationNeeded(SmileList flags, CompileScope tillScope);
static void PopulateTillBranchTargets(CompileScope tillScope, TillContinuationInfo tillInfo, SmileList flags);
static void RemoveTillContinuation(CompiledBlock compiledBlock,
	IntermediateInstruction tillBlockInstr, CompileScope tillScope, SmileList flags);

// Form: [$till [flag1 flag2 flag3 ...] body [[flag1 when1] [flag2 when2] ...]]
CompiledBlock Compiler_CompileTill(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	SmileList flags;
	SmileObject body;
	SmileList whens;
	CompileScope tillScope;
	Int tillContinuationVariableIndex;
	Int numFlags, numWhens;
	TillContinuationInfo tillInfo;
	Bool realContinuationNeeded;
	CompiledBlock compiledBlock, tillBlock, childBlock, whensBlock, nullBlock;
	IntermediateInstruction tillBlockInstr, loopLabel, loopJmp, nullLabel, exitLabel;

	//---------------------------------------------------------
	// Phase 1.  Validation.

	// Make sure this [$till] is well-formed, with two or three args, one of which is a list of flags.
	if (!ValidateTill(compiler, args, &flags, &body, &whens))
		return CompiledBlock_CreateError();

	//---------------------------------------------------------
	// Phase 2.  Setup.

	compiledBlock = CompiledBlock_Create();

	// Construct a compiler scope for the flag declarations.
	tillScope = Compiler_BeginScope(compiler, PARSESCOPE_TILLDO);

	// Allocate an invisible local variable to hold the special till-escape-continuation object.
	tillContinuationVariableIndex = CompilerFunction_AddLocal(compiler->currentFunction, 0);

	// Count up how many flags there are, since everything depends on knowing about the set of flags.
	numFlags = CountFlags(compiler, flags);

	//---------------------------------------------------------
	// Phase 3.  Compile the when clauses and the null clause.

	// Construct the code to load the till's actual live continuation into the till-continuation variable.
	// This also sets up the TillContinuationInfo object, which will hold all the when-branch targets.
	tillBlock = GenerateNewTillBlock(compiler, numFlags, tillContinuationVariableIndex, &tillInfo);

	// Declare variables for each flag, in the till-scope, and attach those variables to the till-continuation.
	nullLabel = IntermediateInstruction_Create(Op_Label);
	exitLabel = IntermediateInstruction_Create(Op_Label);
	DefineVariablesForFlags(compiler, flags, numFlags, tillScope, tillInfo, tillContinuationVariableIndex, nullLabel);

	// Make the block for the whens.
	whensBlock = CompiledBlock_Create();
	numWhens = CompileWhens(compiler, nullLabel, numFlags, whensBlock, tillScope, whens, exitLabel, tillContinuationVariableIndex, compileFlags);
	nullBlock = CompileNullCase(compiler, nullLabel, numFlags, numWhens, tillContinuationVariableIndex, compileFlags);
	if (nullBlock != NULL)
		CompiledBlock_AppendChild(whensBlock, nullBlock);

	//---------------------------------------------------------
	// Phase 4.  Core loop.

	// Emit the till instruction itself.
	tillBlockInstr = CompiledBlock_AppendChild(compiledBlock, tillBlock);

	// The real "till loop" starts here.
	loopLabel = EMIT0(Op_Label, 0);

	// Evaluate the till loop's body.
	childBlock = Compiler_CompileExpr(compiler, body, compileFlags | COMPILE_FLAG_NORESULT);
	Compiler_EmitNoResult(compiler, childBlock);	// We don't care about the result of eval'ing the body.
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	// Loop back up and do it again.
	loopJmp = EMIT0(Op_Jmp, 0);
	loopJmp->p.branchTarget = loopLabel;

	// The scope with the till flag names is now done.
	Compiler_EndScope(compiler);

	//---------------------------------------------------------
	// Phase 5.  Attach the terminating cases after the core loop.

	CompiledBlock_AppendChild(compiledBlock, whensBlock);
	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, exitLabel);

	//---------------------------------------------------------
	// Phase 6.  Determine if we really need a true continuation or not.

	realContinuationNeeded = IsRealContinuationNeeded(flags, tillScope);
	tillInfo->realContinuationNeeded = realContinuationNeeded;

	//---------------------------------------------------------
	// Phase 7.  Cleanup.

	if (realContinuationNeeded) {
		// We need a true escape continuation for this to work.  So fill in the
		// holes in the TillContinuationInfo with the IntermediateInstructions that
		// will be the branch targets for each flag (symbol).
		PopulateTillBranchTargets(tillScope, tillInfo, flags);
	}
	else {
		// If we really don't need a true escape-continuation for this till (i.e., all
		// exits are through simple jump instructions in the same closure), then go
		// back and remove all the instructions that allocate and manage the actual
		// escape-continuation on the heap at runtime.  This is an optimization, to be
		// sure, but it's a big one, and allows a till that only uses local escapes to
		// run as fast as (or even faster than) a while-loop.
		RemoveTillContinuation(compiledBlock, tillBlockInstr, tillScope, flags);
	}

	return compiledBlock;
}

/// <summary>
/// Validate the [$till] loop, ensuring that it was declared correctly.
/// Returns True if it was declared correctly, or False if it is not well-formed.
/// </summary>
static Bool ValidateTill(Compiler compiler, SmileList args, SmileList *flags, SmileObject *body, SmileList *whens)
{
	// args[0] is the list of till flags. (required)
	// args[1] is the till body. (required)
	// args[2] is a list of whens. (optional)

	// Make sure it's at least of the form [$till [flags...] body ...]
	if (SMILE_KIND(args) != SMILE_KIND_LIST
		|| (SMILE_KIND(args->a) != SMILE_KIND_LIST && SMILE_KIND(args->a) != SMILE_KIND_NULL)
		|| SMILE_KIND(args->d) != SMILE_KIND_LIST) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$till]: Expression is not well-formed.")));
		return False;
	}

	// Get the list of flags.
	*flags = (SmileList)args->a;
	args = (SmileList)args->d;

	// Get the body object.
	*body = args->a;
	args = (SmileList)args->d;

	// Get the list of whens.
	if (SMILE_KIND(args) == SMILE_KIND_LIST) {
		*whens = (SmileList)args->a;
		args = (SmileList)args->d;
	}
	else *whens = NullList;

	// Make sure there are no other arguments.
	if (SMILE_KIND(args) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$till]: Expression is not well-formed.")));
		return False;
	}

	return True;
}

/// <summary>
/// Count how many flags are defined, and make sure that it's a valid list.
/// </summary>
/// <returns>The number of flags, or zero (and adds an error message) if the list is invalid.</returns>
static Int CountFlags(Compiler compiler, SmileList flags)
{
	Int numFlags;

	// Count up how many flags there are, and make sure the list is valid.
	numFlags = SmileList_SafeLength(flags);
	if (numFlags < 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(flags, getSourceLocation),
			String_FromC("Cannot compile [$till]: List of terminating flags is an invalid form.")));
		return 0;
	}
	if (numFlags == 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(flags, getSourceLocation),
			String_FromC("Cannot compile [$till]: List of terminating flags must not be empty.")));
		return 0;
	}

	return numFlags;
}

/// <summary>
/// Spin through each of the flags in the [$till] loop's flags list, and define a special
/// local variable for that flag in the till's scope.  Also, attach each one of those special
/// flag variables to the till-continuation, so that it can be invoked later on.
/// </summary>
static void DefineVariablesForFlags(Compiler compiler, SmileList flags, Int numFlags, CompileScope tillScope,
	TillContinuationInfo tillInfo, Int tillContinuationVariableIndex, IntermediateInstruction nullLabel)
{
	SmileList temp;
	SmileSymbol smileSymbol;
	CompiledTillSymbol compiledTillSymbol, *allCompiledTillSymbols;
	Int index;

	tillInfo->numSymbols = numFlags;

	// Allocate an array to hold all the flags' metadata, and their branch targets.
	tillInfo->symbols = allCompiledTillSymbols = GC_MALLOC_STRUCT_ARRAY(CompiledTillSymbol, numFlags);
	if (allCompiledTillSymbols == NULL)
		Smile_Abort_OutOfMemory();
	tillInfo->branchTargetAddresses = GC_MALLOC_STRUCT_ARRAY(Int32, numFlags);
	if (tillInfo->branchTargetAddresses == NULL)
		Smile_Abort_OutOfMemory();
	tillInfo->branchTargetInstructions = GC_MALLOC_STRUCT_ARRAY(IntermediateInstruction, numFlags);
	if (tillInfo->branchTargetInstructions == NULL)
		Smile_Abort_OutOfMemory();

	// For each flag, construct a flag variable with a CompiledTillSymbol object attached.
	for (temp = flags, index = 0; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = LIST_REST(temp), index++) {
		if (SMILE_KIND(temp->a) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(temp, getSourceLocation),
				String_FromC("Cannot compile [$till]: List of flags must contain only symbols.")));
		}
		smileSymbol = (SmileSymbol)temp->a;

		compiledTillSymbol = (CompiledTillSymbol)CompileScope_DefineSymbol(tillScope, smileSymbol->symbol, PARSEDECL_TILL, tillContinuationVariableIndex);
		compiledTillSymbol->tillIndex = index;
		compiledTillSymbol->whenLabel = nullLabel;
		allCompiledTillSymbols[index] = compiledTillSymbol;
	}

	if (SMILE_KIND(temp) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(flags, getSourceLocation),
			String_FromC("Cannot compile [$till]: List of flags must contain only symbols.")));
	}
}

/// <summary>
/// Emit the instructions necessary to construct a real escape continuation on the heap.
/// This escape continuation contains branch targets for each defined flag, and is a true
/// continuation:  It can be invoked from deep within a nested function, as long as that
/// nested function does so within the till's dynamic extent (this is the definition of
/// an 'escape' continuation; 'escape' continuations must obey stack semantics, acting like
/// non-local returns).
/// </summary>
static CompiledBlock GenerateNewTillBlock(Compiler compiler, Int numFlags, Int tillContinuationVariableIndex,
	TillContinuationInfo *tillInfo)
{
	CompiledBlock compiledBlock = CompiledBlock_Create();
	IntermediateInstruction instr;

	// Construct the till's continuation metadata.
	*tillInfo = Compiler_AddTillContinuationInfo(compiler, compiler->currentFunction->userFunctionInfo, numFlags);

	// Load a "till continuation" into the variable we reserved for it.  The till continuation
	// is only used if child functions need to escape to it.  If no children invoke it, these
	// two instructions will be deleted at the end of all this.
	EMIT1(Op_NewTill, +1, index = (*tillInfo)->tillIndex);
	EMIT1(Op_StpLoc0, -1, index = tillContinuationVariableIndex);

	return compiledBlock;
}

/// <summary>
/// Generate a block that destroys the dynamic extent of the given till, so that a till can only
/// be used as an escape continuation (and not as a way to rewind time itself).
/// </summary>
static CompiledBlock GenerateEndTillBlock(Compiler compiler, Int tillContinuationVariableIndex)
{
	CompiledBlock compiledBlock;
	IntermediateInstruction instr;

	compiledBlock = CompiledBlock_Create();

	EMIT1(Op_LdLoc0, +1, index = tillContinuationVariableIndex);
	EMIT0(Op_EndTill, -1);

	return compiledBlock;
}

/// <summary>
/// Compile all the [when] clauses, and update the till object to point to their emitted
/// instructions.  This returns the number of [when] clauses emitted.
/// </summary>
static Int CompileWhens(Compiler compiler, IntermediateInstruction nullLabel, Int numFlags,
	CompiledBlock parentBlock, CompileScope tillScope,
	SmileList whens, IntermediateInstruction exitTarget,
	Int tillContinuationVariableIndex, CompileFlags compileFlags)
{
	Int numWhens, index;
	Int oldSourceLocation;
	SmileSymbol smileSymbol;
	CompiledTillSymbol compiledTillSymbol;
	IntermediateInstruction instr;
	CompiledBlock compiledBlock, childBlock;
	SmileList whenList;

	if (SMILE_KIND(whens) != SMILE_KIND_LIST)
		return 0;

	numWhens = SmileList_SafeLength(whens);
	if (numWhens < 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(whens, getSourceLocation),
			String_FromC("Cannot compile [$till]: When clauses are not a valid list form.")));
		return 0;
	}
	if (numWhens == 0)
		return 0;

	// Emit all the [when] bodies, and for each, go back and update any branches, and the till continuation,
	// to match their addresses.  Any near branches will get relative offsets, and the till continuation will
	// get an absolute address.
	for (index = 0; SMILE_KIND(whens) == SMILE_KIND_LIST; whens = LIST_REST(whens), index++) {

		whenList = (SmileList)whens->a;

		// Make sure this [when] is well-formed.
		if (SMILE_KIND(whenList) != SMILE_KIND_LIST
			|| SMILE_KIND(whenList->a) != SMILE_KIND_SYMBOL
			|| SMILE_KIND(whenList->d) != SMILE_KIND_LIST
			|| SMILE_KIND(((SmileList)whenList->d)->d) != SMILE_KIND_NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(whens, getSourceLocation),
				String_FromC("Cannot compile [$till]: Whens must be sub-lists of the form [symbol body].")));
			continue;
		}

		// Get the flag object for this [when].  The flag object will have any nearby branches attached to it.
		smileSymbol = (SmileSymbol)whenList->a;
		compiledTillSymbol = (CompiledTillSymbol)CompileScope_FindSymbolHere(tillScope, smileSymbol->symbol);
		if (compiledTillSymbol == NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(whens, getSourceLocation),
				String_Format("Cannot compile [$till]: When clause '%S' does not match any of the loop's flags.",
				SymbolTable_GetName(Smile_SymbolTable, smileSymbol->symbol))));
			continue;
		}
		if (compiledTillSymbol->whenLabel != nullLabel) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(whens, getSourceLocation),
				String_Format("Cannot compile [$till]: When clause '%S' is defined multiple times.",
				SymbolTable_GetName(Smile_SymbolTable, smileSymbol->symbol))));
			continue;
		}

		// Create a new block for the [when] clause.
		compiledBlock = CompiledBlock_Create();
		oldSourceLocation = Compiler_SetSourceLocationFromList(compiler, whenList);

		// Emit the label that will be branched to, either by escape continuation or, if possible, by a local Jmp.
		compiledTillSymbol->whenLabel = EMIT0(Op_Label, 0);

		// End the dynamic extent of the till loop itself.
		CompiledBlock_AppendChild(compiledBlock, GenerateEndTillBlock(compiler, tillContinuationVariableIndex));

		// Compile the body of the [when], and leave it on the stack as the output.
		childBlock = Compiler_CompileExpr(compiler, ((SmileList)whenList->d)->a, compileFlags);
		Compiler_MakeStackMatchCompileFlags(compiler, childBlock, compileFlags);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// Branch to the exit label for this till-loop, if and only if this is not
		// the last when-flag (and no null case is needed).
		if (!(numWhens == numFlags && SMILE_KIND(whens->d) == SMILE_KIND_NULL)) {
			instr = EMIT0(Op_Jmp, 0);
			instr->p.branchTarget = exitTarget;
		}

		// Attach the finished [when] block to the parent's collection.
		CompiledBlock_AppendChild(parentBlock, compiledBlock);

		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
	}

	return numWhens;
}

/// <summary>
/// Generate the till's special null case, if needed.  The null case is used for any till flag that
/// does not have a matching [when] clause.  Returns the label for the null case, or (somewhat
/// ironically) returns NULL if there is no null case.
/// </summary>
static CompiledBlock CompileNullCase(Compiler compiler, IntermediateInstruction nullLabel, Int numFlags, Int numWhens,
	Int tillContinuationVariableIndex, CompileFlags compileFlags)
{
	CompiledBlock compiledBlock;

	// If we emitted fewer when-clauses than we have flags, emit the null case.
	// (This loads 'null' on the stack, and is used for any flag without a 'when'.)
	if (numWhens >= numFlags)
		return NULL;

	// Attach a new block for the null case.
	compiledBlock = CompiledBlock_Create();

	// Add the null label to it.
	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, nullLabel);

	// End the dynamic extent of the till loop itself.
	CompiledBlock_AppendChild(compiledBlock, GenerateEndTillBlock(compiler, tillContinuationVariableIndex));

	// Produce a null if the compile flags expect a result.
	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		EMIT0(Op_LdNull, +1);
	}

	return compiledBlock;
}

/// <summary>
/// Now that we have compiled the body, determine if we need to use a real escape continuation to
/// exit from deep inside some function, or if we can just get away with using a simple Jmp
/// instruction.
/// </summary>
static Bool IsRealContinuationNeeded(SmileList flags, CompileScope tillScope)
{
	SmileSymbol smileSymbol;
	CompiledTillSymbol compiledTillSymbol;
	Bool realContinuationNeeded = False;

	// Finalize each of the flags.
	for (; SMILE_KIND(flags) == SMILE_KIND_LIST; flags = LIST_REST(flags)) {

		smileSymbol = (SmileSymbol)flags->a;
		compiledTillSymbol = (CompiledTillSymbol)CompileScope_FindSymbolHere(tillScope, smileSymbol->symbol);

		if (compiledTillSymbol->base.wasReadDeep || compiledTillSymbol->base.wasWrittenDeep) {
			// Deep access (i.e., from within a child closure).  So we can't just use a simple
			// jump for all the till-loop's branches at this point; we have no choice but to
			// allocate a true continuation for at least one of the flags.
			realContinuationNeeded = True;
		}
	}

	return realContinuationNeeded;
}

/// <summary>
/// When we know we need a real escape continuation for this till to work correctly,
/// we have to fill in its TillContinuationInfo with the target branch instructions, so
/// that the generated 'escape' instructions will be able to branch to the right target.
///
/// Eventually, when the CompiledFunction is converted to actual ByteCode, the array of
/// target instructions will be transformed into an array of instruction offsets.
/// </summary>
static void PopulateTillBranchTargets(CompileScope tillScope, TillContinuationInfo tillInfo, SmileList flags)
{
	SmileList temp;
	Int index;
	CompiledTillSymbol compiledTillSymbol;
	SmileSymbol smileSymbol;

	for (temp = flags, index = 0; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = LIST_REST(temp), index++) {
		smileSymbol = (SmileSymbol)temp->a;
		compiledTillSymbol = (CompiledTillSymbol)CompileScope_FindSymbolHere(tillScope, smileSymbol->symbol);
		tillInfo->branchTargetInstructions[index] = compiledTillSymbol->whenLabel;
	}
}

/// <summary>
/// If we determine that this till doesn't actually need a true escape continuation, then let's
/// not spend CPU or memory at runtime creating or maintaining one!  We do this by removing
/// all of the NewTill/EndTill instructions that we generated, since there are no matching TillEsc
/// instructions that require them.
/// </summary>
static void RemoveTillContinuation(CompiledBlock compiledBlock,
	IntermediateInstruction tillBlockInstr, CompileScope tillScope, SmileList flags)
{
	SmileList temp;
	Int index;
	CompiledTillSymbol compiledTillSymbol;
	SmileSymbol smileSymbol;
	IntermediateInstruction instr;
	CompiledBlock endTillBlock;

	// This till doesn't need an escape continuation or cleanup, so first get rid of
	// the load-till instruction.  This may result in the stack max being too high
	// by one, and will result in an unneeded (empty) local variable slot, but
	// the slightly higher memory consumption is a small price to pay for the
	// substantial improvement in execution time.
	CompiledBlock_DetachInstruction(compiledBlock, tillBlockInstr);

	// Now, spin over each of the branch-target blocks and remove their successive
	// LdLoc0/EndTill blocks, since those aren't needed either.
	for (temp = flags, index = 0; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = LIST_REST(temp), index++) {
		smileSymbol = (SmileSymbol)temp->a;
		compiledTillSymbol = (CompiledTillSymbol)CompileScope_FindSymbolHere(tillScope, smileSymbol->symbol);

		// Get the 'label' instruction.  The EndTill block should immediately follow it.
		instr = compiledTillSymbol->whenLabel->next;

		// Safety check:  Make sure we're removing what we think we're removing, and abort if we aren't.
		endTillBlock = instr->p.childBlock;
		if (instr->opcode != Op_Block
			|| endTillBlock == NULL
			|| !(endTillBlock->numInstructions == 0 && endTillBlock->first == NULL
				|| endTillBlock->numInstructions == 2 && endTillBlock->first->next->opcode == Op_EndTill))
			Smile_Abort_FatalError("Compiler generated an invalid EndTill block.");

		// We don't know enough to detach the Op_Block instruction itself, but we *can* remove
		// its children.
		CompiledBlock_Clear(endTillBlock);
	}
}

/// <summary>
/// Given a collection of TillContinuationInfo objects whose attached IntermediateInstructions
/// have been given actual addresses, copy those addresses into the finished branch-target array,
/// and then release the IntermediateInstructions back to the GC.  This releases most of the
/// TillContinuationInfo back to the GC, leaving behind only the branch targets.
/// </summary>
void Compiler_ResolveTillBranchTargets(TillContinuationInfo *tillInfos, Int numTillInfos)
{
	Int i, j;

	for (i = 0; i < numTillInfos; i++) {

		TillContinuationInfo tillInfo = tillInfos[i];

		for (j = 0; j < tillInfo->numSymbols; j++) {

			if (tillInfo->realContinuationNeeded)
				tillInfo->branchTargetAddresses[j] = tillInfo->branchTargetInstructions[j]->instructionAddress;

			tillInfo->branchTargetInstructions[j] = NULL;
			tillInfo->symbols[j] = NULL;
		}

		if (!tillInfo->realContinuationNeeded)
			tillInfo->branchTargetAddresses = NULL;

		tillInfo->branchTargetInstructions = NULL;
		tillInfo->symbols = NULL;
	}
}
