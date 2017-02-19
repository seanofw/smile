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

// Form: [$or x y z ...]
void Compiler_CompileOr(Compiler compiler, SmileList args)
{
	ByteCodeSegment segment = compiler->currentFunction->byteCodeSegment;
	SmileList temp;
	Int i, length;
	SmileObject condition;
	Bool not;

	Int localBts[16];
	Int *bts;
	Int trueOffset;
	Int jmp, jmpLabel, jmpDelta;
	Int offset;

	// Must be a well-formed expression of the form [$or x y z ...].
	if ((length = SmileList_Length(args)) <= 0) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [or]: Expression is not well-formed.")));
		return;
	}

	// Create somewhere to store the byte-code branches, if there are a lot of them.
	if (length > 16) {
		bts = (Int *)GC_MALLOC_ATOMIC(sizeof(Int) * length);
		if (bts == NULL)
			Smile_Abort_OutOfMemory();
	}
	else {
		bts = localBts;
	}

	// Emit all of the conditionals.
	for (i = 0, temp = args; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d, i++) {

		// Extract off any [$not] operators, and if there were any, invert the branch below.
		condition = temp->a;
		not = Compiler_StripNots(&condition);

		// Compile the next expression.
		Compiler_CompileExpr(compiler, temp->a);

		// If truthy, branch to result in 'true'.
		bts[i] = EMIT0(not ? Op_Bf : Op_Bt, -1);

		// It's truthy, so keep going.
	}

	// We failed all the tests, so the result is false.
	EMIT1(Op_LdBool, +1, boolean = False);
	jmp = EMIT0(Op_Jmp, 0);

	// Now handle the truthy case.
	trueOffset = segment->numByteCodes;
	EMIT1(Op_LdBool, +1, boolean = True);

	// Add a branch target for the jump.
	jmpLabel = EMIT0(Op_Label, 0);

	// Now fill in all the branch deltas for the conditional branches.
	for (i = 0; i < length; i++) {
		FIX_BRANCH(bts[i], trueOffset - bts[i]);
	}

	// And fill in the branch delta for the unconditional branch.
	jmpDelta = jmpLabel - jmp;
	FIX_BRANCH(jmp, jmpDelta);
	FIX_BRANCH(jmpLabel, -jmpDelta);

	compiler->currentFunction->currentStackDepth--;	// We actually have one fewer on the stack than the automatic count.
}
