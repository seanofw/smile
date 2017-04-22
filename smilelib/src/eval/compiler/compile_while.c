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

static CompiledBlock CompileWhileWithPreAndPost(Compiler compiler, Bool not, SmileObject condition, SmileObject preClause, SmileObject postClause, CompileFlags compileFlags);
static CompiledBlock CompileWhileWithPre(Compiler compiler, Bool not, SmileObject condition, SmileObject preClause, CompileFlags compileFlags);
static CompiledBlock CompileWhileWithPost(Compiler compiler, Bool not, SmileObject condition, SmileObject postClause, CompileFlags compileFlags);
static CompiledBlock CompileWhileWithNoBody(Compiler compiler, Bool not, SmileObject condition, CompileFlags compileFlags);

// Form: [$while pre-body cond post-body]
CompiledBlock Compiler_CompileWhile(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	SmileObject condition, preClause, postClause;
	Int postKind, tailKind;
	Bool not, hasPre, hasPost;

	// Must be an expression of the form [$while cond postBody] or [$while pre-body cond post-body].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	// Get the condition.
	condition = args->a;

	// Get the body.
	args = (SmileList)args->d;
	postClause = args->a;

	// Figure out how this form ends, and consume the rest of it.
	if ((postKind = SMILE_KIND(args->d)) == SMILE_KIND_LIST) {

		// There are three parts, so this is a pre-cond-post form.
		args = (SmileList)args->d;
		preClause = condition;
		condition = postClause;
		postClause = args->a;

		// This should be the end of the form.
		if ((tailKind = SMILE_KIND(args->d)) != SMILE_KIND_NULL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
				String_FromC("Cannot compile [$while]: Expression is not well-formed.")));
			return CompiledBlock_CreateError();
		}
	}
	else if (postKind == SMILE_KIND_NULL) {
		// If only two parts, there is no pre-clause.
		preClause = NullObject;
	}
	else {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$while]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	// Extract off any [$not] operators, and if there were any, we'll invert the branches.
	not = Compiler_StripNots(&condition);

	// Find out the structure of this [$while] form.
	hasPre = (SMILE_KIND(preClause) != SMILE_KIND_NULL);
	hasPost = (SMILE_KIND(postClause) != SMILE_KIND_NULL);

	// Dispatch to an optimized compile depending on which flavor of [$while] this is.
	if (hasPre && hasPost) {
		return CompileWhileWithPreAndPost(compiler, not, condition, preClause, postClause, compileFlags);
	}
	else if (hasPre) {
		return CompileWhileWithPre(compiler, not, condition, preClause, compileFlags);
	}
	else if (hasPost) {
		return CompileWhileWithPost(compiler, not, condition, postClause, compileFlags);
	}
	else {
		return CompileWhileWithNoBody(compiler, not, condition, compileFlags);
	}
}

// Form: do {...} while cond then {...}
static CompiledBlock CompileWhileWithPreAndPost(Compiler compiler, Bool not, SmileObject condition, SmileObject preClause, SmileObject postClause, CompileFlags compileFlags)
{
	IntermediateInstruction instr, jmpLabel, bLabel;
	CompiledBlock compiledBlock, preBlock, postBlock, condBlock;

	compiledBlock = CompiledBlock_Create();

	jmpLabel = IntermediateInstruction_Create(Op_Label);
	bLabel = IntermediateInstruction_Create(Op_Label);

	// Emit this in order of:
	//
	//   l1:
	//       preBlock (result only based on flags)
	//
	//       condBlock (require result)
	//       branch l2
	//
	//       pop1 (if needed, based on result of preBlock)
	//
	//       postBlock (no result)
	//
	//       jmp l1
	//
	//   l2:	// left with the last preBlock's value on the stack.

	CompiledBlock_AttachInstruction(compiledBlock, NULL, jmpLabel);

	preBlock = Compiler_CompileExpr(compiler, preClause, compileFlags);
	CompiledBlock_AppendChild(compiledBlock, preBlock);
	Compiler_MakeStackMatchCompileFlags(compiler, compiledBlock, compileFlags);

	condBlock = Compiler_CompileExpr(compiler, condition, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, condBlock);
	CompiledBlock_AppendChild(compiledBlock, condBlock);

	instr = EMIT0(not ? Op_Bf : Op_Bt, -1);
	instr->p.branchTarget = bLabel;

	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		EMIT0(Op_Pop1, -1);
	}

	postBlock = Compiler_CompileExpr(compiler, postClause, compileFlags | COMPILE_FLAG_NORESULT);
	Compiler_EmitNoResult(compiler, postBlock);
	CompiledBlock_AppendChild(compiledBlock, postBlock);

	instr = EMIT0(Op_Jmp, 0);
	instr->p.branchTarget = jmpLabel;

	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, bLabel);

	// By the time we reach this point, if the compileFlags requested a result,
	// there will be one instance of that result actually left, so even though the
	// automatic count says the stack is balanced at zero, there will be if the
	// compileFlags requested a result.
	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		compiledBlock->finalStackDelta++;
	}

	return compiledBlock;
}

// Form: do {...} while cond
static CompiledBlock CompileWhileWithPre(Compiler compiler, Bool not, SmileObject condition, SmileObject preClause, CompileFlags compileFlags)
{
	IntermediateInstruction instr, jmpLabel, bLabel;
	CompiledBlock compiledBlock, preBlock, condBlock;

	compiledBlock = CompiledBlock_Create();

	jmpLabel = IntermediateInstruction_Create(Op_Label);
	bLabel = IntermediateInstruction_Create(Op_Label);

	// Emit this in order of:
	//
	//       jmp l1
	//
	//   l2:
	//       pop1 (if needed, based on result of preBlock)
	//
	//   l1:
	//       preBlock (result only based on flags)
	//
	//       condBlock (require result)
	//       branch l2
	//
	//   // stack may be left with last preClause sitting on it.

	instr = EMIT0(Op_Jmp, 0);
	instr->p.branchTarget = jmpLabel;

	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, bLabel);

	// Note: We don't add the Pop1 here because there's not yet anything on the
	// stack to pop and it would cause the stack counters to end up negative.

	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, jmpLabel);

	preBlock = Compiler_CompileExpr(compiler, preClause, compileFlags);
	CompiledBlock_AppendChild(compiledBlock, preBlock);
	Compiler_MakeStackMatchCompileFlags(compiler, compiledBlock, compileFlags);

	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		// Go back and insert the pop after the bLabel, now that the stack
		// has something poppable on it.
		instr = IntermediateInstruction_Create(Op_Pop1);
		CompiledBlock_AttachInstruction(compiledBlock, bLabel, instr);
		compiledBlock->finalStackDelta += -1;
	}

	condBlock = Compiler_CompileExpr(compiler, condition, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, condBlock);
	CompiledBlock_AppendChild(compiledBlock, condBlock);

	instr = EMIT0(not ? Op_Bf : Op_Bt, -1);
	instr->p.branchTarget = bLabel;

	// By the time we reach this point, if the compileFlags requested a result,
	// there will be one instance of that result actually left, so even though the
	// automatic count says the stack is balanced at zero, there will be if the
	// compileFlags requested a result.
	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		compiledBlock->finalStackDelta++;
	}

	return compiledBlock;
}

// Form: while cond do {...}
static CompiledBlock CompileWhileWithPost(Compiler compiler, Bool not, SmileObject condition, SmileObject postClause, CompileFlags compileFlags)
{
	IntermediateInstruction instr, jmpLabel, bLabel;
	CompiledBlock compiledBlock, postBlock, condBlock;

	compiledBlock = CompiledBlock_Create();

	jmpLabel = IntermediateInstruction_Create(Op_Label);
	bLabel = IntermediateInstruction_Create(Op_Label);

	// Emit this in order of:
	//
	//       ldnull   (Initial result, if a result is required and we never enter the loop)
	//       jmp l1
	//
	//   l2:
	//       pop1    (pop last postClause (or initial null), if a result is required)
	//       postBlock    (result depends on compile flags)
	//
	//   l1:
	//       condBlock (result always)
	//       branch l2
	//
	//   // stack is left with either the initial null or the last postClause.

	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		EMIT0(Op_LdNull, +1);
		compiledBlock->finalStackDelta--;	// This will get popped in the first iteration.
	}

	instr = EMIT0(Op_Jmp, 0);
	instr->p.branchTarget = jmpLabel;

	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, bLabel);

	postBlock = Compiler_CompileExpr(compiler, postClause, compileFlags);
	CompiledBlock_AppendChild(compiledBlock, postBlock);
	Compiler_MakeStackMatchCompileFlags(compiler, compiledBlock, compileFlags);

	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		compiledBlock->finalStackDelta--;	// Logically pop the previous iteration's content.
											
		// Go back and insert the pop after the bLabel, now that the stack
		// has something poppable on it.
		instr = IntermediateInstruction_Create(Op_Pop1);
		CompiledBlock_AttachInstruction(compiledBlock, bLabel, instr);
	}

	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, jmpLabel);

	condBlock = Compiler_CompileExpr(compiler, condition, compileFlags & ~COMPILE_FLAG_NORESULT);
	CompiledBlock_AppendChild(compiledBlock, condBlock);
	Compiler_EmitRequireResult(compiler, compiledBlock);

	instr = EMIT0(not ? Op_Bf : Op_Bt, -1);
	instr->p.branchTarget = bLabel;

	// By the time we reach this point, if the compileFlags requested a result,
	// there will be one instance of that result actually left, so even though the
	// automatic count says the stack is balanced at zero, there will be if the
	// compileFlags requested a result.
	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		compiledBlock->finalStackDelta++;
	}

	return compiledBlock;
}

// Form: while cond {}
static CompiledBlock CompileWhileWithNoBody(Compiler compiler, Bool not, SmileObject condition, CompileFlags compileFlags)
{
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr, bLabel;

	compiledBlock = CompiledBlock_Create();

	// Emit this in order of:
	//
	//   l1:
	//       condBlock (result required)
	//       branch l1
	//
	//       ldnull (if flags require a result)
	//
	//   // stack is left with a null on it, since there is no body.

	bLabel = EMIT0(Op_Label, 0);

	childBlock = Compiler_CompileExpr(compiler, condition, compileFlags & ~COMPILE_FLAG_NORESULT);
	Compiler_EmitRequireResult(compiler, childBlock);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	instr = EMIT0(not ? Op_Bf : Op_Bt, -1);
	instr->p.branchTarget = bLabel;

	if (!(compileFlags & COMPILE_FLAG_NORESULT)) {
		EMIT0(Op_LdNull, +1);
	}

	return compiledBlock;
}
