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

// Form: [$quote expr]
CompiledBlock Compiler_CompileQuote(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	Int objectIndex;
	CompiledBlock compiledBlock;
	IntermediateInstruction instr;

	// Quoting something that won't actually be needed results in nothing.
	if (compileFlags & COMPILE_FLAG_NORESULT)
		return CompiledBlock_Create();

	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->d) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$quote]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	if (SMILE_KIND(args->a) == SMILE_KIND_SYMBOL) {
		// A quoted symbol can just be loaded directly.
		compiledBlock = CompiledBlock_Create();
		EMIT1(Op_LdSym, +1, symbol = ((SmileSymbol)args->a)->symbol);
		return compiledBlock;
	}
	else if (SMILE_KIND(args->a) != SMILE_KIND_LIST) {
		// It's neither a list nor a symbol, so quoting it just results in *it*, whatever it is.
		compiledBlock = Compiler_CompileExpr(compiler, args->a, compileFlags);
		Compiler_MakeStackMatchCompileFlags(compiler, compiledBlock, compileFlags);
		return compiledBlock;
	}
	else {
		// Load the quoted form as a literal stored object.
		compiledBlock = CompiledBlock_Create();
		objectIndex = Compiler_AddObject(compiler, args->a);
		EMIT1(Op_LdObj, +1, index = objectIndex);
		return compiledBlock;
	}
}
