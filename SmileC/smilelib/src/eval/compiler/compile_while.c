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

static void CompileWhileWithPreAndPost(Compiler compiler, Bool not, SmileObject condition, SmileObject preClause, SmileObject postClause);
static void CompileWhileWithPre(Compiler compiler, Bool not, SmileObject condition, SmileObject preClause);
static void CompileWhileWithPost(Compiler compiler, Bool not, SmileObject condition, SmileObject postClause);
static void CompileWhileWithNoBody(Compiler compiler, Bool not, SmileObject condition);

// Form: [$while pre-body cond post-body]
void Compiler_CompileWhile(Compiler compiler, SmileList args)
{
	SmileObject condition, preClause, postClause;
	Int postKind, tailKind;
	Bool not, hasPre, hasPost;

	// Must be an expression of the form [$while cond postBody] or [$while pre-body cond post-body].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_LIST) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [if]: Expression is not well-formed.")));
		return;
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
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
				String_FromC("Cannot compile [$while]: Expression is not well-formed.")));
			return;
		}
	}
	else if (postKind == SMILE_KIND_NULL) {
		// If only two parts, there is no pre-clause.
		preClause = NullObject;
	}
	else {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$while]: Expression is not well-formed.")));
		return;
	}

	// Extract off any [$not] operators, and if there were any, we'll invert the branches.
	not = Compiler_StripNots(&condition);

	// Find out the structure of this [$while] form.
	hasPre = (SMILE_KIND(preClause) != SMILE_KIND_NULL);
	hasPost = (SMILE_KIND(postClause) != SMILE_KIND_NULL);

	// Dispatch to an optimized compile depending on which flavor of [$while] this is.
	if (hasPre && hasPost) {
		CompileWhileWithPreAndPost(compiler, not, condition, preClause, postClause);
	}
	else if (hasPre) {
		CompileWhileWithPre(compiler, not, condition, preClause);
	}
	else if (hasPost) {
		CompileWhileWithPost(compiler, not, condition, postClause);
	}
	else {
		CompileWhileWithNoBody(compiler, not, condition);
	}
}

// Form: do {...} while cond then {...}
static void CompileWhileWithPreAndPost(Compiler compiler, Bool not, SmileObject condition, SmileObject preClause, SmileObject postClause)
{
	ByteCodeSegment segment;
	Int bDelta, jmpDelta;
	Int b, jmp, bLabel, jmpLabel;
	Int offset;

	segment = compiler->currentFunction->byteCodeSegment;

	// Emit this in order of:
	//
	//   l1: eval preClause
	//
	//       eval cond
	//       branch l2
	//
	//       pop1		// pop the last preClause
	//
	//       eval postClause
	//       pop1
	//
	//       jmp l1
	//
	//   l2:	// left with the last preClause on the stack.

	jmpLabel = EMIT0(Op_Label, 0);

	Compiler_CompileExpr(compiler, preClause);

	Compiler_CompileExpr(compiler, condition);

	b = EMIT0(not ? Op_Bf : Op_Bt, -1);

	Compiler_EmitPop1(compiler);

	Compiler_CompileExpr(compiler, postClause);
	Compiler_EmitPop1(compiler);

	jmp = EMIT0(Op_Jmp, 0);

	bLabel = EMIT0(Op_Label, 0);

	bDelta = bLabel - b;
	FIX_BRANCH(b, bDelta);
	FIX_BRANCH(bLabel, -bDelta);

	jmpDelta = jmpLabel - jmp;
	FIX_BRANCH(jmp, jmpDelta);
	FIX_BRANCH(jmpLabel, -jmpDelta);

	// By the time we reach this point, one iteration will be left on the stack.
	compiler->currentFunction->currentStackDepth++;
}

// Form: do {...} while cond
static void CompileWhileWithPre(Compiler compiler, Bool not, SmileObject condition, SmileObject preClause)
{
	ByteCodeSegment segment;
	Int bDelta, jmpDelta;
	Int b, jmp, bLabel, jmpLabel;
	Int offset;

	segment = compiler->currentFunction->byteCodeSegment;

	// Emit this in order of:
	//
	//       jmp l1
	//
	//   l2: pop1	// pop last preClause from stack
	//
	//   l1: eval preClause
	//
	//       eval cond
	//       branch l2
	//
	//   // stack is left with last preClause sitting on it.

	jmp = EMIT0(Op_Jmp, 0);

	bLabel = EMIT0(Op_Label, 0);

	Compiler_EmitPop1(compiler);

	jmpLabel = EMIT0(Op_Label, 0);

	Compiler_CompileExpr(compiler, preClause);

	Compiler_CompileExpr(compiler, condition);

	b = EMIT0(not ? Op_Bf : Op_Bt, -1);

	bDelta = bLabel - b;
	FIX_BRANCH(b, bDelta);
	FIX_BRANCH(bLabel, -bDelta);

	jmpDelta = jmpLabel - jmp;
	FIX_BRANCH(jmp, jmpDelta);
	FIX_BRANCH(jmpLabel, -jmpDelta);

	// By the time we reach this point, one iteration will be left on the stack.
	compiler->currentFunction->currentStackDepth++;
}

// Form: while cond do {...}
static void CompileWhileWithPost(Compiler compiler, Bool not, SmileObject condition, SmileObject postClause)
{
	ByteCodeSegment segment;
	Int bDelta, jmpDelta;
	Int b, jmp, bLabel, jmpLabel;
	Int offset;

	segment = compiler->currentFunction->byteCodeSegment;

	// Emit this in order of:
	//
	//       ldnull	// Initial result, if the initial condition is false.
	//       jmp l1
	//
	//   l2: pop1	// pop last postClause (or initial null).
	//       eval postClause
	//
	//   l1: eval cond
	//       branch l2
	//
	//   // stack is left with either the initial null or the last postClause.

	EMIT0(Op_LdNull, +1);

	jmp = EMIT0(Op_Jmp, 0);

	bLabel = EMIT0(Op_Label, 0);

	Compiler_EmitPop1(compiler);

	Compiler_CompileExpr(compiler, postClause);

	jmpLabel = EMIT0(Op_Label, 0);

	Compiler_CompileExpr(compiler, condition);

	b = EMIT0(not ? Op_Bf : Op_Bt, -1);

	bDelta = bLabel - b;
	FIX_BRANCH(b, bDelta);
	FIX_BRANCH(bLabel, -bDelta);

	jmpDelta = jmpLabel - jmp;
	FIX_BRANCH(jmp, jmpDelta);
	FIX_BRANCH(jmpLabel, -jmpDelta);
}

// Form: while cond {}
static void CompileWhileWithNoBody(Compiler compiler, Bool not, SmileObject condition)
{
	ByteCodeSegment segment;
	Int bDelta;
	Int b, bLabel;
	Int offset;

	segment = compiler->currentFunction->byteCodeSegment;

	// Emit this in order of:
	//
	//   l1: eval cond
	//       branch l1
	//
	//       ldnull
	//
	//   // stack is left with a null on it, since there is no body.

	bLabel = EMIT0(Op_Label, 0);

	Compiler_CompileExpr(compiler, condition);

	b = EMIT0(not ? Op_Bf : Op_Bt, -1);

	EMIT0(Op_LdNull, +1);

	bDelta = bLabel - b;
	FIX_BRANCH(b, bDelta);
	FIX_BRANCH(bLabel, -bDelta);
}
