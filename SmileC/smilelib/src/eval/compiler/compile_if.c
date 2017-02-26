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

// Form: [$if cond then-clause else-clause]
void Compiler_CompileIf(Compiler compiler, SmileList args)
{
	SmileObject condition, thenClause, elseClause, temp;
	Int elseKind;
	Bool not;
	ByteCodeSegment segment;
	Int bfDelta, jmpDelta;
	Int bf, jmp, bfLabel, jmpLabel;
	Int offset;

	segment = compiler->currentFunction->byteCodeSegment;

	// Must be an expression of the form [$if cond then-clause] or [$if cond then-clause else-clause].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return;
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
			return;
		}
	}
	else if (elseKind == SMILE_KIND_NULL) {
		// If no else clause, this should end the form.
		elseClause = NullObject;
	}
	else {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return;
	}

	// Extract off any [$not] operators, and if there were any, swap then/else clauses.
	not = Compiler_StripNots(&condition);
	if (not) {
		temp = elseClause;
		elseClause = thenClause;
		thenClause = temp;
	}

	// Evaluate the condition.
	Compiler_CompileExpr(compiler, condition);

	// Emit the conditional logic.
	bf = EMIT0(Op_Bf, -1);
	Compiler_CompileExpr(compiler, thenClause);
	jmp = EMIT0(Op_Jmp, 0);
	bfLabel = EMIT0(Op_Label, 0);
	Compiler_CompileExpr(compiler, elseClause);
	jmpLabel = EMIT0(Op_Label, 0);

	// By the time we reach this point, only 'then' or 'else' will be left on the stack.
	compiler->currentFunction->currentStackDepth--;

	// Fill in the relative branch targets.
	bfDelta = bfLabel - bf;
	FIX_BRANCH(bf, bfDelta);
	FIX_BRANCH(bfLabel, -bfDelta);
	jmpDelta = jmpLabel - jmp;
	FIX_BRANCH(jmp, jmpDelta);
	FIX_BRANCH(jmpLabel, -jmpDelta);
}
