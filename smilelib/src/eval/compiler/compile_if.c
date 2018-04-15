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

// Form: [$if cond then-clause else-clause]
CompiledBlock Compiler_CompileIf(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	SmileObject condition, thenClause, elseClause, temp;
	Int elseKind;
	Bool not;
	CompiledBlock compiledBlock, condBlock, trueBlock, falseBlock;
	IntermediateInstruction bf, jmp, bfLabel, jmpLabel;
	Int baselineStackDelta;

	// Must be an expression of the form [$if cond then-clause] or [$if cond then-clause else-clause].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return CompiledBlock_Create();
	}

	// Get the condition.
	condition = args->a;

	// Get the then-clause.
	args = (SmileList)args->d;
	thenClause = args->a;

	// Figure out how this form ends, and consume the rest of it.
	if ((elseKind = SMILE_KIND(args->d)) == SMILE_KIND_LIST) {

		// If there's an else-clause, consume it.
		args = (SmileList)args->d;
		elseClause = args->a;

		// This should be the end of the form.
		if ((elseKind = SMILE_KIND(args->d)) != SMILE_KIND_NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
				String_FromC("Cannot compile [if]: Expression is not well-formed.")));
			return CompiledBlock_Create();
		}
	}
	else if (elseKind == SMILE_KIND_NULL) {
		// If no else clause, this should end the form.
		elseClause = NullObject;
	}
	else {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return CompiledBlock_Create();
	}

	// Extract off any [$not] operators, and if there were any, swap then/else clauses.
	not = Compiler_StripNots(&condition);
	if (not) {
		temp = elseClause;
		elseClause = thenClause;
		thenClause = temp;
	}

	// Compile the condition.
	condBlock = Compiler_CompileExpr(compiler, condition, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, condBlock);

	// Compile the 'true' side.
	trueBlock = Compiler_CompileExpr(compiler, thenClause, compileFlags);
	Compiler_MakeStackMatchCompileFlags(compiler, trueBlock, compileFlags);

	// Compile the 'false' side.
	falseBlock = Compiler_CompileExpr(compiler, elseClause, compileFlags);
	Compiler_MakeStackMatchCompileFlags(compiler, falseBlock, compileFlags);

	// Now emit the instructions that best fit this conditional.
	if (trueBlock->first != NULL && falseBlock->first != NULL) {
		// Have both a 'then' clause and an 'else' one, so emit the full 'if'.
		compiledBlock = CompiledBlock_Create();
		CompiledBlock_AppendChild(compiledBlock, condBlock);

		bf = EMIT0(Op_Bf, -1);

		baselineStackDelta = compiledBlock->finalStackDelta;
		CompiledBlock_AppendChild(compiledBlock, trueBlock);
		compiledBlock->finalStackDelta = baselineStackDelta;

		if (trueBlock->blockFlags & BLOCK_FLAG_ESCAPE)
			jmp = NULL;
		else jmp = EMIT0(Op_Jmp, 0);

		bfLabel = EMIT0(Op_Label, 0);
		CompiledBlock_AppendChild(compiledBlock, falseBlock);

		if (trueBlock->blockFlags & BLOCK_FLAG_ESCAPE)
			jmpLabel = NULL;
		else jmpLabel = EMIT0(Op_Label, 0);

		// Point the jumps at the appropriate target labels.
		bf->p.branchTarget = bfLabel;
		bfLabel->p.branchTarget = bf;
		if (!(trueBlock->blockFlags & BLOCK_FLAG_ESCAPE)) {
			jmp->p.branchTarget = jmpLabel;
			jmpLabel->p.branchTarget = jmp;
		}

		return compiledBlock;
	}
	else if (trueBlock->first != NULL) {
		// Have only a 'then' clause, with no 'else' clause.
		compiledBlock = CompiledBlock_Create();
		CompiledBlock_AppendChild(compiledBlock, condBlock);
		bf = EMIT0(Op_Bf, -1);
		CompiledBlock_AppendChild(compiledBlock, trueBlock);
		bfLabel = EMIT0(Op_Label, 0);

		// Point the jumps at the appropriate target labels.
		bf->p.branchTarget = bfLabel;
		bfLabel->p.branchTarget = bf;

		return compiledBlock;
	}
	else if (falseBlock->first != NULL) {
		// Have only an 'else' clause, with no 'then' clause.
		compiledBlock = CompiledBlock_Create();
		CompiledBlock_AppendChild(compiledBlock, condBlock);
		bf = EMIT0(Op_Bt, -1);
		CompiledBlock_AppendChild(compiledBlock, falseBlock);
		bfLabel = EMIT0(Op_Label, 0);

		// Point the jumps at the appropriate target labels.
		bf->p.branchTarget = bfLabel;
		bfLabel->p.branchTarget = bf;

		return compiledBlock;
	}
	else {
		// Have neither clause, which means we can recompile the condition in no-output mode.
		condBlock = Compiler_CompileExpr(compiler, condition, compileFlags);
		Compiler_EmitNoResult(compiler, condBlock);
		return condBlock;
	}
}
