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

static Bool Compiler_DecodeScopeVariable(Compiler compiler, Int index, SmileObject obj, Symbol *symbol, Int *declKind)
{
	SmileList sublist;

	*declKind = PARSEDECL_VARIABLE;

	if (SMILE_KIND(obj) == SMILE_KIND_SYMBOL) {
		*symbol = ((SmileSymbol)obj)->symbol;
		return True;
	}

	if (SMILE_KIND(obj) != SMILE_KIND_LIST) goto fail;
	sublist = (SmileList)obj;

	if (SMILE_KIND(sublist->a) != SMILE_KIND_SYMBOL) goto fail;
	*symbol = ((SmileSymbol)sublist->a)->symbol;

	for (sublist = LIST_REST(sublist); SMILE_KIND(sublist) == SMILE_KIND_LIST; sublist = LIST_REST(sublist)) {
		Symbol modifier;
		if (SMILE_KIND(sublist->a) != SMILE_KIND_SYMBOL) goto fail;
		modifier = ((SmileSymbol)sublist->a)->symbol;
		if (modifier == Smile_KnownSymbols.auto_) {
			*declKind = PARSEDECL_SETONCEAUTO;
		}
		else if (modifier == Smile_KnownSymbols.set_once) {
			*declKind = PARSEDECL_SETONCECONST;
		}
		else goto fail;
	}

	return True;

fail:
	if (compiler != NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(obj, getSourceLocation),
			String_Format("Cannot compile [$scope]: Variable #%d is not a valid local variable declaration.", index + 1)));
	}
	return False;
}

// Given a [$scope [vars ...] a b c ...] form that is the outermost expression in a module, figure
// out how its content will be laid out in the resulting closure, and return that as a dictionary
// that maps symbol names to variable offsets within that closure.
VarDict Compiler_PrecomputeModuleClosureLayout(SmileObject scope, ParseScope parseScope)
{
	SmileList scopeVars, temp;
	VarDict varDict;
	SmileList args;
	Symbol symbol;
	struct VarInfoStruct varInfo;
	Int32 offset;
	ParseDecl parseDecl;
	Int declKind;

	// This must be a list that starts with [$scope ...]
	if (SMILE_KIND(scope) != SMILE_KIND_LIST || SMILE_KIND((args = (SmileList)scope)->a) != SMILE_KIND_SYMBOL
		|| ((SmileSymbol)args->a)->symbol != SMILE_SPECIAL_SYMBOL__SCOPE)
		return NULL;
	args = (SmileList)args->d;

	// The [$scope] expression must be of the form:  [$scope [locals...] ...].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->a) != SMILE_KIND_LIST
		|| !(SMILE_KIND(args->d) == SMILE_KIND_LIST || SMILE_KIND(args->d) == SMILE_KIND_NULL))
		return NULL;

	// Spin over the [locals...] list, which must be well-formed, and must consist only of symbols
	// or sublists of symbols, and construct the dictionary that describes the closure offsets of
	// its entries.
	varDict = VarDict_Create();
	scopeVars = (SmileList)args->a;
	offset = 0;
	for (temp = scopeVars; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
		if (!Compiler_DecodeScopeVariable(NULL, offset, temp->a, &symbol, &declKind))
			continue;

		symbol = ((SmileSymbol)temp->a)->symbol;

		parseDecl = ParseScope_FindDeclarationHere(parseScope, symbol);
		declKind = parseDecl != NULL ? parseDecl->declKind : PARSEDECL_INCLUDE;

		varInfo.kind =
			  declKind == PARSEDECL_ARGUMENT ? VAR_KIND_ARG
			: declKind == PARSEDECL_VARIABLE ? VAR_KIND_VAR
			: declKind == PARSEDECL_AUTO || declKind == PARSEDECL_SETONCEAUTO
				|| declKind == PARSEDECL_CONST || declKind == PARSEDECL_SETONCECONST ? VAR_KIND_CONST
			: VAR_KIND_GLOBAL;
		varInfo.offset = offset;
		varInfo.symbol = symbol;
		varInfo.value = NullObject;
		VarDict_Add(varDict, symbol, &varInfo);

		offset++;
	}

	if (SMILE_KIND(temp) != SMILE_KIND_NULL)
		return NULL;

	return varDict;
}

// Form: [$scope [vars...] a b c ...]
CompiledBlock Compiler_CompileScope(Compiler compiler, SmileList args, CompileFlags compileFlags)
{
	CompileScope scope;
	SmileList scopeVars, temp;
	Int numScopeVars, localIndex;
	CompiledBlock compiledBlock, initBlock;
	Symbol symbol;
	Int declKind;

	// The [$scope] expression must be of the form:  [$scope [locals...] ...].
	if (SMILE_KIND(args) != SMILE_KIND_LIST || SMILE_KIND(args->a) != SMILE_KIND_LIST
		|| !(SMILE_KIND(args->d) == SMILE_KIND_LIST || SMILE_KIND(args->d) == SMILE_KIND_NULL)) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$scope]: Expression is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	scope = Compiler_BeginScope(compiler, PARSESCOPE_SCOPEDECL);

	// Declare the [locals...] list, which must be well-formed, and must consist only of symbols,
	// and emit NullLoc0 instructions for each local to ensure consistent execution behavior.
	// TODO: Optimize this by tracking assignments and NOT emitting NullLoc0 for locals that are always assigned before reading.
	scopeVars = (SmileList)args->a;
	numScopeVars = 0;
	initBlock = CompiledBlock_Create();
	for (temp = scopeVars; SMILE_KIND(temp) == SMILE_KIND_LIST; temp = (SmileList)temp->d) {
		if (!Compiler_DecodeScopeVariable(compiler, numScopeVars, temp->a, &symbol, &declKind)) {
			Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(temp, getSourceLocation),
				String_Format("Cannot compile [$scope]: Variable #%d is not a valid local variable declaration.", numScopeVars + 1)));
		}

		localIndex = CompilerFunction_AddLocal(compiler->currentFunction, symbol);
		CompileScope_DefineSymbol(scope, symbol, declKind, localIndex);
		Compiler_SetSourceLocationFromList(compiler, temp);
		CompiledBlock_Emit(initBlock, Op_NullLoc0, 0, compiler->currentFunction->currentSourceLocation)->u.index = localIndex;
	}
	if (SMILE_KIND(temp) != SMILE_KIND_NULL) {
		Compiler_AddMessage(compiler, ParseMessage_Create(PARSEMESSAGE_ERROR, SMILE_VCALL(args, getSourceLocation),
			String_FromC("Cannot compile [$scope]: Local-variable list is not well-formed.")));
		return CompiledBlock_CreateError();
	}

	// Compile the rest of the [scope] as though it was just a [progn].
	Compiler_SetSourceLocationFromList(compiler, args);
	compiledBlock = Compiler_CompileProgN(compiler, (SmileList)args->d, compileFlags);

	Compiler_EndScope(compiler);

	// Combine the initializations with the actual work, and return it.
	compiledBlock = CompiledBlock_Combine(initBlock, compiledBlock);

	return compiledBlock;
}
