//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
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

#include <smile/parsing/parser.h>
#include <smile/parsing/internal/parserinternal.h>
#include <smile/parsing/internal/parsedecl.h>
#include <smile/parsing/internal/parsescope.h>

static void ParseScope_DeclareGlobalOperators(ParseScope parseScope);

/// <summary>
/// Create a root scope that contains only the eighteen or so global forms.
/// </summary>
/// <returns>The newly-created root scope.</returns>
ParseScope ParseScope_CreateRoot(void)
{
	ParseScope parseScope = GC_MALLOC_STRUCT(struct ParseScopeStruct);

	parseScope->kind = PARSESCOPE_OUTERMOST;
	parseScope->parentScope = NULL;
	parseScope->closure = Closure_CreateDynamic(NULL, ClosureInfo_Create(NULL));

	ParseScope_DeclareGlobalOperators(parseScope);

	return parseScope;
}

/// <summary>
/// Declare the eighteen-or-so global operators in the given (presumably root-level) scope.
/// </summary>
/// <param name="parseScope">The scope in which to declare the root-level operators.</param>
static void ParseScope_DeclareGlobalOperators(ParseScope parseScope)
{
	ParseScope_Declare(parseScope, Smile_KnownSymbols.equals_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.op_equals_, PARSEDECL_PRIMITIVE);

	ParseScope_Declare(parseScope, Smile_KnownSymbols.if_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.while_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.var_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.till_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.catch_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.fn_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.scope_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.progn_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.quote_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.new_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.is_, PARSEDECL_PRIMITIVE);
	ParseScope_Declare(parseScope, Smile_KnownSymbols.typeof_, PARSEDECL_PRIMITIVE);
}

/// <summary>
/// Create a child scope under some given parent scope.
/// </summary>
/// <param name="parentScope">The parent scope under which the new child should be created.</param>
/// <param name="kind">What kind of scope to create (a PARSESCOPE_* value).</param>
/// <returns>The newly-created child scope.</returns>
ParseScope ParseScope_CreateChild(ParseScope parentScope, Int kind)
{
	ParseScope parseScope = GC_MALLOC_STRUCT(struct ParseScopeStruct);

	parseScope->kind = kind;
	parseScope->parentScope = parentScope;
	parseScope->closure = Closure_CreateDynamic(parentScope->closure, ClosureInfo_Create(parentScope->closure->closureInfo));

	return parseScope;
}

/// <summary>
/// Declare or redeclare a name in the given scope.
/// </summary>
/// <param name="scope">The scope where the declaration will be created.</param>
/// <param name="symbol">The name to declare.</param>
/// <param name="kind">What kind of thing to declare that name as (one of the PARSEDECL_* values,
/// like ARGUMENT or VARIABLE or CONST).</param>
/// <returns>The new declaration, if a declaration was created.  If this declaration attempt
/// resulted in a conflict with an existing declaration, this will return NULL.</returns>
ParseDecl ParseScope_DeclareHere(ParseScope scope, Symbol symbol, Int kind)
{
	ParseDecl parseDecl;

	// Construct the base declaration object for this name.
	parseDecl = ParseDecl_Create(symbol, kind, scope->closure->closureInfo->numVariables, NULL);

	if (Closure_HasHere(scope->closure, symbol)) {
		// Already declared.  This isn't necessarily an error, but it depends on what
		// kind of thing is being declared and how it relates to what was already declared.

		// TODO: Check kinds and make sure this is a valid redeclaration.
		return NULL;
	}

	Closure_Let(scope->closure, symbol, (SmileObject)parseDecl);

	// Last, return the declaration object itself.
	return parseDecl;
}
