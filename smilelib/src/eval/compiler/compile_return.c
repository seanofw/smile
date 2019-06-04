//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2019 Sean Werkema
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

// Form: [$return] or [$return value]
CompiledBlock Compiler_CompileReturn(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	CompiledBlock compiledBlock, childBlock;

	compiledBlock = CompiledBlock_Create();

	Int oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);
	Compiler_SetSourceLocationFromList(compiler, args);

	if (SMILE_KIND(args) == SMILE_KIND_NULL) {
		// Naked [$return], so we're implicitly returning null.
		EMIT0(Op_LdNull, +1);
		EMIT0(Op_Ret, -1);
		compiledBlock->blockFlags |= BLOCK_FLAG_ESCAPE;
	}
	else if (SMILE_KIND(args) == SMILE_KIND_LIST && SMILE_KIND(args->d) == SMILE_KIND_NULL) {
		// Compile the return expression...
		childBlock = Compiler_CompileExpr(compiler, args->a, compileFlags & ~COMPILE_FLAG_NORESULT);

		// ...and return it.
		EMIT0(Op_Ret, -1);
		compiledBlock->blockFlags |= BLOCK_FLAG_ESCAPE;
	}
	else {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$return]: Expression is not well-formed.")));
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
		return CompiledBlock_CreateError();
	}

	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	return compiledBlock;
}
