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
#include <smile/smiletypes/smilepair.h>
#include <smile/smiletypes/text/smilesymbol.h>
#include <smile/parsing/parsemessage.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static Bool ValidateTill(Compiler compiler, SmileList args, SmileList *flags, SmileObject *body, SmileList *whens);
static Int DefineVariablesForFlags(Compiler compiler, SmileList flags, CompileScope tillScope, Int tillContinuationVariableIndex);
static Int CompileWhens(Compiler compiler, CompiledBlock parentBlock, CompileScope tillScope, SmileList whens,
	IntermediateInstruction exitTarget, CompileFlags compileFlags);
static IntermediateInstruction GenerateNullCase(Compiler compiler, Int numFlags, Int numWhens,
	CompiledBlock parentBlock, CompileFlags compileFlags);
static CompiledBlock GenerateTillContinuation(Compiler compiler, Int numFlags, Int tillContinuationVariableIndex,
	TillContinuationInfo *tillInfo);
static Bool IsRealContinuationNeeded(Compiler compiler, SmileList flags, CompileScope tillScope);
static void FinalizeTillContinuation(Compiler compiler, CompiledBlock compiledBlock, IntermediateInstruction tillBlockInstr,
	Int tillContinuationVariableIndex, Bool realContinuationNeeded);

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
	CompiledBlock compiledBlock, tillBlock, childBlock;
	IntermediateInstruction instr, tillBlockInstr, loopLabel, loopJmp, nullLabel, exitLabel;

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

	// Declare variables for each flag, in the till-scope.
	numFlags = DefineVariablesForFlags(compiler, flags, tillScope, tillContinuationVariableIndex);

	exitLabel = IntermediateInstruction_Create(Op_Label);

	//---------------------------------------------------------
	// Phase 3.  Core loop.

	// Emit code to load the till's actual live continuation into the till-continuation variable.
	tillBlock = GenerateTillContinuation(compiler, numFlags, tillContinuationVariableIndex, &tillInfo);
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
	// Phase 4.  When clauses and the null clause.

	numWhens = CompileWhens(compiler, compiledBlock, tillScope, whens, exitLabel, compileFlags);
	nullLabel = GenerateNullCase(compiler, numFlags, numWhens, compiledBlock, compileFlags);
	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, exitLabel);

	//---------------------------------------------------------
	// Phase 5.  Determine if we really need a true continuation or not.

	realContinuationNeeded = IsRealContinuationNeeded(compiler, flags, tillScope);

	//---------------------------------------------------------
	// Phase 6.  Cleanup.

	// If we really don't need a true escape-continuation for this till (i.e., all exits are through
	// simple jump instructions in the same closure), then remove the instructions that allocate an
	// escape-continuation on the heap.  Otherwise, mark the 'till' continuation as finished.
	FinalizeTillContinuation(compiler, compiledBlock, tillBlockInstr, tillContinuationVariableIndex, realContinuationNeeded);
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
/// Spin through each of the flags in the [$till] loop's flags list, and define a special
/// local variable for that flag in the till's scope.  Also, attach each one of those special
/// flag variables to the till-continuation, so that it can be invoked later on.
/// </summary>
static Int DefineVariablesForFlags(Compiler compiler, SmileList flags, CompileScope tillScope, Int tillContinuationVariableIndex)
{
	SmileList temp;
	SmileSymbol smileSymbol;
	CompiledTillSymbol compiledTillSymbol;
	Int numFlags;

	// For each flag, construct a flag variable with a CompiledTillSymbol object attached.
	for (temp = flags, numFlags = 0; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = LIST_REST(temp), numFlags++) {
		if (SMILE_KIND(temp->a) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(temp, getSourceLocation),
				String_FromC("Cannot compile [$till]: List of flags must contain only symbols.")));
		}
		smileSymbol = (SmileSymbol)temp->a;

		compiledTillSymbol = (CompiledTillSymbol)CompileScope_DefineSymbol(tillScope, smileSymbol->symbol, PARSEDECL_TILL, tillContinuationVariableIndex);
		compiledTillSymbol->tillIndex = numFlags;
		compiledTillSymbol->whenLabel = NULL;
		compiledTillSymbol->firstJmp = NULL;
	}

	if (SMILE_KIND(temp) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(flags, getSourceLocation),
			String_FromC("Cannot compile [$till]: List of flags must contain only symbols.")));
	}

	if (numFlags <= 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(flags, getSourceLocation),
			String_FromC("Cannot compile [$till]: List of terminating flags must not be empty.")));
	}

	return numFlags;
}

/// <summary>
/// Emit the instructions necessary to construct a real escape continuation on the heap.
/// This escape continuation contains branch targets for each defined flag, and is a true
/// continuation:  It can be invoked from deep within a nested function, as long as that
/// nested function does so within the till's dynamic extent (this is the definition of
/// an 'escape' continuation; 'escape' continuations must obey stack semantics, acting like
/// non-local returns).
/// </summary>
static CompiledBlock GenerateTillContinuation(Compiler compiler, Int numFlags, Int tillContinuationVariableIndex,
	TillContinuationInfo *tillInfo)
{
	CompiledBlock compiledBlock = CompiledBlock_Create();
	IntermediateInstruction instr;

	// Construct the till's continuation metadata.
	*tillInfo = Compiler_AddTillContinuationInfo(compiler, compiler->currentFunction->userFunctionInfo, numFlags);

	// Load a "till continuation" into the variable we reserved for it.  The till continuation
	// is only used if child functions need to escape to it.  If no children invoke it, these
	// two instructions will be replaced with simple NOP instructions at the end of all this.
	// (Subsequent peephole optimizations can remove the NOPs.)
	EMIT1(Op_NewTill, +1, index = (*tillInfo)->tillIndex);
	EMIT1(Op_StpLoc0, -1, index = tillContinuationVariableIndex);

	return compiledBlock;
}

/// <summary>
/// Compile all the [when] clauses, and update the till object to point to their emitted
/// instructions.  This returns the number of [when] clauses emitted.
/// </summary>
static Int CompileWhens(Compiler compiler, CompiledBlock parentBlock, CompileScope tillScope, SmileList whens,
	IntermediateInstruction exitTarget, CompileFlags compileFlags)
{
	Int numWhens = 0;
	SmileSymbol smileSymbol;
	CompiledTillSymbol compiledTillSymbol;
	IntermediateInstruction instr;
	CompiledBlock compiledBlock, childBlock;

	if (SMILE_KIND(whens) != SMILE_KIND_LIST)
		return 0;

	// Emit all the [when] bodies, and for each, go back and update any branches, and the till continuation,
	// to match their addresses.  Any near branches will get relative offsets, and the till continuation will
	// get an absolute address.
	for (; SMILE_KIND(whens) == SMILE_KIND_LIST; whens = LIST_REST(whens)) {

		// Make sure this [when] is well-formed.
		if (SMILE_KIND(whens->a) != SMILE_KIND_LIST
			|| SMILE_KIND(((SmileList)whens->a)->a) != SMILE_KIND_SYMBOL
			|| SMILE_KIND(((SmileList)whens->a)->d) != SMILE_KIND_LIST
			|| SMILE_KIND(((SmileList)((SmileList)whens->a)->d)->d) != SMILE_KIND_NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(whens, getSourceLocation),
				String_FromC("Cannot compile [$till]: Whens must be sub-lists of the form [symbol body].")));
			continue;
		}

		// Get the flag object for this [when].  The flag object will have any nearby branches attached to it.
		smileSymbol = (SmileSymbol)((SmileList)whens->a)->a;
		compiledTillSymbol = (CompiledTillSymbol)CompileScope_FindSymbolHere(tillScope, smileSymbol->symbol);
		if (compiledTillSymbol == NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(whens, getSourceLocation),
				String_Format("Cannot compile [$till]: When clause '%S' does not match any of the loop's flags.",
				SymbolTable_GetName(Smile_SymbolTable, smileSymbol->symbol))));
			continue;
		}
		if (compiledTillSymbol->whenLabel != NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(whens, getSourceLocation),
				String_Format("Cannot compile [$till]: When clause '%S' is defined multiple times.",
				SymbolTable_GetName(Smile_SymbolTable, smileSymbol->symbol))));
			continue;
		}

		// Attach a new block for the [when] clause.
		compiledBlock = CompiledBlock_Create();
		CompiledBlock_AppendChild(parentBlock, compiledBlock);

		// Emit the label that will be branched to, either by escape continuation or, if possible, by a local Jmp.
		compiledTillSymbol->whenLabel = EMIT0(Op_Label, 0);

		// Compile the body of the [when], and leave it on the stack as the output.
		childBlock = Compiler_CompileExpr(compiler, ((SmileList)((SmileList)whens->a)->d)->a, compileFlags);
		Compiler_MakeStackMatchCompileFlags(compiler, childBlock, compileFlags);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// Branch to the exit label for this till-loop.
		instr = EMIT0(Op_Jmp, 0);
		instr->p.branchTarget = exitTarget;

		numWhens++;
	}

	return numWhens;
}

/// <summary>
/// Generate the till's special null case, if needed.  The null case is used for any till flag that
/// does not have a matching [when] clause.  Returns the label for the null case, or (somewhat
/// ironically) returns NULL if there is no null case.
/// </summary>
static IntermediateInstruction GenerateNullCase(Compiler compiler, Int numFlags, Int numWhens,
	CompiledBlock parentBlock, CompileFlags compileFlags)
{
	IntermediateInstruction nullLabel;
	CompiledBlock compiledBlock;

	// If we emitted fewer when-clauses than we have flags, emit the null case.
	// (This loads 'null' on the stack, and is used for any flag without a 'when'.)
	if (numWhens >= numFlags)
		return NULL;

	// Attach a new block for the null case.
	compiledBlock = CompiledBlock_Create();
	CompiledBlock_AppendChild(parentBlock, compiledBlock);

	// Add a label to it, and produce a null if the compile flags expect a result.
	nullLabel = EMIT0(Op_Label, 0);
	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		EMIT0(Op_LdNull, +1);
	}

	return nullLabel;
}

/// <summary>
/// Now that we have compiled the body, determine if we need to use a real escape continuation to
/// exit from deep inside some function, or if we can just get away with using a simple Jmp
/// instruction.
/// </summary>
static Bool IsRealContinuationNeeded(Compiler compiler, SmileList flags, CompileScope tillScope)
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
/// If we've emitted an unnecessary true continuation for this till, go back and un-emit it
/// by replacing it with NOPs.  The NOPs take time, but considerably less than allocating
/// an unnecessary real escape continuation on the heap.  Otherwise, add an instruction that
/// causes the dynamic extent of 'till' to be marked as concluded, so that the till's
/// continuation may only be used as an 'escape' continuation.
/// </summary>
static void FinalizeTillContinuation(Compiler compiler, CompiledBlock compiledBlock, IntermediateInstruction tillBlockInstr,
	Int tillContinuationVariableIndex, Bool realContinuationNeeded)
{
	IntermediateInstruction instr;

	if (realContinuationNeeded) {
		// Mark this till has having its dynamic extent now exited.
		EMIT1(Op_LdLoc0, +1, index = tillContinuationVariableIndex);
		EMIT0(Op_EndTill, -1);
	}
	else {
		// This till doesn't need an escape continuation or cleanup, so get rid of
		// the load-till instruction.  This may result in the stack max being too high
		// by one, but the likelihood that the max wasn't matched or exceeded by something
		// else inside the loop is very low (almost impossible), so it's no big deal
		// that we're off here:  Worst-case scenario, we waste one word of memory.
		CompiledBlock_DetachInstruction(compiledBlock, tillBlockInstr);
	}
}
