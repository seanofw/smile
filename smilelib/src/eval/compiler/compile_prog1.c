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

// Form: [$prog1 a b c ...]
void Compiler_CompileProg1(Compiler compiler, SmileList args)
{
	Int oldSourceLocation;

	if (SMILE_KIND(args) != SMILE_KIND_LIST)
		return;

	oldSourceLocation = Compiler_SetAssignedSymbol(compiler, 0);

	// Compile this expression, and keep it.
	Compiler_CompileExpr(compiler, args->a);
	args = LIST_REST(args);

	Compiler_RevertSourceLocation(compiler, oldSourceLocation);

	for (; SMILE_KIND(args) == SMILE_KIND_LIST; args = LIST_REST(args)) {

		// Compile this next expression...
		Compiler_RevertSourceLocation(compiler, oldSourceLocation);
		Compiler_SetSourceLocationFromList(compiler, args);
		Compiler_CompileExpr(compiler, args->a);

		// ...and discard its result.
		Compiler_SetSourceLocationFromList(compiler, args);
		Compiler_EmitPop1(compiler);
	}

	Compiler_RevertSourceLocation(compiler, oldSourceLocation);
}
