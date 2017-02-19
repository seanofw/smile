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

// Form: [$scope [vars...] a b c ...]
void Compiler_CompileScope(Compiler compiler, SmileList args)
{
	CompileScope scope;
	SmileList scopeVars, temp;
	Int numScopeVars, localIndex;

	// The [$scope] expression must be of the form:  [$scope [locals...] ...].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->a) != SMILE_KIND_LIST
		|| !(SMILE_KIND(args->d) == SMILE_KIND_LIST || SMILE_KIND(args->d) == SMILE_KIND_NULL)) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$scope]: Expression is not well-formed.")));
		return;
	}

	scope = Compiler_BeginScope(compiler, PARSESCOPE_SCOPEDECL);

	// Declare the [locals...] list, which must be well-formed, and must consist only of symbols.
	scopeVars = (SmileList)args->a;
	numScopeVars = 0;
	for (temp = scopeVars; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
		if (SMILE_KIND(temp->a) != SMILE_KIND_SYMBOL) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(temp),
				String_Format("Cannot compile [$scope]: Variable #%d is not a valid local variable name.", numScopeVars + 1)));
		}

		Symbol symbol = ((SmileSymbol)temp->a)->symbol;
		localIndex = CompilerFunction_AddLocal(compiler->currentFunction, symbol);
		CompileScope_DefineSymbol(scope, symbol, PARSEDECL_VARIABLE, localIndex);
	}
	if (SMILE_KIND(temp) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SmileList_GetSourceLocation(args),
			String_FromC("Cannot compile [$scope]: Local-variable list is not well-formed.")));
		return;
	}

	// Compile the rest of the [scope] as though it was just a [progn].
	Compiler_CompileProgN(compiler, (SmileList)args->d);

	Compiler_EndScope(compiler);
}
