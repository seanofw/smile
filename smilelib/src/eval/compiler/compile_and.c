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

// Form: [$and x y z ...]
CompiledBlock Compiler_CompileAnd(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	SmileList temp;
	Int i, length;
	SmileObject condition;
	Bool not;
	CompiledBlock compiledBlock, childBlock;
	IntermediateInstruction instr;
	IntermediateInstruction branchLabel, jmpLabel;

	// Must be a well-formed expression of the form [$and x y z ...].
	if ((length = SmileList_Length(args)) <= 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$and]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	compiledBlock = CompiledBlock_Create();
	branchLabel = IntermediateInstruction_Create(Op_Label);
	jmpLabel = IntermediateInstruction_Create(Op_Label);

	// Emit all of the conditionals.
	for (i = 0, temp = args; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d, i++) {

		// Extract off any [$not] operators, and if there were any, invert the branch below.
		condition = temp->a;
		not = Compiler_StripNots(&condition);

		// Compile the next expression.
		childBlock = Compiler_CompileExpr(compiler, condition, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitRequireResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// If falsy, branch to result in 'false'.
		instr = EMIT0(not ? Op_Bt : Op_Bf, -1);
		instr->p.branchTarget = branchLabel;

		// It's truthy, so keep going.
	}

	// We passed all the tests, so the result is true.
	EMIT1(Op_LdBool, +1, boolean = True);
	instr = EMIT0(Op_Jmp, 0);

	// Now handle the falsy case.
	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, branchLabel);
	EMIT1(Op_LdBool, +1, boolean = False);

	// Add the branch target for the jump.
	CompiledBlock_AttachInstruction(compiledBlock, compiledBlock->last, jmpLabel);

	compiledBlock->finalStackDelta--;	// We actually have one fewer on the stack than the automatic count.

	return compiledBlock;
}
