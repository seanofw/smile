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
static Int CompileWhens(Compiler compiler, CompileScope tillScope, SmileList whens, Int initialStackDepth);
static Int GenerateNullCase(Compiler compiler, Int initialStackDepth, Int numFlags, Int numWhens);
static Int GenerateExitLabel(Compiler compiler, Int initialStackDepth, Int numWhens);
static Bool ResolveBranches(Compiler compiler, SmileList flags, CompileScope tillScope, TillContinuationInfo tillInfo, Int exitLabel);
static void ResolveWhenJumps(ByteCodeSegment segment, CompiledTillSymbol compiledTillSymbol);
static void GenerateTillContinuation(Compiler compiler, Int numFlags, Int tillContinuationVariableIndex,
	Int *loadTillAddress, TillContinuationInfo *tillInfo);
static void FinalizeTillContinuation(Compiler compiler, Int loadTillAddress, Int tillContinuationVariableindex,
	Bool realContinuationNeeded);

// Form: [$till [flag1 flag2 flag3 ...] body [[flag1 when1] [flag2 when2] ...]]
void Compiler_CompileTill(Compiler compiler, SmileList args)
{
	SmileList flags;
	SmileObject body;
	SmileList whens;
	CompileScope tillScope;
	Int offset;
	Int tillContinuationVariableIndex;
	Int loopLabel, loopJmp;
	Int nullLabel, exitLabel;
	Int numFlags, numWhens;
	Int loadTillAddress;
	Int initialStackDepth;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	TillContinuationInfo tillInfo;
	Bool realContinuationNeeded;

	//---------------------------------------------------------
	// Phase 1.  Validation.

	// Make sure this [$till] is well-formed, with two or three args, one of which is a list of flags.
	if (!ValidateTill(compiler, args, &flags, &body, &whens))
		return;

	//---------------------------------------------------------
	// Phase 2.  Setup.

	// Construct a compiler scope for the flag declarations.
	tillScope = Compiler_BeginScope(compiler, PARSESCOPE_TILLDO);

	// Allocate an invisible local variable to hold the special till-escape-continuation object.
	tillContinuationVariableIndex = CompilerFunction_AddLocal(compiler->currentFunction, 0);

	// Declare variables for each flag, in the till-scope.
	numFlags = DefineVariablesForFlags(compiler, flags, tillScope, tillContinuationVariableIndex);

	// Record the initial stack depth.  The 'when' clauses all start at this depth,
	// and leave exactly one object on the stack.
	initialStackDepth = compiler->currentFunction->currentStackDepth;

	//---------------------------------------------------------
	// Phase 3.  Core loop.

	// Emit code to load the till's actual live continuation into the till-continuation variable.
	GenerateTillContinuation(compiler, numFlags, tillContinuationVariableIndex, &loadTillAddress, &tillInfo);

	// The real "till loop" starts here.
	loopLabel = EMIT0(Op_Label, 0);

	// Evaluate the till loop's body.
	Compiler_CompileExpr(compiler, body);
	Compiler_EmitPop1(compiler);	// We don't care about the result of eval'ing the body.

	// Loop back up and do it again.
	loopJmp = EMIT0(Op_Jmp, 0);
	FIX_BRANCH(loopJmp, loopJmp - loopLabel);

	Compiler_EndScope(compiler);

	//---------------------------------------------------------
	// Phase 4.  When clauses and the null clause.

	numWhens = CompileWhens(compiler, tillScope, whens, initialStackDepth);
	nullLabel = GenerateNullCase(compiler, initialStackDepth, numFlags, numWhens);
	exitLabel = GenerateExitLabel(compiler, initialStackDepth, numWhens);

	// After all the when/null clauses, there will be exactly one item left on the stack.
	compiler->currentFunction->currentStackDepth = initialStackDepth;
	ApplyStackDelta(compiler->currentFunction, +1);

	//---------------------------------------------------------
	// Phase 5.  Branch resolution.

	realContinuationNeeded = ResolveBranches(compiler, flags, tillScope, tillInfo, exitLabel);

	//---------------------------------------------------------
	// Phase 6.  Cleanup.

	// If we really don't need a true escape-continuation for this till (i.e., all exits are through
	// simple jump instructions in the same closure), then remove the instructions that allocate an
	// escape-continuation on the heap.  Otherwise, mark the 'till' continuation as finished.
	FinalizeTillContinuation(compiler, loadTillAddress, tillContinuationVariableIndex, realContinuationNeeded);
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
		compiledTillSymbol->whenLabelAddress = 0;
		compiledTillSymbol->exitJmpAddress = 0;
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
static void GenerateTillContinuation(Compiler compiler, Int numFlags, Int tillContinuationVariableIndex,
	Int *loadTillAddress, TillContinuationInfo *tillInfo)
{
	Int offset;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;

	// Construct the till's continuation metadata.
	*tillInfo = Compiler_AddTillContinuationInfo(compiler, compiler->currentFunction->userFunctionInfo, numFlags);

	// Load a "till continuation" into the variable we reserved for it.  The till continuation
	// is only used if child functions need to escape to it.  If no children invoke it, these
	// two instructions will be replaced with simple NOP instructions at the end of all this.
	// (Subsequent peephole optimizations can remove the NOPs.)
	*loadTillAddress = EMIT1(Op_NewTill, +1, index = (*tillInfo)->tillIndex);
	EMIT1(Op_StpLoc0, -1, index = tillContinuationVariableIndex);
}

/// <summary>
/// Compile all the [when] clauses, and update the till object to point to their emitted
/// instructions.  This returns the number of [when] clauses emitted.  (Note:  This damages
/// the stack-depth counter.)
/// </summary>
static Int CompileWhens(Compiler compiler, CompileScope tillScope, SmileList whens, Int initialStackDepth)
{
	Int numWhens = 0;
	Int offset;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	SmileSymbol smileSymbol;
	CompiledTillSymbol compiledTillSymbol;

	if (SMILE_KIND(whens) != SMILE_KIND_LIST)
		return 0;

	// Emit all the [when] bodies, and for each, go back and update any branches, and the till continuation,
	// to match their addresses.  Any near branches will get relative offsets, and the till continuation will
	// get an absolute address.
	for (; SMILE_KIND(whens) == SMILE_KIND_LIST; whens = LIST_REST(whens)) {

		// Reset the stack.
		compiler->currentFunction->currentStackDepth = initialStackDepth;

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
		if (compiledTillSymbol->whenLabelAddress) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(whens, getSourceLocation),
				String_Format("Cannot compile [$till]: When clause '%S' is defined multiple times.",
				SymbolTable_GetName(Smile_SymbolTable, smileSymbol->symbol))));
			continue;
		}

		// Emit the label that will be branched to, either by escape continuation or, if possible, by local Jmp.
		compiledTillSymbol->whenLabelAddress = EMIT0(Op_Label, 0);

		// Compile the body of the [when], and leave it on the stack as the output.
		Compiler_CompileExpr(compiler, ((SmileList)((SmileList)whens->a)->d)->a);

		// Branch to the exit label for this till-loop.
		compiledTillSymbol->exitJmpAddress = EMIT0(Op_Jmp, 0);

		numWhens++;
	}

	return numWhens;
}

/// <summary>
/// Generate the till's special null case, if needed.  The null case is used for any till flag that
/// does not have a matching [when] clause.  Returns the absolute address in the segment of the
/// null case's Op_Label, if it is emitted (-1 if it is not).  (Note:  This damages the stack-depth counter.)
/// </summary>
static Int GenerateNullCase(Compiler compiler, Int initialStackDepth, Int numFlags, Int numWhens)
{
	Int nullLabel;
	Int offset;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;

	compiler->currentFunction->currentStackDepth = initialStackDepth;

	// If we emitted fewer when-clauses than we have flags, emit the null case.
	// (This loads 'null' on the stack, and is used for any flag without a 'when'.)
	if (numWhens < numFlags) {
		nullLabel = EMIT0(Op_Label, 0);
		EMIT0(Op_LdNull, +1);
	}
	else nullLabel = -1;

	return nullLabel;
}

/// <summary>
/// Generate the till's exit label, if it is needed.  It is needed if one or more
/// [when] clauses is emitted, since they branch to it; if only the null case is emitted,
/// then we don't need the exit label.  Returns the absolute address of the exit label,
/// if it is emitted (-1 if it is not).  (Note:  This damages the stack-depth counter.)
/// </summary>
static Int GenerateExitLabel(Compiler compiler, Int initialStackDepth, Int numWhens)
{
	Int offset;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;

	compiler->currentFunction->currentStackDepth = initialStackDepth;

	// If we emitted at least one when clause, then also emit the exit label, which continues
	// the program past the till loop.
	if (numWhens > 0)
		return EMIT0(Op_Label, 0);
	else
		return -1;
}

/// <summary>
/// Given a simple linked-list of TillFlagJmps attached to a given till-flag symbol, spin
/// over that list and update those Op_Jmps to point to the till-flag's when label (or the
/// null label, as appropriate).
/// </summary>
static void ResolveWhenJumps(ByteCodeSegment segment, CompiledTillSymbol compiledTillSymbol)
{
	TillFlagJmp jumps;

	for (jumps = compiledTillSymbol->firstJmp; jumps != NULL; jumps = jumps->next) {
		segment->byteCodes[jumps->offset].u.index = compiledTillSymbol->whenLabelAddress - jumps->offset;
	}
}

/// <summary>
/// For all of the flags in this [$till] form, go back and resolve all their unconditional branch
/// offsets.  This updates each [when] clause to branch to the exit label; and updates any unconditional
/// flag jumps to point to their respective [when] clauses (or to the null label as necessary).  This
/// also updates the till-continuation's info to point to each of the labels of the compiled when-clauses.
/// This returns true if a real continuation is needed by this till-loop, or false if the till-loop uses
/// exclusively local Op_Jmps.
/// </summary>
static Bool ResolveBranches(Compiler compiler, SmileList flags, CompileScope tillScope, TillContinuationInfo tillInfo, Int exitLabel)
{
	SmileSymbol smileSymbol;
	CompiledTillSymbol compiledTillSymbol;
	Bool realContinuationNeeded;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;

	realContinuationNeeded = False;

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

		if (!compiledTillSymbol->whenLabelAddress) {
			// Update any flags that don't point to a [when] to point to the nullLabel instead, so
			// that everything has a defined branch target.
			compiledTillSymbol->whenLabelAddress = exitLabel;
			compiledTillSymbol->exitJmpAddress = 0;
		}
		else {
			// Go back and update any [when] branches' tail jumps to point to the exit label.
			segment->byteCodes[compiledTillSymbol->exitJmpAddress].u.index = exitLabel - compiledTillSymbol->exitJmpAddress;
		}

		// Go to any emitted unconditional jumps in this segment and correct them to point to
		// their respective when/null targets.
		ResolveWhenJumps(segment, compiledTillSymbol);

		// Update the till's continuation metadata to point to the appropriate exit clause for this flag.
		tillInfo->offsets[compiledTillSymbol->tillIndex] = compiledTillSymbol->whenLabelAddress;
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
static void FinalizeTillContinuation(Compiler compiler, Int loadTillAddress, Int tillContinuationVariableIndex,
	Bool realContinuationNeeded)
{
	Int offset;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;

	if (realContinuationNeeded) {
		// Mark this till has having its dynamic extent now exited.
		EMIT1(Op_LdLoc0, +1, index = tillContinuationVariableIndex);
		EMIT0(Op_EndTill, -1);
	}
	else {
		// This till doesn't need an escape continuation or cleanup.
		segment->byteCodes[loadTillAddress].opcode = Op_Nop;
		segment->byteCodes[loadTillAddress + 1].opcode = Op_Nop;
	}
}
