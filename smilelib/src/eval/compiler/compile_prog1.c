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

// Form: [$prog1 a b c ...]
CompiledBlock Compiler_CompileProg1(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	Int oldSourceLocation;
	CompiledBlock compiledBlock, childBlock;

	compiledBlock = CompiledBlock_Create();

	if (SMILE_KIND(args) != SMILE_KIND_LIST)
		return compiledBlock;

	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);

	// Compile this expression, and keep it.
	childBlock = Compiler_CompileExpr(compiler, args->a, compileFlags);
	Compiler_MakeStackMatchCompileFlags(compiler, childBlock, compileFlags);
	CompiledBlock_AppendChild(compiledBlock, childBlock);

	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	// Compile the rest, and toss their values.
	for (args = LIST_REST(args); SMILE_KIND(args) == SMILE_KIND_LIST; args = LIST_REST(args)) {

		// Compile this next expression...
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
		Compiler_SetSourceLocationFromList(compiler, args);
		childBlock = Compiler_CompileExpr(compiler, args->a, compileFlags & ~COMPILE_FLAG_NORESULT);
		Compiler_EmitNoResult(compiler, childBlock);
		CompiledBlock_AppendChild(compiledBlock, childBlock);

		// ...and discard its result.
		Compiler_SetSourceLocationFromList(compiler, args);
	}

	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	return compiledBlock;
}
