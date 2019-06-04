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

// Form: [$progn a b c ...]
CompiledBlock Compiler_CompileProgN(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	Int namedSourceLocation, namelessSourceLocation;
	SmileList next;
	Bool isLast;
	CompiledBlock compiledBlock, childBlock;

	compiledBlock = CompiledBlock_Create();

	if (SMILE_KIND(args) != SMILE_KIND_LIST)
		return compiledBlock;

	namedSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);
	namelessSourceLocation = compiler->currentFunction->currentSourceLocation;
	Compiler_RevertSourceLocation(compiler, namedSourceLocation);

	for (;;) {
		next = LIST_REST(args);
		isLast = (SMILE_KIND(next) != SMILE_KIND_LIST);

		// Compile this next expression...
		Compiler_RevertSourceLocation(compiler, isLast ? namedSourceLocation : namelessSourceLocation);
		Compiler_SetSourceLocationFromList(compiler, args);
		childBlock = Compiler_CompileExpr(compiler, args->a,
			isLast ? compileFlags : (compileFlags | COMPILE_FLAG_NORESULT));

		// Only keep the value if this is the last instruction.
		if (isLast) {
			Compiler_MakeStackMatchCompileFlags(compiler, childBlock, compileFlags);
		}
		else {
			Compiler_EmitNoResult(compiler, childBlock);
		}

		// Add this child to the progn.
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// ...and if it's the last one, keep its value.
		if (isLast) break;

		// Otherwise, move to the next expression.
		Compiler_SetSourceLocationFromList(compiler, args);
		args = next;
	}

	Compiler_RevertSourceLocation(compiler, namedSourceLocation);

	return compiledBlock;
}
