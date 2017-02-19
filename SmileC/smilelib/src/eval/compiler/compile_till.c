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

// Form: [$till [flag1 flag2 flag3 ...] body [[flag1 when1] [flag2 when2] ...]]
void Compiler_CompileTill(Compiler compiler, SmileList args)
{
	// args[0] is the list of till flags. (required)
	// args[1] is the till body. (required)
	// args[2] is a list of whens. (optional)
	SmileList flags;
	SmileObject body;
	SmileList whens;
	CompileScope compileScope;
	SmileSymbol smileSymbol;
	Int offset;
	Int tillContinuationVariableIndex, index;
	Int loopLabel, loopJmp;
	Int nullLabel, exitLabel;
	Int numFlags, numWhens;
	Int loadTill;
	Int initialStackDepth;
	CompiledTillSymbol *flagSymbols;
	CompiledTillSymbol compiledLocalSymbol;
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	TillContinuationInfo tillInfo;
	Bool realContinuationNeeded;

	//---------------------------------------------------------
	// Phase 1.  Validation.

	// Make sure it's at least of the form [$till [flags...] body ...]
	if (SMILE_KIND(args) != SMILE_KIND_LIST
		|| (SMILE_KIND(args->a) != SMILE_KIND_LIST && SMILE_KIND(args->a) != SMILE_KIND_NULL)
		|| SMILE_KIND(args->d) != SMILE_KIND_LIST) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$till]: Expression is not well-formed.")));
		return;
	}

	// Get the list of flags.
	flags = (SmileList)args->a;
	args = (SmileList)args->d;

	// Get the body object.
	body = args->a;
	args = (SmileList)args->d;

	// Get the list of whens.
	if (SMILE_KIND(args) == SMILE_KIND_LIST) {
		whens = (SmileList)args->a;
		args = (SmileList)args->d;
	}
	else whens = NullList;

	// Make sure there are no other arguments.
	if (SMILE_KIND(args) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$till]: Expression is not well-formed.")));
		return;
	}

	//---------------------------------------------------------
	// Phase 2.  Setup.

	// Construct a compiler scope for the flag declarations.
	compileScope = Compiler_BeginScope(compiler, PARSESCOPE_TILLDO);

	// Allocate an invisible local variable to hold the special till-escape-continuation object.
	tillContinuationVariableIndex = CompilerFunction_AddLocal(compiler->currentFunction, 0);

	// Allocate an array to hold information for each of the till-loop's flags.
	numFlags = SmileList_Length(flags);
	if (numFlags <= 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(flags),
			String_FromC("Cannot compile [$till]: List of terminating flags must not be empty.")));
	}
	flagSymbols = GC_MALLOC_STRUCT_ARRAY(CompiledTillSymbol, numFlags);
	if (flagSymbols == NULL)
		Smile_Abort_OutOfMemory();

	// Populate the flags array with a CompiledTillSymbol object for each flag.
	index = 0;
	for (; SMILE_KIND(flags) == SMILE_KIND_LIST; flags = LIST_REST(flags)) {
		if (SMILE_KIND(flags->a) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(flags),
				String_FromC("Cannot compile [$till]: List of flags must contain only symbols.")));
		}
		smileSymbol = (SmileSymbol)flags->a;

		compiledLocalSymbol = (CompiledTillSymbol)CompileScope_DefineSymbol(compileScope, smileSymbol->symbol, PARSEDECL_TILL, tillContinuationVariableIndex);
		compiledLocalSymbol->tillIndex = index;
		compiledLocalSymbol->whenLabelAddress = 0;
		compiledLocalSymbol->exitJmpAddress = 0;
		compiledLocalSymbol->firstJmp = NULL;

		flagSymbols[index++] = compiledLocalSymbol;
	}

	// Record the initial stack depth.  The 'when' clauses all start at this depth,
	// and leave exactly one object on the stack.
	initialStackDepth = compiler->currentFunction->currentStackDepth;

	//---------------------------------------------------------
	// Phase 3.  Core loop.

	// Construct the till's continuation metadata.
	tillInfo = Compiler_AddTillContinuationInfo(compiler, compiler->currentFunction->userFunctionInfo, numFlags);

	// Load a "till continuation" into the variable we reserved for it.  The till continuation
	// is only used if child functions need to escape to it.  If no children invoke it, these
	// two instructions will be replaced with simple NOP instructions at the end of all this.
	// (Subsequent peephole optimizations can remove the NOPs.)
	loadTill = EMIT1(Op_LdTill, +1, index = tillInfo->tillIndex);
	EMIT1(Op_StpLoc0, -1, index = tillContinuationVariableIndex);

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
	// Phase 4.  When clauses and the implicit default clause.

	numWhens = 0;
	if (SMILE_KIND(whens) == SMILE_KIND_LIST) {
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
				Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(whens),
					String_FromC("Cannot compile [$till]: Whens must be sub-lists of the form [symbol body].")));
				continue;
			}

			// Get the flag object for this [when].  The flag object will have any nearby branches attached to it.
			smileSymbol = (SmileSymbol)((SmileList)whens->a)->a;
			compiledLocalSymbol = (CompiledTillSymbol)CompileScope_FindSymbolHere(compileScope, smileSymbol->symbol);
			if (compiledLocalSymbol == NULL) {
				Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(whens),
					String_Format("Cannot compile [$till]: When clause '%S' does not match any of the loop's flags.",
					SymbolTable_GetName(Smile_SymbolTable, smileSymbol->symbol))));
				continue;
			}
			if (compiledLocalSymbol->whenLabelAddress) {
				Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(whens),
					String_Format("Cannot compile [$till]: When clause '%S' is defined multiple times.",
					SymbolTable_GetName(Smile_SymbolTable, smileSymbol->symbol))));
				continue;
			}

			// Emit the label that will be branched to, either by escape continuation or, if possible, by local Jmp.
			compiledLocalSymbol->whenLabelAddress = EMIT0(Op_Label, 0);

			// Compile the body of the [when], and leave it on the stack as the output.
			Compiler_CompileExpr(compiler, ((SmileList)((SmileList)whens->a)->d)->a);

			// Branch to the exit label for this till-loop.
			compiledLocalSymbol->exitJmpAddress = EMIT0(Op_Jmp, 0);

			numWhens++;
		}
	}

	// Reset the stack.
	compiler->currentFunction->currentStackDepth = initialStackDepth;

	// If we emitted fewer when-clauses than we have flags, emit the null case.
	// (This loads 'null' on the stack, and is used for any flag without a 'when'.)
	if (numWhens < numFlags) {
		nullLabel = EMIT0(Op_Label, 0);
		EMIT0(Op_LdNull, +1);
	}
	else nullLabel = -1;

	// Finally, if we at least one when clause, then also emit the exit label, which continues
	// the program past this point.
	if (numWhens > 0) {
		exitLabel = EMIT0(Op_Label, 0);
	}
	else exitLabel = -1;

	//---------------------------------------------------------
	// Phase 5.  Branch resolution.

	// Finalize each of the flags.
	realContinuationNeeded = False;
	for (; SMILE_KIND(flags) == SMILE_KIND_LIST; flags = LIST_REST(flags)) {
		TillFlagJmp jumps;

		smileSymbol = (SmileSymbol)flags->a;
		compiledLocalSymbol = (CompiledTillSymbol)CompileScope_FindSymbolHere(compileScope, smileSymbol->symbol);

		if (compiledLocalSymbol->base.wasReadDeep || compiledLocalSymbol->base.wasWrittenDeep) {
			// Deep access (i.e., from within a child closure).  So we can't just use a simple
			// jump for all the till-loop's branches at this point; we have no choice but to
			// allocate a true continuation for at least one of the flags.
			realContinuationNeeded = True;
		}

		if (!compiledLocalSymbol->whenLabelAddress) {
			// Update any flags that don't point to a [when] to point to the nullLabel instead, so
			// that everything has a defined branch target.
			compiledLocalSymbol->whenLabelAddress = exitLabel;
			compiledLocalSymbol->exitJmpAddress = 0;
		}
		else {
			// Go back and update any [when] branches' tail jumps to point to the exit label.
			segment->byteCodes[compiledLocalSymbol->exitJmpAddress].u.index = exitLabel - compiledLocalSymbol->exitJmpAddress;
		}

		// Go to any emitted unconditional jumps in this segment and correct them to point to
		// their respective when/null targets.
		for (jumps = compiledLocalSymbol->firstJmp; jumps != NULL; jumps = jumps->next) {
			segment->byteCodes[jumps->offset].u.index = compiledLocalSymbol->whenLabelAddress - jumps->offset;
		}

		// Update the till's continuation metadata to point to the appropriate exit clause for this flag.
		tillInfo->offsets[compiledLocalSymbol->tillIndex] = compiledLocalSymbol->whenLabelAddress;
	}

	//---------------------------------------------------------
	// Phase 6.  Cleanup.

	// If we really don't need a true escape-continuation for this till (i.e., all exits are through
	// simple jump instructions in the same closure), then remove (using NOPs) the initial instructions
	// that would allocate a true escape-continuation.  (The substituted NOPs will still be much faster
	// than the escape-continuation's heap allocation would be.)
	if (!realContinuationNeeded) {
		segment->byteCodes[loadTill].opcode = Op_Nop;
		segment->byteCodes[loadTill + 1].opcode = Op_Nop;
	}
}
